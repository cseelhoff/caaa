#include "map_cache.hpp"
#include "array_functions.hpp"
#include "game_state_memory.hpp"
#include "player.hpp"

LandArray LAND_VALUE = {0};
LandAirArray LAND_DIST = {{{0}}};
AirArray AIR_CONN_COUNT = {0};
AirA2AArray AIR_CONNECTIONS = {{{0}}};
AirAirArray AIR_DIST = {{{0}}};
LandAirArray LAND_PATH = {{{MAX_INT}}};
LandAirArray LAND_PATH_ALT = {{{MAX_INT}}};
LandLandArray LANDS_WITHIN_2_MOVES;
LandArray LANDS_WITHIN_2_MOVES_COUNT = {0};
LandSeaArray LOAD_WITHIN_2_MOVES = {{{0}}};
LandArray LOAD_WITHIN_2_MOVES_COUNT = {0};
CanalSeaSeaArray SEAS_WITHIN_1_MOVE = {{{{{0}}}}};
CanalSeaArray SEAS_WITHIN_1_MOVE_COUNT = {{{0}}};
CanalSeaSeaArray SEAS_WITHIN_2_MOVES = {{{{{0}}}}};
CanalSeaArray SEAS_WITHIN_2_MOVES_COUNT = {{{0}}};
AirhopAirAirArray AIR_WITHIN_X_MOVES = {{{{{0}}}}};
AirhopAirArray AIR_WITHIN_X_MOVES_COUNT = {{{0}}};
AirhopAirLandArray AIR_TO_LAND_WITHIN_X_MOVES = {{{{0}}}};
AirhopLandArray AIR_TO_LAND_WITHIN_X_MOVES_COUNT = {{{0}}};
CanalSeaSeaArray SEA_DIST = {{{{{0}}}}};
CanalSeaSeaArray SEA_PATH = {{{{{MAX_INT}}}}};
CanalSeaSeaArray SEA_PATH_ALT = {{{{{MAX_INT}}}}};
LandArray LAND_TO_LAND_COUNT = {0};
LandL2LArray LAND_TO_LAND_CONN = {{{0}}};
LandArray LAND_TO_SEA_COUNT = {0};
LandL2SArray LAND_TO_SEA_CONN = {{{0}}};
SeaArray SEA_TO_SEA_COUNT = {0};
SeaS2SArray SEA_TO_SEA_CONN = {{{0}}};
SeaArray SEA_TO_LAND_COUNT = {0};
SeaS2LArray SEA_TO_LAND_CONN = {{{0}}};
FightermovesAirAirArray AIRS_X_TO_4_MOVES_AWAY = {{{{{0}}}}};
FightermovesAirArray AIRS_X_TO_4_MOVES_AWAY_COUNT = {{{0}}};
PlayerArray PLAYER_TEAM = {{0}};

void initialize_constants() {
  initialize_enemies();
  initialize_land_dist();
  land_dist_floyd_warshall();
  initialize_sea_dist();
  initialize_air_dist();
  initialize_land_path();
  initialize_sea_path();
  initialize_within_x_moves();
  intialize_airs_x_to_4_moves_away();
  initialize_skip_4air_precals();
}
void initialize_enemies() {
  for (uint player = 0; player < PLAYERS_COUNT; player++) {
    PLAYER_TEAM[player] = PLAYERS[player].team;
    for (uint enemy = 0; enemy < PLAYERS_COUNT; enemy++) {
      if (!PLAYERS[player].is_allied[enemy]) {
        ENEMIES[player].push_back(enemy);
      }
    }
  }
}
void initialize_land_dist() {
  FILL_2D_ARRAY(LAND_DIST, MAX_INT)
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    LAND_VALUE[src_land] = LANDS[src_land].land_value;
    Land land = LANDS[src_land];
    // initialize LAND_TO_LAND_CONN
    uint land_conn_count = land.land_conn_count;
    LAND_TO_LAND_COUNT[src_land] = land_conn_count;
    COPY_SUB_ARRAY(land.land_conns, LAND_TO_LAND_CONN[src_land], LAND_TO_LAND_COUNT[src_land]);
    // set_l2l_land_dist_to_one
    for (uint conn_idx = 0; conn_idx < land_conn_count; conn_idx++) {
      uint dst_land = land.land_conns[conn_idx];
      LAND_DIST[src_land][dst_land] = 1;
      LAND_DIST[dst_land][src_land] = 1;
    }
    // initialize LAND_TO_SEA_CONN
    uint sea_conn_count = land.sea_conn_count;
    LAND_TO_SEA_COUNT[src_land] = sea_conn_count;
    COPY_SUB_ARRAY(land.sea_conns, LAND_TO_SEA_CONN[src_land], LAND_TO_SEA_COUNT[src_land]);
    LAND_DIST[src_land][src_land] = 0;
    // set_l2s_land_dist_to_one
    for (uint conn_idx = 0; conn_idx < sea_conn_count; conn_idx++) {
      uint dst_air = land.sea_conns[conn_idx] + LANDS_COUNT;
      LAND_DIST[src_land][dst_air] = 1;
    }
  }
}

void land_dist_floyd_warshall() {
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
      for (uint air_dst = 0; air_dst < AIRS_COUNT; air_dst++) {
        uint new_dist = LAND_DIST[src_land][land_idx] + LAND_DIST[land_idx][air_dst];
        if (new_dist < LAND_DIST[src_land][air_dst]) {
          LAND_DIST[src_land][air_dst] = new_dist;
        }
      }
    }
  }
}

void initialize_sea_dist() {
  FILL_3D_ARRAY(SEA_DIST, MAX_INT);
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    initialize_s2s_connections(src_sea);
    initialize_s2l_connections(src_sea);
  }
  for (uint canal_idx = 0; canal_idx < CANAL_STATES; canal_idx++) {
    initialize_sea_dist_zero(canal_idx);
    set_s2s_sea_dist_to_one(canal_idx);
    initialize_canals(canal_idx);
    sea_dist_floyd_warshall(canal_idx);
  }
}
void initialize_s2s_connections(uint src_sea) {
  SEA_TO_SEA_COUNT[src_sea] = SEAS[src_sea].sea_conn_count;
  for (uint conn_idx = 0; conn_idx < SEA_TO_SEA_COUNT[src_sea]; conn_idx++) {
    SEA_TO_SEA_CONN[src_sea][conn_idx] = SEAS[src_sea].sea_conns[conn_idx];
  }
}
void initialize_s2l_connections(uint src_sea) {
  SEA_TO_LAND_COUNT[src_sea] = SEAS[src_sea].land_conn_count;
  for (uint conn_idx = 0; conn_idx < SEA_TO_LAND_COUNT[src_sea]; conn_idx++) {
    SEA_TO_LAND_CONN[src_sea][conn_idx] = SEAS[src_sea].land_conns[conn_idx];
  }
}
void initialize_sea_dist_zero(uint canal_idx) {
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    SEA_DIST[canal_idx][src_sea][src_sea] = 0;
  }
}
void set_s2s_sea_dist_to_one(uint canal_idx) {
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    for (uint conn_idx = 0; conn_idx < SEAS[src_sea].sea_conn_count; conn_idx++) {
      uint dst_sea = SEAS[src_sea].sea_conns[conn_idx];
      SEA_DIST[canal_idx][src_sea][dst_sea] = 1;
      SEA_DIST[canal_idx][dst_sea][src_sea] = 1;
    }
  }
}
void initialize_canals(uint canal_idx) {
  // convert canal_state to a bitmask and loop through CANALS for those
  // enabled for example if canal_state is 0, do not process any items in
  // CANALS, if canal_state is 1, process the first item in CANALS, if
  // canal_state is 2, process the second item in CANALS, if canal_state is
  // 3, process the first and second items in CANALS, etc.
  for (uint conn_idx = 0; conn_idx < CANALS_COUNT; conn_idx++) {
    if ((canal_idx & (1U << conn_idx)) == 0) {
      continue;
    }
    TwoSeas seas = CANALS[conn_idx].seas;
    SEA_DIST[canal_idx][seas[0]][seas[1]] = 1;
    SEA_DIST[canal_idx][seas[1]][seas[0]] = 1;
  }
}
void sea_dist_floyd_warshall(uint canal_idx) {
  for (uint sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (uint dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
        uint new_dist =
            SEA_DIST[canal_idx][src_sea][sea_idx] + SEA_DIST[canal_idx][sea_idx][dst_sea];
        if (new_dist < SEA_DIST[canal_idx][src_sea][dst_sea]) {
          SEA_DIST[canal_idx][src_sea][dst_sea] = new_dist;
        }
      }
    }
  }
}
void initialize_air_dist() {
  FILL_2D_ARRAY(AIR_DIST, MAX_INT)
  initialize_air_dist_zero();
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    set_l2l_air_dist_to_one(src_land);
    set_l2s_air_dist_to_one(src_land);
  }
  for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    set_s2l_air_dist_to_one(src_sea);
    set_s2s_air_dist_to_one(src_sea);
  }
  air_dist_floyd_warshall();
}
void initialize_air_dist_zero() {
  for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
    AIR_DIST[src_air][src_air] = 0;
  }
}
void set_l2l_air_dist_to_one(uint src_land) {
  L2LConn l2l_conn = LAND_TO_LAND_CONN[src_land];
  for (uint conn_idx = 0; conn_idx < LAND_TO_LAND_COUNT[src_land]; conn_idx++) {
    uint dst_air = l2l_conn[conn_idx];
    AIR_CONNECTIONS[src_land][AIR_CONN_COUNT[src_land]] = dst_air;
    AIR_CONN_COUNT[src_land]++;
    AIR_DIST[src_land][dst_air] = 1;
    AIR_DIST[dst_air][src_land] = 1;
  }
}
void set_l2s_air_dist_to_one(uint src_land) {
  for (uint conn_idx = 0; conn_idx < LAND_TO_SEA_COUNT[src_land]; conn_idx++) {
    uint dst_air = LAND_TO_SEA_CONN[src_land][conn_idx] + LANDS_COUNT;
    AIR_CONNECTIONS[src_land][AIR_CONN_COUNT[src_land]] = dst_air;
    AIR_CONN_COUNT[src_land]++;
    AIR_DIST[src_land][dst_air] = 1;
    AIR_DIST[dst_air][src_land] = 1;
  }
}
void set_s2l_air_dist_to_one(uint src_sea) {
  uint src_air = src_sea + LANDS_COUNT;
  for (uint conn_idx = 0; conn_idx < SEA_TO_LAND_COUNT[src_sea]; conn_idx++) {
    uint dst_air = SEA_TO_LAND_CONN[src_sea][conn_idx];
    AIR_CONNECTIONS[src_air][AIR_CONN_COUNT[src_air]] = dst_air;
    AIR_CONN_COUNT[src_air]++;
    AIR_DIST[src_air][dst_air] = 1;
    AIR_DIST[dst_air][src_air] = 1;
  }
}
void set_s2s_air_dist_to_one(uint src_sea) {
  uint src_air = src_sea + LANDS_COUNT;
  for (uint conn_idx = 0; conn_idx < SEA_TO_SEA_COUNT[src_sea]; conn_idx++) {
    uint dst_air = SEA_TO_SEA_CONN[src_sea][conn_idx] + LANDS_COUNT;
    AIR_CONNECTIONS[src_air][AIR_CONN_COUNT[src_air]] = dst_air;
    AIR_CONN_COUNT[src_air]++;
    AIR_DIST[src_air][dst_air] = 1;
    AIR_DIST[dst_air][src_air] = 1;
  }
}
void air_dist_floyd_warshall() {
  for (uint air_index = 0; air_index < AIRS_COUNT; air_index++) {
    for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
      for (uint dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
        uint new_dist = AIR_DIST[src_air][air_index] + AIR_DIST[air_index][dst_air];
        if (new_dist < AIR_DIST[src_air][dst_air]) {
          AIR_DIST[src_air][dst_air] = new_dist;
        }
      }
    }
  }
}
void initialize_land_path() {
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (uint conn_idx = 0; conn_idx < LAND_TO_LAND_COUNT[src_land]; conn_idx++) {
      uint mid_land = LAND_TO_LAND_CONN[src_land][conn_idx];
      set_land_path_for_l2l(src_land, mid_land, LAND_PATH);
      set_land_path_for_l2s(src_land, mid_land, LAND_PATH);
      // Iterate the loop in reverse order for alt path
      uint mid_land_alt = LAND_TO_LAND_CONN[src_land][LAND_TO_LAND_COUNT[src_land] - 1 - conn_idx];
      set_land_path_for_l2l(src_land, mid_land_alt, LAND_PATH_ALT);
      set_land_path_for_l2s(src_land, mid_land_alt, LAND_PATH_ALT);
    }
  }
}
void set_land_path_for_l2l(uint src_land, uint intermediate_land, LandAirArray land_path) {
  for (uint conn_idx2 = 0; conn_idx2 < LAND_TO_LAND_COUNT[intermediate_land]; conn_idx2++) {
    uint dst_land2 = LAND_TO_LAND_CONN[intermediate_land][conn_idx2];
    if (LAND_DIST[src_land][dst_land2] == 2) {
      land_path[src_land][dst_land2] = intermediate_land;
    }
  }
}
void set_land_path_for_l2s(uint src_land, uint intermediate_land, LandAirArray land_path) {
  for (uint conn_idx2 = 0; conn_idx2 < LAND_TO_SEA_COUNT[intermediate_land]; conn_idx2++) {
    uint dst_air = LAND_TO_SEA_CONN[intermediate_land][conn_idx2] + LANDS_COUNT;
    if (LAND_DIST[src_land][dst_air] == 2) {
      land_path[src_land][dst_air] = intermediate_land;
    }
  }
}
void initialize_sea_path() {
  for (uint canal_state_idx = 0; canal_state_idx < CANAL_STATES; canal_state_idx++) {
    for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (uint conn_idx = 0; conn_idx < SEA_TO_SEA_COUNT[src_sea]; conn_idx++) {
        initialize_sea_path_2(conn_idx, src_sea, canal_state_idx, &SEA_PATH);
        initialize_sea_path_2(SEA_TO_SEA_COUNT[src_sea] - 1 - conn_idx, src_sea, canal_state_idx,
                              &SEA_PATH_ALT);
      }
    }
  }
}

void initialize_sea_path_2(uint mid_sea_idx, uint src_sea, uint canal_state_idx,
                           CanalSeaSeaArray* sea_path_ptr) {
  uint mid_sea = SEA_TO_SEA_CONN[src_sea][mid_sea_idx];
  for (uint conn_idx2 = 0; conn_idx2 < SEA_TO_SEA_COUNT[mid_sea]; conn_idx2++) {
    uint dst_sea = SEA_TO_SEA_CONN[mid_sea][conn_idx2];
    if (SEA_DIST[canal_state_idx][src_sea][dst_sea] == 2) {
      (*sea_path_ptr)[canal_state_idx][src_sea][dst_sea] = mid_sea;
    }
  }
}

void initialize_within_x_moves() {
  for (uint src_land = 0; src_land < LANDS_COUNT; src_land++) {
    initialize_land_within_2_moves(src_land);
    initialize_load_within_2_moves(src_land);
  }
  initialize_sea_within_x_moves();
  initialize_air_within_x_moves();
  initialize_air_to_land_within_x_moves();
}
void initialize_land_within_2_moves(uint src_land) {
  for (uint dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
    if (src_land == dst_land) {
      continue;
    }
    if (LAND_DIST[src_land][dst_land] <= 2) {
      LANDS_WITHIN_2_MOVES[src_land][LANDS_WITHIN_2_MOVES_COUNT[src_land]++] = dst_land;
    }
  }
}
void initialize_load_within_2_moves(uint src_land) {
  for (uint dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
    if (LAND_DIST[src_land][dst_sea + LANDS_COUNT] <= 2) {
      LOAD_WITHIN_2_MOVES[src_land][LOAD_WITHIN_2_MOVES_COUNT[src_land]++] = dst_sea;
    }
  }
}
void initialize_sea_within_x_moves() {
  for (uint canal_state_idx = 0; canal_state_idx < CANAL_STATES; canal_state_idx++) {
    for (uint src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (uint dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
        if (src_sea == dst_sea) {
          continue;
        }
        if (SEA_DIST[canal_state_idx][src_sea][dst_sea] <= 2) {
          SEAS_WITHIN_2_MOVES[canal_state_idx][src_sea]
                             [SEAS_WITHIN_2_MOVES_COUNT[canal_state_idx][src_sea]++] = dst_sea;
        } else {
          continue;
        }
        if (SEA_DIST[canal_state_idx][src_sea][dst_sea] <= 1) {
          SEAS_WITHIN_1_MOVE[canal_state_idx][src_sea]
                            [SEAS_WITHIN_1_MOVE_COUNT[canal_state_idx][src_sea]++] = dst_sea;
        }
      }
    }
  }
}
void initialize_air_within_x_moves() {
  for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (uint dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      if (src_air == dst_air) {
        continue;
      }
      for (uint i = 0; i < (MAX_AIR_HOPS - 1); i++) {
        if (AIR_DIST[src_air][dst_air] <= i + 1) {
          AIR_WITHIN_X_MOVES[i][src_air][AIR_WITHIN_X_MOVES_COUNT[i][src_air]++] = dst_air;
        }
      }
      if (dst_air >= LANDS_COUNT) { // bombers can only end on land
        continue;
      }
      if (AIR_DIST[src_air][dst_air] <= MAX_AIR_HOPS) {
        AIR_WITHIN_X_MOVES[MAX_AIR_HOPS - 1][src_air]
                          [AIR_WITHIN_X_MOVES_COUNT[MAX_AIR_HOPS - 1][src_air]] = dst_air;
        AIR_WITHIN_X_MOVES_COUNT[MAX_AIR_HOPS - 1][src_air]++;
      }
    }
  }
}
void initialize_air_to_land_within_x_moves() {
  for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (uint dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
      if (src_air == dst_land) {
        continue;
      }
      for (uint i = 0; i < MAX_AIR_HOPS; i++) {
        if (AIR_DIST[src_air][dst_land] <= i + 1) {
          uint count = AIR_TO_LAND_WITHIN_X_MOVES_COUNT[i][src_air];
          AIR_TO_LAND_WITHIN_X_MOVES[i][src_air][count] = dst_land;
          (AIR_TO_LAND_WITHIN_X_MOVES_COUNT[i][src_air])++;
        }
      }
    }
  }
}

void intialize_airs_x_to_4_moves_away() {
  for (uint moves = 1; moves <= 4; moves++) {
    for (uint src_air = 0; src_air < AIRS_COUNT; src_air++) {
      AirArray air_within_x_moves = AIR_WITHIN_X_MOVES[4 - 1][src_air];
      uint air_within_x_moves_count = AIR_WITHIN_X_MOVES_COUNT[4 - 1][src_air];
      for (uint dst_air_idx = 0; dst_air_idx < air_within_x_moves_count; dst_air_idx++) {
        uint dst_air = air_within_x_moves[dst_air_idx];
        uint dist = AIR_DIST[src_air][dst_air];
        if (dist >= moves) {
          AIRS_X_TO_4_MOVES_AWAY[moves - 1][dst_air]
                                [AIRS_X_TO_4_MOVES_AWAY_COUNT[moves - 1][dst_air_idx]++] = src_air;
        }
      }
    }
  }
}
