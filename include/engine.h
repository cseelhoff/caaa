#pragma once
#include "game_state.h"
#include "typedefs.h"
#include <stdbool.h>
#include <stdint.h>

void initializeGameData();
void load_game_data();
void play_full_turn();
double get_score();
void set_seed(uint16_t seed);

GameState* clone_state(GameState* game_state);
void free_state(GameState* game_state);
ActionArray* get_possible_actions(GameState* game_state, int* num_actions);
void apply_action(GameState* game_state, Action action);
bool is_terminal_state(GameState* game_state);
double evaluate_state(GameState* game_state);
double random_play_until_terminal(GameState* game_state);
GameState* get_game_state_copy();
