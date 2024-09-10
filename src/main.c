#include "engine.h"
#include "mcts.h"
#include <stdint.h>
#include <stdio.h>

int main() {
  printf("Starting CAAA \n");
  initialize_constants();
  //json = serialize_game_data_to_json(&state);
  // write_json_to_file("game_data_0.json", json);
  //cJSON_Delete(json);
  load_game_data("game_data.json");
  GameState* initial_state = get_game_state_copy();
  // Initialize the game state

  MCTSNode* root = mcts_search(initial_state, 1000000);
  uint8_t best_action = select_best_action(root);
  //print_mcts(root);
  print_mcts_tree(root, 0);
  printf("Best action: %d\n", best_action);
  // Apply the best action to the game state
  //apply_action(initial_state, best_action);

  // Continue with the game loop
}
int main2() {
  printf("Starting CAAA \n");
  initialize_constants();
  load_game_data("rel5410.json");
  refresh_full_cache();
  load_single_game();  
}