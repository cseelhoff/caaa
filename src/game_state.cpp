#include "game_state.hpp"
#include "array_functions.hpp"
#include "engine.hpp"
#include "land.hpp"
#include "map_cache.hpp"
#include "player.hpp"
#include "sea.hpp"
#include "units/units.hpp"
#include <array>
#include <iomanip>
#include <sstream>
#include <sys/types.h>
#include <tuple>

void refresh_full_cache(GameState& state) {
  refresh_economy(state);
  refresh_land_armies(state);
  refresh_sea_navies(state);
  refresh_allies(state);
  refresh_canals(state);
  refresh_economy(state);
  refresh_fleets(state);
  refresh_land_path_blocked(state);
  refresh_sea_path_blocked(state.cache);
}
void refresh_eot_cache(GameState& state) {
  refresh_allies(state);
  refresh_canals(state);
  refresh_economy(state);
  refresh_fleets(state);
  refresh_land_path_blocked(state);
  refresh_sea_path_blocked(state.cache);
}

void refresh_economy(GameState& state, GameCache& cache) {
  cache.income_per_turn.fill(0);
  cache.total_factory_count.fill(0);
  cache.team_units_count.fill(0);
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    uint player_idx = state.land_owners[land_idx];
    if (state.factory_max[land_idx] > 0) {
      cache.factory_locations.ref(player_idx, cache.total_factory_count[player_idx]++) = land_idx;
    }
    cache.income_per_turn[player_idx] += LANDS[land_idx].land_value;
  }
}

template <typename ArrayType>
void refresh_units(std::vector<ArrayType> idle_unit_arrays, GameCache& cache, uint player_idx,
                   uint terr_count, uint offset) {
  uint team = PLAYER_TEAM[player_idx];
  for (uint terr_idx = 0; terr_idx < terr_count; terr_idx++) {
    uint total_units = 0;
    for (auto& array : idle_unit_arrays) {
      total_units += array.at(terr_idx);
    }
    cache.total_player_units.ref(player_idx, terr_idx + offset) = total_units;
    cache.team_units_count.ref(team, terr_idx + offset) += total_units;
  }
}

void refresh_land_units(GameState& state, GameCache& cache, uint player_idx) {
  std::vector<LandArray> idle_unit_arrays = {
      state.idle_land_fighters.arr(player_idx), state.idle_land_bombers.arr(player_idx),
      state.idle_land_infantry.arr(player_idx), state.idle_land_artillery.arr(player_idx),
      state.idle_land_tanks.arr(player_idx),    state.idle_land_aaguns.arr(player_idx)};
  refresh_units(idle_unit_arrays, cache, player_idx, LANDS_COUNT, 0);
}

void refresh_sea_units(GameState& state, GameCache& cache, uint player_idx) {
  std::vector<SeaArray> idle_unit_arrays = {
      state.idle_sea_fighters.arr(player_idx),    state.idle_sea_transempty.arr(player_idx),
      state.idle_sea_trans1i.arr(player_idx),     state.idle_sea_trans1a.arr(player_idx),
      state.idle_sea_trans1t.arr(player_idx),     state.idle_sea_trans2i.arr(player_idx),
      state.idle_sea_trans1i1a.arr(player_idx),   state.idle_sea_trans1i1t.arr(player_idx),
      state.idle_sea_submarines.arr(player_idx),  state.idle_sea_destroyers.arr(player_idx),
      state.idle_sea_carriers.arr(player_idx),    state.idle_sea_cruisers.arr(player_idx),
      state.idle_sea_battleships.arr(player_idx), state.idle_sea_bs_damaged.arr(player_idx),
  };
  refresh_units(idle_unit_arrays, cache, player_idx, SEAS_COUNT, LANDS_COUNT);
}

void refresh_total_player_units(GameState& state, GameCache& cache) {
  cache.total_player_units.fill(0);
  for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    refresh_land_units(state, cache, player_idx);
    refresh_sea_units(state, cache, player_idx);
  }
}
void refresh_canals(GameState& state, GameCache& cache) {
  cache.canal_state = 0;
  uint current_turn = state.current_turn;
  for (uint canal_idx = 0; canal_idx < CANALS_COUNT; canal_idx++) {
    if (PLAYERS[current_turn].is_allied[state.land_owners[CANALS[canal_idx].lands[0]]] &&
        PLAYERS[current_turn].is_allied[state.land_owners[CANALS[canal_idx].lands[1]]]) {
      cache.canal_state += 1U << canal_idx;
    }
  }
}

void refresh_transports_with_cargo_space(GameState& state, GameCache& cache, uint player_idx,
                                         uint sea_idx) {
  cache.transports_with_large_cargo_space[sea_idx] =
      state.idle_sea_transempty.val(player_idx, sea_idx) +
      state.idle_sea_trans1i.val(player_idx, sea_idx);
  cache.transports_with_small_cargo_space[sea_idx] =
      cache.transports_with_large_cargo_space[sea_idx] +
      state.idle_sea_trans1a.val(player_idx, sea_idx) +
      state.idle_sea_trans1t.val(player_idx, sea_idx);
}

void refresh_fleets(GameState& state, GameCache& cache) {
  FILL_ARRAY(cache.allied_carriers, 0);
  FILL_ARRAY(cache.enemy_units_count, 0);
  FILL_ARRAY(cache.enemy_destroyers_total, 0);
  FILL_ARRAY(cache.enemy_blockade_total, 0);
  FILL_ARRAY(cache.transports_with_large_cargo_space, 0);
  FILL_ARRAY(cache.transports_with_small_cargo_space, 0);
  uint current_turn = state.current_turn;
  const Player current_player = PLAYERS[current_turn];
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    uint air_idx = sea_idx + LANDS_COUNT;
    for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      if (current_player.is_allied[player_idx]) {
        cache.allied_carriers[sea_idx] += state.idle_sea_carriers.val(player_idx, sea_idx);
      } else {
        cache.enemy_units_count[air_idx] += cache.total_player_units.val(player_idx, air_idx);
        cache.enemy_destroyers_total[sea_idx] += state.idle_sea_destroyers.val(player_idx, sea_idx);
        cache.enemy_blockade_total[sea_idx] += state.idle_sea_destroyers.val(player_idx, sea_idx) +
                                               state.idle_sea_carriers.val(player_idx, sea_idx) +
                                               state.idle_sea_cruisers.val(player_idx, sea_idx) +
                                               state.idle_sea_battleships.val(player_idx, sea_idx) +
                                               state.idle_sea_bs_damaged.val(player_idx, sea_idx);
      }
    }
    refresh_transports_with_cargo_space(state, cache, current_turn, sea_idx);
  }
}
void refresh_land_path_blocked(GameState& state, GameCache& cache) {
  cache.land_path_blocked.fill(false);
  LandArray& factory_max = state.factory_max;
  AirArray& enemy_units_count = cache.enemy_units_count;
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    uint lands_within_2_moves_count = LANDS_WITHIN_2_MOVES_COUNT[src_land];
    LandArray lands_within_2_moves = LANDS_WITHIN_2_MOVES[src_land];
    BoolLandArray land_path_blocked = cache.land_path_blocked.arr(src_land);
    for (uint conn_idx = 0; conn_idx < lands_within_2_moves_count; conn_idx++) {
      uint dst_land = lands_within_2_moves[conn_idx];
      uint nextLandMovement = LAND_PATH[src_land][dst_land];
      uint nextLandMovementAlt = LAND_PATH_ALT[src_land][dst_land];
      land_path_blocked[dst_land] =
          (enemy_units_count[nextLandMovement] > 0 || state.factory_max[nextLandMovement] > 0) &&
          (enemy_units_count[nextLandMovementAlt] > 0 || factory_max[nextLandMovementAlt] > 0);
    }
  }
}
void refresh_sea_path_blocked(GameCache& cache) {
  uint canal_state = cache.canal_state;
  SeaSeaArray sea_path = SEA_PATH[canal_state];
  SeaSeaArray sea_path_alt = SEA_PATH_ALT[canal_state];
  SeaArray& enemy_blockade_total = cache.enemy_blockade_total;
  SeaArray& enemy_destroyers_total = cache.enemy_destroyers_total;
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    SeaArray seaPathArray = sea_path[src_sea];
    SeaArray seaPathArrayAlt = sea_path_alt[src_sea];
    uint seas_within_2_moves_count = SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea];
    SeaArray seas_within_2_moves = SEAS_WITHIN_2_MOVES[canal_state][src_sea];
    BoolSeaArray sea_path_blocked = cache.sea_path_blocked.arr(src_sea);
    BoolSeaArray sub_path_blocked = cache.sub_path_blocked.arr(src_sea);
    for (uint conn_idx = 0; conn_idx < seas_within_2_moves_count; conn_idx++) {
      uint dst_sea = seas_within_2_moves[conn_idx];
      uint nextSeaMovement = seaPathArray[dst_sea];
      uint nextSeaMovementAlt = seaPathArrayAlt[dst_sea];
      sea_path_blocked[dst_sea] =
          enemy_blockade_total[nextSeaMovement] > 0 && enemy_blockade_total[nextSeaMovementAlt] > 0;
      sub_path_blocked[dst_sea] = enemy_destroyers_total[nextSeaMovement] > 0 &&
                                  enemy_destroyers_total[nextSeaMovementAlt] > 0;
    }
  }
}

constexpr uint UNIT_NAME_WIDTH = 14;

std::string get_printable_status(GameState& state) {
  std::ostringstream oss;
  oss << "---\n";
  oss << get_printable_status_lands(state);
  oss << get_printable_status_seas(state) << "\n";
  oss << PLAYERS[state.current_turn].color;
  oss << PLAYERS[state.current_turn].name;
  oss << "\033[0m"
      << ": " << state.money[0] << " IPC\n";
  return oss.str();
}

void append_land_unit(GameState& state, std::ostringstream& oss, uint player_idx, uint unit_idx,
                      uint land_idx) {

  uint unit_count = get_idle_land_units(state).at(unit_idx)->val(player_idx, land_idx);
  if (unit_count > 0) {
    oss << PLAYERS[player_idx].color << PLAYERS[player_idx].name << " " << std::left
        << std::setw(UNIT_NAME_WIDTH) << NAMES_UNIT_LAND[unit_idx] << std::right << std::setw(3)
        << unit_count;
    std::vector<uint> units_here = get_active_land_units(state).at(unit_idx)->at(land_idx);
    for (const uint unit_here : units_here) {
      oss << std::right << std::setw(3) << unit_here;
    }
    oss << "\n";
  }
}

void get_printable_active_land_units(GameState& state, std::ostringstream& oss) {
  uint player_idx = state.current_turn;
  const char* my_color = PLAYERS[player_idx].color;
  oss << my_color;
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    if (state.cache.total_player_units.val(player_idx, land_idx) > 0) {
      for (uint unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
        append_land_unit(state, oss, player_idx, unit_idx, land_idx);
      }
    }
  }
}

std::string get_printable_status_lands(GameState& state) {
  std::ostringstream oss;
  uint current_player_idx = state.current_turn;
  LandArray& land_owners = state.land_owners;
  LandArray& factory_dmg = state.factory_dmg;
  LandArray& factory_max = state.factory_max;
  oss << "                 |Tot| 0| 1| 2| 3| 4| 5| 6|\n";
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    uint land_owner = land_owners[land_idx];
    oss << PLAYERS[land_owner].color << land_idx << " ";
    oss << LANDS[land_idx].name << ": " << PLAYERS[land_owner].name;
    oss << " " << state.builds_left.at(land_idx);
    oss << "/" << factory_dmg[land_idx];
    oss << "/" << factory_max[land_idx];
    oss << "/" << LANDS[land_idx].land_value;
    oss << " Combat: " << state.combat_status.at(land_idx);
    get_printable_active_land_units(state, oss);
    oss << "\033[0m";
    for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      if (current_player_idx == player_idx ||
          state.cache.total_player_units.val(player_idx, land_idx) == 0) {
        continue;
      }
      oss << PLAYERS[player_idx].color;
      for (uint unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
        uint unit_count = get_idle_land_units(state).at(unit_idx)->val(player_idx, land_idx);
        // uint unit_count = state.idle_land_fighters.val(player_idx, land_idx);
        if (unit_count > 0) {
          oss << PLAYERS[player_idx].name << " ";
          oss << std::left << std::setw(UNIT_NAME_WIDTH) << NAMES_UNIT_LAND[unit_idx];
          oss << std::right << std::setw(3) << unit_count << "\n";
        }
      }
      oss << "\033[0m";
    }
  }
  oss << "\n";
  return oss.str();
}

void append_sea_unit(std::ostringstream& oss, uint player_idx, uint unit_idx, uint sea_idx,
                     GameState& state) {

  uint unit_count = get_idle_sea_units(state).at(unit_idx)->val(player_idx, sea_idx);
  if (unit_count > 0) {
    oss << PLAYERS[player_idx].color;
    oss << PLAYERS[player_idx].name;
    oss << " ";
    oss << std::left << std::setw(UNIT_NAME_WIDTH) << NAMES_UNIT_SEA[unit_idx];
    oss << std::right << std::setw(3) << unit_count;
    std::vector<uint> units_here = get_active_sea_units(state).at(unit_idx)->at(sea_idx);
    for (const uint unit_here : units_here) {
      oss << std::right << std::setw(3) << unit_here;
    }
    oss << "\n";
  }
}

void get_printable_active_sea_units(std::ostringstream& oss, GameState& state, GameCache& cache) {
  uint player_idx = state.current_turn;
  const char* my_color = PLAYERS[player_idx].color;
  oss << my_color;
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    uint air_idx = LANDS_COUNT + sea_idx;
    if (cache.total_player_units.val(player_idx, air_idx) > 0) {
      for (uint unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT; unit_idx++) {
        append_sea_unit(oss, player_idx, unit_idx, sea_idx, state);
      }
    }
  }
}

std::string get_printable_status_seas(GameState& state, GameCache& cache) {
  std::ostringstream oss;
  uint current_player_idx = state.current_turn;
  oss << "                 |Tot| 0| 1| 2| 3| 4| 5| 6|\n";
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    uint air_idx = LANDS_COUNT + sea_idx;
    oss << air_idx << " " << SEAS[sea_idx].name;
    oss << " Combat: " << state.combat_status.at(air_idx);
    oss << PLAYERS[state.current_turn].color;
    get_printable_active_sea_units(oss, state, cache);
    oss << "\033[0m";
    for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      if (current_player_idx == player_idx ||
          cache.total_player_units.val(player_idx, air_idx) == 0) {
        continue;
      }
      oss << PLAYERS[player_idx].color;
      for (uint unit_idx = 0; unit_idx < SEA_UNIT_TYPES_COUNT - 1; unit_idx++) {
        uint unit_count = get_idle_sea_units(state).at(unit_idx)->val(player_idx, sea_idx);
        if (unit_count > 0) {
          oss << PLAYERS[player_idx].name << " ";
          oss << std::left << std::setw(UNIT_NAME_WIDTH) << NAMES_UNIT_SEA[unit_idx];
          oss << std::right << std::setw(3) << unit_count << "\n";
        }
      }
      oss << "\033[0m";
    }
  }
  oss << "\n";
  return oss.str();
}
