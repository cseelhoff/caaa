#include "air.h"
#include "canal.h"
#include "land.h"
#include "sea.h"
#include "typedefs.h"
#include "units/bomber.h"
#include <stdint.h>
#include <string.h>

AirIndex AIR_CONN_COUNT[AIRS_COUNT] = {0};
AirConnection AIR_CONNECTIONS[AIRS_COUNT] = {0};
Distance AIR_DIST[AIRS_COUNT][AIRS_COUNT] = {0};
AirDistances LAND_DIST[LANDS_COUNT] = {0};
LandIndex LAND_PATH[LAND_MOVE_SIZE][LANDS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
LandIndex LAND_PATH_ALT[LANDS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
LandIndex land_path1[LANDS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
AirIndexMatrix AIR_WITHIN_X_MOVES[BOMBER_MOVES_MAX] = {0};
AirIndex AIR_WITHIN_X_MOVES_COUNT[BOMBER_MOVES_MAX][AIRS_COUNT] = {0};
LandArray AIR_TO_LAND_WITHIN_X_MOVES[BOMBER_MOVES_MAX][AIRS_COUNT] = {0};
LandConnIndex AIR_TO_LAND_WITHIN_X_MOVES_COUNT[BOMBER_MOVES_MAX][AIRS_COUNT] = {0};
AirIndex AIR_PATH[AIR_MOVE_SIZE][AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
AirIndex air_path2[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
AirIndex air_path3[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
AirIndex air_path4[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
AirIndex air_path5[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};
AirIndex air_path6[AIRS_COUNT][AIRS_COUNT] = {MAX_UINT8_T};

inline LandIndex get_land_from_conn(LandConnections* land_connections, LandConnIndex land_conn_idx) {
  return (*land_connections)[land_conn_idx];
}

inline AirIndex convert_sea_to_air(SeaIndex sea_idx) { return sea_idx + LANDS_COUNT; }
inline AirDistances* get_land_dist_land(LandIndex land_idx) { return &LAND_DIST[land_idx]; }

inline AirIndex convert_land_to_air(LandIndex land_idx) { return (AirIndex)land_idx; }


inline char* get_land_name(LandIndex land_idx) { return LANDS[land_idx].name; }

inline char* get_sea_name(SeaIndex sea_idx) { return SEAS[sea_idx].name; }

inline void set_air_distance(AirDistances* air_dist, AirIndex dst_air, Distance dist) {
  (*air_dist)[dst_air] = dist;
}

inline AirDistances* get_land_to_air_dist(LandIndex land_idx) {
  return &LAND_DIST[land_idx];
}

void generate_land_dist() {
  // Initialize the total_land_distance array
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    AirDistances* air_dist = get_land_to_air_dist(land_idx);
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (AirIndex dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      if (land_idx != dst_air) {
        set_air_distance(air_dist, dst_air, MAX_UINT8_T);
      }
    }
  }
}

inline void set_land_dist(LandIndex src_land, AirIndex dst_air, Distance dist) {
  LAND_DIST[src_land][dst_air] = dist;
}

inline LandConnections* get_l2l_conn(LandIndex land_idx) {
  return &LANDS[land_idx].land_connections;
}

inline LandConnIndex get_l2l_count(LandIndex land_idx) {
  return LANDS[land_idx].land_conn_count;
}
inline AirIndex get_land_to_sea(LandIndex land_idx, SeaConnIndex sea_conn_idx) {
  return LANDS[land_idx].sea_connections[sea_conn_idx] + LANDS_COUNT;
}
inline SeaConnIndex get_l2s_count(LandIndex land_idx) {
  return LANDS[land_idx].sea_conn_count;
}


void populate_initial_distances() {
  for (LandIndex src_land = 0; src_land < LANDS_COUNT; src_land++) {
    for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
      LandConnections* land_to_land_conn = get_l2l_conn(land_idx);
      LandIndex land_conn_count = get_l2l_count(land_idx);
#pragma unroll 4 // Adjust the number based on your optimization needs
      for (LandConnIndex land_conn_idx = 0; land_conn_idx < land_conn_count; land_conn_idx++) {
        AirIndex dst_air = convert_land_to_air(get_land_from_conn(land_to_land_conn, land_conn_idx));
        set_land_dist(src_land, dst_air, 1);
        set_land_dist(dst_air, src_land, 1);
      }
      // Populate initial distances based on connected_sea_index
      SeaIndex sea_conn_count = get_l2s_count(land_idx);
#pragma unroll 4 // Adjust the number based on your optimization needs
      for (SeaConnIndex sea_conn_idx = 0; sea_conn_idx < sea_conn_count; sea_conn_idx++) {
        set_land_dist(src_land, get_land_to_sea(land_idx, sea_conn_idx), 1);
      }
    }
  }
}

void floyd_warshall() {
  // Floyd-Warshall algorithm to compute shortest paths
  for (LandIndex land_idx2 = 0; land_idx2 < LANDS_COUNT; land_idx2++) {
        AirDistances* land_dist_ref2 = get_land_to_air_dist(land_idx2);
    for (LandIndex land_idx1 = 0; land_idx1 < LANDS_COUNT; land_idx1++) {
        AirDistances* land_dist_ref1 = get_land_to_air_dist(land_idx1);
#pragma unroll
      for (AirIndex air_idx = 0; air_idx < AIRS_COUNT; air_idx++) {

        if ((*land_dist_ref2)[land_idx1] + (*land_dist_ref1)[air_idx] <
            (*land_dist_ref2)[air_idx]) {
          (*land_dist_ref2)[air_idx] =
              (*land_dist_ref2)[land_idx1] + (*land_dist_ref1)[air_idx];
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
  LandConnIndex land_conn_idx = 0;
  SeaConnIndex sea_conn_idx = 0;
  AirIndex dst_air = 0;
  for (int src_land = 0; src_land < LANDS_COUNT; src_land++) {
    LandConnections* land_to_land_conn = get_l2l_conn(src_land);
    LandIndex land_to_land_count = get_land_to_land_count(src_land);
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (land_conn_idx = 0; land_conn_idx < land_to_land_count; land_conn_idx++) {
      dst_air = convert_land_to_air(get_land_from_conn(land_to_land_conn, land_conn_idx));
      AIR_CONNECTIONS[src_land][AIR_CONN_COUNT[src_land]] = dst_air;
      AIR_CONN_COUNT[src_land]++;
      AIR_DIST[src_land][dst_air] = 1;
      AIR_DIST[dst_air][src_land] = 1;
    }
    SeaConnections* land_to_sea_conn = get_l2s_conn(src_land);
    SeaIndex land_to_sea_count = get_l2s_count(src_land);
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (sea_conn_idx = 0; sea_conn_idx < land_to_sea_count; sea_conn_idx++) {
      dst_air = convert_sea_to_air(get_sea_from_l2s_conn(land_to_sea_conn, sea_conn_idx));
      AIR_CONNECTIONS[src_land][AIR_CONN_COUNT[src_land]] = dst_air;
      AIR_CONN_COUNT[src_land]++;
      AIR_DIST[src_land][dst_air] = 1;
      AIR_DIST[dst_air][src_land] = 1;
    }
  }

  for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    AirIndex src_air = convert_sea_to_air(src_sea);
    LandConnections* sea_to_land_conn = get_s2l_conn(src_sea);
    SeaIndex sea_conn_count = get_s2l_count(src_sea);
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (land_conn_idx = 0; land_conn_idx < sea_conn_count; land_conn_idx++) {
      dst_air = convert_land_to_air(get_land_from_conn(sea_to_land_conn, land_conn_idx));
      AIR_CONNECTIONS[src_air][AIR_CONN_COUNT[src_air]] = dst_air;
      AIR_CONN_COUNT[src_air]++;
      AIR_DIST[src_air][dst_air] = 1;
      AIR_DIST[dst_air][src_air] = 1;
    }
    SeaConnections* sea_to_sea_conn = get_s2s_conn(src_sea);
    SeaIndex sea_to_sea_count = get_s2s_count(src_sea);
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (sea_conn_idx = 0; sea_conn_idx < sea_to_sea_count; sea_conn_idx++) {
      dst_air = convert_sea_to_air(get_sea_from_conn(sea_to_sea_conn, sea_conn_idx));
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
  LandConnections* land_to_land_conn = get_l2l_conn(cur_land);
  LandIndex land_conn_count = get_land_to_land_count(cur_land);
  for (LandConnIndex conn_idx = 0; conn_idx < land_conn_count; conn_idx++) {
    LandIndex next_land = get_land_from_conn(land_to_land_conn, conn_idx);
    Distance next_dist = LAND_DIST[next_land][dst_air];
    if (next_dist < min_dist) {
      generate_LandMoveDst(hop + 1, src_land, dst_air, next_land, next_dist);
    }
  }
}
void generate_AirMoveDst(Distance hop, AirIndex src_air, AirIndex dst_air, AirIndex cur_air, Distance min_dist) {
  if (hop > MAX_AIR_HOPS)
    return;
  if (min_dist <= hop)
    AIR_PATH[hop - MIN_AIR_HOPS][src_air][dst_air] = cur_air;
  for (AirConnIndex conn_idx = 0; conn_idx < AIR_CONN_COUNT[cur_air]; conn_idx++) {
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
    LandIndex* lands_within_1_move_count = get_land_to_land_count_ref(src_land);
    LandConnections* lands_within_1_move = get_l2l_conn(src_land);
    LandIndex* lands_within_2_moves_count = get_lands_within_2_moves_count_ref(src_land);
    LandArray* lands_within_2_moves = get_lands_within_2_moves(src_land);
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
    LandIndex* load_within_2_moves_count = get_load_within_2_moves_count_ref(src_land);
    SeaConnections* load_within_2_moves = get_load_within_2_moves(src_land);
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
          LandIndex count = AIR_TO_LAND_WITHIN_X_MOVES_COUNT[i][src_air];
          AIR_TO_LAND_WITHIN_X_MOVES[i][src_air][count] = dst_land;
          (AIR_TO_LAND_WITHIN_X_MOVES_COUNT[i][src_air])++;
        }
      }
    }
  }
}

void generate_landMoveAllDestination() {
  for (LandIndex src_land = 0; src_land < LANDS_COUNT; src_land++) {
    AirDistances* air_dist = get_land_dist_land(src_land);
    for (AirIndex dst_air = 0; dst_air < AIRS_COUNT; dst_air++) {
      Distance min_dist = get_air_distance(air_dist, dst_air);
      generate_LandMoveDst(1, src_land, dst_air, src_land, min_dist);
      if (min_dist <= 1)
        LAND_PATH_ALT[src_land][dst_air] = src_land;
      LandConnections* land_to_land_conn = get_l2l_conn(src_land);
      LandIndex land_conn_count = get_land_to_land_count(src_land);
      for (LandConnIndex conn_idx = land_conn_count; conn_idx >= 0; conn_idx--) {
        LandIndex next_land = get_land_from_conn(land_to_land_conn, conn_idx);
        Distance next_dist = LAND_DIST[next_land][dst_air];
      }
    }
  }  
  memcpy(land_path1, LAND_PATH[0], sizeof(land_path1));
}
