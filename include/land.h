#pragma once
#include "canal.h"
#include "sea.h"
#include "typedefs.h"
#include <stdint.h>

#define LANDS_COUNT 5
#define MAX_LAND_TO_SEA_CONNECTIONS 4 // land to sea within 2 moves
#define MAX_LAND_TO_LAND_CONNECTIONS 6
#define MIN_LAND_HOPS 1
#define MAX_LAND_HOPS 2
#define LAND_MOVE_SIZE (1 + MAX_LAND_HOPS - MIN_LAND_HOPS)

typedef LandIndex LandArray[LANDS_COUNT];

typedef struct {
  char* name;
  PlayerIndex original_owner_index;
  Dollars land_value;
  SeaConnIndex sea_conn_count;
  LandConnIndex land_conn_count;
  SeaConnections sea_connections;
  LandConnections land_connections;
} __attribute__((aligned(ALIGNMENT_32))) Land;

void generate_land_value_array();
LandIndex get_land_to_land(LandIndex land_idx, LandConnIndex conn_idx);
LandIndex* get_l2l_count_ref(LandIndex land_idx);
LandConnIndex get_l2l_count(LandIndex land_idx);
char* get_land_name(LandIndex land_idx);
LandConnections* get_l2l_conn(LandIndex land_idx);
SeaConnIndex get_l2s_count(LandIndex land_idx);
SeaConnections* get_l2s_conn(LandIndex land_idx);
AirIndex get_land_to_sea(LandIndex land_idx, SeaConnIndex sea_conn_idx);
LandIndex* get_lands_within_2_moves_count_ref(LandIndex land_idx);
LandArray* get_lands_within_2_moves(LandIndex land_idx);
LandIndex* get_load_within_2_moves_count_ref(LandIndex land_idx);
SeaIndex get_load_within_2_moves_count(LandIndex land_idx);
SeaConnections* get_load_within_2_moves(LandIndex land_idx);
Dollars get_land_value(LandIndex land_idx);
LandIndex get_lands_within_2_moves_count(LandIndex land_idx);
PlayerIndex get_original_owner_index(LandIndex land_idx);
