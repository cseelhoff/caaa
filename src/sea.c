#include "sea.h"
#include "canal.h"
#include "land.h"
#include "typedefs.h"
#include "units/units.h"
//  ATLANTIC         BALTIC         ATLANTIC
//  PACIFIC | USA | ATLANTIC | ENG | BALTIC | GER | RUS | JAP | PAC
typedef Sea Seas[SEAS_COUNT];
const Seas SEAS = {"Pacific",  1, 2, {1, 0, 0, 0, 0, 0, 0}, {0, 4, 0, 0, 0, 0},
                              "Atlantic", 2, 2, {0, 2, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0},
                              "Baltic",   1, 2, {1, 0, 0, 0, 0, 0, 0}, {0, 4, 0, 0, 0, 0}};

typedef SeaConnIndex SeaConnIndices[SEAS_COUNT];
SeaConnIndices SEA_TO_SEA_COUNT = {0};

typedef SeaConnections S2SConnections[SEAS_COUNT];
S2SConnections SEA_TO_SEA_CONN = {0};

typedef LandConnections S2LConnections[SEAS_COUNT];
S2LConnections SEA_TO_LAND_CONN = {0};

typedef SeaDistancesSources SeaDistancesSourcesCanals[CANAL_STATES];
SeaDistancesSourcesCanals SEA_DIST = {{MAX_UINT8_T}};

typedef SeaMatrix SeaMatrixCanals[CANAL_STATES];
#define MAX_SEA_MOVES 2
typedef SeaMatrixCanals SeaMatrixCanalsMoves[MAX_SEA_MOVES];
SeaMatrixCanalsMoves SEAS_WITHIN_X_MOVES = {0};

typedef SeaCountsArray SeaCountsArrayCanals[CANAL_STATES];
typedef SeaCountsArrayCanals SeaCountsArrayCanalsMoves[MAX_SEA_MOVES];
SeaCountsArrayCanalsMoves SEAS_WITHIN_X_MOVES_COUNT = {0};
// SeaMatrix SEAS_WITHIN_2_MOVES[CANAL_STATES] = {0};
// SeaCountsArray SEAS_WITHIN_2_MOVES_COUNT[CANAL_STATES] = {0};
#define MIN_SEA_HOPS 1
#define MAX_SEA_HOPS 2
//#define SEA_MOVE_SIZE (1 + MAX_SEA_HOPS - MIN_SEA_HOPS)
typedef LandIndex S2LCounts[SEAS_COUNT];
S2LCounts SEA_TO_LAND_COUNT = {0};

typedef SeaMatrix SeaMatrixCanals[CANAL_STATES];
SeaMatrixCanals SEA_PATH1 = {MAX_UINT8_T};
SeaMatrixCanals SEA_PATH2 = {MAX_UINT8_T};
SeaMatrixCanals SEA_PATH1_ALT = {MAX_UINT8_T};

inline LandConnections* get_s2l_conn(SeaIndex src_sea) { return &SEA_TO_LAND_CONN[src_sea]; }
inline LandConnIndex get_s2l_count(SeaIndex src_sea) { return SEAS[src_sea].land_conn_count; }
inline char* get_sea_name(SeaIndex sea_idx) { return SEAS[sea_idx].name; }

inline SeaConnections* get_s2s_conn(SeaIndex src_sea) { return &SEA_TO_SEA_CONN[src_sea]; }

inline SeaConnIndex get_s2s_count(SeaIndex src_sea) { return SEAS[src_sea].sea_conn_count; }
inline Distance get_sea_dist(CanalState canal_state, SeaIndex src_sea, SeaIndex dst_sea) {
  return SEA_DIST[canal_state][src_sea][dst_sea];
}
inline void add_seas_within_x_moves(Distance moves, CanalState canal_state, SeaIndex src_sea,
                                    SeaIndex dst_sea) {
  SEAS_WITHIN_X_MOVES[moves][canal_state][src_sea]
                     [SEAS_WITHIN_X_MOVES_COUNT[moves][canal_state][src_sea]++] = dst_sea;
}

inline SeaIndex get_sea_path1(CanalState canal_state, SeaIndex src_sea, SeaIndex dst_sea) {
  return SEA_PATH1[canal_state][src_sea][dst_sea];
}
inline SeaIndex get_sea_path1_alt(CanalState canal_state, SeaIndex src_sea, SeaIndex dst_sea) {
  return SEA_PATH1_ALT[canal_state][src_sea][dst_sea];
}

inline LandIndex get_land_from_s2l_conn(LandConnections* sea_to_land_conn, LandConnIndex conn_idx) {
  return (*sea_to_land_conn)[conn_idx];
}

inline LandIndex get_sea_to_land_count(SeaIndex sea_idx) { return SEA_TO_LAND_COUNT[sea_idx]; }
inline LandConnections* get_sea_to_land_conn(SeaIndex sea_idx) {
  return &SEA_TO_LAND_CONN[sea_idx];
}
inline SeaIndex get_seas_within_1_move_count(CanalState canal_state, SeaIndex src_sea) {
  return SEAS_WITHIN_X_MOVES_COUNT[0][canal_state][src_sea];
}
inline SeaArray* get_seas_within_1_move(CanalState canal_state, SeaIndex src_sea) {
  return &SEAS_WITHIN_X_MOVES[0][canal_state][src_sea];
}

inline SeaArray* get_seas_within_2_moves(CanalState canal_state, SeaIndex src_sea) {
  return &SEAS_WITHIN_X_MOVES[1][canal_state][src_sea];
}

inline SeaIndex get_seas_within_2_moves_count(CanalState canal_state, SeaIndex src_sea) {
  return SEAS_WITHIN_X_MOVES_COUNT[1][canal_state][src_sea];
}

static inline void set_sea_to_sea_count(SeaIndex src_sea, SeaIndex sea_to_sea_count) {
  SEA_TO_SEA_COUNT[src_sea] = sea_to_sea_count;
}

void initialize_sea_dist(CanalState canal_idx) {
  for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (SeaIndex dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
      if (src_sea != dst_sea) {
        SEA_DIST[canal_idx][src_sea][dst_sea] = MAX_UINT8_T;
      }
    }
  }
}

inline void set_sea_dist(CanalState canal_idx, SeaIndex src_sea, SeaIndex dst_sea, Distance dist) {
  SEA_DIST[canal_idx][src_sea][dst_sea] = dist;
}

void populate_initial_distances_sea(CanalState canal_idx) {
  for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    SeaIndex sea_conn_count = get_sea_conn_count(src_sea);
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (SeaConnIndex conn_idx = 0; conn_idx < SEAS[src_sea].sea_conn_count; conn_idx++) {
      SeaIndex dst_sea = SEAS[src_sea].sea_connections[conn_idx];
      set_sea_dist(canal_idx, src_sea, dst_sea, 1);
      set_sea_dist(canal_idx, dst_sea, src_sea, 1);
    }
  }
}

void process_canals(CanalState canal_idx) {
#pragma unroll
  for (CanalState cur_canal = 0; cur_canal < CANALS_COUNT; cur_canal++) {
    if ((canal_idx & (1U << cur_canal)) == 0) {
      continue;
    }
    SEA_DIST[canal_idx][get_canal_sea(cur_canal, 0)][get_canal_sea(cur_canal, 1)] = 1;
    SEA_DIST[canal_idx][get_canal_sea(cur_canal, 1)][get_canal_sea(cur_canal, 0)] = 1;
  }
}

void floyd_warshall_sea(CanalState canal_idx) {
  for (SeaIndex sea_idx = 0; sea_idx < SEAS_COUNT; sea_idx++) {
    for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
#pragma unroll
      for (SeaIndex dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
        if (SEA_DIST[canal_idx][src_sea][sea_idx] + SEA_DIST[canal_idx][sea_idx][dst_sea] <
            SEA_DIST[canal_idx][src_sea][dst_sea]) {
          SEA_DIST[canal_idx][src_sea][dst_sea] =
              SEA_DIST[canal_idx][src_sea][sea_idx] + SEA_DIST[canal_idx][sea_idx][dst_sea];
        }
      }
    }
  }
}

// initializes directly connected SEA_DIST to 1 (starts at 255)
void initialize_sea_connections() {
  for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
    SeaIndex sea_to_sea_count = get_s2s_count(src_sea);
    set_sea_to_sea_count(src_sea, sea_to_sea_count);
    SeaConnections* sea_to_sea_conn = get_s2s_conn(src_sea);
    const Sea* sea = &SEAS[src_sea];

#pragma unroll 4 // Adjust the number based on your optimization needs
    for (SeaConnIndex sea_conn_idx = 0; sea_conn_idx < MAX_SEA_TO_SEA_CONNECTIONS; sea_conn_idx++) {
      if (sea_conn_idx < sea_to_sea_count) {
        (*sea_to_sea_conn)[sea_conn_idx] = sea->sea_connections[sea_conn_idx];
      }
    }

    uint8_t sea_to_land_count = SEAS[src_sea].land_conn_count;
    SEA_TO_LAND_COUNT[src_sea] = sea_to_land_count;
    uint8_t* sea_to_land_conn = SEA_TO_LAND_CONN[src_sea];
    sea = &SEAS[src_sea];

#pragma unroll 4 // Adjust the number based on your optimization needs
    for (int conn_idx = 0; conn_idx < MAX_SEA_TO_LAND_CONNECTIONS; conn_idx++) {
      if (conn_idx < sea_to_land_count) {
        sea_to_land_conn[conn_idx] = sea->land_connections[conn_idx];
      }
    }
  }
}

inline void set_sea_path(Distance hop, CanalState canal_state, SeaIndex src_sea, SeaIndex dst_sea,
                         SeaIndex cur_sea) {
  if (hop == 1) {
    SEA_PATH1[canal_state][src_sea][dst_sea] = cur_sea;
  } else if (hop == 2) {
    SEA_PATH2[canal_state][src_sea][dst_sea] = cur_sea;
  }
}
#define MAX_STACK_SIZE 1000

typedef struct {
  Distance hop;
  SeaIndex cur_sea;
  Distance min_dist;
} __attribute__((aligned(4))) HopState;

void generate_SeaMoveDst(Distance hop, SeaIndex src_sea, SeaIndex dst_sea, SeaIndex cur_sea,
                         Distance min_dist, CanalState canal_state) {
  HopState stack[MAX_STACK_SIZE];
  int stack_top = 0;

  stack[stack_top++] = (HopState){hop, cur_sea, min_dist};

  while (stack_top > 0) {
    HopState state = stack[--stack_top];

    if (state.hop > MAX_SEA_HOPS) {
      continue;
    }
    if (state.min_dist <= state.hop) {
      set_sea_path(state.hop - MIN_SEA_HOPS, canal_state, src_sea, dst_sea, state.cur_sea);
    }
    SeaConnIndex s2s_count = SEA_TO_SEA_COUNT[state.cur_sea];
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (SeaConnIndex conn_idx = 0; conn_idx < MAX_SEA_TO_SEA_CONNECTIONS; conn_idx++) {
      if (conn_idx >= s2s_count) {
        break;
      }
      SeaIndex next_sea = SEA_TO_SEA_CONN[state.cur_sea][conn_idx];
      Distance next_dist = get_sea_dist(canal_state, next_sea, dst_sea);
      if (next_dist < state.min_dist) {
        if (stack_top < MAX_STACK_SIZE) {
          stack[stack_top++] = (HopState){state.hop + 1, next_sea, next_dist};
        } else {
          // Handle stack overflow error
          // You can log an error message or handle it as needed
          return;
        }
      }
    }
  }
}

void generate_seaMoveAllDestination() {
  for (CanalState canal_state = 0; canal_state < CANAL_STATES; canal_state++) {
    for (SeaIndex src_sea = 0; src_sea < SEAS_COUNT; src_sea++) {
      for (SeaIndex dst_sea = 0; dst_sea < SEAS_COUNT; dst_sea++) {
        Distance min_dist = SEA_DIST[canal_state][src_sea][dst_sea];
        generate_SeaMoveDst(1, src_sea, dst_sea, src_sea, min_dist, canal_state);
        if (min_dist <= 1) {
          SEA_PATH1_ALT[canal_state][src_sea][dst_sea] = src_sea;
        }
        SeaConnIndex s2s_count_m1 = SEA_TO_SEA_COUNT[src_sea] - 1;
        #pragma unroll
        for (SeaConnIndex conn_idx2 = 0; conn_idx2 >= MAX_SEA_TO_SEA_CONNECTIONS; conn_idx2++) {
          if (conn_idx2 > s2s_count_m1) {
            break;
          }
          SeaConnIndex conn_idx = s2s_count_m1 - conn_idx2;
          SeaIndex next_sea = SEA_TO_SEA_CONN[src_sea][conn_idx2];
          SeaIndex next_dist = SEA_DIST[canal_state][next_sea][dst_sea];
        }
      }
    }
  }
}
