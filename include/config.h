#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#define LANDS_COUNT 5
#define MAX_CARGO_COUNT 2
#define MAX_LAND_TO_LAND_CONNECTIONS 6
#define MAX_LAND_TO_SEA_CONNECTIONS 4
#define MAX_SEA_TO_LAND_CONNECTIONS 6
#define MAX_SEA_TO_SEA_CONNECTIONS 7
#define MAX_TEAM_SIZE 3
#define PLAYERS_COUNT 5
#define SEAS_COUNT 3
#define TEAMS_COUNT 2

#define SEA_UNIT_TYPES 14
#define FIGHTERS 0
#define TRANS_EMPTY 1
#define TRANS_1I 2
#define TRANS_1A 3
#define TRANS_1T 4
#define TRANS_2I 5
#define TRANS_1I_1A 6
#define TRANS_1I_1T 7
#define SUBMARINES 8
#define DESTROYERS 9
#define CARRIERS 10
#define BATTLESHIPS 11
#define BS_DAMAGED 12
#define BOMBERS_SEA 13

#define LAND_UNIT_TYPES 6
#define BOMBERS_LAND 1
#define INFANTRY 2
#define ARTILLERY 3
#define TANKS 4
#define AA_GUNS 5

#define STRING_BUFFER_SIZE 64

extern const char* LAND_NAMES[LANDS_COUNT];
extern const char* SEA_NAMES[SEAS_COUNT];
extern const char* PHASES[2];
extern const char* PLAYER_COLORS[PLAYERS_COUNT];
extern const char* PLAYER_NAMES[PLAYERS_COUNT];
extern const char* TEAM_NAMES[TEAMS_COUNT];
const char* NAMES_UNIT_LAND[LAND_UNIT_TYPES] = {
    "fighters", "bombers", "infantry", "artillery", "tanks", "aa_guns"};
const char* SEA_UNIT_NAMES[SEA_UNIT_TYPES] = {
    "Fighters   ", "Trans Empty",  "Trans 1 Inf",  "Trans 1 Art", "Trans 1 Tnk",
    "Trans 2 Inf", "Trans 1I 1A ", "Trans 1I 1T",  "Submarines ", "Destroyers ",
    "Carriers   ", "Battleships",  "BS Damaged  ", "Bombers   "};
extern const uint8_t LAND_TO_LAND_CONNECTIONS[LANDS_COUNT]
                                             [MAX_LAND_TO_LAND_CONNECTIONS];
extern const uint8_t LAND_TO_SEA_CONNECTIONS[LANDS_COUNT]
                                            [MAX_LAND_TO_SEA_CONNECTIONS];
extern const uint8_t SEA_TO_LAND_CONNECTIONS[SEAS_COUNT]
                                            [MAX_SEA_TO_LAND_CONNECTIONS];
extern const uint8_t SEA_TO_SEA_CONNECTIONS[SEAS_COUNT]
                                           [MAX_SEA_TO_SEA_CONNECTIONS];
extern const uint8_t TEAM_PLAYERS[TEAMS_COUNT][MAX_TEAM_SIZE];
extern const uint8_t PLAYER_TEAMS[PLAYERS_COUNT];
const uint8_t STATES_MOVE_LAND[LAND_UNIT_TYPES] = {5, 7, 2, 2, 3, 2};
const uint8_t MAX_MOVE_LAND[LAND_UNIT_TYPES] = {4, 6, 1, 1, 2, 1};


const uint8_t MAX_MOVE_SEA[SEA_UNIT_TYPES] = {4, 2, 2, 2, 2, 2, 2,
                                        2, 2, 2, 2, 2, 2, 6};
const uint8_t STATES_MOVE_SEA[SEA_UNIT_TYPES] = {5, 4, 4, 4, 4, 3, 3,
                                           3, 3, 3, 3, 3, 3, 6};

const uint8_t seaMove2Destination[SEAS_COUNT][SEAS_COUNT] = {0};
const uint8_t seaMove1Destination[SEAS_COUNT][SEAS_COUNT] = {0};
const uint8_t seaMove1DestinationAlt[SEAS_COUNT][SEAS_COUNT] = {0};
const uint8_t seaDistanceMap[SEAS_COUNT][SEAS_COUNT] = {0};
const char* PHASES[2] = {"Combat", "Landing, Purchase"};

typedef struct {
  uint8_t owner_index; // rotates
  uint8_t builds_left;
  uint8_t factory_hp;
  uint8_t factory_max;
  bool conquered;
  uint8_t fighters[5];
  uint8_t bombers[7];
  uint8_t infantry[2];
  uint8_t artillery[2];
  uint8_t tanks[3];
  uint8_t aa_guns[2];
  uint8_t other_units[PLAYERS_COUNT - 1][LAND_UNIT_TYPES];
} LandState;

typedef struct {
  uint8_t fighters[5];
  uint8_t trans_empty[4]; // move remain 0,1,2,s
  uint8_t trans_1i[4];
  uint8_t trans_1a[4];
  uint8_t trans_1t[4];
  uint8_t trans_2i[3]; // move remain 0,1,2
  uint8_t trans_1i_1a[3];
  uint8_t trans_1i_1t[3];
  uint8_t submarines[3];
  uint8_t destroyers[3];
  uint8_t carriers[3];
  uint8_t battleships[3];
  uint8_t bs_damaged[3];
  uint8_t bombers[6]; //move remain 1,2,3,4,5 (also including 0 for easier coding)
  uint8_t other_units[PLAYERS_COUNT - 1][SEA_UNIT_TYPES - 1]; //no parking bombers at sea
} UnitsSea;

typedef struct {
  uint8_t player_index;
  uint8_t phase;
  uint8_t money[PLAYERS_COUNT];
  LandState land_state[LANDS_COUNT];
  UnitsSea units_sea[SEAS_COUNT];
} GameData;

typedef struct {
  uint8_t units_land_type_total[LANDS_COUNT][LAND_UNIT_TYPES];
  uint8_t units_land_player_total[LANDS_COUNT][PLAYERS_COUNT];
  uint8_t units_land_grand_total[LANDS_COUNT];
  uint8_t* units_land_ptr[LANDS_COUNT][LAND_UNIT_TYPES];
  uint8_t units_sea_type_total[SEAS_COUNT][SEA_UNIT_TYPES];
  uint8_t units_sea_player_total[SEAS_COUNT][PLAYERS_COUNT];
  uint8_t units_sea_blockade_total[SEAS_COUNT][PLAYERS_COUNT];
  uint8_t units_sea_grand_total[SEAS_COUNT];
  uint8_t* units_sea_ptr[SEAS_COUNT][SEA_UNIT_TYPES];
  uint8_t income_per_turn[PLAYERS_COUNT];
  uint8_t enemies[PLAYERS_COUNT - 1];
  uint8_t enemies_count;
  char* player_names[PLAYERS_COUNT];
  char* player_colors[PLAYERS_COUNT];
} GameCache;

#endif