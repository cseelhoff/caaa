#include "air.h"
#include "land.h"
#include "sea.h"
#include "typedefs.h"
#include <string.h>

static AirIndexCount AIR_CONN_COUNT[AIRS_COUNT] = {0};
static AirToAirConnection AIR_CONNECTIONS[AIRS_COUNT] = {0};
static Distance AIR_DIST[AIRS_COUNT][AIRS_COUNT] = {0};
static AirIndexArray LAND_DIST[LANDS_COUNT] = {0};
static LandIndex LAND_PATH[LAND_MOVE_SIZE][LANDS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static LandIndex LAND_PATH_ALT[LANDS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static LandIndex land_path1[LANDS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static AirIndexMatrix AIR_WITHIN_X_MOVES[BOMBER_MOVES_MAX] = {0};
static AirIndexCount AIR_WITHIN_X_MOVES_COUNT[BOMBER_MOVES_MAX][AIRS_COUNT] = {0};
static LandIndex AIR_TO_LAND_WITHIN_X_MOVES[BOMBER_MOVES_MAX][AIRS_COUNT][LANDS_COUNT] = {0};
static LandIndexCount AIR_TO_LAND_WITHIN_X_MOVES_COUNT[BOMBER_MOVES_MAX][AIRS_COUNT] = {0};
static AirIndex AIR_PATH[AIR_MOVE_SIZE][AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static AirIndex air_path2[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static AirIndex air_path3[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static AirIndex air_path4[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static AirIndex air_path5[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static AirIndex air_path6[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};

inline AirIndexArray* get_airs_within_x_moves(Movement moves, AirIndex src_air) {
  return &AIR_WITHIN_X_MOVES[moves][src_air];
}

inline uint8_t get_airs_within_x_moves_count(Movement moves, AirIndex src_air) {
  return AIR_WITHIN_X_MOVES_COUNT[moves][src_air];
}

inline AirIndex get_air_from_array(AirIndexArray* air_array, uint8_t air_array_idx) {
  return (*air_array)[air_array_idx];
}

inline AirIndexCount get_air_conn_count(AirIndex air_idx) { return AIR_CONN_COUNT[air_idx]; }

inline AirToAirConnection* get_air_conn(AirIndex air_idx) { return &AIR_CONNECTIONS[air_idx]; }

inline AirIndex get_air_from_a2a(AirToAirConnection* air_conn, uint8_t air_conn_idx) {
  return (*air_conn)[air_conn_idx];
}

inline LandIndex get_land_path1(LandIndex src_land, AirIndex dst_air) {
  return land_path1[src_land][dst_air];
}

inline LandIndex get_land_path_alt(LandIndex src_land, AirIndex dst_air) {
  return LAND_PATH_ALT[src_land][dst_air];
}
inline AirIndex convert_sea_to_air(SeaIndex sea_idx) { return sea_idx + LANDS_COUNT; }
inline LandIndex convert_air_to_land(AirIndex air_idx) { return air_idx; }
inline SeaIndex convert_air_to_sea(AirIndex air_idx) { return air_idx - LANDS_COUNT; }
inline AirIndexArray* get_land_dist_array(LandIndex land_idx) { return &LAND_DIST[land_idx]; }
inline Distance get_land_dist(LandIndex src_land, AirIndex dst_air) { return LAND_DIST[src_land][dst_air]; }

inline Distance get_air_dist(AirIndex src_air, AirIndex dst_air) { return AIR_DIST[src_air][dst_air]; }

void generate_land_dist() {
  // Initialize the total_land_distance array
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (int dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      if (land_idx != dst_air) {
        LAND_DIST[land_idx][dst_air] = MAX_UINT8_T;
      }
    }
  }
}

void populate_initial_distances() {

  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    for (LandIndex i = 0; i < LANDS_COUNT; i++) {
#pragma unroll 4 // Adjust the number based on your optimization needs
      for (LandConnectionIndex j = 0; j < get_land_conn_count(i); j++) {
        LandIndex land_index = get_connected_land_index(i, j);
        LAND_DIST[i][land_index] = 1;
        LAND_DIST[land_index][i] = 1;
      }
      // Populate initial distances based on connected_sea_index
#pragma unroll 4 // Adjust the number based on your optimization needs
      for (SeaConnectionIndex j = 0; j < get_sea_conn_count(i); j++) {
        AirIndex air_index = get_land_to_sea(i, j);
        LAND_DIST[i][air_index] = 1;
      }
    }
  }
}

void floyd_warshall() {
  // Floyd-Warshall algorithm to compute shortest paths
  for (int land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    for (int land_idx2 = 0; land_idx2 < LANDS_COUNT; land_idx2++) {
#pragma unroll
      for (int air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {
        if (LAND_DIST[land_idx2][land_idx] + LAND_DIST[land_idx][air_idx] <
            LAND_DIST[land_idx2][air_idx]) {
          LAND_DIST[land_idx2][air_idx] =
              LAND_DIST[land_idx2][land_idx] + LAND_DIST[land_idx][air_idx];
        }
      }
    }
  }
}

void initialize_air_dist() {
  int src_air = 0;
  int dst_air = 0;
  for (src_air = 0; src_air < AIRS_COUNT; src_air++) {
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      if (src_air != dst_air) {
        AIR_DIST[src_air][dst_air] = MAX_UINT8_T;
      }
    }
  }
}

void populate_initial_distances2() {
  int conn_idx = 0;
  int dst_air = 0;
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (conn_idx = 0; conn_idx < LAND_TO_LAND_COUNT[src_land]; conn_idx++) {
      dst_air = LAND_TO_LAND_CONN[src_land][conn_idx];
      AIR_CONNECTIONS[src_land][AIR_CONN_COUNT[src_land]] = dst_air;
      AIR_CONN_COUNT[src_land]++;
      AIR_DIST[src_land][dst_air] = 1;
      AIR_DIST[dst_air][src_land] = 1;
    }
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (conn_idx = 0; conn_idx < LAND_TO_SEA_COUNT[src_land]; conn_idx++) {
      dst_air = LAND_TO_SEA_CONN[src_land][conn_idx] + LANDS_COUNT;
      AIR_CONNECTIONS[src_land][AIR_CONN_COUNT[src_land]] = dst_air;
      AIR_CONN_COUNT[src_land]++;
      AIR_DIST[src_land][dst_air] = 1;
      AIR_DIST[dst_air][src_land] = 1;
    }
  }

  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    int src_air = src_sea + LANDS_COUNT;
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (conn_idx = 0; conn_idx < SEA_TO_LAND_COUNT[src_sea]; conn_idx++) {
      dst_air = SEA_TO_LAND_CONN[src_sea][conn_idx];
      AIR_CONNECTIONS[src_air][AIR_CONN_COUNT[src_air]] = dst_air;
      AIR_CONN_COUNT[src_air]++;
      AIR_DIST[src_air][dst_air] = 1;
      AIR_DIST[dst_air][src_air] = 1;
    }
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (conn_idx = 0; conn_idx < SEA_TO_SEA_COUNT[src_sea]; conn_idx++) {
      dst_air = SEA_TO_SEA_CONN[src_sea][conn_idx] + LANDS_COUNT;
      AIR_CONNECTIONS[src_air][AIR_CONN_COUNT[src_air]] = dst_air;
      AIR_CONN_COUNT[src_air]++;
      AIR_DIST[src_air][dst_air] = 1;
      AIR_DIST[dst_air][src_air] = 1;
    }
  }
}

inline AirIndexArray* get_airs_winthin_x_moves(Movement moves, AirIndex src_air) {
  return &AIR_WITHIN_X_MOVES[moves][src_air];
}
inline AirIndexCount get_airs_winthin_x_moves_count(Movement moves, AirIndex src_air) {
  return AIR_WITHIN_X_MOVES_COUNT[moves][src_air];
}
inline AirIndex get_air_from_array(AirIndexArray* air_array, uint8_t air_array_idx) {
  return (*air_array)[air_array_idx];
}
inline char* get_air_name(AirIndex air_idx) {
  if (air_idx < LANDS_COUNT)
    return get_land_name(air_idx);
  return get_sea_name(air_idx - LANDS_COUNT);
}
void floyd_warshall_air() {
  for (int air_index = 0; air_index < AIRS_COUNT; air_index++) {
    for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
#pragma unroll
      for (int dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
        if (AIR_DIST[src_air][air_index] + AIR_DIST[air_index][dst_air] <
            AIR_DIST[src_air][dst_air]) {
          AIR_DIST[src_air][dst_air] = AIR_DIST[src_air][air_index] + AIR_DIST[air_index][dst_air];
        }
      }
    }
  }
}
void generate_LandMoveDst(int hop, int src_land, int dst_air, int cur_land, int min_dist) {
  if (hop > MAX_LAND_HOPS) {
    return;
  }

  if (min_dist <= hop)
    LAND_PATH[hop - MIN_LAND_HOPS][src_land][dst_air] = cur_land;

  for (int conn_idx = 0; conn_idx < LAND_TO_LAND_COUNT[cur_land]; conn_idx++) {
    int next_land = LAND_TO_LAND_CONN[cur_land][conn_idx];
    int next_dist = LAND_DIST[next_land][dst_air];
    if (next_dist < min_dist) {
      generate_LandMoveDst(hop + 1, src_land, dst_air, next_land, next_dist);
    }
  }
}
void generate_landMoveAllDestination() {
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (int dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      int min_dist = LAND_DIST[src_land][dst_air];
      generate_LandMoveDst(1, src_land, dst_air, src_land, min_dist);
      if (min_dist <= 1)
        LAND_PATH_ALT[src_land][dst_air] = src_land;
      for (int conn_idx = LAND_TO_LAND_COUNT[src_land]; conn_idx >= 0; conn_idx--) {
        int next_land = LAND_TO_LAND_CONN[src_land][conn_idx];
        int next_dist = LAND_DIST[next_land][dst_air];
      }
    }
  }
  memcpy(land_path1, LAND_PATH[0], sizeof(land_path1));
}

void generate_AirMoveDst(int hop, int src_air, int dst_air, int cur_air, int min_dist) {
  if (hop > MAX_AIR_HOPS)
    return;
  if (min_dist <= hop)
    AIR_PATH[hop - MIN_AIR_HOPS][src_air][dst_air] = cur_air;
  for (int conn_idx = 0; conn_idx < AIR_CONN_COUNT[cur_air]; conn_idx++) {
    int next_air = AIR_CONNECTIONS[cur_air][conn_idx];
    int next_dist = AIR_DIST[next_air][dst_air];
    if (next_dist < min_dist) {
      generate_AirMoveDst(hop + 1, src_air, dst_air, next_air, next_dist);
    }
  }
}
void generate_airMoveAllDestination() {
  for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (int dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      int min_dist = AIR_DIST[src_air][dst_air];
      generate_AirMoveDst(2, src_air, dst_air, src_air, min_dist);
    }
  }
}

void generate_within_x_moves() {
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (int dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
      if (src_land == dst_land) {
        continue;
      }
      if (LAND_DIST[src_land][dst_land] <= 1) {
        LANDS_WITHIN_1_MOVE[src_land][LANDS_WITHIN_1_MOVE_COUNT[src_land]++] = dst_land;
      }
      if (LAND_DIST[src_land][dst_land] <= 2) {
        LANDS_WITHIN_2_MOVES[src_land][LANDS_WITHIN_2_MOVES_COUNT[src_land]++] = dst_land;
      }
    }
    for (int dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
      if (LAND_DIST[src_land][dst_sea] <= 2) {
        LOAD_WITHIN_2_MOVES[src_land][LOAD_WITHIN_2_MOVES_COUNT[src_land]++] = dst_sea;
      }
    }
  }
  for (int canal_state_idx = 0; canal_state_idx < CANAL_STATES; canal_state_idx++) {
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (int dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
        if (src_sea == dst_sea) {
          continue;
        }
        if (SEA_DIST[canal_state_idx][src_sea][dst_sea] <= 1) {
          SEAS_WITHIN_1_MOVE[canal_state_idx][src_sea]
                            [SEAS_WITHIN_1_MOVE_COUNT[canal_state_idx][src_sea]++] = dst_sea;
        }
        if (SEA_DIST[canal_state_idx][src_sea][dst_sea] <= 2) {
          SEAS_WITHIN_2_MOVES[canal_state_idx][src_sea]
                             [SEAS_WITHIN_2_MOVES_COUNT[canal_state_idx][src_sea]++] = dst_sea;
        }
      }
    }
  }
  for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (int dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      if (src_air == dst_air) {
        continue;
      }
      for (int i = 0; i < 5; i++) {
        if (AIR_DIST[src_air][dst_air] <= i + 1) {
          AIR_WITHIN_X_MOVES[i][src_air][AIR_WITHIN_X_MOVES_COUNT[i][src_air]++] = dst_air;
        }
      }
      if (dst_air >= LANDS_COUNT) { // bombers can only end on land
        continue;
      }
      if (AIR_DIST[src_air][dst_air] <= 6) {
        AIR_WITHIN_X_MOVES[5][src_air][AIR_WITHIN_X_MOVES_COUNT[5][src_air]] = dst_air;
        AIR_WITHIN_X_MOVES_COUNT[5][src_air]++;
      }
    }
  }

  for (int src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (int dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
      if (src_air == dst_land) {
        continue;
      }
      for (int i = 0; i < 6; i++) {
        if (AIR_DIST[src_air][dst_land] <= i + 1) {
          int count = AIR_TO_LAND_WITHIN_X_MOVES_COUNT[i][src_air];
          AIR_TO_LAND_WITHIN_X_MOVES[i][src_air][count] = dst_land;
          (AIR_TO_LAND_WITHIN_X_MOVES_COUNT[i][src_air])++;
        }
      }
    }
  }
}