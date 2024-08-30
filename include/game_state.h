#pragma once
#include "air.h"
#include "canal.h"
#include "land.h"
#include "player.h"
#include "sea.h"
#include "team.h"
#include "typedefs.h"
#include "units/units.h"
#include <stdint.h>


typedef struct {
  PlayerIndex owner_idx; // rotates
  int8_t factory_hp;
  uint8_t factory_max;
  uint8_t bombard_max; // bombarded, resets
  FighterStateSums fighters;                        // rotates
  BomberLandStateSums bombers;                     // rotates
  InfantryStateSums infantry;                       // rotates
  ArtilleryStateSums artillery;                     // rotates
  TanksStateSums tanks;                              // rotates
  AAGunsStateSums aa_guns;                          // rotates
} __attribute__((aligned(ALIGNMENT_32))) LandTerr;

typedef struct {
  FightersSeaStateSums fighters;
  // 0 = done moving, 1 = 1 mov left, 2 = 2 mov left, 3 = needs staging
  TransEmptyStateSums trans_empty;
  // 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left, 4 = needs staging
  Trans1IStateSums trans_1i;
  Trans1AStateSums trans_1a;
  Trans1TStateSums trans_1t;
  // 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left
  Trans2IStateSums trans_2i;
  Trans1I1AStateSums trans_1i_1a;
  Trans1I1TStateSums trans_1i_1t;
  SubmarinesStateSums submarines;
  DestroyersStateSums destroyers;
  CarriersStateSums carriers;
  CruisersStateSums cruisers;       // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  BattleshipsStateSums battleships; // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  BSDamagedStateSums bs_damaged;  // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  BombersSeaStateSums bombers;     // move remain 1,2,3,4,5
} __attribute__((aligned(ALIGNMENT_64))) SeaTerr;

typedef struct {
  PlayerIndex player_index; //rotates
  Dollars money[PLAYERS_COUNT]; //rotates
  uint8_t builds_left[AIRS_COUNT]; 
  LandTerr land_terr[LANDS_COUNT];
  SeaTerr sea_terr[SEAS_COUNT];
  LandUnitTypesSumArrayLands other_land_units[PLAYERS_COUNT - 1];
  SeaUnitTypesSumArraySeas other_sea_units[PLAYERS_COUNT - 1];
  bool flagged_for_combat[AIRS_COUNT]; // track retreats
} __attribute__((aligned(ALIGNMENT_128))) GameState;
