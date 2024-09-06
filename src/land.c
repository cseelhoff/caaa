#include "land.h"
#include "typedefs.h"
//  PACIFIC | USA | ATLANTIC | ENG | BALTIC | GER | RUS | JAP | PAC

Land LANDS[LANDS_COUNT] = {"Washington",
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

// static LandIndex LANDS_WITHIN_1_MOVE[LANDS_COUNT][LANDS_COUNT - 1] = {0};
// static LandIndex LANDS_WITHIN_1_MOVE_COUNT[LANDS_COUNT] = {0};
LandArray LANDS_WITHIN_2_MOVES[LANDS_COUNT] = {0};
LandIndex LANDS_WITHIN_2_MOVES_COUNT[LANDS_COUNT] = {0};
SeaConnections LOAD_WITHIN_2_MOVES[LANDS_COUNT] = {0};
LandIndex LOAD_WITHIN_2_MOVES_COUNT[LANDS_COUNT] = {0};
LandIndex LAND_TO_LAND_COUNT[LANDS_COUNT] = {0};
LandConnections LAND_TO_LAND_CONN[LANDS_COUNT] = {0};
SeaIndex LAND_TO_SEA_COUNT[LANDS_COUNT] = {0};
SeaConnections LAND_TO_SEA_CONN[LANDS_COUNT] = {0};

Dollars LAND_VALUE[LANDS_COUNT] = {0};


LandIndex get_l2l_count(LandIndex land_idx) {
  return LANDS[land_idx].land_conn_count;
}
inline LandIndex get_land_to_land(LandIndex land_idx, LandConnIndex conn_idx) {
  return LANDS[land_idx].land_connections[conn_idx];
}

inline LandConnections* get_land_connections(LandIndex land_idx) {
  return &LANDS[land_idx].land_connections;
}

SeaIndex get_sea_conn_count(LandIndex land_idx) {
  return LANDS[land_idx].sea_conn_count;
}
inline AirIndex get_land_to_sea(LandIndex land_idx, SeaConnIndex sea_conn_idx) {
  return LANDS[land_idx].sea_connections[sea_conn_idx] + LANDS_COUNT;
}

inline LandIndex get_land_from_conn(LandConnections* land_to_land_conn,
                                        LandConnIndex conn_idx) {
  return (*land_to_land_conn)[conn_idx];
}

inline Dollars get_land_value(LandIndex land_idx) { return LAND_VALUE[land_idx]; }
inline char* get_land_name(LandIndex land_idx) { return LANDS[land_idx].name; }
inline LandIndex get_land_to_land_count(LandIndex land_idx) {
  return LAND_TO_LAND_COUNT[land_idx];
}
inline LandIndex* get_land_to_land_count_ref(LandIndex land_idx) {
  return &LAND_TO_LAND_COUNT[land_idx];
}
inline LandConnections* get_l2l_conn(LandIndex land_idx) {
  return &LAND_TO_LAND_CONN[land_idx];
}
inline SeaIndex get_l2s_count(LandIndex land_idx) {
  return LAND_TO_SEA_COUNT[land_idx];
}
inline SeaConnections* get_l2s_conn(LandIndex land_idx) {
  return &LAND_TO_SEA_CONN[land_idx];
}
inline LandIndex get_lands_within_2_moves_count(LandIndex land_idx) {
  return LANDS_WITHIN_2_MOVES_COUNT[land_idx];
}
inline LandArray* get_lands_within_2_moves(LandIndex land_idx) {
  return &LANDS_WITHIN_2_MOVES[land_idx];
}

inline SeaIndex get_sea_from_l2s_conn(SeaConnections* land_to_sea_conn,
                                      SeaConnIndex conn_idx) {
  return (*land_to_sea_conn)[conn_idx];
}

void generate_land_value_array() {
  for (LandIndex land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    LAND_VALUE[land_idx] = LANDS[land_idx].land_value;
    LandIndex land_conn_count = LANDS[land_idx].land_conn_count;
    LAND_TO_LAND_COUNT[land_idx] = land_conn_count;
#pragma unroll
    for (LandConnIndex conn_idx = 0; conn_idx < MAX_LAND_TO_LAND_CONNECTIONS; conn_idx++) {
      if (conn_idx < land_conn_count) {
        LAND_TO_LAND_CONN[land_idx][conn_idx] = get_land_to_land(land_idx, conn_idx);
      }
    }
    LAND_TO_SEA_COUNT[land_idx] = LANDS[land_idx].sea_conn_count;
#pragma unroll 4 // Adjust the number based on your optimization needs
    for (SeaConnIndex conn_idx = 0; conn_idx < LAND_TO_SEA_COUNT[land_idx]; conn_idx++) {
      LAND_TO_SEA_CONN[land_idx][conn_idx] = LANDS[land_idx].sea_connections[conn_idx];
    }
  }
}
