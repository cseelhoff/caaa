#include "engine.hpp"
#include "array_functions.hpp"
#include "game_state.hpp"
#include "json_state.hpp"
#include "land.hpp"
#include "map_cache.hpp"
#include "mcts.hpp"
#include "player.hpp"
#include "random_numbers.hpp"
#include "sea.hpp"
#include "units/fighter.hpp"
#include "units/units.hpp"
#include <array>
#include <bits/ranges_util.h>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <pybind11/pybind11.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#ifdef NDEBUG
#define cause_breakpoint() ((void)0)
#else
void cause_breakpoint() { std::cout << "\nbreakpoint\n"; }
#endif

void load_game_data(GameState& state, const std::string& filename) {
  memset(&state, 0, sizeof(state));
  GameStateJson jsonState;
  bool result = load_game_state_from_json(filename, jsonState);
  if (!result) {
    throw std::runtime_error("Failed to load game state from file: " + filename);
  }
  convert_json_to_memory(jsonState, state);
}

void play_full_turn(GameState& state) {
  move_air_units(state, FIGHTERS_AIR);
  move_air_units(state, BOMBERS_AIR);
  stage_transport_units(state, TRANSEMPTY);
  stage_transport_units(state, TRANS1I);
  stage_transport_units(state, TRANS1A);
  stage_transport_units(state, TRANS1T);
  stage_transport_units(state, TRANS2I);
  move_land_unit_type(state, TANKS);
  move_land_unit_type(state, ARTILLERY);
  move_land_unit_type(state, INFANTRY);
  move_transport_units(state);
  move_subs(state);
  move_destroyers_battleships(state);
  resolve_sea_battles(state);
  unload_transports(state);
  resolve_land_battles(state);
  move_land_unit_type(state, AAGUNS);
  land_fighter_units(state);
  land_bomber_units(state);
  buy_units(state);
  crash_air_units(state);
  reset_units_fully(state);
  buy_factory(state);
  collect_money(state);
  rotate_turns(state);
}

bool move_air_units(GameState& state, AirUnitTypeEnum unit_type) {
  debug_checks(state);
  const uint player_idx = state.current_turn;
  const uint max_move_air = MAX_MOVE_AIR[unit_type];
  const uint& answers_remaining = state.cache.answers_remaining;
  std::vector<uint>& valid_moves = state.cache.valid_moves;
  AirArray& total_player_units_player = state.cache.total_player_units.arr(player_idx);
  TeamAirArray& team_units_count = state.cache.team_units_count;
  AirArray& team_units_count_team = team_units_count.arr(PLAYER_TEAM[player_idx]);
  const AirArray& team_units_count_enemy = team_units_count.arr(ENEMY_TEAM[player_idx]);
  const LandArray& factory_dmg = state.factory_dmg;
  const LandArray& factory_max = state.factory_max;
  CombatStatusArray& combat_status = state.combat_status;
  bool units_to_process = false;
  BoolAirArray& canBomberLandHere = state.cache.canBomberLandHere;
  const uint src_air_count = unit_type == FIGHTERS_AIR ? AIRS_COUNT : LANDS_COUNT;
  for (uint src_air = 0; src_air < src_air_count; src_air++) {
    std::vector<uint>& air_units = get_active_air_units(state, src_air, unit_type);
    uint& unmoved_air_units = air_units.at(max_move_air);
    if (unmoved_air_units == 0 || combat_status[src_air] == CombatStatus::PRE_COMBAT) {
      continue;
    }
    if (!units_to_process) {
      units_to_process = true;
      refresh_can_planes_land_here(state, unit_type);
    }
    valid_moves.assign(1, src_air);
    add_valid_air_moves(state, src_air, unit_type);
    while (unmoved_air_units > 0) {
      uint dst_air = valid_moves[0];
      if (valid_moves.size() > 1) {
        if (answers_remaining == 0)
          return true;
        dst_air = get_user_move_input(state, unit_type, src_air);
      }
      if (unit_type == FIGHTERS_AIR) { // todo bombers
        update_move_history_4air(state, src_air, dst_air);
      }
      if (src_air == dst_air) {
        // this is a rare case where an enemy ship is purchased under a unit
        if (unit_type == FIGHTERS_AIR && team_units_count_enemy.at(dst_air) > 0) {
          combat_status[dst_air] = CombatStatus::PRE_COMBAT;
          continue;
        }
        air_units.at(0) += unmoved_air_units;
        unmoved_air_units = 0;
        continue;
      }
      uint airDistance = AIR_DIST[src_air][dst_air];
      if (team_units_count_enemy.at(dst_air) > 0 ||
          (unit_type == BOMBERS_AIR && factory_dmg[dst_air] < factory_max[dst_air] * 2 &&
           !canBomberLandHere[dst_air])) {
        combat_status[dst_air] = CombatStatus::PRE_COMBAT;
      } else {
        airDistance = max_move_air;
      }
      get_active_air_units(state, dst_air, unit_type).at(max_move_air - airDistance)++;
      get_idle_air_units(state, player_idx, dst_air, unit_type)++;
      total_player_units_player.at(dst_air)++;
      team_units_count_team.at(dst_air)++;
      unmoved_air_units--;
      get_idle_air_units(state, player_idx, src_air, unit_type)--;
      total_player_units_player.at(src_air)--;
      team_units_count_team.at(src_air)--;
    }
  }
  if (units_to_process) {
    clear_move_history(state);
  }
  return false;
}
void refresh_can_planes_land_here(GameState& state, AirUnitTypeEnum unit_type) {
  if (unit_type == FIGHTERS_AIR) {
    refresh_can_fighters_land_here(state);
  } else {
    refresh_can_bombers_land_here(state);
  }
}
void add_valid_air_moves(GameState& state, uint src_air, AirUnitTypeEnum unit_type) {
  if (unit_type == FIGHTERS_AIR) {
    add_valid_fighter_moves(state, src_air);
  } else {
    add_valid_bomber_moves(state, src_air);
  }
}
void refresh_can_fighters_land_here(GameState& state) {
  BoolAirArray& canFighterLandHere = state.cache.canFighterLandHere;
  const SeaArray& allied_carriers = state.cache.allied_carriers;
  const LandArray& factory_max = state.factory_max;
  BoolAirArray& canFighterLandIn1Move = state.cache.canFighterLandIn1Move;
  const LandArray& land_owners = state.land_owners;
  const CombatStatusArray& combat_status = state.combat_status;
  const uint current_turn = state.current_turn;
  const Player& player = PLAYERS[current_turn];
  FILL_ARRAY(canFighterLandHere, 0);
  // refresh_canFighterLandHere
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    uint land_owner = land_owners[land_idx];
    // is allied owned and not recently conquered?
    canFighterLandHere[land_idx] =
        (player.is_allied[land_owner] && combat_status[land_idx] == CombatStatus::NO_COMBAT);
    // check for possiblity to build carrier under fighter
    if (land_owner == state.current_turn && factory_max[land_idx] > 0) {
      uint land_to_sea_count = LAND_TO_SEA_COUNT[land_idx];
      L2SConn land_to_sea_conn = LAND_TO_SEA_CONN[land_idx];
      for (uint conn_idx = 0; conn_idx < land_to_sea_count; conn_idx++) {
        canFighterLandHere[LANDS_COUNT + land_to_sea_conn[conn_idx]] = true;
      }
    }
  }
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    if (allied_carriers[sea_idx] > 0) {
      canFighterLandHere[sea_idx + LANDS_COUNT] = true;
      // if player owns these carriers, then landing area is 2 spaces away
      // if (sea_units_state[sea_idx][CARRIERS][CARRIER_MOVES_MAX] > 0) {
      if (get_active_sea_units(state).at(CARRIERS)->at(sea_idx)[CARRIER_MOVES_MAX] > 0) {
        S2SConn sea_to_sea_conn = SEA_TO_SEA_CONN[sea_idx];
        uint sea_to_sea_count = SEA_TO_SEA_COUNT[sea_idx];
        for (uint conn_idx = 0; conn_idx < sea_to_sea_count; conn_idx++) {
          uint connected_sea1 = sea_to_sea_conn[conn_idx];
          canFighterLandHere[LANDS_COUNT + connected_sea1] = true;
          uint sea_to_sea_count2 = SEA_TO_SEA_COUNT[connected_sea1];
          S2SConn sea_to_sea_conn2 = SEA_TO_SEA_CONN[connected_sea1];
          for (uint conn2_idx = 0; conn2_idx < sea_to_sea_count2; conn2_idx++) {
            canFighterLandHere[LANDS_COUNT + sea_to_sea_conn2[conn2_idx]] = true;
          }
        }
      }
    }
  }
  // refresh_canFighterLandIn1Move
  for (uint air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {
    canFighterLandIn1Move[air_idx] = false;
    uint air_conn_count = AIR_CONN_COUNT[air_idx];
    for (uint conn_idx = 0; conn_idx < air_conn_count; conn_idx++) {
      if (canFighterLandHere[AIR_CONNECTIONS[air_idx][conn_idx]]) {
        canFighterLandIn1Move[air_idx] = true;
        break;
      }
    }
  }
}

void add_valid_fighter_moves(GameState& state, uint src_air) {
  const uint enemy_team_idx = ENEMY_TEAM[state.current_turn];
  std::vector<uint>& valid_moves = state.cache.valid_moves;
  const BoolAirArray& canFighterLandHere = state.cache.canFighterLandHere;
  const BoolAirArray& canFighterLandIn1Move = state.cache.canFighterLandIn1Move;
  const AirArray near_air = AIR_WITHIN_X_MOVES[FIGHTER_MOVES_MAX - 1][src_air];
  const uint near_air_count = AIR_WITHIN_X_MOVES_COUNT[FIGHTER_MOVES_MAX - 1][src_air];
  for (uint i = 0; i < near_air_count; i++) {
    uint dst_air = near_air[i];
    uint air_dist = AIR_DIST[src_air][dst_air];
    if (air_dist <= 2 || canFighterLandHere[dst_air] ||
        (air_dist == 3 && canFighterLandIn1Move[dst_air])) {

      if (!canFighterLandHere[dst_air] &&
          state.cache.team_units_count.val(enemy_team_idx, dst_air) == 0) // waste of a move
        continue;
      // add_valid_air_move_if_history_allows_X(dst_air, src_air, air_dist);
      if (!state.skipped_moves[src_air][dst_air]) {
        valid_moves.push_back(dst_air);
      }
    }
  }
}

void update_move_history_4air(GameState& state, uint src_air, uint dst_air) {
  std::vector<uint>& valid_moves = state.cache.valid_moves;
  // get a list of newly skipped valid_actions
  while (true) {
    uint valid_action = valid_moves.back();
    if (valid_action == dst_air) {
      break;
    }
    assert(valid_moves.size() > 0);
    state.skipped_moves[src_air][valid_action] = true;
    apply_skip(state, src_air, valid_action);
    valid_moves.pop_back();
  }
}

void clear_move_history(GameState& state) { FILL_2D_ARRAY(state.skipped_moves, false) }

void apply_skip(GameState& state, uint src_air, uint dst_air) {
  for (uint i = 0; i < AIRS_COUNT; i++) {
    if (state.skipped_moves[dst_air][i]) {
      state.skipped_moves[src_air][i] = true;
    }
  }
}

void refresh_can_bombers_land_here(GameState& state) {
  BoolAirArray& canBomberLandHere = state.cache.canBomberLandHere;
  const LandArray& land_owners = state.land_owners;
  const Player current_player = PLAYERS[state.current_turn];
  const CombatStatusArray& combat_status = state.combat_status;
  // check if any bombers have full moves remaining
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    canBomberLandHere[land_idx] = (current_player.is_allied[land_owners[land_idx]] &&
                                   combat_status[land_idx] == CombatStatus::NO_COMBAT);
  }
  //  refresh_canBomberLandIn1Move
  BoolAirArray& canBomberLandIn1Move = state.cache.canBomberLandIn1Move;
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    canBomberLandIn1Move[land_idx] = false;
    const uint land_conn_count = LANDS[land_idx].land_conn_count;
    const L2LConn connected_land_index = LANDS[land_idx].land_conns;
    for (uint conn_idx = 0; conn_idx < land_conn_count; conn_idx++) {
      if (canBomberLandHere[connected_land_index[conn_idx]]) {
        canBomberLandIn1Move[land_idx] = true;
        break;
      }
    }
  }
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    canBomberLandIn1Move[LANDS_COUNT + sea_idx] = false;
    const uint land_conn_count = SEAS[sea_idx].land_conn_count;
    const S2LConn connected_land_index = SEAS[sea_idx].land_conns;
    for (uint conn_idx = 0; conn_idx < land_conn_count; conn_idx++) {
      if (canBomberLandHere[connected_land_index[conn_idx]]) {
        canBomberLandIn1Move[LANDS_COUNT + sea_idx] = true;
        break;
      }
    }
  }
  BoolAirArray& canBomberLandIn2Moves = state.cache.canBomberLandIn2Moves;
  // refresh_canBomberLandIn2Moves
  for (uint air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {
    canBomberLandIn2Moves[air_idx] = false;
    const uint air_conn_count = AIR_CONN_COUNT[air_idx];
    const A2AConn air_conn = AIR_CONNECTIONS[air_idx];
    for (uint conn_idx = 0; conn_idx < air_conn_count; conn_idx++) {
      if (canBomberLandIn1Move[air_conn[conn_idx]]) {
        canBomberLandIn2Moves[air_idx] = true;
        break;
      }
    }
  }
}

void add_valid_bomber_moves(GameState& state, uint src_air) {
  const BoolAirArray& canBomberLandHere = state.cache.canBomberLandHere;
  const BoolAirArray& canBomberLandIn1Move = state.cache.canBomberLandIn1Move;
  const BoolAirArray& canBomberLandIn2Moves = state.cache.canBomberLandIn2Moves;
  const AirArray& enemy_units_count =
      state.cache.team_units_count.arr(ENEMY_TEAM[state.current_turn]);
  const LandArray& factory_max = state.factory_max;
  const LandArray& factory_dmg = state.factory_dmg;
  const AirArray& near_air = AIR_WITHIN_X_MOVES[BOMBER_MOVES_MAX - 1][src_air];
  const BoolAirAirArray& skipped_moves = state.skipped_moves;
  const auto& air_dist_src_air = AIR_DIST[src_air];
  std::vector<uint>& valid_moves = state.cache.valid_moves;
  uint near_air_count = AIR_WITHIN_X_MOVES_COUNT[BOMBER_MOVES_MAX - 1][src_air];
  for (uint i = 0; i < near_air_count; i++) {
    uint dst_air = near_air[i];
    uint air_dist = air_dist_src_air[dst_air];
    if (air_dist <= 3 || canBomberLandHere[dst_air] ||
        (air_dist == 4 && canBomberLandIn2Moves[dst_air]) ||
        (air_dist == 5 && canBomberLandIn1Move[dst_air])) {
      if (!canBomberLandHere[dst_air] && enemy_units_count[dst_air] == 0) {
        if (dst_air >= LANDS_COUNT ||
            factory_dmg[dst_air] == factory_max[dst_air] * 2) // waste of a move
          continue;
      }
      // add_valid_air_move_if_history_allows_X(dst_air, src_air, air_dist);
      if (!skipped_moves[src_air][dst_air]) {
        valid_moves.push_back(dst_air);
      }
    }
  }
}

uint getUserInput(GameState& state) {
  std::string buffer; // Buffer to hold input string
  uint user_input = 0;
  std::vector<uint>& valid_moves = state.cache.valid_moves;
  while (true) {
    // 0-valid_moves_count
    std::cout << "Enter a number between 0 and 255: ";
    if (std::getline(std::cin, buffer)) {
      try {
        int temp_input = std::stoi(buffer);
        if (temp_input >= 0) {
          user_input = static_cast<uint>(temp_input);
        }
        if (user_input >= 0) {
          for (uint valid_move : valid_moves) {
            if (user_input == valid_move) {
              return user_input;
            }
          }
          return valid_moves[0];
        }
      } catch (const std::invalid_argument& e) {
        // Handle invalid input
      } catch (const std::out_of_range& e) {
        // Handle out of range input
      }
    }
  }
}

uint getAIInput(GameState& state) {
  state.cache.answers_remaining--;
  uint selected_action = state.cache.selected_action;
  std::vector<uint>& valid_moves = state.cache.valid_moves;
  if (selected_action < ACTION_COUNT) {
    assert(std::ranges::find(valid_moves, selected_action) != valid_moves.end());
    return selected_action;
  }
  return valid_moves[RANDOM_NUMBERS[state.seed++] % valid_moves.size()];
}

uint get_user_move_input(GameState& state, uint unit_type, uint src_air) {
  if (PLAYERS[state.current_turn].is_human) {
    std::ostringstream oss;
    oss << "Moving ";
    if (src_air < LANDS_COUNT) {
      oss << NAMES_UNIT_LAND[unit_type] << " From: " << LANDS[src_air].name;
    } else {
      oss << NAMES_UNIT_SEA[unit_type] << " From: " << SEAS[src_air - LANDS_COUNT].name;
    }
    oss << " Valid Moves: ";
    std::vector<uint>& valid_moves = state.cache.valid_moves;
    for (uint valid_move : valid_moves) {
      oss << valid_move << " ";
    }
    return getUserInput(state);
  }
  return getAIInput(state);
}

bool stage_transport_units(GameState& state, SeaUnitTypesEnum unit_type) {
  debug_checks(state);
  const uint player_idx = state.current_turn;
  const uint& answers_remaining = state.cache.answers_remaining;
  const SeaSeaArray& sea_dist = SEA_DIST[state.cache.canal_state];
  const SeaArray& enemy_blockade_total = state.cache.enemy_blockade_total;
  AirArray& total_player_units_player = state.cache.total_player_units.arr(player_idx);
  AirArray& team_units_count_team = state.cache.team_units_count.arr(PLAYER_TEAM[player_idx]);
  SeaArray& transports_with_small_cargo_space = state.cache.transports_with_small_cargo_space;
  SeaArray& transports_with_large_cargo_space = state.cache.transports_with_large_cargo_space;
  CombatStatusArray& combat_status = state.combat_status;
  bool units_to_process = false;
  // loop through transports with "3" moves remaining (that aren't full),
  // start at sea 0 to n
  const uint staging_state = STATES_MOVE_SEA[unit_type] - 1;
  const uint done_staging = staging_state - 1;
  std::vector<uint>& valid_moves = state.cache.valid_moves;
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    std::vector<uint>& sea_units = get_active_sea_units(state).at(unit_type)->at(src_sea);
    uint& unmoved_sea_units = sea_units.at(staging_state);
    if (unmoved_sea_units == 0) {
      continue;
    }
    units_to_process = true;
    const uint src_air = src_sea + LANDS_COUNT;
    valid_moves.assign(1, src_air);
    add_valid_sea_moves(state, src_sea, 2);
    const SeaArray& sea_dist_src_sea = sea_dist[src_sea];
    const uint enemy_blockade_total_src_sea = enemy_blockade_total[src_sea];
    while (unmoved_sea_units > 0) {
      uint dst_air = valid_moves[0];
      if (valid_moves.size() > 1) {
        if (answers_remaining == 0)
          return true;
        dst_air = get_user_move_input(state, unit_type, src_air);
      }
      // update_move_history(dst_air, src_sea); todo update_move_history_4air
      if (src_air == dst_air) {
        sea_units.at(done_staging) += unmoved_sea_units;
        unmoved_sea_units = 0;
        continue;
      }
      const uint dst_sea = dst_air - LANDS_COUNT;
      uint sea_distance = sea_dist_src_sea[dst_air];
      if (enemy_blockade_total_src_sea > 0) {
        combat_status[dst_air] = CombatStatus::PRE_COMBAT;
        sea_distance = MAX_MOVE_SEA[unit_type];
        continue;
      }
      get_active_sea_units(state).at(unit_type)->at(dst_sea)[staging_state - sea_distance - 1]++;
      get_idle_sea_units(state).at(unit_type)->ref(state.current_turn, dst_sea)++;
      total_player_units_player.at(dst_air)++;
      team_units_count_team.at(dst_air)++;
      transports_with_small_cargo_space[dst_sea]++;
      unmoved_sea_units--;
      get_idle_sea_units(state).at(unit_type)->ref(state.current_turn, src_sea)--;
      total_player_units_player.at(src_air)--;
      team_units_count_team.at(src_air)--;
      transports_with_small_cargo_space[src_sea]--;
      if (unit_type <= TRANS1I) {
        transports_with_large_cargo_space[src_sea]--;
        transports_with_large_cargo_space[dst_sea]++;
      }
    }
  }
  if (units_to_process) {
    clear_move_history(state);
  }
  return false;
}

void add_valid_sea_moves(GameState& state, uint src_sea, uint moves_remaining) {
  std::vector<uint>& valid_moves = state.cache.valid_moves;
  const uint canal_state = state.cache.canal_state;
  if (moves_remaining == 2) {
    // check for moving from sea to sea (two moves away)
    const uint seas_within_2_moves_count = SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea];
    const SeaArray& seas_within_2_moves = SEAS_WITHIN_2_MOVES[canal_state][src_sea];
    const BoolSeaArray& is_sea_path_blocked_src_sea = state.cache.sea_path_blocked.arr(src_sea);
    for (uint sea_idx = 0; sea_idx < seas_within_2_moves_count; sea_idx++) {
      const uint dst_sea = seas_within_2_moves[sea_idx];
      if (is_sea_path_blocked_src_sea[dst_sea]) {
        continue;
      }
      const uint dst_air = dst_sea + LANDS_COUNT;
      if (state.skipped_moves[src_sea + LANDS_COUNT][dst_air]) {
        continue;
      }
      valid_moves.push_back(dst_air);
    }
  }
  return;
  // check for moving from sea to sea (one move away)
  const uint seas_within_1_move_count = SEAS_WITHIN_1_MOVE_COUNT[canal_state][src_sea];
  const SeaArray& seas_within_1_move = SEAS_WITHIN_2_MOVES[canal_state][src_sea];
  for (uint sea_idx = 0; sea_idx < seas_within_1_move_count; sea_idx++) {
    const uint dst_sea = seas_within_1_move[sea_idx];
    const uint dst_air = dst_sea + LANDS_COUNT;
    if (state.skipped_moves[src_sea + LANDS_COUNT][dst_air]) {
      continue;
    }
    valid_moves.push_back(dst_air);
  }
}

bool load_transport(GameState& state, GameCache& cache, uint land_unit_type, uint src_land,
                    uint dst_sea, uint land_unit_state) {
  uint player_idx = state.current_turn;
  Seaunittypes new_trans_type_array = NEW_TRANS_TYPE_AFTER_LOAD[land_unit_type];
  uint starting_type = (UNIT_WEIGHTS[land_unit_type] > 2) ? TRANS1I : TRANS1T;
  for (uint trans_type1 = 0; trans_type1 < TRANS1T; trans_type1++) {
    uint trans_type = starting_type - trans_type1;
    if (trans_type < TRANSEMPTY) {
      break;
    }
    std::vector<uint> units_sea_ptr_dst_sea_trans_type =
        get_active_sea_units(state).at(trans_type)->at(dst_sea);
    uint states_unloading = STATES_UNLOADING[trans_type];
    for (uint trans_state1 = 0; trans_state1 < states_unloading; trans_state1++) {
      uint trans_state = STATES_MOVE_SEA[trans_type] - STATES_STAGING[trans_type] - trans_state1;
      if (units_sea_ptr_dst_sea_trans_type[trans_state] > 0) {
        units_sea_ptr_dst_sea_trans_type[trans_state]--;
        get_idle_sea_units(state).at(trans_type)->ref(player_idx, dst_sea)--;
        uint new_trans_type = new_trans_type_array[trans_type];
        get_active_sea_units(state).at(new_trans_type)->at(dst_sea)[trans_state]++;
        get_idle_sea_units(state).at(new_trans_type)->ref(player_idx, dst_sea)++;
        cache.total_player_units.ref(player_idx, src_land)--;
        cache.team_units_count.ref(player_idx, src_land)--;
        get_idle_land_units(state).at(src_land)->ref(player_idx, land_unit_type)--;
        get_active_land_units(state).at(land_unit_type)->at(src_land)[land_unit_state]--;
        refresh_transports_with_cargo_space(state, player_idx, dst_sea);
        return true;
      }
    }
  }
  printf("Error: Failed to load transport\n");
  cause_breakpoint();
  return false;
}

void add_valid_land_moves(GameState& state, uint src_land, uint moves_remaining, uint unit_type) {
  const BoolLandArray& land_path_blocked_src_land = state.cache.land_path_blocked.arr(src_land);
  const BoolAirArray& skipped_moves_src_land = state.skipped_moves[src_land];
  const SeaArray& transports_with_large_cargo_space = state.cache.transports_with_large_cargo_space;
  const SeaArray& transports_with_small_cargo_space = state.cache.transports_with_small_cargo_space;
  const LandArray& land_owners = state.land_owners;
  std::vector<uint>& valid_moves = state.cache.valid_moves;
  if (moves_remaining == 2) {
    // check for moving from land to land (two moves away)
    const uint lands_within_2_moves_count = LANDS_WITHIN_2_MOVES_COUNT[src_land];
    const LandArray lands_within_2_moves = LANDS_WITHIN_2_MOVES[src_land];
    const AirArray land_dist = LAND_DIST[src_land];
    for (uint land_idx = 0; land_idx < lands_within_2_moves_count; land_idx++) {
      uint dst_land = lands_within_2_moves[land_idx];
      if (skipped_moves_src_land[dst_land] ||
          (land_dist[dst_land] == 2 && land_path_blocked_src_land[dst_land])) {
        continue;
      }
      valid_moves.push_back(dst_land);
    }
    // check for moving from land to sea (two moves away)
    const uint load_within_2_moves_count = LOAD_WITHIN_2_MOVES_COUNT[src_land];
    const SeaArray load_within_2_moves = LOAD_WITHIN_2_MOVES[src_land];
    for (uint sea_idx = 0; sea_idx < load_within_2_moves_count; sea_idx++) {
      uint dst_sea = load_within_2_moves[sea_idx];
      if (transports_with_large_cargo_space[dst_sea] == 0) { // assume large, only tanks move 2
        continue;
      }
      uint dst_air = dst_sea + LANDS_COUNT;
      if (skipped_moves_src_land[dst_air] ||
          (land_dist[dst_air] == 1 && land_path_blocked_src_land[dst_air])) {
        continue;
      }
      valid_moves.push_back(dst_air);
    }
  } else {
    // check for moving from land to land (one move away)
    uint lands_within_1_move_count = LAND_TO_LAND_COUNT[src_land];
    L2LConn lands_within_1_move = LAND_TO_LAND_CONN[src_land];
    bool is_non_combat_unit = ATTACK_UNIT_LAND[unit_type] == 0;
    bool is_unloadable_unit = UNIT_WEIGHTS[unit_type] > 5;
    bool is_heavy_unit = UNIT_WEIGHTS[unit_type] > 2;
    for (uint land_idx = 0; land_idx < lands_within_1_move_count; land_idx++) {
      const uint dst_land = lands_within_1_move[land_idx];
      if (skipped_moves_src_land[dst_land] ||
          (is_non_combat_unit && !PLAYERS[state.current_turn].is_allied[land_owners[dst_land]])) {
        continue;
      }
      valid_moves.push_back(dst_land);
    }
    // check for moving from land to sea (one move away)
    if (is_unloadable_unit) {
      return;
    }
    const uint land_to_sea_count = LAND_TO_SEA_COUNT[src_land];
    const L2SConn land_to_sea_conn = LAND_TO_SEA_CONN[src_land];
    for (uint sea_idx = 0; sea_idx < land_to_sea_count; sea_idx++) {
      const uint dst_sea = land_to_sea_conn[sea_idx];
      if (transports_with_small_cargo_space[dst_sea] == 0 ||
          (is_heavy_unit && transports_with_large_cargo_space[dst_sea] == 0)) {
        continue;
      }
      const uint dst_air = dst_sea + LANDS_COUNT;
      if (state.skipped_moves[src_land][dst_air]) {
        continue;
      }
      valid_moves.push_back(dst_air);
    }
  }
}

void add_valid_sub_moves(GameState& state, uint src_sea, uint moves_remaining) {
  const uint canal_state = state.cache.canal_state;
  std::vector<uint>& valid_moves = state.cache.valid_moves;
  if (moves_remaining == 2) {
    // check for moving from sea to sea (two moves away)
    const uint seas_within_2_moves_count = SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea];
    const SeaArray& seas_within_2_moves = SEAS_WITHIN_2_MOVES[canal_state][src_sea];
    const BoolSeaArray sub_path_blocked_src_sea = state.cache.sub_path_blocked.arr(src_sea);
    for (uint sea_idx = 0; sea_idx < seas_within_2_moves_count; sea_idx++) {
      uint dst_sea = seas_within_2_moves[sea_idx];
      if (sub_path_blocked_src_sea[dst_sea]) {
        continue;
      }
      uint dst_air = dst_sea + LANDS_COUNT;
      if (state.skipped_moves[src_sea + LANDS_COUNT][dst_air]) {
        continue;
      }
      valid_moves.push_back(dst_air);
    }
  } else {
    // check for moving from sea to sea (one move away)
    const uint seas_within_1_move_count = SEAS_WITHIN_1_MOVE_COUNT[canal_state][src_sea];
    const SeaArray& seas_within_1_move = SEAS_WITHIN_1_MOVE[canal_state][src_sea];
    for (uint sea_idx = 0; sea_idx < seas_within_1_move_count; sea_idx++) {
      const uint dst_air = seas_within_1_move[sea_idx] + LANDS_COUNT;
      if (state.skipped_moves[src_sea + LANDS_COUNT][dst_air]) {
        continue;
      }
      valid_moves.push_back(dst_air);
    }
  }
}

void conquer_land(GameState& state, uint dst_land) {
#ifdef DEBUG
  if (state.cache.actually_print) {
    setPrintableStatus();
    std::ostringstream oss;
    oss << printableGameStatus << "\n";
    oss << "conquer_land: dst_land=" << dst_land << "\n";
    std::string output = oss.str();
    printf("%s", output.c_str());
  }
#endif
  uint old_owner_id = *owner_idx[dst_land];
  if (PLAYERS[(state.current_turn + old_owner_id) % PLAYERS_COUNT].capital_territory_index ==
      dst_land) {
    state.money[0] += state.money[old_owner_id];
    state.money[old_owner_id] = 0;
  }
  income_per_turn[old_owner_id] -= LAND_VALUE[dst_land];
  uint new_owner_id = 0;
  uint orig_owner_id =
      (LANDS[dst_land].original_owner_index + PLAYERS_COUNT - state.current_turn) % PLAYERS_COUNT;
  if (is_allied_0[orig_owner_id]) {
    new_owner_id = orig_owner_id;
  }
#ifdef DEBUG
  if (state.cache.actually_print) {
    printf("conquer_land: old_owner_id=%d new_owner_id=%d orig_owner_id=%d\n", old_owner_id,
           new_owner_id, orig_owner_id);
  }
#endif
  *owner_idx[dst_land] = new_owner_id;
  income_per_turn[new_owner_id] += LAND_VALUE[dst_land];
  factory_locations[new_owner_id][total_factory_count[new_owner_id]++] = dst_land;
  total_factory_count[old_owner_id]--;
  for (uint i = 0; i < total_factory_count[old_owner_id]; i++) {
    if (factory_locations[old_owner_id][i] == dst_land) {
#ifdef DEBUG
      if (state.cache.actually_print) {
        printf("DEBUG: Found factory at %s\n", LANDS[dst_land].name);
      }
#endif
      for (uint j = i; j < total_factory_count[old_owner_id]; j++) {
#ifdef DEBUG
        if (state.cache.actually_print) {
          printf("DEBUG: Moving factory at %s\n", LANDS[factory_locations[old_owner_id][j]].name);
        }
#endif
        factory_locations[old_owner_id][j] = factory_locations[old_owner_id][j + 1];
      }
      break;
    }
  }
#ifdef DEBUG
  if (state.cache.actually_print) {
    setPrintableStatus();
    std::ostringstream oss;
    oss << printableGameStatus << "\n";
    for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      for (uint factory_index = 0; factory_index < total_factory_count[player_idx];
           factory_index++) {
        uint factory_location = factory_locations[player_idx][factory_index];
        if (*owner_idx[factory_location] != player_idx) {
          oss << "DEBUG: Player " << PLAYERS[(state.current_turn + player_idx) % PLAYERS_COUNT].name
              << " has an unowned factory at " << LANDS[factory_location].name << "\n";
        }
      }
    }
    std::string output = oss.str();
    printf("%s", output.c_str());
  }
#endif
}

bool move_land_unit_type(uint unit_type) {
  debug_checks(state);
  bool units_to_process = false;
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (uint moves_remaining1 = 0; moves_remaining1 < MAX_MOVE_LAND[unit_type];
         moves_remaining1++) {
      uint moves_remaining = MAX_MOVE_LAND[unit_type] - moves_remaining1;
      uint* total_units = &land_units_state[src_land][unit_type][moves_remaining];
      if (*total_units == 0) {
        continue;
      }
      units_to_process = true;
      valid_moves[0] = src_land;
      valid_moves_count = 1;
      add_valid_land_moves(src_land, moves_remaining, unit_type);
      while (*total_units > 0) {
        units_to_process = true;
        uint dst_air = valid_moves[0];
        if (valid_moves_count > 1) {
          if (answers_remaining == 0) {
            return true;
          }
          dst_air = get_user_move_input(unit_type, src_land);
        }
#ifdef DEBUG
        if (state.cache.actually_print) {
          setPrintableStatus();
          std::ostringstream oss;
          oss << printableGameStatus << "\n";
          oss << "DEBUG: player: " << PLAYERS[state.current_turn].name << " moving land unit "
              << unit_type << ", src_land: " << src_land << ", dst_air: " << dst_air
              << ", enemy count: " << enemy_units_count[dst_air] << "\n";
          std::string output = oss.str();
          printf("%s", output.c_str());
        }
#endif
        // update_move_history(dst_air, src_land);
        if (src_land == dst_air) {
          land_units_state[src_land][unit_type][0] += *total_units;
          *total_units = 0;
          continue;
        }
        if (dst_air >= LANDS_COUNT) {
          load_transport(unit_type, src_land, dst_air - LANDS_COUNT, moves_remaining);
          // recalculate valid moves since transport cargo has changed
          valid_moves_count = 1;
          add_valid_land_moves(src_land, moves_remaining, unit_type);
          continue;
        }
        state.combat_status[dst_air] =
            enemy_units_count[dst_air] > 0 ? CombatStatus::PRE_COMBAT : CombatStatus::NO_COMBAT;
        // if the destination is not blitzable, then end unit turn
        uint landDistance = LAND_DIST[src_land][dst_air];
        if (is_allied_0[*owner_idx[dst_air]] || enemy_units_count[dst_air] > 0) {
          landDistance = moves_remaining;
        }
        land_units_state[dst_air][unit_type][moves_remaining - landDistance]++;
        current_player_land_unit_types[dst_air][unit_type]++;
        total_player_land_units[0][dst_air]++;
        current_player_land_unit_types[src_land][unit_type]--;
        total_player_land_units[0][src_land]--;
        *total_units -= 1;
        if (!is_allied_0[*owner_idx[dst_air]] && enemy_units_count[dst_air] == 0) {
#ifdef DEBUG
          if (state.cache.actually_print) {
            printf("Conquering land");
          }
#endif
          conquer_land(dst_air);
          state.combat_status[dst_air] = CombatStatus::PRE_COMBAT;
        }
#ifdef DEBUG
        debug_checks();
#endif
      }
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}

void skip_empty_transports() {
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
#pragma unroll
    for (uint cur_state1 = 0; cur_state1 <= TRANSEMPTY_STATES - TRANSEMPTY_STAGING_STATES -
                                                TRANSEMPTY_UNLOADING_STATES - 1;
         cur_state1++) {
      uint cur_state = TRANSEMPTY_STATES - TRANSEMPTY_STAGING_STATES - cur_state1;
      uint* total_ships = &sea_units_state[src_sea][TRANSEMPTY][cur_state];
      if (*total_ships == 0) {
        continue;
      }
      sea_units_state[src_sea][TRANSEMPTY][0] += *total_ships;
      *total_ships = 0;
    }
  }
}
typedef uint unit_states[5];
typedef unit_states* unit_states_ptr;

bool move_transport_units() {
  bool units_to_process = false;
  skip_empty_transports();
#ifdef DEBUG
  if (state.cache.actually_print) {
    printf("DEBUG: move_transport_units\n");
  }
#endif
  for (uint unit_type = TRANS1I; unit_type <= TRANS1I1T;
       unit_type++) { // there should be no TRANSEMPTY
    uint max_state = STATES_MOVE_SEA[unit_type] - STATES_STAGING[unit_type];
    // uint done_moving = 1;//STATES_UNLOADING[unit_type];
    // uint min_state = 2;//STATES_UNLOADING[unit_type] + 1;
    // clear_move_history();
    for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      // for (uint cur_state = max_state; cur_state >= min_state; cur_state--) {
      for (uint i = 1; i < 3; i++) {
        uint cur_state = max_state - i;
        uint* total_ships = &sea_units_state[src_sea][unit_type][cur_state];
        // unit_states_ptr unit_states = total_ships;
        if (*total_ships == 0) {
          continue;
        }
        units_to_process = true;
        uint moves_remaining = cur_state - 1; // STATES_UNLOADING[unit_type];
        uint src_air = src_sea + LANDS_COUNT;
        valid_moves[0] = src_air;
        valid_moves_count = 1;
        add_valid_sea_moves(src_sea, moves_remaining);
        while (*total_ships > 0) {
          units_to_process = true;
          uint dst_air = valid_moves[0];
          if (valid_moves_count > 1) {
            if (answers_remaining == 0)
              return true;
            dst_air = get_user_move_input(unit_type, src_air);
          }
#ifdef DEBUG
          if (state.cache.actually_print) {
            printf("DEBUG: moving transport units unit_type: %d, src_air: %d, dst_air: %d\n",
                   unit_type, src_air, dst_air);
          }
#endif
          // update_move_history(dst_air, src_air);
          uint dst_sea = dst_air - LANDS_COUNT;
          if (enemy_blockade_total[dst_sea] > 0) {
#ifdef DEBUG
            if (state.cache.actually_print) {
              printf("Enemy units detected, flagging for combat\n");
            }
#endif
            state.combat_status[dst_air] = CombatStatus::PRE_COMBAT;
          }
          if (src_air == dst_air) {
            sea_units_state[src_sea][unit_type][1] += *total_ships;
            *total_ships = 0;
            continue;
          }
          sea_units_state[dst_sea][unit_type][1]++;
          current_player_sea_unit_types[dst_sea][unit_type]++;
          total_player_sea_units[0][dst_sea]++;
          (*total_ships)--;
          current_player_sea_unit_types[src_sea][unit_type]--;
          total_player_sea_units[0][src_sea]--;
          if (unit_type <= TRANS1T) {
            transports_with_small_cargo_space[dst_sea]++;
            transports_with_small_cargo_space[src_sea]--;
            if (unit_type <= TRANS1I) {
              transports_with_large_cargo_space[dst_sea]++;
              transports_with_large_cargo_space[src_sea]--;
            }
          }
        }
      }
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}
bool move_subs() {
  bool units_to_process = false;
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    uint* total_subs = &sea_units_state[src_sea][SUBMARINES][SUB_UNMOVED];
    if (*total_subs == 0) {
      continue;
    }
    uint src_air = src_sea + LANDS_COUNT;
    valid_moves[0] = src_air;
    valid_moves_count = 1;
    add_valid_sub_moves(src_sea, SUB_MOVES_MAX);
    while (*total_subs > 0) {
      units_to_process = true;
      uint dst_air = valid_moves[0];
      if (valid_moves_count > 1) {
        if (answers_remaining == 0) {
          return true;
        }
        dst_air = get_user_move_input(SUBMARINES, src_air);
      }
#ifdef DEBUG
      setPrintableStatus();
      std::ostringstream oss;
      oss << printableGameStatus << "\n";
      oss << "DEBUG: moving sub units unit_type: " << SUBMARINES << ", src_air: " << src_air
          << ", dst_air: " << dst_air << "\n";
      std::string output = oss.str();
      std::cout << output;
#endif
      // update_move_history(dst_air, src_air);
      uint dst_sea = dst_air - LANDS_COUNT;
      if (enemy_units_count[dst_sea] > 0) {
#ifdef DEBUG
        if (state.cache.actually_print) {
          std::cout << "Submarine moving to where enemy units are present, flagging for combat\n";
        }
#endif
        state.combat_status.at(dst_sea) = CombatStatus::PRE_COMBAT;
        // break;
      }
      if (src_air == dst_air) {
        sea_units_state[src_sea][SUBMARINES][SUB_DONE_MOVING] += *total_subs;
        *total_subs = 0;
        continue;
      }
      sea_units_state[dst_sea][SUBMARINES][SUB_DONE_MOVING]++;
      current_player_sea_unit_types[dst_sea][SUBMARINES]++;
      total_player_sea_units[0][dst_sea]++;
      *total_subs -= 1;
      current_player_sea_unit_types[src_sea][SUBMARINES]--;
      total_player_sea_units[0][src_sea]--;
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}

bool move_destroyers_battleships() {
  bool units_to_process = false;
  for (uint unit_type = DESTROYERS; unit_type <= BS_DAMAGED; unit_type++) {
    uint unmoved = UNMOVED_SEA[unit_type];
    uint done_moving = DONE_MOVING_SEA[unit_type];
    uint moves_remaining = MAX_MOVE_SEA[unit_type];
    // clear_move_history();
    for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      uint* total_ships = &sea_units_state[src_sea][unit_type][unmoved];
      if (*total_ships == 0) {
        continue;
      }
      units_to_process = true;
      uint src_air = src_sea + LANDS_COUNT;
      valid_moves[0] = src_air;
      valid_moves_count = 1;
      add_valid_sea_moves(src_sea, moves_remaining);
      while (*total_ships > 0) {
        units_to_process = true;
        uint dst_air = valid_moves[0];
        if (valid_moves_count > 1) {
          if (answers_remaining == 0) {
            return true;
          }
          dst_air = get_user_move_input(unit_type, src_air);
        }
#ifdef DEBUG
        if (state.cache.actually_print) {
          setPrintableStatus();
          std::ostringstream oss;
          oss << printableGameStatus << "\n";
          oss << "DEBUG: moving ships units unit_type: " << unit_type << ", src_air: " << src_air
              << ", dst_air: " << dst_air << "\n";
          std::string output = oss.str();
          std::cout << output;
        }
#endif
        // update_move_history(dst_air, src_air);
        if (enemy_units_count[dst_air] > 0) {
#ifdef DEBUG
          if (state.cache.actually_print) {
            std::cout << "Moving large ships. Enemy units detected, flagging for combat\n";
          }
#endif
          state.combat_status.at(dst_air) = CombatStatus::PRE_COMBAT;
          // break;
        }
        if (src_air == dst_air) {
          sea_units_state[src_sea][unit_type][done_moving] += *total_ships;
          *total_ships = 0;
          continue;
        }
        uint dst_sea = dst_air - LANDS_COUNT;
        sea_units_state[dst_sea][unit_type][done_moving]++;
        current_player_sea_unit_types[dst_sea][unit_type]++;
        total_player_sea_units[0][dst_sea]++;
        *total_ships -= 1;
        current_player_sea_unit_types[src_sea][unit_type]--;
        total_player_sea_units[0][src_sea]--;
        if (unit_type == CARRIERS) {
          carry_allied_fighters(src_sea, dst_sea);
        }
#ifdef DEBUG
        debug_checks();
#endif
      }
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}

void carry_allied_fighters(uint src_sea, uint dst_sea) {
#ifdef DEBUG
  if (state.cache.actually_print) {
    std::ostringstream oss;
    oss << "DEBUG: carry_allied_fighters: src_sea: " << src_sea << ", dst_sea: " << dst_sea << "\n";
    setPrintableStatus();
    oss << printableGameStatus << "\n";
    std::string output = oss.str();
    std::cout << output;
  }
  debug_checks();
#endif
  uint allied_fighters_moved = 0;
  for (uint other_player_idx = 1; other_player_idx < PLAYERS_COUNT; other_player_idx++) {
    if (!is_allied_0[other_player_idx]) {
      continue;
    }
    while (total_player_sea_unit_types[other_player_idx][src_sea][FIGHTERS] > 0) {
      total_player_sea_unit_types[other_player_idx][src_sea][FIGHTERS]--;
      total_player_sea_units[other_player_idx][src_sea]--;
      total_player_sea_unit_types[other_player_idx][dst_sea][FIGHTERS]++;
      total_player_sea_units[other_player_idx][dst_sea]++;
      if (allied_fighters_moved == 1) {
        return;
      }
      allied_fighters_moved++;
    }
  }
}

bool resolve_sea_battles() {
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    // check if battle is over (e.g. untargetable subs/air/trans or zero units)
    // target options:
    // 1. attacker has air, defender has std ships or air
    // 2. attacker has any ships, defender has any non-transports
    // 3. defender has air, attacker has std ships or air
    // 4. defender has any ships, attacker has any non-transports
    uint src_air = src_sea + LANDS_COUNT;
    // if not flagged for combat, continue
    if (state.combat_status.at(src_air) == CombatStatus::NO_COMBAT) {
      continue;
    }
    if (total_player_sea_units[0][src_sea] == 0) {
      continue;
    }
#ifdef DEBUG
    if (state.cache.actually_print) {
      setPrintableStatus();
      std::ostringstream oss;
      oss << printableGameStatus << "\n";
      oss << "DEBUG: resolve_sea_battles: src_sea: " << src_sea << "\n";
      std::string output = oss.str();
      std::cout << output;
    }
#endif
    bool defender_submerged = current_player_sea_unit_types[src_sea][DESTROYERS] == 0;
    if (total_player_sea_units[0][src_sea] == 2) {
      // does enemy only have submerged subs?
      if (defender_submerged) {
        uint total_enemy_subs = 0;
        for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          total_enemy_subs +=
              total_player_sea_unit_types[enemies_0[enemy_idx]][src_sea][SUBMARINES];
        }
        if (total_enemy_subs == enemy_units_count[src_sea]) {
          continue;
        }
      }
      // combat is proceeding, so disable bombardment capabilities of ships
      for (uint unit_type = CRUISERS; unit_type <= BS_DAMAGED; unit_type++) {
        sea_units_state[src_sea][unit_type][0] += sea_units_state[src_sea][unit_type][1];
        sea_units_state[src_sea][unit_type][1] = 0;
      }
    }
    while (true) {
#ifdef DEBUG
      if (state.cache.actually_print) {
        setPrintableStatus();
        std::ostringstream oss;
        oss << printableGameStatus << "\n";
        if (current_player_land_unit_types[2][FIGHTERS] == 255) {
          oss << "DEBUG: units_land_ptr[0][FIGHTERS][0] == 255\n";
        }
        std::string output = oss.str();
        std::cout << output;
      }
#endif
      // check for retreat option
      if (state.combat_status.at(src_air) == CombatStatus::MID_COMBAT) {
        if (current_player_sea_unit_types[src_sea][FIGHTERS] +
                    current_player_sea_unit_types[src_sea][BOMBERS_SEA] +
                    current_player_sea_unit_types[src_sea][SUBMARINES] +
                    current_player_sea_unit_types[src_sea][DESTROYERS] +
                    current_player_sea_unit_types[src_sea][CARRIERS] +
                    current_player_sea_unit_types[src_sea][CRUISERS] +
                    current_player_sea_unit_types[src_sea][BATTLESHIPS] +
                    current_player_sea_unit_types[src_sea][BS_DAMAGED] >
                0 ||
            enemy_blockade_total[src_sea] == 0) {
          valid_moves[0] = src_air;
          valid_moves_count = 1;
        } else {
          valid_moves_count = 0;
        }
        S2SConn sea_to_sea_conn = SEA_TO_SEA_CONN[src_sea];
        for (uint sea_conn_idx = 0; sea_conn_idx < SEA_TO_SEA_COUNT[src_sea]; sea_conn_idx++) {
          uint sea_dst = sea_to_sea_conn[sea_conn_idx];
          if (enemy_blockade_total[sea_dst] == 0) {
            valid_moves[valid_moves_count++] = sea_dst + LANDS_COUNT;
          }
        }
        if (valid_moves_count > 0) {
          uint dst_air = 0;
          if (valid_moves_count == 1) {
            dst_air = valid_moves[0];
          } else {
            if (answers_remaining == 0) {
              return true;
            }
            dst_air = ask_to_retreat();
          }
          // if retreat, move units to retreat zone immediately and end battle
          uint dst_sea = dst_air - LANDS_COUNT;
          if (sea_dist[src_sea][dst_sea] == 1 &&
              state.combat_status.at(dst_air) == CombatStatus::NO_COMBAT) {
            sea_retreat(src_sea, dst_sea);
            break;
          }
        }
      }
      state.combat_status.at(src_air) = CombatStatus::MID_COMBAT;
      // uint* units11 = &sea_units_state[src_sea][DESTROYERS][0];
      bool targets_exist = false;
      if (current_player_sea_unit_types[src_sea][DESTROYERS] > 0) {
        if (enemy_units_count[src_air] > 0) {
          targets_exist = true;
        }
      } else if (current_player_sea_unit_types[src_sea][CARRIERS] +
                     current_player_sea_unit_types[src_sea][CRUISERS] +
                     current_player_sea_unit_types[src_sea][BATTLESHIPS] +
                     current_player_sea_unit_types[src_sea][BS_DAMAGED] >
                 0) {
        if (enemy_units_count[src_air] > 0) {
          for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
            uint enemy_player_idx = enemies_0[enemy_idx];
            if (total_player_sea_units[enemy_player_idx][src_sea] -
                    total_player_sea_unit_types[enemy_player_idx][src_sea][SUBMARINES] >
                0) {
              targets_exist = true;
              break;
            }
          }
        }
      } else if (current_player_sea_unit_types[src_sea][SUBMARINES] >
                 0) { // no blockade ships, only subs
        for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          uint enemy_player_idx = enemies_0[enemy_idx];
          if (total_player_sea_units[enemy_player_idx][src_sea] -
                  (total_player_sea_unit_types[enemy_player_idx][src_sea][FIGHTERS] +
                   total_player_sea_unit_types[enemy_player_idx][src_sea][SUBMARINES]) >
              0) {
            targets_exist = true;
            break;
          }
        }
      } else if (current_player_sea_unit_types[src_sea][FIGHTERS] +
                     current_player_sea_unit_types[src_sea][BOMBERS_SEA] >
                 0) { // no ships, only air
        for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          uint enemy_player_idx = enemies_0[enemy_idx];
          if (total_player_sea_units[enemy_player_idx][src_sea] -
                  total_player_sea_unit_types[enemy_player_idx][src_sea][SUBMARINES] >
              0) {
            targets_exist = true;
            break;
          }
        }
      }
#ifdef DEBUG
      debug_checks();
#endif
      // todo only allow hits to hit valid targets
      // untargetable battle
      if (!targets_exist) {
        if (enemy_units_count[src_air] > 0) {
          total_player_sea_units[0][src_sea] -= total_player_sea_unit_types[0][src_sea][TRANSEMPTY];
          total_player_sea_unit_types[0][src_sea][TRANSEMPTY] = 0;
          sea_units_state[src_sea][TRANSEMPTY][0] = 0;
          for (uint trans_unit_type = TRANS1I; trans_unit_type <= TRANS1I1T; trans_unit_type++) {
            total_player_sea_units[0][src_sea] -=
                total_player_sea_unit_types[0][src_sea][trans_unit_type];
            total_player_sea_unit_types[0][src_sea][trans_unit_type] = 0;
            sea_units_state[src_sea][trans_unit_type][1] = 0;
          }
        }
        // allied_carriers[src_sea] =
        //     data.units_sea[src_sea].carriers[0] + data.units_sea[src_sea].carriers[1];
        // for (uint player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
        //   allied_carriers[src_sea] +=
        //       other_sea_units_ptr[player_idx][src_sea][CARRIERS] * is_allied_0[player_idx];
        // }
        state.combat_status.at(src_air) = CombatStatus::NO_COMBAT;
        break;
      }
      // fire subs (defender always submerges if possible)
      uint attacker_damage = sea_units_state[src_sea][SUBMARINES][0] * SUB_ATTACK; // todo FIX
      // uint attacker_damage = current_player_sea_unit_types[src_sea][SUBMARINES] * SUB_ATTACK;
      // //todo FIX
      uint attacker_hits = get_attacker_hits(attacker_damage);
      uint defender_damage = 0;
      uint defender_hits = 0;
      if (!defender_submerged) {
        defender_damage = 0; // todo skip if no subs
        for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          defender_damage += total_player_sea_unit_types[enemies_0[enemy_idx]][src_sea][SUBMARINES];
        }
        defender_hits = get_defender_hits(defender_damage);
#ifdef DEBUG
        debug_checks();
#endif
        if (defender_hits > 0) {
          remove_sea_attackers(src_sea, defender_hits);
        }
#ifdef DEBUG
        debug_checks();
#endif
      }
// remove casualties
#ifdef DEBUG
      debug_checks();
#endif
      if (attacker_hits > 0) {
        remove_sea_defenders(src_sea, attacker_hits, defender_submerged);
      }
      // fire all ships and air for both sides
      attacker_damage = 0;
      for (uint unit_type_idx = 0; unit_type_idx < BLOCKADE_UNIT_TYPES_COUNT; unit_type_idx++) {
        uint unit_type = BLOCKADE_UNIT_TYPES[unit_type_idx];
        attacker_damage +=
            current_player_sea_unit_types[src_sea][unit_type] * ATTACK_UNIT_SEA[unit_type];
      }
      attacker_damage +=
          current_player_sea_unit_types[src_sea][FIGHTERS] * ATTACK_UNIT_SEA[FIGHTERS];
      attacker_damage +=
          current_player_sea_unit_types[src_sea][BOMBERS_SEA] * ATTACK_UNIT_SEA[BOMBERS_SEA];
      attacker_hits = get_attacker_hits(attacker_damage);
      defender_damage = 0;
      for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        Seaunittypes enemy_units = total_player_sea_unit_types[enemies_0[enemy_idx]][src_sea];
        for (uint unit_type = 0; unit_type < BLOCKADE_UNIT_TYPES_COUNT; unit_type++) {
          defender_damage += enemy_units[unit_type] * DEFENSE_UNIT_SEA[unit_type];
        }
        defender_damage += enemy_units[FIGHTERS] * DEFENSE_UNIT_SEA[FIGHTERS];
      }
      defender_hits = get_defender_hits(defender_damage);
// remove casualties
#ifdef DEBUG
      debug_checks();
#endif
      if (defender_hits > 0) {
        remove_sea_attackers(src_sea, defender_hits);
      }
      if (attacker_hits > 0) {
        remove_sea_defenders(src_sea, attacker_hits, defender_submerged);
      }
      if (enemy_units_count[src_air] == 0 || total_player_sea_units[0][src_sea] == 0) {
        state.combat_status.at(src_air) = CombatStatus::NO_COMBAT;
        break;
      }
    }
  }
  return false;
}
constexpr uint DICE_SIDES = 6;
uint get_attacker_hits(uint attacker_damage) {
  uint attacker_hits = 0;
  if (answers_remaining < 2) {
    if (PLAYERS[state.current_turn].is_allied[unlucky_player_idx]) {
      attacker_hits = (attacker_damage / DICE_SIDES);
    } else {
      attacker_hits = (attacker_damage / DICE_SIDES) + (1 < attacker_damage % DICE_SIDES ? 1 : 0);
    }
  } else {
    attacker_hits =
        (attacker_damage / DICE_SIDES) +
        (RANDOM_NUMBERS[random_number_index++] % DICE_SIDES < attacker_damage % DICE_SIDES ? 1 : 0);
  }
  return attacker_hits;
}

uint get_defender_hits(uint defender_damage) {
  uint defender_hits = 0;
  if (answers_remaining < 2) {
    if (PLAYERS[state.current_turn].is_allied[unlucky_player_idx]) {
      defender_hits = (defender_damage / DICE_SIDES) + (1 < defender_damage % DICE_SIDES ? 1 : 0);
    } else {
      defender_hits = (defender_damage / DICE_SIDES);
    }
  } else {
    defender_hits =
        (defender_damage / DICE_SIDES) +
        (RANDOM_NUMBERS[random_number_index++] % DICE_SIDES < defender_damage % DICE_SIDES ? 1 : 0);
  }
  return defender_hits;
}

void sea_retreat(uint src_sea, uint dst_sea) {
#ifdef DEBUG
  if (state.cache.actually_print) {
    debug_checks();
    printf("DEBUG: retreating to sea: %d\n", dst_sea);
  }
#endif
  for (uint unit_type = TRANSEMPTY; unit_type <= BS_DAMAGED; unit_type++) {
    uint unit_state_count = current_player_sea_unit_types[src_sea][unit_type];
    sea_units_state[dst_sea][unit_type][0] += unit_state_count;
    current_player_sea_unit_types[dst_sea][unit_type] += unit_state_count;
    total_player_sea_units[0][dst_sea] += unit_state_count;
    total_player_sea_units[0][src_sea] -= unit_state_count;
    sea_units_state[src_sea][unit_type][0] = 0;
    sea_units_state[src_sea][unit_type][1] = 0;
    current_player_sea_unit_types[src_sea][unit_type] = 0;
  }
  state.combat_status.at(src_sea + LANDS_COUNT) = CombatStatus::NO_COMBAT;
#ifdef DEBUG
  debug_checks();
#endif
}

uint ask_to_retreat() {
  if (PLAYERS[state.current_turn].is_human) {
    setPrintableStatus();
    std::ostringstream oss;
    oss << printableGameStatus;
    oss << "To where do you want to retreat (255 for no)? ";
    std::string output = oss.str();
    std::cout << output << std::endl;
    return getUserInput();
  }
  return getAIInput();
}
void remove_land_defenders(uint src_land, uint hits) {
  for (uint unit_idx = 0; unit_idx < DEFENDER_LAND_UNIT_TYPES_COUNT; unit_idx++) {
    for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
      uint enemy_player_idx = enemies_0[enemy_idx];
      uint* total_units = &total_player_land_unit_types[enemy_player_idx][src_land]
                                                       [ORDER_OF_LAND_DEFENDERS[unit_idx]];
      if (*total_units > 0) {
#ifdef DEBUG
        if (state.cache.actually_print) {
          std::ostringstream oss;
          oss << "DEBUG: remove_land_defenders: src_land: " << src_land << ", hits: " << hits
              << ", unit_idx: " << ORDER_OF_LAND_DEFENDERS[unit_idx] << ", enemy_idx: " << enemy_idx
              << "\n";
          std::string output = oss.str();
          std::cout << output;
        }
#endif
        if (*total_units < hits) {
          hits -= *total_units;
          total_player_land_units[enemy_player_idx][src_land] -= *total_units;
          enemy_units_count[src_land] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          total_player_land_units[enemy_player_idx][src_land] -= hits;
          enemy_units_count[src_land] -= hits;
          return;
        }
      }
    }
  }
}
void remove_land_attackers(uint src_land, uint hits) {
  uint* total_units = 0;
  for (uint unit_idx = 0; unit_idx < ATTACKER_LAND_UNIT_TYPES_COUNT_1; unit_idx++) {
    uint unit_type = ORDER_OF_LAND_ATTACKERS_1[unit_idx];
    total_units = &land_units_state[src_land][unit_type][0];
    if (*total_units > 0) {
#ifdef DEBUG
      if (state.cache.actually_print) {
        std::ostringstream oss;
        oss << "DEBUG: remove_land_attackers: unit_type: " << unit_type << " src_land: " << src_land
            << ", hits: " << hits << "\n";
        std::string output = oss.str();
        std::cout << output;
      }
#endif
      if (*total_units < hits) {
        hits -= *total_units;
        total_player_land_units[0][src_land] -= *total_units;
        *total_units = 0;
        total_player_land_unit_types[0][src_land][unit_type] = 0;
      } else {
        *total_units -= hits;
        total_player_land_unit_types[0][src_land][unit_type] -= hits;
        total_player_land_units[0][src_land] -= hits;
        hits = 0;
        return;
      }
    }
  }
  for (uint unit_idx = 0; unit_idx < ATTACKER_LAND_UNIT_TYPES_COUNT_2; unit_idx++) {
    uint unit_type = ORDER_OF_LAND_ATTACKERS_2[unit_idx];
    if (total_player_land_unit_types[0][src_land][unit_type] == 0) {
      continue;
    }
    for (uint cur_state = 1; cur_state < STATES_MOVE_LAND[unit_type] - 1; cur_state++) {
      total_units = &land_units_state[src_land][unit_type][cur_state];
      if (*total_units > 0) {
#ifdef DEBUG
        if (state.cache.actually_print) {
          std::ostringstream oss;
          oss << "DEBUG: remove_land_attackers: unit_type: " << unit_type
              << " src_land: " << src_land << ", hits: " << hits << "\n";
          std::cout << oss.str();
        }
#endif

        if (*total_units < hits) {
          hits -= *total_units;
          total_player_land_unit_types[0][src_land][unit_type] -= *total_units;
          total_player_land_units[0][src_land] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          total_player_land_unit_types[0][src_land][unit_type] -= hits;
          total_player_land_units[0][src_land] -= hits;
          hits = 0;
          return;
        }
      }
    }
  }
}
void remove_sea_defenders(uint src_sea, uint hits, bool defender_submerged) {
#ifdef DEBUG
  if (state.cache.actually_print) {
    std::ostringstream oss;
    oss << "DEBUG: remove_sea_defenders: src_sea: " << src_sea << ", hits: " << hits << "\n";
    std::cout << oss.str();
  }
  debug_checks();
#endif
  uint src_air = src_sea + LANDS_COUNT;
  for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
    uint enemy_player_idx = enemies_0[enemy_idx];
    uint* total_battleships = &total_player_sea_unit_types[enemy_player_idx][src_sea][BATTLESHIPS];
    uint* total_bs_damaged = &total_player_sea_unit_types[enemy_player_idx][src_sea][BS_DAMAGED];
    if (*total_battleships > 0) {
#ifdef DEBUG
      if (state.cache.actually_print) {
        std::ostringstream oss;
        oss << "DEBUG: remove_sea_defenders: src_sea: " << src_sea << ", hits: " << hits
            << ", total_battleships: " << *total_battleships << "\n";
        std::cout << oss.str();
      }
#endif
      if (*total_battleships < hits) {
        hits -= *total_battleships;
        *total_bs_damaged += *total_battleships;
        *total_battleships = 0;
#ifdef DEBUG
        debug_checks();
#endif
      } else {
        *total_bs_damaged += hits;
        *total_battleships -= hits;
        hits = 0;
#ifdef DEBUG
        debug_checks();
#endif
        return;
      }
    }
  }
  if (!defender_submerged) {
    for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
      uint enemy_player_idx = enemies_0[enemy_idx];
      uint* total_units = &total_player_sea_unit_types[enemy_player_idx][src_sea][SUBMARINES];
      if (*total_units > 0) {
#ifdef DEBUG
        if (state.cache.actually_print) {
          std::ostringstream oss;
          oss << "DEBUG: remove_sea_defenders subs: src_sea: " << src_sea << ", hits: " << hits
              << ", total_units: " << *total_units << "\n";
          std::cout << oss.str();
        }
#endif
        if (*total_units < hits) {
          hits -= *total_units;
          total_player_sea_units[enemy_player_idx][src_sea] -= *total_units;
          enemy_units_count[src_air] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          total_player_sea_units[enemy_player_idx][src_sea] -= hits;
          enemy_units_count[src_air] -= hits;
          hits = 0;
          return;
        }
      }
    }
  }

  // skipping submarines
  for (uint unit_idx = 1; unit_idx < DEFENDER_SEA_UNIT_TYPES_COUNT; unit_idx++) {
    for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
      uint enemy_player_idx = enemies_0[enemy_idx];
      uint* total_units =
          &total_player_sea_unit_types[enemy_player_idx][src_sea][ORDER_OF_SEA_DEFENDERS[unit_idx]];
      if (*total_units > 0) {
#ifdef DEBUG
        if (state.cache.actually_print) {
          std::ostringstream oss;
          oss << "DEBUG: remove_sea_defenders: unit_type: " << ORDER_OF_SEA_DEFENDERS[unit_idx]
              << " src_sea: " << src_sea << ", hits: " << hits << ", total_units: " << *total_units
              << "\n";
          std::cout << oss.str();
        }
#endif
        if (*total_units < hits) {
          hits -= *total_units;
          total_player_sea_units[enemy_player_idx][src_sea] -= *total_units;
          enemy_units_count[src_air] -= *total_units;
          if (unit_idx >= DESTROYERS && unit_idx <= BS_DAMAGED) {
            enemy_blockade_total[src_sea] -= *total_units;
          }
          *total_units = 0;
        } else {
          *total_units -= hits;
          total_player_sea_units[enemy_player_idx][src_sea] -= hits;
          enemy_units_count[src_air] -= hits;
          if (unit_idx >= DESTROYERS && unit_idx <= BS_DAMAGED) {
            enemy_blockade_total[src_sea] -= hits;
          }
          hits = 0;
          return;
        }
      }
    }
  }
#ifdef DEBUG
  if (state.cache.actually_print) {
    setPrintableStatus();
    std::ostringstream oss;
    oss << printableGameStatus << "\n";
    std::cout << oss.str();
  }
#endif
}

void remove_sea_attackers(uint src_sea, uint hits) {
#ifdef DEBUG
  debug_checks();
#endif
#ifdef DEBUG
  if (state.cache.actually_print) {
    std::ostringstream oss;
    oss << "DEBUG: remove_sea_attackers: src_sea: " << src_sea << ", hits: " << hits << "\n";
    std::cout << oss.str();
  }
#endif
  uint* total_battleships = &sea_units_state[src_sea][BATTLESHIPS][0];
  uint* total_bs_damaged = &sea_units_state[src_sea][BS_DAMAGED][0];
  if (*total_battleships > 0) {
    if (*total_battleships < hits) {
#ifdef DEBUG
      if (state.cache.actually_print) {
        std::ostringstream oss;
        oss << "DEBUG: remove_sea_attackers: src_sea: " << src_sea << ", hits: " << hits
            << ", total_battleships: " << *total_battleships << "\n";
        std::cout << oss.str();
      }
#endif
      hits -= *total_battleships;
      *total_bs_damaged += *total_battleships;
      current_player_sea_unit_types[src_sea][BS_DAMAGED] += *total_battleships;
      *total_battleships = 0;
      current_player_sea_unit_types[src_sea][BATTLESHIPS] = 0;
    } else {
      *total_bs_damaged += hits;
      current_player_sea_unit_types[src_sea][BS_DAMAGED] += hits;
      *total_battleships -= hits;
      current_player_sea_unit_types[src_sea][BATTLESHIPS] -= hits;
      hits = 0;
      return;
    }
  }
  uint* total_units = 0;
  for (uint unit_idx = 0; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_1; unit_idx++) {
    uint unit_type = ORDER_OF_SEA_ATTACKERS_1[unit_idx];
    uint* total_units = &sea_units_state[src_sea][unit_type][0];
    if (*total_units > 0) {
#ifdef DEBUG
      if (state.cache.actually_print) {
        printf("DEBUG: remove_sea_attackers: unit_type: %d src_sea: %d, hits: %d\n", unit_type,
               src_sea, hits);
      }
#endif
      if (*total_units < hits) {
        hits -= *total_units;
        total_player_sea_units[0][src_sea] -= *total_units;
        *total_units = 0;
        current_player_sea_unit_types[src_sea][unit_type] = 0;
      } else {
        *total_units -= hits;
        total_player_sea_units[0][src_sea] -= hits;
        current_player_sea_unit_types[src_sea][unit_type] -= hits;
        hits = 0;
        return;
      }
    }
  }
  total_units = &sea_units_state[src_sea][CARRIERS][0];
  if (*total_units > 0) {
    if (*total_units < hits) {
#ifdef DEBUG
      if (state.cache.actually_print) {
        printf("DEBUG: remove_sea_attackers carriers: unit_type: %d src_sea: %d, hits: %d\n",
               CARRIERS, src_sea, hits);
      }
#endif
      hits -= *total_units;
      total_player_sea_units[0][src_sea] -= *total_units;
      *total_units = 0;
      current_player_sea_unit_types[src_sea][CARRIERS] = 0;
      allied_carriers[src_sea] = 0;
    } else {
      *total_units -= hits;
      current_player_sea_unit_types[src_sea][CARRIERS] -= hits;
      allied_carriers[src_sea] -= hits;
      total_player_sea_units[0][src_sea] -= hits;
      hits = 0;
      return; // todo: do allied fighters have a chance to land?
    }
  }
  total_units = &sea_units_state[src_sea][CRUISERS][0];
  if (*total_units > 0) {
    if (*total_units < hits) {
#ifdef DEBUG
      if (state.cache.actually_print) {
        printf("DEBUG: remove_sea_attackers cruisers: unit_type: %d src_sea: %d, hits: %d\n",
               CRUISERS, src_sea, hits);
      }
#endif
      hits -= *total_units;
      total_player_sea_units[0][src_sea] -= *total_units;
      *total_units = 0;
      current_player_sea_unit_types[src_sea][CRUISERS] = 0;
    } else {
      *total_units -= hits;
      current_player_sea_unit_types[src_sea][CRUISERS] -= hits;
      total_player_sea_units[0][src_sea] -= hits;
      hits = 0;
      return;
    }
  }
  for (uint unit_idx = 0; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_2; unit_idx++) {
    uint unit_type = ORDER_OF_SEA_ATTACKERS_2[unit_idx];
    if (total_player_sea_unit_types[0][src_sea][unit_type] == 0)
      continue;
    for (uint cur_state = 0; cur_state < STATES_MOVE_SEA[unit_type]; cur_state++) {
      total_units = &sea_units_state[src_sea][unit_type][cur_state];
      if (*total_units > 0) {
#ifdef DEBUG
        if (state.cache.actually_print) {
          printf("DEBUG: remove_sea_attackers: unit_type: %d src_sea: %d, hits: %d\n", unit_type,
                 src_sea, hits);
        }
#endif
        if (*total_units < hits) {
          hits -= *total_units;
          total_player_sea_units[0][src_sea] -= *total_units;
          current_player_sea_unit_types[src_sea][unit_type] -= *total_units;
          *total_units = 0;
        } else {
          *total_units -= hits;
          current_player_sea_unit_types[src_sea][unit_type] -= hits;
          total_player_sea_units[0][src_sea] -= hits;
          hits = 0;
          return;
        }
      }
    }
  }
  for (uint unit_idx = 0; unit_idx < ATTACKER_SEA_UNIT_TYPES_COUNT_3; unit_idx++) {
    uint unit_type = ORDER_OF_SEA_ATTACKERS_3[unit_idx];
    uint* total_units = &sea_units_state[src_sea][unit_type][0];
    if (*total_units > 0) {
#ifdef DEBUG
      if (state.cache.actually_print) {
        printf("DEBUG: remove_sea_attackers: unit_type: %d src_sea: %d, hits: %d\n", unit_type,
               src_sea, hits);
      }
#endif
      if (*total_units < hits) {
        hits -= *total_units;
        total_player_sea_units[0][src_sea] -= *total_units;
        if (unit_type <= TRANS1T) {
          transports_with_small_cargo_space[src_sea] -= *total_units;
          if (unit_type <= TRANS1I) {
            transports_with_large_cargo_space[src_sea] -= *total_units;
          }
        }
        *total_units = 0;
        current_player_sea_unit_types[src_sea][unit_type] = 0;
      } else {
        *total_units -= hits;
        current_player_sea_unit_types[src_sea][unit_type] -= hits;
        total_player_sea_units[0][src_sea] -= hits;
        if (unit_type <= TRANS1T) {
          transports_with_small_cargo_space[src_sea] -= hits;
          if (unit_type <= TRANS1I) {
            transports_with_large_cargo_space[src_sea] -= hits;
          }
        }
        hits = 0;
        return;
      }
    }
  }
#ifdef DEBUG
  debug_checks();
#endif
}

bool unload_transports() {
  bool units_to_process = false;
  for (uint unit_type = TRANS1I; unit_type <= TRANS1I1T; unit_type++) {
    uint unloading_state = STATES_UNLOADING[unit_type];
    uint unload_cargo1 = UNLOAD_CARGO1[unit_type];
    uint unload_cargo2 = UNLOAD_CARGO2[unit_type];
    //    clear_move_history();
    for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      uint* total_units = &sea_units_state[src_sea][unit_type][unloading_state];
      if (*total_units == 0) {
        continue;
      }
      units_to_process = true;
      uint src_air = src_sea + LANDS_COUNT;
      valid_moves[0] = src_air;
      valid_moves_count = 1;
      add_valid_unload_moves(src_sea);
      while (*total_units > 0) {
        uint dst_air = valid_moves[0];
        if (valid_moves_count > 1) {
          if (answers_remaining == 0) {
            return true;
          }
          dst_air = get_user_move_input(unit_type, src_air);
        }
#ifdef DEBUG
        if (state.cache.actually_print) {
          std::ostringstream oss;
          oss << "DEBUG: unload_transports unit_type: " << unit_type << ", src_sea: " << src_sea
              << ", dst_air: " << dst_air << ", unload_cargo: " << unload_cargo1 << ", "
              << unload_cargo2 << "\n";
          setPrintableStatus();
          oss << printableGameStatus << "\n";
          std::cout << oss.str();
        }
#endif
        // update_move_history(dst_air, src_sea);
        if (src_air == dst_air) {
          sea_units_state[src_sea][unit_type][0] += *total_units;
          *total_units = 0;
          continue;
        }
        (*bombard_max[dst_air])++;
        land_units_state[dst_air][unload_cargo1][0]++;
        current_player_land_unit_types[dst_air][unload_cargo1]++;
        total_player_land_units[0][dst_air]++;
        sea_units_state[src_sea][TRANSEMPTY][0]++;
        current_player_sea_unit_types[src_sea][TRANSEMPTY]++;
        current_player_sea_unit_types[src_sea][unit_type]--;
        *total_units -= 1;
        if (unit_type > TRANS1T) {
          (*bombard_max[dst_air])++;
          land_units_state[dst_air][unload_cargo2][0]++;
          current_player_land_unit_types[dst_air][unload_cargo2]++;
          total_player_land_units[0][dst_air]++;
        }
        if (!is_allied_0[*owner_idx[dst_air]]) {
          state.combat_status[dst_air] = CombatStatus::PRE_COMBAT;
          if (enemy_units_count[dst_air] == 0) {
            conquer_land(dst_air);
          }
        }
#ifdef DEBUG
        debug_checks();
#endif
      }
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}
constexpr uint MAX_COMBAT_ROUNDS = 100;
bool resolve_land_battles() {
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    // check if battle is over
    if (state.combat_status.at(src_land) == CombatStatus::NO_COMBAT) {
      continue;
    }
#ifdef DEBUG
    if (state.cache.actually_print) {
      std::cout << "Resolve land combat in: " << src_land << ", Name: " << LANDS[src_land].name
                << std::endl;
      setPrintableStatus();
      std::cout << printableGameStatus << std::endl;
    }
#endif
    Landunittypes other_land_units_0_src_land = current_player_land_unit_types[src_land];
    uint* units_land_player_total_0_src_land = &total_player_land_units[0][src_land];
    uint attacker_damage = 0;
    uint attacker_hits = 0;
    // check if no friendlies remain
    if (*units_land_player_total_0_src_land == 0) {
#ifdef DEBUG
      if (state.cache.actually_print) {
        printf("No friendlies remain");
      }
#endif
      continue;
    }
    if (state.combat_status[src_land] == CombatStatus::PRE_COMBAT) {

      // only bombers exist
      Landunittypes other_land_units_ptr_0_src_land = total_player_land_unit_types[0][src_land];
      uint* bombers_count = &other_land_units_ptr_0_src_land[BOMBERS_LAND_AIR];
      if (*bombers_count > 0 && total_player_land_units[0][src_land] == *bombers_count) {
        if (*factory_dmg[src_land] < *factory_max[src_land] * 2) {
#ifdef DEBUG
          if (state.cache.actually_print) {
            printf("Strategic Bombing");
          }
#endif
          // fire_strat_aaguns();
          uint defender_damage = *bombers_count;
          uint defender_hits = get_defender_hits(defender_damage);
          if (defender_hits > 0) {
            uint* units_land_ptr_src_land_bombers = land_units_state[src_land][BOMBERS_LAND_AIR];
            for (uint cur_state = 1; cur_state < BOMBER_LAND_STATES - 1; cur_state++) {
              uint* total_units = &units_land_ptr_src_land_bombers[cur_state];
              if (*total_units < defender_hits) {
                defender_hits -= *total_units;
                *bombers_count -= *total_units;
                *units_land_player_total_0_src_land -= *total_units;
                *total_units = 0;
              } else {
                *total_units -= defender_hits;
                *bombers_count -= defender_hits;
                *units_land_player_total_0_src_land -= defender_hits;
                defender_hits = 0;
                continue;
              }
            }
          }
          attacker_damage = *bombers_count * 21;
          attacker_hits = get_attacker_hits(attacker_damage);
        }
        *factory_dmg[src_land] =
            std::max(*factory_dmg[src_land] + attacker_hits, *factory_max[src_land] * 2);
        continue;
      }

#ifdef DEBUG
      if (state.cache.actually_print) {
        printf("Normal Land Combat\n");
      }
#endif

      // bombard_shores
      if (*bombard_max[src_land] > 0) {
        attacker_damage = 0;
#ifdef DEBUG
        if (state.cache.actually_print) {
          printf("Sea Bombardment\n");
        }
#endif
        for (uint unit_type1 = 0; unit_type1 <= BS_DAMAGED - CRUISERS; unit_type1++) {
          uint unit_type = BS_DAMAGED - unit_type1;
          for (uint sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[src_land]; sea_idx++) {
            uint src_sea = LAND_TO_SEA_CONN[src_land][sea_idx];
            uint* total_bombard_ships = sea_units_state[src_sea][unit_type];
            while (total_bombard_ships[1] > 0 && *bombard_max[src_land] > 0) {
              attacker_damage += ATTACK_UNIT_SEA[unit_type];
              total_bombard_ships[0]++;
              total_bombard_ships[1]--;
              (*bombard_max[src_land])--;
            }
          }
        }
        *bombard_max[src_land] = 0;
        attacker_hits = get_attacker_hits(attacker_damage);
        if (attacker_hits > 0) {
          remove_land_defenders(src_land, attacker_hits);
        }
      }
      // check if can fire tactical aaguns
      uint total_air_units = other_land_units_ptr_0_src_land[FIGHTERS] +
                             other_land_units_ptr_0_src_land[BOMBERS_LAND_AIR];
      uint defender_damage;
      uint defender_hits;
      if (total_air_units > 0) {
        uint total_aaguns = 0;
        for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
          total_aaguns += total_player_land_unit_types[enemies_0[enemy_idx]][src_land][AAGUNS];
        }
        if (total_aaguns > 0) {
#ifdef DEBUG
          if (state.cache.actually_print) {
            printf("Firing AA");
          }
#endif
          // fire_tact_aaguns();
          defender_damage = total_air_units * 3;
          defender_hits = get_defender_hits(defender_damage);
          if (defender_hits > 0) {
            for (uint cur_state = 0; cur_state < FIGHTER_STATES; cur_state++) {
              uint* total_units = &land_units_state[src_land][FIGHTERS][cur_state];
              if (*total_units < defender_hits) {
                defender_hits -= *total_units;
                other_land_units_ptr_0_src_land[FIGHTERS] -= *total_units;
                *units_land_player_total_0_src_land -= *total_units;
                *total_units = 0;
              } else {
                *total_units -= defender_hits;
                other_land_units_ptr_0_src_land[FIGHTERS] -= defender_hits;
                *units_land_player_total_0_src_land -= defender_hits;
                defender_hits = 0;
                break;
              }
            }
          }
          if (defender_hits > 0) {
            for (uint cur_state = 0; cur_state < BOMBER_LAND_STATES; cur_state++) {
              uint* total_units = &land_units_state[src_land][BOMBERS_LAND_AIR][cur_state];
              if (*total_units < defender_hits) {
                defender_hits -= *total_units;
                other_land_units_ptr_0_src_land[BOMBERS_LAND_AIR] -= *total_units;
                *units_land_player_total_0_src_land -= *total_units;
                *total_units = 0;
              } else {
                *total_units -= defender_hits;
                other_land_units_ptr_0_src_land[BOMBERS_LAND_AIR] -= defender_hits;
                *units_land_player_total_0_src_land -= defender_hits;
                defender_hits = 0;
                break;
              }
            }
          }
#ifdef DEBUG
          debug_checks();
#endif
        }
      }
      if (*units_land_player_total_0_src_land == 0) {
#ifdef DEBUG
        if (state.cache.actually_print) {
          std::cout << "No friendlies remain";
        }
#endif
        continue;
      }
      if (enemy_units_count[src_land] == 0) {
        // if infantry, artillery, tanks exist then capture
        if (other_land_units_0_src_land[INFANTRY] + other_land_units_0_src_land[ARTILLERY] +
                other_land_units_0_src_land[TANKS] >
            0) {
          conquer_land(src_land);
        }
        continue;
      }
    }
    uint combat_rounds = 0;
    while (true) {
      combat_rounds++;
      // print land location name
#ifdef DEBUG
      if (state.cache.actually_print) {
        std::cout << "Current land battle start src_land: " << src_land
                  << ", Name: " << LANDS[src_land].name << std::endl;
        setPrintableStatus();
        std::cout << printableGameStatus << std::endl;
      }
#endif
      if (*units_land_player_total_0_src_land == 0) {
#ifdef DEBUG
        if (state.cache.actually_print) {
          printf("No friendlies remain");
        }
#endif
        break;
      }

      if (state.combat_status[src_land] == CombatStatus::MID_COMBAT) {
        // ask to retreat (0-255, any non valid retreat zone is considered a no)
        valid_moves[0] = src_land;
        valid_moves_count = 1;
        L2LConn land_to_land_conn = LAND_TO_LAND_CONN[src_land];
        uint land_to_land_count = LAND_TO_LAND_COUNT[src_land];
        for (uint land_conn_idx = 0; land_conn_idx < land_to_land_count; land_conn_idx++) {
          uint land_dst = land_to_land_conn[land_conn_idx];
          if (enemy_units_count[land_dst] == 0 &&
              state.combat_status[land_dst] == CombatStatus::NO_COMBAT &&
              is_allied_0[*owner_idx[land_dst]])
            valid_moves[valid_moves_count++] = land_dst;
        }
        uint dst_air;
        if (valid_moves_count == 1) {
          dst_air = valid_moves[0];
        } else {
          if (answers_remaining == 0)
            return true;
          dst_air = ask_to_retreat();
        }
        // if retreat, move units to retreat zone immediately and end battle
        // max 100 combat rounds for fighters facing aa guns with unlucky dice
        if (src_land != dst_air || combat_rounds > MAX_COMBAT_ROUNDS) {
#ifdef DEBUG
          if (state.cache.actually_print) {
            printf("Retreating land_battle from: %d to: %d\n", src_land, dst_air);
          }
#endif
          for (uint unit_type = INFANTRY; unit_type <= TANKS; unit_type++) {
            uint total_units = land_units_state[src_land][unit_type][0];
            land_units_state[dst_air][unit_type][0] += total_units;
            current_player_land_unit_types[dst_air][unit_type] += total_units;
            total_player_land_units[0][dst_air] += total_units;
            current_player_land_unit_types[src_land][unit_type] -= total_units;
            *units_land_player_total_0_src_land -= total_units;
            land_units_state[src_land][unit_type][0] = 0;
          }
          state.combat_status[src_land] = CombatStatus::NO_COMBAT;
#ifdef DEBUG
          debug_checks();
#endif
          break;
        }
      }
      state.combat_status[src_land] = CombatStatus::MID_COMBAT;
      // land_battle
      uint infantry_count = current_player_land_unit_types[src_land][INFANTRY];
      uint artillery_count = current_player_land_unit_types[src_land][ARTILLERY];
      attacker_damage =
          (current_player_land_unit_types[src_land][FIGHTERS] * FIGHTER_ATTACK) +
          (current_player_land_unit_types[src_land][BOMBERS_LAND_AIR] * BOMBER_ATTACK) +
          (infantry_count * INFANTRY_ATTACK) + (artillery_count * ARTILLERY_ATTACK) +
          (current_player_land_unit_types[src_land][TANKS] * TANK_ATTACK);
      // add damage for the minimum of count of infantry/artillery
      attacker_damage += infantry_count < artillery_count ? infantry_count : artillery_count;
      attacker_hits = get_attacker_hits(attacker_damage);
      uint defender_damage = 0;
      uint defender_hits = 0;
#ifdef DEBUG
      if (state.cache.actually_print) {
        printf("Enemy Count: %d\n", enemy_units_count[src_land]);
      }
#endif
      for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
        Landunittypes land_units = total_player_land_unit_types[enemies_0[enemy_idx]][src_land];
        defender_damage += (land_units[INFANTRY] * INFANTRY_DEFENSE) +
                           (land_units[ARTILLERY] * ARTILLERY_DEFENSE) +
                           (land_units[TANKS] * TANK_DEFENSE) +
                           (land_units[FIGHTERS] * FIGHTER_DEFENSE) +
                           (land_units[BOMBERS_LAND_AIR] * BOMBER_DEFENSE);
        defender_hits = get_defender_hits(defender_damage);
      }
      if (defender_hits > 0) {
#ifdef DEBUG
        if (state.cache.actually_print) {
          printf("Defender Hits: %d", defender_hits);
        }
#endif
        remove_land_attackers(src_land, defender_hits);
#ifdef DEBUG
        debug_checks();
#endif
      }
      if (attacker_hits > 0) {
#ifdef DEBUG
        if (state.cache.actually_print) {
          printf("Attacker Hits: %d", attacker_hits);
        }
#endif
        remove_land_defenders(src_land, attacker_hits);
#ifdef DEBUG
        debug_checks();
#endif
      }

      if (*units_land_player_total_0_src_land == 0) {
#ifdef DEBUG
        if (state.cache.actually_print) {
          printf("No friendlies remain");
        }
#endif
        break;
      }
      if (enemy_units_count[src_land] == 0) {
        // if infantry, artillery, tanks exist then capture
        if (other_land_units_0_src_land[INFANTRY] + other_land_units_0_src_land[ARTILLERY] +
                other_land_units_0_src_land[TANKS] >
            0) {
          conquer_land(src_land);
        }
        break;
      }
    }
  }
  return false;
}

void add_valid_unload_moves(uint src_sea) {
  S2LConn near_land = SEA_TO_LAND_CONN[src_sea];
  uint near_land_count = SEA_TO_LAND_COUNT[src_sea];
  for (uint land_idx = 0; land_idx < near_land_count; land_idx++) {
    uint dst_land = near_land[land_idx];
    // add_valid_air_move_if_history_allows_X(dst_land, src_sea + LANDS_COUNT, 1);
    if (!state.skipped_moves[src_sea + LANDS_COUNT][dst_land].bit) {
      valid_moves[valid_moves_count++] = dst_land;
    }
  }
}

void add_valid_fighter_landing(uint src_air, uint remaining_moves) {
  AirArray near_air = AIR_WITHIN_X_MOVES[remaining_moves - 1][src_air];
  uint near_air_count = AIR_WITHIN_X_MOVES_COUNT[remaining_moves - 1][src_air];
  for (uint i = 0; i < near_air_count; i++) {
    uint dst_air = near_air[i];
    if (canFighterLandHere[dst_air]) {
      // add_valid_air_move_if_history_allows_X(dst_air, src_air, remaining_moves);
      if (!state.skipped_moves[src_air][dst_air].bit) {
        valid_moves[valid_moves_count++] = dst_air;
      }
    }
  }
}

void refresh_can_fighter_land_here() {
  LandArray& land_owners = state.land_owners;
  LandArray& factory_max = state.factory_max;
  //  refresh_canFighterLandHere_final
  FILL_ARRAY(canFighterLandHere, 0);
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    // if (allied_carriers[sea_idx] > 0) {
    canFighterLandHere[sea_idx + LANDS_COUNT] = allied_carriers[sea_idx] > 0;
    //}
  }
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    uint land_owner = land_owners[land_idx];
    // is allied owned and not recently conquered?
    canFighterLandHere[land_idx] =
        is_allied_0[land_owner] && state.combat_status[land_idx] == CombatStatus::NO_COMBAT;
    // check for possiblity to build carrier under fighter
    if (factory_max[land_idx] > 0 && land_owner == state.current_turn) {
      uint land_to_sea_count = LAND_TO_SEA_COUNT[land_idx];
      for (uint conn_idx = 0; conn_idx < land_to_sea_count; conn_idx++) {
        canFighterLandHere[LANDS_COUNT + LAND_TO_SEA_CONN[land_idx][conn_idx]] = true;
      }
    }
  }
}

bool land_fighter_units() {
  bool units_to_process = false;
  //  check if any fighters have moves remaining
  for (uint cur_state = 1; cur_state < FIGHTER_STATES - 1;
       cur_state++) { // todo optimize to find next fighter faster
    for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
      uint* total_fighter_count = &air_units_state[src_air][FIGHTERS][cur_state];
      if (*total_fighter_count == 0) {
        continue;
      }
      if (!units_to_process) {
        units_to_process = true;
        refresh_can_fighter_land_here();
      }
      // valid_moves[0] = src_air;
      valid_moves_count = 0;
      add_valid_fighter_landing(src_air, cur_state);
      if (valid_moves_count == 0 || canFighterLandHere[src_air]) {
        valid_moves[valid_moves_count++] = src_air;
      }
      while (*total_fighter_count > 0) {
        uint dst_air = valid_moves[0];
        if (valid_moves_count > 1) {
          if (answers_remaining == 0)
            return true;
          dst_air = get_user_move_input(FIGHTERS, src_air);
        }
#ifdef DEBUG
        if (state.cache.actually_print) {
          setPrintableStatus();
          std::cout << printableGameStatus << std::endl;
          std::cout << "DEBUG: player: " << PLAYERS[state.current_turn].name << " landing fighters "
                    << FIGHTERS << ", src_air: " << src_air << ", dst_air: " << dst_air
                    << std::endl;
        }
#endif
        // update_move_history(dst_air, src_air);
        if (src_air == dst_air) {
          air_units_state[src_air][FIGHTERS][0]++;
          *total_fighter_count -= 1;
          continue;
        }
        air_units_state[dst_air][FIGHTERS][0]++;
        if (dst_air < LANDS_COUNT) {
          total_player_land_units[0][dst_air]++;
          current_player_land_unit_types[dst_air][FIGHTERS]++;
        } else {
          uint dst_sea = dst_air - LANDS_COUNT;
          total_player_sea_units[0][dst_sea]++;
          current_player_sea_unit_types[dst_sea][FIGHTERS]++;
        }
        if (src_air < LANDS_COUNT) {
          total_player_land_units[0][src_air]--;
          current_player_land_unit_types[src_air][FIGHTERS]--;
        } else {
          uint src_sea = src_air - LANDS_COUNT;
          total_player_sea_units[0][src_sea]--;
          current_player_sea_unit_types[src_sea][FIGHTERS]--;
        }
        *total_fighter_count -= 1;
#ifdef DEBUG
        debug_checks();
#endif
      }
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}

void add_valid_bomber_landing(uint src_air, uint movement_remaining) {
#ifdef DEBUG
  if (state.cache.actually_print) {
    printf("movement_remaining: %d\n", movement_remaining);
  }
#endif
  LandArray near_land = AIR_TO_LAND_WITHIN_X_MOVES[movement_remaining - 1][src_air];
  for (uint i = 0; i < AIR_TO_LAND_WITHIN_X_MOVES_COUNT[movement_remaining - 1][src_air]; i++) {
    uint dst_air = near_land[i];
    if (canBomberLandHere[dst_air]) {
#ifdef DEBUG
      if (state.cache.actually_print) {
        printf("Adding valid move: %d\n", dst_air);
      }
#endif
      valid_moves[valid_moves_count++] = dst_air;
    }
  }
}
void refresh_can_bomber_land_here() {
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    // is allied owned and not recently conquered?
    canBomberLandHere[land_idx] = is_allied_0[*owner_idx[land_idx]] &&
                                  state.combat_status[land_idx] == CombatStatus::NO_COMBAT;
  }
}

bool land_bomber_units() {
  bool units_to_process = false;
#ifdef DEBUG
  if (state.cache.actually_print) {
    printf("Landing Bombers\n");
  }
#endif
  // check if any bombers have moves remaining
  for (uint cur_state1 = 0; cur_state1 < BOMBER_LAND_STATES - 2;
       cur_state1++) { // todo optimize to find next bomber faster
                       //    clear_move_history();
    for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
      uint cur_state = (src_air < LANDS_COUNT ? cur_state1 + 1 : cur_state1);
      uint* total_bomber_count = &air_units_state[src_air][BOMBERS_LAND_AIR][cur_state];
      if (*total_bomber_count == 0) {
        continue;
      }
      if (!units_to_process) {
        units_to_process = true;
        refresh_can_bomber_land_here();
      }
#ifdef DEBUG
      if (state.cache.actually_print) {
        printf("Bomber Count: %d with state %d, in location %d\n", *total_bomber_count, cur_state,
               src_air);
      }
#endif
      // valid_moves[0] = src_air;
      valid_moves_count = 0;
      uint movement_remaining = cur_state + (src_air < LANDS_COUNT ? 0 : 1);
      add_valid_bomber_landing(src_air, movement_remaining);
      while (*total_bomber_count > 0) {
        units_to_process = true;
        if (valid_moves_count == 0) {
          valid_moves[valid_moves_count++] = src_air;
        }
        uint dst_air = valid_moves[0];
        if (valid_moves_count > 1) {
          if (answers_remaining == 0) {
            return true;
          }
          dst_air = get_user_move_input(BOMBERS_LAND_AIR, src_air);
        }
#ifdef DEBUG
        if (state.cache.actually_print) {
          setPrintableStatus();
          std::cout << printableGameStatus << std::endl;
          std::cout << "valid_moves_count: " << valid_moves_count << std::endl;
          std::cout << "valid_moves: ";
          for (uint i = 0; i < valid_moves_count; i++) {
            std::cout << valid_moves[i] << " ";
          }
          std::cout << std::endl;
          std::cout << "DEBUG: player: " << PLAYERS[state.current_turn].name << " landing bombers "
                    << BOMBERS_LAND_AIR << ", src_air: " << src_air << ", dst_air: " << dst_air
                    << std::endl;
        }
#endif
        // update_move_history(dst_air, src_air);
        if (src_air == dst_air) {
          air_units_state[src_air][BOMBERS_LAND_AIR][0]++;
          *total_bomber_count -= 1;
#ifdef DEBUG
          debug_checks();
#endif
          continue;
        }
        air_units_state[dst_air][BOMBERS_LAND_AIR][0]++;
        total_player_land_units[0][dst_air]++;
        current_player_land_unit_types[dst_air][BOMBERS_LAND_AIR]++;
        if (src_air < LANDS_COUNT) {
          total_player_land_units[0][src_air]--;
          current_player_land_unit_types[src_air][BOMBERS_LAND_AIR]--;
        } else {
          uint src_sea = src_air - LANDS_COUNT;
          total_player_sea_units[0][src_sea]--;
          current_player_sea_unit_types[src_sea][BOMBERS_LAND_AIR]--;
        }
        *total_bomber_count -= 1;
#ifdef DEBUG
        debug_checks();
#endif
      }
#ifdef DEBUG
      debug_checks();
#endif
    }
  }
  if (units_to_process) {
    clear_move_history();
  }
  return false;
}
bool end_turn() {
  valid_moves_count = 1;
  valid_moves[0] = MAX_INT;
  if (answers_remaining == 0) {
    return true;
  }
  (void)getAIInput();
  return false;
}
bool buy_units() {
  bool units_to_process = false;
#ifdef DEBUG
  if (state.cache.actually_print) {
    setPrintableStatus();
    std::cout << printableGameStatus << "\nBuying Units\n" << std::endl;
  }
#endif
  for (uint factory_idx = 0; factory_idx < total_factory_count[0]; factory_idx++) {
    uint dst_land = factory_locations[0][factory_idx];
#ifdef DEBUG
    if (*owner_idx[dst_land] != 0) {
      if (state.cache.actually_print) {
        printf("DEBUG: player: %s cannot buy units at %s\n", PLAYERS[state.current_turn].name,
               LANDS[dst_land].name);
      }
      cause_breakpoint();
    }
#endif
    if (state.builds_left.at(dst_land) == 0) {
      continue;
    }
    uint repair_cost = 0;
    // buy sea units
    for (uint sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[dst_land]; sea_idx++) {
      units_to_process = false;
      uint dst_sea = LAND_TO_SEA_CONN[dst_land][sea_idx];
      uint dst_air = dst_sea + LANDS_COUNT;
      valid_moves[0] = SEA_UNIT_TYPES_COUNT; // pass all units
      uint last_purchased = 0;
      while (state.builds_left.at(dst_air) > 0) {
        if (state.money[0] < TRANSPORT_COST) {
          state.builds_left.at(dst_air) = 0;
          break;
        }
        uint units_built = *factory_max[dst_land] - state.builds_left.at(dst_land);
        if (*factory_max[dst_land] < 1 + units_built + *factory_dmg[dst_land]) {
          repair_cost = 1 + units_built + *factory_dmg[dst_land] - *factory_max[dst_land];
        }
        // add all units that can be bought
        valid_moves_count = 1;
        for (uint unit_type_idx1 = 0; unit_type_idx1 <= COST_UNIT_SEA_COUNT - 1; unit_type_idx1++) {
          uint unit_type_idx = COST_UNIT_SEA_COUNT - 1 - unit_type_idx1;
          uint unit_type = BUY_UNIT_SEA[unit_type_idx];
          // if (unit_type < last_purchased) //state.skipped_moves
          //   break;
          if (state.skipped_moves[0][unit_type].bit) {
            last_purchased = unit_type;
            break;
          }
          if (state.money[0] < COST_UNIT_SEA[unit_type] + repair_cost) {
            continue;
          }
          if (unit_type == FIGHTERS) {
            uint total_fighters = 0;
            for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
              total_fighters += total_player_sea_unit_types[player_idx][dst_sea][FIGHTERS];
            }
            if (allied_carriers[dst_sea] * 2 <= total_fighters) {
              continue;
            }
          }
          valid_moves[valid_moves_count++] = unit_type;
        }
        if (valid_moves_count == 1) {
          state.builds_left.at(dst_air) = 0;
          break;
        }
        if (answers_remaining == 0) {
          return true;
        }
        units_to_process = true;
        uint purchase = get_user_purchase_input(dst_air);
        if (purchase == SEA_UNIT_TYPES_COUNT) { // pass all units
          state.builds_left[dst_air] = 0;
          break;
        }
#ifdef DEBUG
        if (state.cache.actually_print) {
          // print which player is buying which unit at which location
          printf("Player %d buying %s at %s\n", state.current_turn, NAMES_UNIT_SEA[purchase],
                 SEAS[dst_sea].name);
        }
#endif
        for (uint sea_idx2 = sea_idx; sea_idx2 < LAND_TO_SEA_COUNT[dst_land]; sea_idx2++) {
          state.builds_left.at(LAND_TO_SEA_CONN[dst_land][sea_idx2] + LANDS_COUNT)--;
        }
        state.builds_left.at(dst_land)--;
        *factory_dmg[dst_land] -= repair_cost;
        state.money[0] -= COST_UNIT_SEA[purchase] + repair_cost;
        sea_units_state[dst_sea][purchase][0]++;
        total_player_sea_units[0][dst_sea]++;
        current_player_sea_unit_types[dst_sea][purchase]++;
        if (purchase > last_purchased) {
          for (uint unit_type2 = last_purchased; unit_type2 < purchase; unit_type2++) {
            state.skipped_moves[0][unit_type2].bit = true;
          }

          last_purchased = purchase;
        }
      }
      if (units_to_process) {
        clear_move_history();
      }
    }
    // buy land units
    valid_moves[0] = LAND_UNIT_TYPES_COUNT; // pass all units
    uint last_purchased = 0;
    units_to_process = false;
    while (state.builds_left.at(dst_land) > 0) {
      if (state.money[0] < INFANTRY_COST) {
        state.builds_left.at(dst_land) = 0;
        break;
      }
      uint units_built = *factory_max[dst_land] - state.builds_left.at(dst_land);
      if (*factory_max[dst_land] < 1 + units_built + *factory_dmg[dst_land]) {
        repair_cost = 1 + units_built + *factory_dmg[dst_land] - *factory_max[dst_land];
      }
      // add all units that can be bought
      valid_moves_count = 1;
      for (uint unit_type1 = 0; unit_type1 <= LAND_UNIT_TYPES_COUNT - 1; unit_type1++) {
        uint unit_type = LAND_UNIT_TYPES_COUNT - 1 - unit_type1;
        // if (unit_type < last_purchased)
        //   break;
        if (state.skipped_moves[0][unit_type].bit) {
          last_purchased = unit_type;
          break;
        }
        if (state.money[0] < COST_UNIT_LAND[unit_type] + repair_cost) {
          continue;
        }
        valid_moves[valid_moves_count++] = unit_type;
      }
      if (valid_moves_count == 1) {
        state.builds_left.at(dst_land) = 0;
        break;
      }
      if (answers_remaining == 0) {
        return true;
      }
      units_to_process = true;
      uint purchase = get_user_purchase_input(dst_land);
      if (purchase == LAND_UNIT_TYPES_COUNT) { // pass all units
        state.builds_left.at(dst_land) = 0;
        break;
      }
#ifdef DEBUG
      if (state.cache.actually_print) {
        // print which player is buying which unit at which location
        printf("Player %d buying %s at %s\n", state.current_turn, NAMES_UNIT_LAND[purchase],
               LANDS[dst_land].name);
      }
#endif
      state.builds_left.at(dst_land)--;
      *factory_dmg[dst_land] -= repair_cost;
      state.money[0] -= COST_UNIT_LAND[purchase] + repair_cost;
      land_units_state[dst_land][purchase][0]++;
      total_player_land_units[0][dst_land]++;
      total_player_land_unit_types[0][dst_land][purchase]++;
      if (purchase > last_purchased) {
        for (uint unit_type2 = last_purchased; unit_type2 < purchase; unit_type2++) {
          state.skipped_moves[0][unit_type2].bit = true;
        }
      }
      last_purchased = purchase;
    }
    if (units_to_process) {
      clear_move_history();
    }
  }
#ifdef DEBUG
  if (state.cache.actually_print) {
    setPrintableStatus();
    std::cout << printableGameStatus << std::endl;
  }
#endif
  return false;
}

void crash_air_units() {
  // crash planes not on friendly land
  refresh_can_planes_land_here();
  for (uint air_idx = 0; air_idx < LANDS_COUNT; air_idx++) {
    if (canFighterLandHere[air_idx]) {
      continue;
    }
    if (current_player_land_unit_types[air_idx][FIGHTERS] == 0) {
      continue;
    }
#ifdef DEBUG
    if (state.cache.actually_print) {
      printf("DEBUG: Crashing fighters at %d\n", air_idx);
    }
#endif
    total_player_land_units[0][air_idx] -= current_player_land_unit_types[air_idx][FIGHTERS];
    current_player_land_unit_types[air_idx][FIGHTERS] = 0;
    air_units_state[air_idx][FIGHTERS][0] = 0;
  }
  // crash planes not on allied carriers
  for (uint air_idx = LANDS_COUNT; air_idx < AIRS_COUNT; air_idx++) {
    // units_air_ptr[air_idx][BOMBERS_LAND_AIR][0] = 0;
    uint sea_idx = air_idx - LANDS_COUNT;
    uint free_space = allied_carriers[sea_idx] * 2;
    for (uint player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      free_space -= total_player_sea_unit_types[player_idx][sea_idx][FIGHTERS];
    }
    uint* total_fighter_count = &air_units_state[air_idx][FIGHTERS][0];
    if (free_space < *total_fighter_count) {
      uint fighters_lost = *total_fighter_count - free_space;
#ifdef DEBUG
      if (state.cache.actually_print) {
        printf("DEBUG: Crashing %d fighters at %d\n", fighters_lost, air_idx);
      }
#endif
      total_player_sea_units[0][sea_idx] -= fighters_lost;
      current_player_sea_unit_types[sea_idx][FIGHTERS] -= fighters_lost;
      *total_fighter_count = free_space;
    }
  }
}
void reset_units_fully() {
  // reset battleship health
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    sea_units_state[sea_idx][BATTLESHIPS][0] += current_player_sea_unit_types[sea_idx][BS_DAMAGED];
    current_player_sea_unit_types[sea_idx][BATTLESHIPS] +=
        current_player_sea_unit_types[sea_idx][BS_DAMAGED];
    sea_units_state[sea_idx][BS_DAMAGED][0] = 0;
    sea_units_state[sea_idx][BS_DAMAGED][1] = 0;
    current_player_sea_unit_types[sea_idx][BS_DAMAGED] = 0;
  }
}
// todo BUY FACTORY
void buy_factory() {}
void collect_money() {
  // if player still owns their capital, collect income
  state.money[0] +=
      (income_per_turn[0] * (*owner_idx[PLAYERS[state.current_turn].capital_territory_index] == 0));
}

uint get_user_purchase_input(GameState& state, uint src_air, Actions& valid_moves,
                             uint valid_moves_count) {
  if (PLAYERS[state.current_turn].is_human) {
    std::ostringstream oss;
    oss << "Purchasing at ";
    if (src_air < LANDS_COUNT) {
      oss << LANDS[src_air].name << " " << valid_moves[0] << "=Finished ";
      for (uint i = 1; i < valid_moves_count; i++) {
        oss << valid_moves[i] << "=" << NAMES_UNIT_LAND[valid_moves[i]] << " ";
      }
    } else {
      oss << SEAS[src_air - LANDS_COUNT].name << " " << valid_moves[0] << "=Finished ";
      for (uint i = 1; i < valid_moves_count; i++) {
        oss << valid_moves[i] << "=" << NAMES_UNIT_SEA[valid_moves[i]] << " ";
      }
    }
    std::cout << oss.str();
    return getUserInput(valid_moves, valid_moves_count);
  }
  return getAIInput(valid_moves, valid_moves_count, answers_remaining, selected_action);
}

void rotate_turns() {
  // rotate units
#ifdef DEBUG
  if (state.cache.actually_print) {
    setPrintableStatus();
    std::cout << printableGameStatus << std::endl;
    std::cout << "DEBUG: Rotating turns" << std::endl;
  }
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    if (total_player_land_units[0][land_idx] > 0 && enemy_units_count[land_idx] > 0) {
      printf("DEBUG: Player %s has %d units at %s\n", PLAYERS[state.current_turn].name,
             total_player_land_units[0][land_idx], LANDS[land_idx].name);
      cause_breakpoint();
    }
  }
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    if (total_player_sea_units[0][sea_idx] > 0 && enemy_units_count[sea_idx + LANDS_COUNT] > 0) {
      if (state.cache.actually_print) {
        printf("DEBUG: Player %s has %d units at %s\n", PLAYERS[state.current_turn].name,
               total_player_sea_units[0][sea_idx], SEAS[sea_idx].name);
      }
    }
  }
#endif
  COPY_FULL_ARRAY(current_player_land_unit_types, total_land_unit_types_temp);
  total_land_unit_types_temp = current_player_land_unit_types;
  COPY_FULL_ARRAY(state.idle_land_units[0], current_player_land_unit_types);
  // memmove(&state.other_land_units[0], &state.other_land_units[1],
  //         sizeof(state.other_land_units[0]) * (PLAYERS_COUNT - 2));
  std::move(&state.idle_land_units[1], &state.idle_land_units[PLAYERS_COUNT - 1],
            &state.idle_land_units[0]);
  // std::uninitialized_move_n(state.other_land_units[1], PLAYERS_COUNT - 2,
  // state.other_land_units[0]);
  memcpy(&state.idle_land_units[PLAYERS_COUNT - 2], &total_land_unit_types_temp,
         sizeof(state.idle_land_units[0]));
  for (uint dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
    LandStateJson* land0 = &state.land_states[dst_land];
    Landunittypes land1 = current_player_land_unit_types[dst_land];
    // todo optimize restructuring
    memset(&state.land_states[dst_land].fighters, 0, sizeof(state.land_states[0].fighters));
    land0->fighters[FIGHTER_STATES - 1] = land1[FIGHTERS];
    memset(&state.land_states[dst_land].bombers, 0, sizeof(state.land_states[0].bombers));
    land0->bombers[BOMBER_LAND_STATES - 1] = land1[BOMBERS_LAND_AIR];
    memset(&state.land_states[dst_land].infantry, 0, sizeof(state.land_states[0].infantry));
    land0->infantry[INFANTRY_STATES - 1] = land1[INFANTRY];
    memset(&state.land_states[dst_land].artillery, 0, sizeof(state.land_states[0].artillery));
    land0->artillery[ARTILLERY_STATES - 1] = land1[ARTILLERY];
    memset(&state.land_states[dst_land].tanks, 0, sizeof(state.land_states[0].tanks));
    land0->tanks[TANK_STATES - 1] = land1[TANKS];
    memset(&state.land_states[dst_land].aaguns, 0, sizeof(state.land_states[0].aaguns));
    land0->aaguns[AA_GUN_STATES - 1] = land1[AAGUNS];
  }
  memcpy(&total_sea_units_temp, &current_player_sea_unit_types, SEA_UNIT_TYPES_COUNT * SEAS_COUNT);
  //  memcpy(&other_sea_units_0, &data.other_sea_units[0], OTHER_SEA_UNITS_SIZE);
  for (uint dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
    memcpy(&current_player_sea_unit_types[dst_sea], &state.idle_sea_units[0][dst_sea],
           SEA_UNIT_TYPES_COUNT - 1);
  }
  // memmove(&state.other_sea_units[0], &state.other_sea_units[1],
  //         sizeof(state.other_sea_units[0]) * (PLAYERS_COUNT - 2));
  std::move(&state.idle_sea_units[1], &state.idle_sea_units[PLAYERS_COUNT - 1],
            &state.idle_sea_units[0]);
  // std::uninitialized_move_n(state.other_sea_units[1], PLAYERS_COUNT - 2,
  // state.other_sea_units[0]);
  //  memcpy(&data.other_sea_units[PLAYERS_COUNT - 2], &other_sea_units_temp, OTHER_SEA_UNITS_SIZE);
  memset(&state.sea_states, 0, sizeof(state.sea_states));
  for (uint dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
    memcpy(&state.idle_sea_units[PLAYERS_COUNT - 2][dst_sea], &total_sea_units_temp[dst_sea],
           SEA_UNIT_TYPES_COUNT - 1);
    for (uint unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
      SeaStateJson* sea0 = &state.sea_states[dst_sea];
      Seaunittypes sea1 = current_player_sea_unit_types[dst_sea];
      sea0->fighters[FIGHTER_STATES - 1] = sea1[FIGHTERS];
      sea0->transempty[TRANSEMPTY_STATES - 1] = sea1[TRANSEMPTY];
      sea0->trans1i[TRANS1I_STATES - 1] = sea1[TRANS1I];
      sea0->trans1a[TRANS1A_STATES - 1] = sea1[TRANS1A];
      sea0->trans1t[TRANS1T_STATES - 1] = sea1[TRANS1T];
      sea0->trans2i[TRANS2I_STATES - 1] = sea1[TRANS2I];
      sea0->trans1i1a[TRANS1I1A_STATES - 1] = sea1[TRANS1I1A];
      sea0->trans1i1t[TRANS1I1T_STATES - 1] = sea1[TRANS1I1T];
      sea0->submarines[SUB_STATES - 1] = sea1[SUBMARINES];
      sea0->destroyers[DESTROYER_STATES - 1] = sea1[DESTROYERS];
      sea0->carriers[CARRIER_STATES - 1] = sea1[CARRIERS];
      sea0->cruisers[CRUISER_STATES - 1] = sea1[CRUISERS];
      sea0->battleships[BATTLESHIP_STATES - 1] = sea1[BATTLESHIPS];
      sea0->bs_damaged[BATTLESHIP_STATES - 1] = sea1[BS_DAMAGED];
      sea0->bombers[BOMBER_SEA_STATES - 1] = sea1[BOMBERS_SEA];
    }
  }
  uint temp_money = state.money[0];
  memmove(&state.money[0], &state.money[1], sizeof(state.money[0]) * (PLAYERS_COUNT - 1));
  state.money[PLAYERS_COUNT - 1] = temp_money;
  income_per_turn[PLAYERS_COUNT] = income_per_turn[0];
  memmove(&income_per_turn[0], &income_per_turn[1], sizeof(income_per_turn[0]) * PLAYERS_COUNT);
  total_factory_count[PLAYERS_COUNT] = total_factory_count[0];
  memmove(&total_factory_count[0], &total_factory_count[1],
          sizeof(total_factory_count[0]) * PLAYERS_COUNT);
  memcpy(&factory_locations[PLAYERS_COUNT], &factory_locations[0], sizeof(factory_locations[0]));
  memmove(&factory_locations[0], &factory_locations[1],
          sizeof(factory_locations[0]) * PLAYERS_COUNT);
  memcpy(&total_player_land_units[PLAYERS_COUNT], &total_player_land_units[0],
         sizeof(total_player_land_units[0]));
  memmove(&total_player_land_units[0], &total_player_land_units[1],
          sizeof(total_player_land_units[0]) * PLAYERS_COUNT);
  memcpy(&total_player_sea_units[PLAYERS_COUNT], &total_player_sea_units[0],
         sizeof(total_player_sea_units[0]));
  memmove(&total_player_sea_units[0], &total_player_sea_units[1],
          sizeof(total_player_sea_units[0]) * PLAYERS_COUNT);
  // reset combat flags
  memset(&state.combat_status, CombatStatus::NO_COMBAT, sizeof(state.combat_status));
  state.current_turn = (state.current_turn + 1) % PLAYERS_COUNT;

  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    state.land_states[land_idx].owner_idx =
        (state.land_states[land_idx].owner_idx + PLAYERS_COUNT - 1) % PLAYERS_COUNT;
  }

  for (uint factory_idx = 0; factory_idx < total_factory_count[0]; factory_idx++) {
    uint dst_land = factory_locations[0][factory_idx];
    state.builds_left[dst_land] = *factory_max[dst_land];
    for (uint sea_idx = 0; sea_idx < LAND_TO_SEA_COUNT[dst_land]; sea_idx++) {
      state.builds_left[LAND_TO_SEA_CONN[dst_land][sea_idx] + LANDS_COUNT] +=
          *factory_max[dst_land];
    }
  }

  refresh_eot_cache();
#ifdef DEBUG
  if (state.cache.actually_print) {
    std::cout << "DEBUG: Cache refreshed. Player " << PLAYERS[state.current_turn].name << "'s turn"
              << std::endl;
    setPrintableStatus();
    std::cout << printableGameStatus << std::endl;
  }
  for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    for (uint factory_index = 0; factory_index < total_factory_count[player_idx]; factory_index++) {
      uint factory_location = factory_locations[player_idx][factory_index];
      if (*owner_idx[factory_location] != player_idx) {
        printf("DEBUG: Player %s has a unowned factory at %s\n", PLAYERS[state.current_turn].name,
               LANDS[factory_location].name);
        cause_breakpoint();
      }
    }
  }
#endif
}

double get_score() {

  // Evaluate the game state and return a score
  uint allied_score = 1; // one helps prevent division by zero
  uint enemy_score = 1;
  allied_score += state.money[0];
  for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    uint score = state.money[player_idx];
    for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
      for (uint unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
        score += total_player_land_unit_types[player_idx][land_idx][unit_type] *
                 COST_UNIT_LAND[unit_type];
      }
    }
    for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
      for (uint unit_type = 0; unit_type < SEA_UNIT_TYPES_COUNT; unit_type++) {
        score +=
            total_player_sea_unit_types[player_idx][sea_idx][unit_type] * COST_UNIT_SEA[unit_type];
      }
    }
    if (PLAYERS[state.current_turn].is_allied[(state.current_turn + player_idx) % PLAYERS_COUNT]) {
      allied_score += score;
    } else {
      enemy_score += score;
    }
  }
  //  double score = ((double)allied_score / (double)(enemy_score + allied_score));
  // return ((double)2 * (double)allied_score / (double)(enemy_score + allied_score)) - (double)1;
  return ((double)allied_score / (double)(enemy_score + allied_score));
}

GameStateJson* get_game_state_copy() {
  // GameState* game_state = (GameState*)malloc(sizeof(GameState));
  // memcpy(game_state, &state, sizeof(GameState));
  auto new_state = std::make_unique<GameStateJson>(state);
  return new_state.release();
}

// Implement these functions based on your game logic
GameStateJson* clone_state(GameStateJson* game_state) {
  // Use the copy constructor to create a deep copy of the game state
  auto new_state = std::make_unique<GameStateJson>(*game_state);
  return new_state.release();
}

void get_possible_actions(GameStateJson* game_state, uint* num_actions, ActionsPtr actions) {
  unlucky_player_idx = 0;
  // memcpy(&state, game_state, sizeof(GameState));
  state = *game_state;
  refresh_full_cache();
  answers_remaining = 0;
  random_number_index = 0;
  // clear_move_history();
  while (true) {
    if (move_fighter_units())
      break;
    if (move_bomber_units())
      break;
    if (stage_transport_units())
      break;
    if (move_land_unit_type(TANKS))
      break;
    if (move_land_unit_type(ARTILLERY))
      break;
    if (move_land_unit_type(INFANTRY))
      break;
    if (move_transport_units())
      break;
    if (move_subs())
      break;
    if (move_destroyers_battleships())
      break;
    if (resolve_sea_battles())
      break;
    if (unload_transports())
      break;
    if (resolve_land_battles())
      break;
    if (move_land_unit_type(AAGUNS))
      break;
    if (land_fighter_units())
      break;
    if (land_bomber_units())
      break;
    if (buy_units())
      break;
    // if (end_turn())
    //   break;
    crash_air_units();
    reset_units_fully();
    buy_factory();
    collect_money();
    rotate_turns();
  }
  *num_actions = valid_moves_count;
  // memcpy(actions, valid_moves, valid_moves_count * sizeof(uint));
  std::memcpy(actions, &valid_moves, valid_moves_count * sizeof(uint));
  // std::copy(&valid_moves,&valid_moves + valid_moves_count, actions);
}
void apply_action(GameStateJson* game_state, uint action) {
  // Apply the action to the game state

#ifdef DEBUG
  if (state.cache.actually_print) {
    printf("DEBUG: copying state and Applying action %d\n", action);
  }
#endif
  // uint starting_player = game_state->player_index;
  // if (starting_player >= PLAYERS_COUNT) {
  //   game_state->player_index -= PLAYERS_COUNT;
  //   return;
  // }
  // memcpy(&state, game_state, sizeof(GameState));
  state = *game_state;
  refresh_full_cache();
  //  unlucky_player_idx = state.player_index;
  unlucky_player_idx = 0;
  answers_remaining = 1;
  selected_action = action;
  random_number_index = 0;
  // clear_move_history();
  //  RANDOM_NUMBERS[random_number_index] = action;
  while (true) {
    if (move_fighter_units())
      break;
    if (move_bomber_units())
      break;
    if (stage_transport_units())
      break;
    if (move_land_unit_type(TANKS))
      break;
    if (move_land_unit_type(ARTILLERY))
      break;
    if (move_land_unit_type(INFANTRY))
      break;
    if (move_transport_units())
      break;
    if (move_subs())
      break;
    if (move_destroyers_battleships())
      break;
    if (resolve_sea_battles())
      break;
    if (unload_transports())
      break;
    if (resolve_land_battles())
      break;
    if (move_land_unit_type(AAGUNS))
      break;
    if (land_fighter_units())
      break;
    if (land_bomber_units())
      break;
    if (buy_units())
      break;
    // if (end_turn())
    //   break;
    crash_air_units();
    reset_units_fully();
    buy_factory();
    collect_money();
    rotate_turns();
  }
  // if (PLAYERS[starting_player % PLAYERS_COUNT].is_allied[(state.player_index)]) {
  //   // printf("(adding to player_index) Player %s is allied with player %s\n",
  //   // PLAYERS[starting_player % PLAYERS_COUNT].name,
  //   //       PLAYERS[state.player_index].name);
  //   state.player_index += PLAYERS_COUNT;
  // }
  // memcpy(game_state, &state, sizeof(GameState));
  *game_state = state;
}
void set_seed(uint new_seed) { random_number_index = new_seed; }

double random_play_until_terminal(GameStateJson* game_state) {
  // memcpy(&state, game_state, sizeof(GameState));
  state = *game_state;
  refresh_full_cache();
  answers_remaining = 100000;
  selected_action = ACTION_COUNT;
  double score = get_score();
  max_loops = 1000;
  random_number_index = static_cast<uint>(rand() % RANDOM_NUMBERS_SIZE);
  // clear_move_history();
  while (score > 0.01 && score < 0.99 && max_loops-- > 0) {
// printf("max_loops: %d\n", max_loops);
//  if(max_loops == 2) {
//    setPrintableStatus();
//    printf("%s\n", printableGameStatus);
//    printf("DEBUG: max_loops reached\n");
//  }
// if (max_loops % 100 == 0) {
//   printf("max_loops: %d\n", max_loops);
// }
#if DEBUG
    debug_checks();
    move_fighter_units();
    debug_checks();
    move_bomber_units();
    debug_checks();
    stage_transport_units();
    debug_checks();
    move_land_unit_type(TANKS);
    debug_checks();
    move_land_unit_type(ARTILLERY);
    debug_checks();
    move_land_unit_type(INFANTRY);
    debug_checks();
    move_transport_units();
    debug_checks();
    move_subs();
    debug_checks();
    move_destroyers_battleships();
    debug_checks();
    resolve_sea_battles();
    debug_checks();
    unload_transports();
    debug_checks();
    resolve_land_battles();
    debug_checks();
    move_land_unit_type(AAGUNS);
    debug_checks();
    land_fighter_units();
    debug_checks();
    land_bomber_units();
    debug_checks();
    buy_units();
    debug_checks();
    crash_air_units();
    debug_checks();
    reset_units_fully();
    debug_checks();
    buy_factory();
    debug_checks();
    collect_money();
    debug_checks();
    rotate_turns();
    debug_checks();
#else
    move_fighter_units();
    move_bomber_units();
    stage_transport_units();
    move_land_unit_type(TANKS);
    move_land_unit_type(ARTILLERY);
    move_land_unit_type(INFANTRY);
    move_transport_units();
    move_subs();
    move_destroyers_battleships();
    resolve_sea_battles();
    unload_transports();
    resolve_land_battles();
    move_land_unit_type(AAGUNS);
    land_fighter_units();
    land_bomber_units();
    buy_units();
    crash_air_units();
    reset_units_fully();
    buy_factory();
    collect_money();
    rotate_turns();
#endif
    score = get_score();
  }
  if (state.current_turn % 2 == 1) {
    score = 1 - score;
  }
  return score;
}

bool is_terminal_state(GameStateJson* game_state) {
  // Return true if the game is over
  double score = evaluate_state(game_state);
  return (score > 0.99 || score < 0.01);
}

double evaluate_state(GameStateJson* game_state) {
  uint starting_player = game_state->current_turn;
  if (starting_player >= PLAYERS_COUNT) {
    game_state->current_turn -= PLAYERS_COUNT;
  }
  // Evaluate the game state and return a score
  uint allied_score = 1; // one helps prevent division by zero
  uint enemy_score = 1;
  allied_score += game_state->money[0];
  for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    uint score = game_state->money[player_idx];
    for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
      for (uint unit_type = 0; unit_type < LAND_UNIT_TYPES_COUNT; unit_type++) {
        score += total_player_land_unit_types[player_idx][land_idx][unit_type] *
                 COST_UNIT_LAND[unit_type];
      }
    }
    for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
      for (uint unit_type = 0; unit_type < SEA_UNIT_TYPES_COUNT; unit_type++) {
        score +=
            total_player_sea_unit_types[player_idx][sea_idx][unit_type] * COST_UNIT_SEA[unit_type];
      }
    }
    if (PLAYERS[game_state->current_turn]
            .is_allied[(game_state->current_turn + player_idx) % PLAYERS_COUNT]) {
      allied_score += score;
    } else {
      enemy_score += score;
    }
  }
  double score =
      (static_cast<double>(allied_score) / static_cast<double>(enemy_score + allied_score));
  game_state->current_turn = starting_player;
  if (starting_player >= PLAYERS_COUNT) {
    return 1 - score;
  }
  return score;
}
void load_single_game() {
  random_number_index = 34282;
  answers_remaining = 100000;
  selected_action = ACTION_COUNT;
  double score = get_score();
  actually_print = true;
  max_loops = 1000;
  uint player_idx = 4;
  uint sea_idx1 = 2;
  printf("\nINITIAL: %d, %d, %d\n", total_player_sea_units[player_idx][sea_idx1],
         current_player_sea_unit_types[sea_idx1][0],
         total_player_sea_unit_types[player_idx][sea_idx1][0]);
  setPrintableStatus();
  std::cout << printableGameStatus << std::endl;
  while (score > 0.01 && score < 0.99 && max_loops-- > 0) {
    // printf("max_loops: %d\n", max_loops);
    //  if(max_loops == 2) {
    //    setPrintableStatus();
    //    printf("%s\n", printableGameStatus);
    //    printf("DEBUG: max_loops reached\n");
    //  }
    // if (max_loops % 100 == 0) {
    //   printf("max_loops: %d\n", max_loops);
    // }
    debug_checks();
    move_fighter_units();
    move_bomber_units();
    stage_transport_units();
    move_land_unit_type(TANKS);
    move_land_unit_type(ARTILLERY);
    move_land_unit_type(INFANTRY);
    move_transport_units();
    move_subs();
    move_destroyers_battleships();
    resolve_sea_battles();
    unload_transports();
    resolve_land_battles();
    move_land_unit_type(AAGUNS);
    land_fighter_units();
    land_bomber_units();
    buy_units();
    //    end_turn();
    crash_air_units();
    reset_units_fully();
    buy_factory();
    collect_money();
    rotate_turns();
    score = get_score();
  }
}
#ifdef NDEBUG
#define debug_checks(GameState& state) ((void)0)
#else
void debug_checks(GameState& state) {
  step_id++;
  uint player_idx = state.current_turn;
  // if (step_id == 1999998819) {
  //   actually_print = true;
  // }
  if (state.cache.actually_print) {
    std::cout << "  loops: " << max_loops << "  step_id: " << step_id
              << "  answers_remaining: " << answers_remaining
              << "  select_action: " << selected_action << std::endl;
  }
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    for (uint unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
      uint temp_unit_type_total = 0;
      std::vector<uint> units_here = get_active_land_units(state).at(unit_idx)->at(land_idx);
      for (const uint unit_here : units_here) {
        temp_unit_type_total += unit_here;
      }
      uint idle_land_units = get_idle_land_units(state).at(unit_idx)->val(player_idx, land_idx);
      if (temp_unit_type_total != idle_land_units) {
        std::cout << "temp_unit_type_total " << temp_unit_type_total << " != idle_land_units "
                  << idle_land_units << std::endl;
        cause_breakpoint();
      }
    }
  }
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    for (uint unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
      uint temp_unit_type_total = 0;
      std::vector<uint> units_here = get_active_sea_units(state).at(unit_idx)->at(sea_idx);
      for (const uint unit_here : units_here) {
        temp_unit_type_total += unit_here;
      }
      uint idle_land_units = get_idle_sea_units(state).at(unit_idx)->val(player_idx, sea_idx);
      if (temp_unit_type_total != idle_land_units) {
        std::cout << "temp_unit_type_total " << temp_unit_type_total << " != idle_land_units["
                  << sea_idx << "][" << unit_idx << "] " << idle_land_units << std::endl;
        cause_breakpoint();
      }
    }
  }
  // constexpr uint MAX_TOTAL_UNITS = 9000;
  // for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
  //   for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
  //     uint total = 0;
  //     uint total_data = 0;
  //     for (uint unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
  //       if (player_idx == 0 || unit_idx < LAND_UNIT_TYPES_COUNT) {
  //         total += total_player_land_unit_types[player_idx][land_idx][unit_idx];
  //       }
  //       if (player_idx == 0) {
  //         total_data += current_player_land_unit_types[land_idx][unit_idx];
  //       } else {
  //         if (unit_idx < LAND_UNIT_TYPES_COUNT) {
  //           total_data += state.idle_land_units.at(player_idx - 1)[land_idx][unit_idx];
  //         }
  //       }
  //     }
  //     if (total != total_player_land_units[player_idx][land_idx] || total != total_data) {
  //       std::cout << "total " << total << " != total_player_land_units[player_idx][land_idx] "
  //                 << total_player_land_units[player_idx][land_idx] << " != total_data "
  //                 << total_data;
  //       cause_breakpoint();
  //     }
  //     if (total_player_land_units[player_idx][land_idx] < 0 ||
  //         total_player_land_units[player_idx][land_idx] > MAX_TOTAL_UNITS) {
  //       std::cout << "units_land_player_total[player_idx][land_idx] < 0" << std::endl;
  //       cause_breakpoint();
  //     }
  //     if (enemy_units_count[land_idx] < 0 || enemy_units_count[land_idx] > MAX_TOTAL_UNITS) {
  //       std::cout << "enemy_units_count[land_idx] < 0" << std::endl;
  //       cause_breakpoint();
  //     }
  //     total = 0;
  //     for (uint unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
  //       if (total_player_land_unit_types[player_idx][land_idx][unit_idx] < 0 ||
  //           total_player_land_unit_types[player_idx][land_idx][unit_idx] > MAX_TOTAL_UNITS) {
  //         std::cout << "total_player_land_unit_types[player_idx][land_idx][unit_idx] < 0";
  //         cause_breakpoint();
  //       }
  //       total += total_player_land_unit_types[player_idx][land_idx][unit_idx];
  //     }
  //     if (total != total_player_land_units[player_idx][land_idx]) {
  //       std::cout << "total " << total << " != total_player_land_units[player_idx][land_idx] "
  //                 << total_player_land_units[player_idx][land_idx] << std::endl;
  //       cause_breakpoint();
  //     }
  //     uint enemy_total = 0;
  //     for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
  //       uint enemy_player_idx = enemies_0[enemy_idx];
  //       enemy_total += total_player_land_units[enemy_player_idx][land_idx];
  //     }
  //     if (enemy_total != enemy_units_count[land_idx]) {
  //       std::cout << "enemy_total " << enemy_total << " != enemy_units_count[land_idx] "
  //                 << enemy_units_count[land_idx] << std::endl;
  //       cause_breakpoint();
  //     }
  //   }
  //   for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
  //     if (total_player_sea_units[player_idx][sea_idx] < 0 ||
  //         total_player_sea_units[player_idx][sea_idx] > MAX_TOTAL_UNITS) {
  //       std::cout << "units_sea_player_total[player_idx][sea_idx] < 0" << std::endl;
  //       cause_breakpoint();
  //     }
  //     for (uint unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
  //       if (total_player_sea_unit_types[player_idx][sea_idx][unit_idx] < 0 ||
  //           total_player_sea_unit_types[player_idx][sea_idx][unit_idx] > MAX_TOTAL_UNITS) {
  //         std::cout << "total_player_sea_unit_types[player_idx][sea_idx][unit_idx] < 0";
  //         cause_breakpoint();
  //       }
  //     }
  //     if (enemy_units_count[sea_idx + LANDS_COUNT] < 0 ||
  //         enemy_units_count[sea_idx + LANDS_COUNT] > MAX_TOTAL_UNITS) {
  //       std::cout << "enemy_units_count[land_idx] < 0";
  //       cause_breakpoint();
  //     }
  //     uint total = 0;
  //     uint total_data = 0;
  //     for (uint unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
  //       if (player_idx == 0 || unit_idx < SEA_UNIT_TYPES_COUNT - 1) {
  //         total += total_player_sea_unit_types[player_idx][sea_idx][unit_idx];
  //       }
  //       if (player_idx == 0) {
  //         total_data += current_player_sea_unit_types[sea_idx][unit_idx];
  //       } else {
  //         if (unit_idx < SEA_UNIT_TYPES_COUNT - 1) {
  //           total_data += state.idle_sea_units.at(player_idx - 1)[sea_idx][unit_idx];
  //         }
  //       }
  //     }
  //     if (total != total_player_sea_units[player_idx][sea_idx] || total != total_data) {
  //       std::cout << "total " << total << " != total_player_sea_units[player_idx " << player_idx
  //                 << "][sea_idx " << sea_idx << "] " <<
  //                 total_player_sea_units[player_idx][sea_idx]
  //                 << " != total_data " << total_data;
  //       cause_breakpoint();
  //     }
  //     uint enemy_total = 0;
  //     for (uint enemy_idx = 0; enemy_idx < enemies_count_0; enemy_idx++) {
  //       uint enemy_player_idx = enemies_0[enemy_idx];
  //       enemy_total += total_player_sea_units[enemy_player_idx][sea_idx];
  //     }
  //     if (enemy_total != enemy_units_count[sea_idx + LANDS_COUNT]) {
  //       std::cout << "enemy_total " << enemy_total
  //                 << " != enemy_units_count[sea_idx + LANDS_COUNT] "
  //                 << enemy_units_count[sea_idx + LANDS_COUNT];
  //       cause_breakpoint();
  //     }
  //   }
  // }
}
#endif

PYBIND11_MODULE(engine, handle) {
  handle.doc() = "caaa engine doc";
  handle.def("random_play_until_terminal", &random_play_until_terminal);
  handle.def("clone_state", &clone_state);
  handle.def("get_possible_actions", &get_possible_actions);
  handle.def("apply_action", &apply_action);
  handle.def("is_terminal_state", &is_terminal_state);
  handle.def("initialize_constants", &initialize_map_constants);
  handle.def("get_game_state_copy", &get_game_state_copy);
  handle.def("evaluate_state", &evaluate_state);
  handle.def("load_game_state_from_json", &load_game_state_from_json);
}
