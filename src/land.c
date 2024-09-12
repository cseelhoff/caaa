#include "land.h"
#include "sea.h"
#include "typedefs.h"
//  PACIFIC | USA | ATLANTIC | ENG | BALTIC | GER | RUS | JAP | PAC

typedef Land Lands[LANDS_COUNT];
Lands LANDS = {"Washington",
                           4,
                           10,
                           2,
                           0,                  // owner, value, sea_con_count, land_conn_count
                           {0, 1, 0, 0},       // sea
                           {0, 0, 0, 0, 0, 0}, // land
                           "London",
                           2,
                           8,
                           2,
                           0,
                           {1, 2, 0, 0},
                           {0, 0, 0, 0, 0, 0},
                           "Berlin",
                           1,
                           10,
                           1,
                           1,
                           {2, 0, 0, 0},
                           {3, 0, 0, 0, 0, 0},
                           "Moscow",
                           0,
                           8,
                           0,
                           2,
                           {0, 0, 0, 0},
                           {2, 4, 0, 0, 0, 0},
                           "Japan",
                           3,
                           8,
                           1,
                           1,
                           {0, 0, 0, 0},
                           {3, 0, 0, 0, 0, 0}};

typedef LandArray LandArrays[LANDS_COUNT];
LandArrays LANDS_WITHIN_2_MOVES = {0};

typedef LandIndex LandIndices[LANDS_COUNT];
LandIndices LANDS_WITHIN_2_MOVES_COUNT = {0};
LandIndices LOAD_WITHIN_2_MOVES_COUNT = {0};
LandIndices LAND_TO_LAND_COUNT = {0};

typedef SeaArray L2SArrays[LANDS_COUNT];
L2SArrays LOAD_WITHIN_2_MOVES = {0};

typedef LandConnections L2LConnections[LANDS_COUNT];
L2LConnections LAND_TO_LAND_CONN = {0};

typedef SeaIndex L2SCount[LANDS_COUNT];
L2SCount LAND_TO_SEA_COUNT = {0};

typedef SeaConnections L2SConnections[LANDS_COUNT];
L2SConnections LAND_TO_SEA_CONN = {0};

typedef Dollars LandValues[LANDS_COUNT]; 
LandValues LAND_VALUE = {0};

// initializes:
// LAND_VALUE, LAND_TO_LAND_COUNT, LAND_TO_LAND_CONN, LAND_TO_SEA_COUNT, LAND_TO_SEA_CONN
void initialize_land_consts() {
#pragma unroll
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    LAND_VALUE[land_idx] = LANDS[land_idx].land_value;
    LandConnIndex land_conn_count = LANDS[land_idx].land_conn_count;
    LAND_TO_LAND_COUNT[land_idx] = land_conn_count;
    initialize_land_to_land_connections(land_idx, land_conn_count);
    SeaConnIndex sea_conn_count = LANDS[land_idx].sea_conn_count;
    LAND_TO_SEA_COUNT[land_idx] = sea_conn_count;
    initialize_land_to_sea_connections(land_idx, sea_conn_count);
  }
}

void initialize_land_to_sea_connections(LandIndex land_idx, SeaConnIndex sea_conn_count) {
#pragma unroll
  for (SeaConnIndex conn_idx = 0; conn_idx < MAX_LAND_TO_SEA_CONNECTIONS; conn_idx++) {
    if (conn_idx < sea_conn_count) {
      LAND_TO_SEA_CONN[land_idx][conn_idx] = get_land_to_sea(land_idx, conn_idx);
    }
  }
}

void initialize_land_to_land_connections(LandIndex land_idx, LandConnIndex land_conn_count) {
#pragma unroll
  for (LandConnIndex conn_idx = 0; conn_idx < MAX_LAND_TO_LAND_CONNECTIONS; conn_idx++) {
    if (conn_idx < land_conn_count) {
      LAND_TO_LAND_CONN[land_idx][conn_idx] = get_land_to_land(land_idx, conn_idx);
    }
  }
}

inline PlayerIndex get_original_owner_index(LandIndex land_idx) {
  return LANDS[land_idx].original_owner_index;
}

inline SeaIndex get_sea_conn_count(LandIndex land_idx) { return LANDS[land_idx].sea_conn_count; }

inline SeaIndex get_load_within_2_moves_count(LandIndex land_idx) {
  return LOAD_WITHIN_2_MOVES_COUNT[land_idx];
}
inline Dollars get_land_value(LandIndex land_idx) { return LAND_VALUE[land_idx]; }

inline LandConnIndex get_l2l_count(LandIndex land_idx) { return LAND_TO_LAND_COUNT[land_idx]; }

inline LandIndex get_lands_within_2_moves_count(LandIndex land_idx) {
  return LANDS_WITHIN_2_MOVES_COUNT[land_idx];
}
inline char* get_land_name(LandIndex land_idx) { return LANDS[land_idx].name; }

inline LandConnections* get_l2l_conn(LandIndex land_idx) {
  return &LAND_TO_LAND_CONN[land_idx];
}
inline LandIndex* get_l2l_count_ref(LandIndex land_idx) { return &LAND_TO_LAND_COUNT[land_idx]; }
inline LandIndex* get_lands_within_2_moves_count_ref(LandIndex land_idx) {
  return &LANDS_WITHIN_2_MOVES_COUNT[land_idx];
}
inline LandArray* get_lands_within_2_moves(LandIndex land_idx) {
  return &LANDS_WITHIN_2_MOVES[land_idx];
}

inline SeaArray* get_load_within_2_moves(LandIndex land_idx) {
  return &LOAD_WITHIN_2_MOVES[land_idx];
}

inline LandIndex* get_load_within_2_moves_count_ref(LandIndex land_idx) {
  return &LOAD_WITHIN_2_MOVES_COUNT[land_idx];
}

inline AirIndex get_land_to_sea(LandIndex land_idx, SeaConnIndex sea_conn_idx) {
  return LANDS[land_idx].sea_connections[sea_conn_idx] + LANDS_COUNT;
}
inline SeaConnIndex get_l2s_count(LandIndex land_idx) { return LAND_TO_SEA_COUNT[land_idx]; }
inline SeaConnections* get_l2s_conn(LandIndex land_idx) { return &LAND_TO_SEA_CONN[land_idx]; }

inline LandIndex get_land_to_land(LandIndex land_idx, LandConnIndex conn_idx) {
  return LANDS[land_idx].land_connections[conn_idx];
}

inline LandConnections* get_land_connections(LandIndex land_idx) {
  return &LANDS[land_idx].land_connections;
}

inline LandIndex get_land_from_conn(LandConnections* land_to_land_conn, LandConnIndex conn_idx) {
  return (*land_to_land_conn)[conn_idx];
}

inline LandIndex get_land_to_land_count(LandIndex land_idx) { return LAND_TO_LAND_COUNT[land_idx]; }
inline LandIndex* get_land_to_land_count_ref(LandIndex land_idx) {
  return &LAND_TO_LAND_COUNT[land_idx];
}

inline SeaIndex get_sea_from_conn(SeaConnections* land_to_sea_conn, SeaConnIndex conn_idx) {
  return (*land_to_sea_conn)[conn_idx];
}
