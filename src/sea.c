#include "sea.h"
#include "canal.h"
#include "typedefs.h"
//  ATLANTIC         BALTIC         ATLANTIC
//  PACIFIC | USA | ATLANTIC | ENG | BALTIC | GER | RUS | JAP | PAC

const Sea SEAS[SEAS_COUNT] = {"Pacific",  1, 2, {1, 0, 0, 0, 0, 0, 0}, {0, 4, 0, 0, 0, 0},
                              "Atlantic", 2, 2, {0, 2, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0},
                              "Baltic",   1, 2, {1, 0, 0, 0, 0, 0, 0}, {0, 4, 0, 0, 0, 0}};

static SeaIndexCount SEA_TO_SEA_COUNT[SEAS_COUNT] = {0};
static SeaToSeaConnections SEA_TO_SEA_CONN[SEAS_COUNT] = {0};
static SeaToLandConnections SEA_TO_LAND_CONN[SEAS_COUNT] = {0};

SeaDistancesSources SEA_DIST[CANAL_STATES] = {0};
SeaConnArray SEAS_WITHIN_1_MOVE[CANAL_STATES] = {0};
SeaCountsArray SEAS_WITHIN_1_MOVE_COUNT[CANAL_STATES] = {0};
SeaMatrix SEAS_WITHIN_2_MOVES[CANAL_STATES] = {0};
SeaCountsArray SEAS_WITHIN_2_MOVES_COUNT[CANAL_STATES] = {0};
//#define MIN_SEA_HOPS 1
//#define MAX_SEA_HOPS 2
//#define SEA_MOVE_SIZE (1 + MAX_SEA_HOPS - MIN_SEA_HOPS)
LandIndexCount SEA_TO_LAND_COUNT[SEAS_COUNT] = {0};
SeaMatrix SEA_PATH1[CANAL_STATES] = {MAX_UINT8_T};
SeaMatrix SEA_PATH2[CANAL_STATES] = {MAX_UINT8_T};
SeaMatrix SEA_PATH_ALT[CANAL_STATES] = {MAX_UINT8_T};

inline SeaIndexCount get_sea_to_sea_count(SeaIndex src_sea) { return SEAS[src_sea].sea_conn_count; }

static inline void set_sea_to_sea_count(SeaIndex src_sea, SeaIndexCount sea_to_sea_count) {
  SEA_TO_SEA_COUNT[src_sea] = sea_to_sea_count;
}

inline SeaToSeaConnections* get_sea_to_sea_conn(SeaIndex src_sea) {
  return &SEA_TO_SEA_CONN[src_sea];
}

void initialize_sea_dist(uint8_t canal_idx) {
  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (int dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
      if (src_sea != dst_sea) {
        SEA_DIST[canal_idx][src_sea][dst_sea] = MAX_UINT8_T;
      }
    }
  }
}

void populate_initial_distances_sea(uint8_t canal_idx) {
  for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (int conn_idx = 0; conn_idx < SEAS[src_sea].sea_conn_count; conn_idx++) {
      int dst_sea = SEAS[src_sea].connected_sea_index[conn_idx];
      SEA_DIST[canal_idx][src_sea][dst_sea] = 1;
      SEA_DIST[canal_idx][dst_sea][src_sea] = 1;
    }
  }
}

void process_canals(uint8_t canal_idx) {
#pragma unroll
  for (uint8_t cur_canal = 0; cur_canal < CANALS_COUNT; cur_canal++) {
    if ((canal_idx & (1U << cur_canal)) == 0) {
      continue;
    }
    SEA_DIST[canal_idx][CANALS[cur_canal].seas[0]][CANALS[cur_canal].seas[1]] = 1;
    SEA_DIST[canal_idx][CANALS[cur_canal].seas[1]][CANALS[cur_canal].seas[0]] = 1;
  }
}

void floyd_warshall(uint8_t canal_idx) {
  for (int sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
#pragma unroll
      for (int dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
        if (SEA_DIST[canal_idx][src_sea][sea_idx] + SEA_DIST[canal_idx][sea_idx][dst_sea] <
            SEA_DIST[canal_idx][src_sea][dst_sea]) {
          SEA_DIST[canal_idx][src_sea][dst_sea] =
              SEA_DIST[canal_idx][src_sea][sea_idx] + SEA_DIST[canal_idx][sea_idx][dst_sea];
        }
      }
    }
  }
}

void initialize_sea_connections() {
  for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    SeaIndexCount sea_to_sea_count = get_sea_to_sea_count(src_sea);
    set_sea_to_sea_count(src_sea, sea_to_sea_count);
    SeaIndex* sea_to_sea_conn = get_sea_to_sea_conn(src_sea);
    const Sea* sea = &SEAS[src_sea];

#pragma unroll 4 // Adjust the number based on your optimization needs
    for (SeaConnectionIndex sea_conn_idx = 0; sea_conn_idx < MAX_SEA_TO_SEA_CONNECTIONS;
         sea_conn_idx++) {
      if (sea_conn_idx < sea_to_sea_count) {
        sea_to_sea_conn[sea_conn_idx] = sea->connected_sea_index[sea_conn_idx];
      }
    }

    uint8_t sea_to_land_count = SEAS[src_sea].land_conn_count;
    SEA_TO_LAND_COUNT[src_sea] = sea_to_land_count;
    uint8_t* sea_to_land_conn = SEA_TO_LAND_CONN[src_sea];
    sea = &SEAS[src_sea];

#pragma unroll 4 // Adjust the number based on your optimization needs
    for (int conn_idx = 0; conn_idx < MAX_SEA_TO_LAND_CONNECTIONS; conn_idx++) {
      if (conn_idx < sea_to_land_count) {
        sea_to_land_conn[conn_idx] = sea->connected_land_index[conn_idx];
      }
    }
  }
}

void generate_SeaMoveDst(int hop, int src_sea, int dst_sea, int cur_sea, int min_dist) {
  if (hop > MAX_SEA_HOPS)
    return;
  if (min_dist <= hop)
    SEA_PATH[hop - MIN_SEA_HOPS][canal_state][src_sea][dst_sea] = cur_sea;
  for (int conn_idx = 0; conn_idx < SEA_TO_SEA_COUNT[cur_sea]; conn_idx++) {
    int next_sea = SEA_TO_SEA_CONN[cur_sea][conn_idx];
    int next_dist = SEA_DIST[canal_state][next_sea][dst_sea];
    if (next_dist < min_dist) {
      generate_SeaMoveDst(hop + 1, src_sea, dst_sea, next_sea, next_dist);
    }
  }
}

void generate_seaMoveAllDestination() {
  for (int canal_state = 0; canal_state < CANAL_STATES; canal_state++) {
    for (int src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (int dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
        int min_dist = SEA_DIST[canal_state][src_sea][dst_sea];
        generate_SeaMoveDst(1, src_sea, dst_sea, src_sea, min_dist);
        if (min_dist <= 1)
          SEA_PATH_ALT[canal_state][src_sea][dst_sea] = src_sea;
        for (int conn_idx = SEA_TO_SEA_COUNT[src_sea]; conn_idx >= 0; conn_idx--) {
          int next_sea = SEA_TO_SEA_CONN[src_sea][conn_idx];
          int next_dist = SEA_DIST[canal_state][next_sea][dst_sea];
        }
      }
    }
  }
}
