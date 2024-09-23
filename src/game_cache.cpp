#include "game_cache.hpp"
#include "game_state_memory.hpp"
#include "land.hpp"
#include "map_cache.hpp"
#include "player.hpp"
#include "sea.hpp"
#include "units/units.hpp"

void refresh_economy(GameStateMemory& state, GameCache& cache) {
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
void refresh_units(std::vector<ArrayType> idle_unit_arrays, GameCache& cache, uint player_idx, uint terr_count, uint offset) {
  uint team = PLAYER_TEAM[player_idx];
  for (uint terr_idx = 0; terr_idx < terr_count; terr_idx++) {
    uint total_units = 0;
    for (auto& array : idle_unit_arrays) {
      total_units += array[terr_idx];
    }
    cache.total_player_units.ref(player_idx, terr_idx + offset) = total_units;
    cache.team_units_count.ref(team, terr_idx + offset) += total_units;
  }
}

void refresh_land_units(GameStateMemory& state, GameCache& cache, uint player_idx) {
  std::vector<LandArray> idle_unit_arrays = {
    state.idle_land_fighters.arr(player_idx),
    state.idle_land_bombers.arr(player_idx),
    state.idle_land_infantry.arr(player_idx),
    state.idle_land_artillery.arr(player_idx),
    state.idle_land_tanks.arr(player_idx),
    state.idle_land_aa_guns.arr(player_idx)
  };
  refresh_units(idle_unit_arrays, cache, player_idx, LANDS_COUNT, 0);
}

void refresh_sea_units(GameStateMemory& state, GameCache& cache, uint player_idx) {
  std::vector<SeaArray> idle_unit_arrays = {
    state.idle_sea_fighters.arr(player_idx),
    state.idle_sea_trans_empty.arr(player_idx),
    state.idle_sea_trans_1i.arr(player_idx),
    state.idle_sea_trans_1a.arr(player_idx),
    state.idle_sea_trans_1t.arr(player_idx),
    state.idle_sea_trans_2i.arr(player_idx),
    state.idle_sea_trans_1i_1a.arr(player_idx),
    state.idle_sea_trans_1i_1t.arr(player_idx),
    state.idle_sea_submarines.arr(player_idx),
    state.idle_sea_destroyers.arr(player_idx),
    state.idle_sea_carriers.arr(player_idx),
    state.idle_sea_cruisers.arr(player_idx),
    state.idle_sea_battleships.arr(player_idx),
    state.idle_sea_bs_damaged.arr(player_idx),
  };
  refresh_units(idle_unit_arrays, cache, player_idx, SEAS_COUNT, LANDS_COUNT);
}

void refresh_total_player_units(GameStateMemory& state, GameCache& cache) {
  cache.total_player_units.fill(0);
  for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
    refresh_land_units(state, cache, player_idx);
    refresh_sea_units(state, cache, player_idx);
  }
}
void refresh_canals(GameStateMemory& state, GameCache& cache) {
  cache.canal_state = 0;
  for (uint canal_idx = 0; canal_idx < CANALS_COUNT; canal_idx++) {
    if (is_allied_0[state.land_owners[CANALS[canal_idx].lands[0]]] &&
        is_allied_0[state.land_owners[CANALS[canal_idx].lands[1]]]) {
      canal_state += 1U << canal_idx;
    }
  }
  COPY_FULL_ARRAY(SEA_DIST[canal_state], sea_dist);
  COPY_FULL_ARRAY(SEA_PATH[canal_state], sea_path);
  COPY_FULL_ARRAY(SEA_PATH_ALT[canal_state], sea_path_alt);
  COPY_FULL_ARRAY(SEAS_WITHIN_1_MOVE[canal_state], seas_within_1_move_canal);
  COPY_FULL_ARRAY(SEAS_WITHIN_2_MOVES[canal_state], seas_within_2_moves_canal);
  COPY_FULL_ARRAY(SEAS_WITHIN_1_MOVE_COUNT[canal_state], seas_within_1_move_count_canal);
  COPY_FULL_ARRAY(SEAS_WITHIN_2_MOVES_COUNT[canal_state], seas_within_2_moves_count_canal);
}

void refresh_fleets(GameStateMemory& state, GameCache& cache) {
  FILL_ARRAY(enemy_destroyers_total, 0);
  FILL_ARRAY(enemy_blockade_total, 0);
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    uint air_idx = sea_idx + LANDS_COUNT;
    Seaunittypes sea_units_0 = current_player_sea_unit_types[sea_idx];
    allied_carriers[sea_idx] = sea_units_0[CARRIERS];
    for (uint player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      Seaunittypes sea_units = total_player_sea_unit_types[player_idx][sea_idx];
      if (is_allied_0[player_idx]) {
        allied_carriers[sea_idx] += sea_units[CARRIERS];
      } else {
        enemy_units_count[air_idx] += total_player_sea_units[player_idx][sea_idx];
        enemy_destroyers_total[sea_idx] += sea_units[DESTROYERS];
        enemy_blockade_total[sea_idx] += sea_units[DESTROYERS] + sea_units[CARRIERS] +
                                         sea_units[CRUISERS] + sea_units[BATTLESHIPS] +
                                         sea_units[BS_DAMAGED];
      }
    }
    transports_with_large_cargo_space[sea_idx] = sea_units_0[TRANS_EMPTY] + sea_units_0[TRANS_1I];
    transports_with_small_cargo_space[sea_idx] = sea_units_0[TRANS_EMPTY] + sea_units_0[TRANS_1I] +
                                                 sea_units_0[TRANS_1A] + sea_units_0[TRANS_1T];
  }
}
void refresh_land_path_blocked(GameStateMemory& state, GameCache& cache) {
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    uint lands_within_2_moves_count = LANDS_WITHIN_2_MOVES_COUNT[src_land];
    LandArray lands_within_2_moves = LANDS_WITHIN_2_MOVES[src_land];
    for (uint conn_idx = 0; conn_idx < lands_within_2_moves_count; conn_idx++) {
      uint dst_land = lands_within_2_moves[conn_idx];
      uint nextLandMovement = LAND_PATH[src_land][dst_land];
      uint nextLandMovementAlt = LAND_PATH_ALT[src_land][dst_land];
      is_land_path_blocked[src_land][dst_land] =
          (enemy_units_count[nextLandMovement] > 0 || *factory_max[nextLandMovement] > 0) &&
          (enemy_units_count[nextLandMovementAlt] > 0 || *factory_max[nextLandMovementAlt] > 0);
    }
  }
}
void refresh_sea_path_blocked(GameStateMemory& state, GameCache& cache) {
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    //    for (uint dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) { // todo this seems excessive
    uint seas_within_2_moves_count = SEAS_WITHIN_2_MOVES_COUNT[canal_state][src_sea];
    SeaArray seas_within_2_moves = SEAS_WITHIN_2_MOVES[canal_state][src_sea];
    for (uint conn_idx = 0; conn_idx < seas_within_2_moves_count; conn_idx++) {
      uint dst_sea = seas_within_2_moves[conn_idx];
      uint nextSeaMovement = sea_path[src_sea][dst_sea];
      uint nextSeaMovementAlt = sea_path_alt[src_sea][dst_sea];
      is_sea_path_blocked[src_sea][dst_sea] =
          enemy_blockade_total[nextSeaMovement] > 0 && enemy_blockade_total[nextSeaMovementAlt] > 0;
      is_sub_path_blocked[src_sea][dst_sea] = enemy_destroyers_total[nextSeaMovement] > 0 &&
                                              enemy_destroyers_total[nextSeaMovementAlt] > 0;
    }
  }
}