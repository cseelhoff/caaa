#pragma once
#include <stdint.h>
#define AA_GUN_NAME "aa_guns"
#define AA_GUN_ATTACK 0
#define AA_GUN_DEFENSE 0
#define AA_GUN_MOVES_MAX 1
#define AA_GUN_COST 5
#define AA_GUN_AA_STRENGTH 1
#define AA_GUN_WEIGHT 6

typedef enum : uint8_t {
    AA_GUN_DONE_MOVING,
    AA_GUN_UNMOVED,
    AA_GUN_STATES
} AAGunState;
