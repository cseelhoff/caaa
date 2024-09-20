#include "engine.h"
#include "mcts.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BASE_10 10
#define DEFAULT_ITERATIONS 1000000000

int main(int argc, char *argv[]) {
  long iterations = DEFAULT_ITERATIONS;
  if (argc >= 2) {
    iterations = strtol(argv[1], NULL, BASE_10);
  }
  printf("Starting CAAA \n");
  initialize_constants();
  //json = serialize_game_data_to_json(&state);
  // write_json_to_file("game_data_0.json", json);
  //cJSON_Delete(json);
  load_game_data("game_data.json");
  GameState* initial_state = get_game_state_copy();
  // Initialize the game state

  MCTSNode* root = mcts_search(initial_state, iterations);
  uint best_action = select_best_action(root);
  //print_mcts(root);
  print_mcts(root);
  printf("Best action: %d\n", best_action);
  // Apply the best action to the game state
  //apply_action(initial_state, best_action);

  // Continue with the game loop
  return 0;
}
int main2() {
  printf("Starting CAAA \n");
  initialize_constants();
  load_game_data("debug_data.json");
  refresh_full_cache();
  load_single_game();  
  return 0;
}