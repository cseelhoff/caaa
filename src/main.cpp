#include "engine.hpp"
#include "game_state_memory.hpp"
#include "json_state.hpp"
#include "mcts.hpp"
#include <iostream>
#include <string>
#include <vector>

#define BASE_10 10
#define DEFAULT_ITERATIONS 1000000000

int main(int argc, char *argv[]) {
  uint iterations = DEFAULT_ITERATIONS;
    if (argc >= 2) {
        std::vector<std::string> args(argv, argv + argc);
        iterations = static_cast<uint>(std::stoul(args[1]));
    }
  std::cout << "Starting CAAA" << std::endl;
  initialize_random_numbers();
  //json = serialize_game_data_to_json(&state);
  // write_json_to_file("game_data_0.json", json);
  //cJSON_Delete(json);
  GameStateMemory game_state;
  load_game_data("game_data.json", game_state);
  initialize_map_constants();
  //GameStateJson* initial_state = get_game_state_copy();
  // Initialize the game state

  MCTSNode* root = mcts_search(game_state, iterations);
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
  initialize_random_numbers();
  GameStateMemory game_state;
  load_game_data("debug_data.json", game_state);
  load_single_game();  
  return 0;
}