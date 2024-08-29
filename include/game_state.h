#pragma once
#include "canal.h"
#include "land.h"
#include "player.h"
#include "sea.h"
#include "air.h"
#include "team.h"
#include "typedefs.h"
#include "units/units.h"
#include <stdint.h>


typedef struct {
  PlayerIndex owner_idx; // rotates
  int8_t factory_hp;
  uint8_t factory_max;
  uint8_t bombard_max; // bombarded, resets
  FighterStateSum fighters;                        // rotates
  BomberLandStateSum bombers;                     // rotates
  InfantryStateSum infantry;                       // rotates
  ArtilleryStateSum artillery;                     // rotates
  TanksStateSum tanks;                              // rotates
  AAGunsStateSum aa_guns;                          // rotates
} __attribute__((aligned(ALIGNMENT_32))) LandState;

typedef struct {
  FightersSeaStateSum fighters;
  // 0 = done moving, 1 = 1 mov left, 2 = 2 mov left, 3 = needs staging
  TransEmptyStateSum trans_empty;
  // 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left, 4 = needs staging
  Trans1IStateSum trans_1i;
  Trans1AStateSum trans_1a;
  Trans1TStateSum trans_1t;
  // 0 = done moving, 1=0mov can unload, 2 = 1 mov left, 3 = 2 mov left
  Trans2IStateSum trans_2i;
  Trans1I1AStateSum trans_1i_1a;
  Trans1I1TStateSum trans_1i_1t;
  SubmarinesStateSum submarines;
  DestroyersStateSum destroyers;
  CarriersStateSum carriers;
  CruisersStateSum cruisers;       // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  BattleshipsStateSum battleships; // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  BSDamagedStateSum bs_damaged;  // 0=no att, 1=0 mov can bombard, 2 = 2 mov
  BombersSeaStateSum bombers;     // move remain 1,2,3,4,5
} __attribute__((aligned(ALIGNMENT_64))) UnitsSea;

typedef struct {
  PlayerIndex player_index; //rotates
  Dollars money[PLAYERS_COUNT]; //rotates
  uint8_t builds_left[AIRS_COUNT]; 
  LandState land_state[LANDS_COUNT];
  UnitsSea units_sea[SEAS_COUNT];
  LandUnitTypeSum other_land_units[PLAYERS_COUNT - 1][LANDS_COUNT][LAND_UNIT_TYPES_COUNT]; // rotates
  SeaUnitTypeSum other_sea_units[PLAYERS_COUNT - 1][SEAS_COUNT][SEA_UNIT_TYPES_COUNT - 1]; // no parking bombers at sea
  bool flagged_for_combat[AIRS_COUNT]; // track retreats
} __attribute__((aligned(ALIGNMENT_128))) GameState;
