#include "engine.h"
#include "mcts.h"
#include <stdint.h>
#include <stdio.h>

int main() {
  printf("Starting CAAA \n");
  initialize_constants();
  load_game_data("game_data.json");
  GameState* initial_state = get_game_state_copy();
  // Initialize the game state

  MCTSNode* root = mcts_search(initial_state, 19000);
  uint8_t best_action = select_best_action(root);
  print_mcts(root);
  printf("Best action: %d\n", best_action);
  // Apply the best action to the game state
  //apply_action(initial_state, best_action);

  // Continue with the game loop
}
int main2() {
  printf("Starting CAAA \n");
  initialize_constants();
  uint16_t start_seed = 201;
  for (uint16_t seed = start_seed; seed < start_seed + 100; seed++) {
    set_seed(seed);
    load_game_data("game_data.json");
    int turns = 100;
    while (turns--) {
      play_full_turn();
      double score = get_score();
      printf("Score: %f\n", score);
    }
  }
  return 0;
}