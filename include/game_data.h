#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "canal.h"
#include "land.h"
#include "player.h"
#include "sea.h"
#include "team.h"
#include "units/aa_gun.h"
#include "units/artillery.h"
#include "units/battleship.h"
#include "units/bomber.h"
#include "units/carrier.h"
#include "units/cruiser.h"
#include "units/destroyer.h"
#include "units/fighter.h"
#include "units/infantry.h"
#include "units/sub.h"
#include "units/tank.h"
#include "units/transport.h"
#include "units/units.h"

#define STRING_BUFFER_SIZE 64
#define AIRS_COUNT LANDS_COUNT + SEAS_COUNT
#define MAX_AIR_TO_AIR_CONNECTIONS 7
#define INFINITY 255

// const char* PHASES[2] = {"Combat", "Landing, Purchase"};

typedef struct {
  uint8_t owner_index; // rotates
  uint8_t builds_left; // resets
  uint8_t factory_hp;
  uint8_t factory_max;
  uint8_t bombard_max;                                     // bombarded, resets
  bool recently_conquered;                                 // resets
  uint8_t fighters[FIGHTER_STATES];                        // rotates
  uint8_t bombers[BOMBER_LAND_STATES];                     // rotates
  uint8_t infantry[INFANTRY_STATES];                       // rotates
  uint8_t artillery[ARTILLERY_STATES];                     // rotates
  uint8_t tanks[TANK_STATES];                              // rotates
  uint8_t aa_guns[AA_GUN_STATES];                          // rotates
  uint8_t other_units[PLAYERS_COUNT - 1][LAND_UNIT_TYPES]; // rotates
} LandState;

typedef struct {
  uint8_t fighters[FIGHTER_STATES];
  // 0 = done moving, 1 = 1 mov left, 2 = 2 mov left, 3 = needs staging
  uint8_t trans_empty[TRANS_EMPTY_STATES];
  // 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left, 4 = needs staging
  uint8_t trans_1i[TRANS_1I_STATES];
  uint8_t trans_1a[TRANS_1A_STATES];
  uint8_t trans_1t[TRANS_1T_STATES];
   // 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left
  uint8_t trans_2i[TRANS_2I_STATES];
  uint8_t trans_1i_1a[TRANS_1I_1A_STATES];
  uint8_t trans_1i_1t[TRANS_1I_1T_STATES];
  uint8_t submarines[SUB_STATES];
  uint8_t destroyers[DESTROYER_STATES];
  uint8_t cruisers[CRUISER_STATES];// 0=no att, 1=0 mov can bombard, 2 = 1 mov, 3 = 2 mov
  uint8_t carriers[CARRIER_STATES];
  uint8_t battleships[BATTLESHIP_STATES]; // 0=no att, 1=0 mov can bombard, 2 = 1 mov, 3 = 2 mov
  uint8_t bs_damaged[BATTLESHIP_STATES];// 0=no att, 1=0 mov can bombard, 2 = 1 mov, 3 = 2 mov
  uint8_t bombers[BOMBER_SEA_STATES]; // move remain 1,2,3,4,5
  uint8_t other_units[PLAYERS_COUNT - 1]
                     [SEA_UNIT_TYPES - 1]; // no parking bombers at sea
} UnitsSea;

typedef struct {
  uint8_t player_index;
  //uint8_t phase;
  uint8_t money[PLAYERS_COUNT];
  LandState land_state[LANDS_COUNT];
  UnitsSea units_sea[SEAS_COUNT];
  bool unresolved_combat[AIRS_COUNT]; // track retreats
} GameData;

typedef struct {
  //uint8_t units_land_type_total[LANDS_COUNT][LAND_UNIT_TYPES];
  uint8_t units_land_player_total[LANDS_COUNT][PLAYERS_COUNT];
  uint8_t units_land_grand_total[LANDS_COUNT];
  uint8_t* units_land_ptr[LANDS_COUNT][LAND_UNIT_TYPES];
  //uint8_t units_sea_type_total[SEAS_COUNT][SEA_UNIT_TYPES];
  uint8_t units_sea_player_total[SEAS_COUNT][PLAYERS_COUNT];
  uint8_t units_sea_blockade_total[SEAS_COUNT][PLAYERS_COUNT];
  uint8_t units_sea_grand_total[SEAS_COUNT];
  uint8_t* units_sea_ptr[SEAS_COUNT][SEA_UNIT_TYPES];
  uint8_t* units_air_ptr[AIRS_COUNT][AIR_UNIT_TYPES];
  uint8_t income_per_turn[PLAYERS_COUNT];
  uint8_t enemies[PLAYERS_COUNT - 1];
  uint8_t enemies_count;
  uint8_t canal_state;
  char* player_names[PLAYERS_COUNT];
  char* player_colors[PLAYERS_COUNT];
} GameCache;
void build_landMove2Destination() ;
void build_landMove1Destination();
void build_landMove1DestinationAlt();
void build_seaMove2Destination();
void build_seaMove1Destination();
void build_seaMove1DestinationAlt();
void build_total_air_distance();
void build_total_land_distance();
void build_total_sea_distance();
void build_airMove2Destination();
void build_airMove3Destination();
void build_airMove4Destination();
void build_airMove5Destination();
void build_airMove6Destination();
void build_canBomberLandHere();
void build_canBomberLandIn1Move();
void build_canBomberLandIn2Moves();
void getUserInput();
void getAIInput();
void initializeGameData();
void setPrintableStatus();
void buildCache();
void setPrintableStatusLands();
void setPrintableStatusSeas();
void stage_transport_units();
void move_transport_units();
void move_sea_units();
void move_fighter_units();
void move_bomber_units();
void move_tanks();
void move_artillery();
void move_infantry();
void resolve_sea_battles();
void unload_transports();
void bomb_factories();
void bombard_shores();
void fire_aa_guns();
void resolve_land_battles();
void land_air_units();
void move_aa_guns();
void reset_units_fully();
void buy_units();
void crash_air_units();
void collect_money();
#endif
