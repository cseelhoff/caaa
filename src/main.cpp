#include "engine.hpp"
#include "mcts.hpp"
#include <string>
#include <vector>
#include <iostream>

#define BASE_10 10
#define DEFAULT_ITERATIONS 1000000000

int main(int argc, char *argv[]) {
  uint iterations = DEFAULT_ITERATIONS;
    if (argc >= 2) {
        std::vector<std::string> args(argv, argv + argc);
        iterations = static_cast<uint>(std::stoul(args[1]));
    }
  std::cout << "Starting CAAA" << std::endl;
  initialize_constants();
  //json = serialize_game_data_to_json(&state);
  // write_json_to_file("game_data_0.json", json);
  //cJSON_Delete(json);
  load_game_data("game_data.json");
  GameStateJson* initial_state = get_game_state_copy();
  // Initialize the game state

  MCTSNode* root = mcts_search(initial_state, iterations);
  uint best_action = select_best_action(root);
  //print_mcts(root);
  print_mcts(root);
  std::cout << "Best action: " << best_action << std::endl;
  // Apply the best action to the game state
  //apply_action(initial_state, best_action);

  // Continue with the game loop
  return 0;
}
int main2() {
  std::cout << "Starting CAAA" << std::endl;
  initialize_constants();
  load_game_data("debug_data.json");
  refresh_full_cache();
  load_single_game();  
  return 0;
}