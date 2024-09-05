#include "air.h"
#include "canal.h"
#include "land.h"
#include "sea.h"
#include "typedefs.h"
#include "units/bomber.h"
#include <stdint.h>
#include <string.h>

static AirIndexCount AIR_CONN_COUNT[AIRS_COUNT] = {0};
static AirToAirConnection AIR_CONNECTIONS[AIRS_COUNT] = {0};
static Distance AIR_DIST[AIRS_COUNT][AIRS_COUNT] = {0};
static AirDistances LAND_DIST[LANDS_COUNT] = {0};
static LandIndex LAND_PATH[LAND_MOVE_SIZE][LANDS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static LandIndex LAND_PATH_ALT[LANDS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static LandIndex land_path1[LANDS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static AirIndexMatrix AIR_WITHIN_X_MOVES[BOMBER_MOVES_MAX] = {0};
static AirIndexCount AIR_WITHIN_X_MOVES_COUNT[BOMBER_MOVES_MAX][AIRS_COUNT] = {0};
static AirToLandConnection AIR_TO_LAND_WITHIN_X_MOVES[BOMBER_MOVES_MAX][AIRS_COUNT] = {0};
static LandIndexCount AIR_TO_LAND_WITHIN_X_MOVES_COUNT[BOMBER_MOVES_MAX][AIRS_COUNT] = {0};
static AirIndex AIR_PATH[AIR_MOVE_SIZE][AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static AirIndex air_path2[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static AirIndex air_path3[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static AirIndex air_path4[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static AirIndex air_path5[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
static AirIndex air_path6[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};

inline AirToLandConnection* get_air_to_land_within_x_moves(Distance moves, AirIndex src_air) {
  return &AIR_TO_LAND_WITHIN_X_MOVES[moves][src_air];
}

inline LandIndexCount get_air_to_land_within_x_moves_count(Distance moves, AirIndex src_air) {
  return AIR_TO_LAND_WITHIN_X_MOVES_COUNT[moves][src_air];
}

inline LandIndex get_land_from_a2l(AirToLandConnection* air_to_land_conn, LandConnectionIndex land_conn_idx) {
  return (*air_to_land_conn)[land_conn_idx];
}

inline AirIndexArray* get_airs_within_x_moves(Distance moves, AirIndex src_air) {
  return &AIR_WITHIN_X_MOVES[moves][src_air];
}

inline AirIndexCount get_airs_within_x_moves_count(Distance moves, AirIndex src_air) {
  return AIR_WITHIN_X_MOVES_COUNT[moves][src_air];
}

inline AirIndex get_air_from_array(AirIndexArray* air_array, AirConnectionIndex air_array_idx) {
  return (*air_array)[air_array_idx];
}

inline AirIndexCount get_air_conn_count(AirIndex air_idx) { return AIR_CONN_COUNT[air_idx]; }

inline AirToAirConnection* get_air_conn(AirIndex air_idx) { return &AIR_CONNECTIONS[air_idx]; }

inline AirIndex get_air_from_a2a(AirToAirConnection* air_conn, AirConnectionIndex air_conn_idx) {
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
inline AirDistances* get_land_to_air_dist(LandIndex land_idx) { return &LAND_DIST[land_idx]; }
inline Distance get_land_dist(LandIndex src_land, AirIndex dst_air) {
  return LAND_DIST[src_land][dst_air];
}

inline Distance get_air_dist(AirIndex src_air, AirIndex dst_air) {
  return AIR_DIST[src_air][dst_air];
}

inline AirIndex convert_land_to_air(LandIndex land_idx) { return (AirIndex)land_idx; }

inline AirIndexArray* get_airs_winthin_x_moves(Distance moves, AirIndex src_air) {
  return &AIR_WITHIN_X_MOVES[moves][src_air];
}
inline AirIndexCount get_airs_winthin_x_moves_count(Distance moves, AirIndex src_air) {
  return AIR_WITHIN_X_MOVES_COUNT[moves][src_air];
}
inline char* get_air_name(AirIndex air_idx) {
  if (air_idx < LANDS_COUNT)
    return get_land_name(air_idx);
  return get_sea_name(convert_air_to_sea(air_idx));
}

inline void set_air_distance(AirDistances* air_dist_ref, AirIndex dst_air, Distance dist) {
  (*air_dist_ref)[dst_air] = dist;
}


void generate_land_dist() {
  // Initialize the total_land_distance array
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    AirDistances* land_dist_ref = get_land_to_air_dist(land_idx);
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (AirIndex dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      if (land_idx != dst_air) {
        set_air_distance(land_dist_ref, dst_air, MAX_UINT8_T);
      }
    }
  }
}

void populate_initial_distances() {

  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    for (LandIndex land_idx2 = 0; land_idx2 < LANDS_COUNT; land_idx2++) {
      const LandToLandConnections* land_to_land_conn = get_land_connections(land_idx2);
      LandIndexCount land_conn_count = get_land_conn_count(land_idx2);
#pragma unroll 4 // Adjust the number based on your optimization needs
      for (LandConnectionIndex conn_idx = 0; conn_idx < land_conn_count; conn_idx++) {
        LandIndex land_index = get_land_from_l2l_conn(land_to_land_conn, conn_idx);
        LAND_DIST[land_idx2][land_index] = 1;
        LAND_DIST[land_index][land_idx2] = 1;
      }
      // Populate initial distances based on connected_sea_index
#pragma unroll 4 // Adjust the number based on your optimization needs
      for (SeaConnectionIndex sea_conn_idx = 0; sea_conn_idx < get_l2s_conn_count(land_idx2); sea_conn_idx++) {
        AirIndex air_index = get_land_to_sea(land_idx2, sea_conn_idx);
        LAND_DIST[land_idx2][air_index] = 1;
      }
    }
  }
}

void floyd_warshall() {
  // Floyd-Warshall algorithm to compute shortest paths
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    for (LandIndex land_idx2 = 0; land_idx2 < LANDS_COUNT; land_idx2++) {
#pragma unroll
      for (AirIndex air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {
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
  AirIndex src_air = 0;
  AirIndex dst_air = 0;
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
  LandConnectionIndex land_conn_idx = 0;
  SeaConnectionIndex sea_conn_idx = 0;
  AirIndex dst_air = 0;
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    LandToLandConnections* land_to_land_conn = get_land_to_land_conn(src_land);
    LandIndexCount land_to_land_count = get_land_to_land_count(src_land);
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (land_conn_idx = 0; land_conn_idx < land_to_land_count; land_conn_idx++) {
      dst_air = convert_land_to_air(get_land_from_l2l_conn(land_to_land_conn, land_conn_idx));
      AIR_CONNECTIONS[src_land][AIR_CONN_COUNT[src_land]] = dst_air;
      AIR_CONN_COUNT[src_land]++;
      AIR_DIST[src_land][dst_air] = 1;
      AIR_DIST[dst_air][src_land] = 1;
    }
    LandToSeaConnections* land_to_sea_conn = get_land_to_sea_conn(src_land);
    SeaIndexCount land_to_sea_count = get_land_to_sea_count(src_land);
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (sea_conn_idx = 0; sea_conn_idx < land_to_sea_count; sea_conn_idx++) {
      dst_air = convert_sea_to_air(get_sea_from_l2s_conn(land_to_sea_conn, sea_conn_idx));
      AIR_CONNECTIONS[src_land][AIR_CONN_COUNT[src_land]] = dst_air;
      AIR_CONN_COUNT[src_land]++;
      AIR_DIST[src_land][dst_air] = 1;
      AIR_DIST[dst_air][src_land] = 1;
    }
  }

  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    AirIndex src_air = convert_sea_to_air(src_sea);
    SeaToLandConnections* sea_to_land_conn = get_sea_to_land_conn(src_sea);
    SeaIndexCount sea_conn_count = get_sea_conn_count(src_sea);
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (land_conn_idx = 0; land_conn_idx < sea_conn_count; land_conn_idx++) {
      dst_air = convert_land_to_air(get_land_from_s2l_conn(sea_to_land_conn, land_conn_idx));
      AIR_CONNECTIONS[src_air][AIR_CONN_COUNT[src_air]] = dst_air;
      AIR_CONN_COUNT[src_air]++;
      AIR_DIST[src_air][dst_air] = 1;
      AIR_DIST[dst_air][src_air] = 1;
    }
    SeaToSeaConnections* sea_to_sea_conn = get_sea_to_sea_conn(src_sea);
    SeaIndexCount sea_to_sea_count = get_sea_to_sea_count(src_sea);
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (sea_conn_idx = 0; sea_conn_idx < sea_to_sea_count; sea_conn_idx++) {
      dst_air = convert_sea_to_air(get_sea_from_s2s_conn(sea_to_sea_conn, sea_conn_idx));
      AIR_CONNECTIONS[src_air][AIR_CONN_COUNT[src_air]] = dst_air;
      AIR_CONN_COUNT[src_air]++;
      AIR_DIST[src_air][dst_air] = 1;
      AIR_DIST[dst_air][src_air] = 1;
    }
  }
}

void floyd_warshall_air() {
  for (AirIndex air_index = 0; air_index < AIRS_COUNT; air_index++) {
    for (AirIndex src_air = 0; src_air < AIRS_COUNT; src_air++) {
#pragma unroll
      for (AirIndex dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
        if (AIR_DIST[src_air][air_index] + AIR_DIST[air_index][dst_air] <
            AIR_DIST[src_air][dst_air]) {
          AIR_DIST[src_air][dst_air] = AIR_DIST[src_air][air_index] + AIR_DIST[air_index][dst_air];
        }
      }
    }
  }
}
void generate_LandMoveDst(Distance hop, LandIndex src_land, AirIndex dst_air, LandIndex cur_land, Distance min_dist) {
  if (hop > MAX_LAND_HOPS) {
    return;
  }

  if (min_dist <= hop)
    LAND_PATH[hop - MIN_LAND_HOPS][src_land][dst_air] = cur_land;
  LandToLandConnections* land_to_land_conn = get_land_to_land_conn(cur_land);
  LandIndexCount land_conn_count = get_land_to_land_count(cur_land);
  for (LandConnectionIndex conn_idx = 0; conn_idx < land_conn_count; conn_idx++) {
    LandIndex next_land = get_land_from_l2l_conn(land_to_land_conn, conn_idx);
    Distance next_dist = LAND_DIST[next_land][dst_air];
    if (next_dist < min_dist) {
      generate_LandMoveDst(hop + 1, src_land, dst_air, next_land, next_dist);
    }
  }
}
void generate_landMoveAllDestination() {
  for (LandIndex src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (AirIndex dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      Distance min_dist = LAND_DIST[src_land][dst_air];
      generate_LandMoveDst(1, src_land, dst_air, src_land, min_dist);
      if (min_dist <= 1)
        LAND_PATH_ALT[src_land][dst_air] = src_land;
      LandToLandConnections* land_to_land_conn = get_land_to_land_conn(src_land);
      LandIndexCount land_conn_count = get_land_to_land_count(src_land);
      for (LandConnectionIndex conn_idx = land_conn_count; conn_idx >= 0; conn_idx--) {
        LandIndex next_land = get_land_from_l2l_conn(land_to_land_conn, conn_idx);
        Distance next_dist = LAND_DIST[next_land][dst_air];
      }
    }
  }
  memcpy(land_path1, LAND_PATH[0], sizeof(land_path1));
}

void generate_AirMoveDst(Distance hop, AirIndex src_air, AirIndex dst_air, AirIndex cur_air, Distance min_dist) {
  if (hop > MAX_AIR_HOPS)
    return;
  if (min_dist <= hop)
    AIR_PATH[hop - MIN_AIR_HOPS][src_air][dst_air] = cur_air;
  for (AirConnectionIndex conn_idx = 0; conn_idx < AIR_CONN_COUNT[cur_air]; conn_idx++) {
    AirIndex next_air = AIR_CONNECTIONS[cur_air][conn_idx];
    Distance next_dist = AIR_DIST[next_air][dst_air];
    if (next_dist < min_dist) {
      generate_AirMoveDst(hop + 1, src_air, dst_air, next_air, next_dist);
    }
  }
}
void generate_airMoveAllDestination() {
  for (AirIndex src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (AirIndex dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      Distance min_dist = AIR_DIST[src_air][dst_air];
      generate_AirMoveDst(2, src_air, dst_air, src_air, min_dist);
    }
  }
}


void generate_within_x_moves() {
  for (LandIndex src_land = 0; src_land < LANDS_COUNT; src_land++) {    
    LandIndexCount* lands_within_1_move_count = get_land_to_land_count_ref(src_land);
    LandToLandConnections* lands_within_1_move = get_land_to_land_conn(src_land);
    LandIndexCount* lands_within_2_moves_count = get_lands_within_2_moves_count_ref(src_land);
    LandIndexArray* lands_within_2_moves = get_lands_within_2_moves(src_land);
    for (LandIndex dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
      if (src_land == dst_land) {
        continue;
      }
      if (LAND_DIST[src_land][dst_land] <= 1) {
        (*lands_within_1_move)[*lands_within_1_move_count++] = dst_land;
      }
      if (LAND_DIST[src_land][dst_land] <= 2) {
        (*lands_within_2_moves)[*lands_within_2_moves_count++] = dst_land;
      }
    }
    LandIndexCount* load_within_2_moves_count = get_load_within_2_moves_count_ref(src_land);
    LandToSeaConnections* load_within_2_moves = get_load_within_2_moves(src_land);
    for (SeaIndex dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
      if (LAND_DIST[src_land][dst_sea] <= 2) {
        (*load_within_2_moves)[*load_within_2_moves_count++] = dst_sea;
      }
    }
  }
  for (CanalState canal_state_idx = 0; canal_state_idx < CANAL_STATES; canal_state_idx++) {
    for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (SeaIndex dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
        if (src_sea == dst_sea) {
          continue;
        }
        Distance sea_dist = get_sea_dist(canal_state_idx,src_sea,dst_sea);
        if (sea_dist <= 1) {
          add_seas_within_x_moves(0, canal_state_idx,  src_sea,  dst_sea);
        }
        if (sea_dist <= 2) {
          add_seas_within_x_moves(1, canal_state_idx,  src_sea,  dst_sea);
        }
      }
    }
  }
  for (AirIndex src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (AirIndex dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      if (src_air == dst_air) {
        continue;
      }
      for (Distance i = 0; i < 5; i++) {
        if (AIR_DIST[src_air][dst_air] <= i + 1) {
          AIR_WITHIN_X_MOVES[i][src_air][AIR_WITHIN_X_MOVES_COUNT[i][src_air]++] = dst_air;
        }
      }
      if (dst_air >= LANDS_COUNT) { // bombers can only end on land
        continue;
      }
      if (AIR_DIST[src_air][dst_air] <= BOMBER_MOVES_MAX) {
        AIR_WITHIN_X_MOVES[BOMBER_MOVES_MAX - 1][src_air][AIR_WITHIN_X_MOVES_COUNT[BOMBER_MOVES_MAX - 1][src_air]] = dst_air;
        AIR_WITHIN_X_MOVES_COUNT[BOMBER_MOVES_MAX - 1][src_air]++;
      }
    }
  }

  for (AirIndex src_air = 0; src_air < AIRS_COUNT; src_air++) {
    for (LandIndex dst_land = 0; dst_land < LANDS_COUNT; dst_land++) {
      if (src_air == dst_land) {
        continue;
      }
      for (Distance i = 0; i < BOMBER_MOVES_MAX; i++) {
        if (AIR_DIST[src_air][dst_land] <= i + 1) {
          LandIndexCount count = AIR_TO_LAND_WITHIN_X_MOVES_COUNT[i][src_air];
          AIR_TO_LAND_WITHIN_X_MOVES[i][src_air][count] = dst_land;
          (AIR_TO_LAND_WITHIN_X_MOVES_COUNT[i][src_air])++;
        }
      }
    }
  }
}