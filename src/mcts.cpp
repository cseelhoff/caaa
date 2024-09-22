#include "mcts.hpp"
#include "game_state.hpp"
#include "player.hpp"
#include <cjson/cJSON.h>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>

// Function prototypes for game-specific logic
extern GameStateJson* clone_state(GameStateJson* state);
extern void get_possible_actions(GameStateJson* state, uint* num_actions, ActionsPtr actions);
extern void apply_action(GameStateJson* state, uint action);
extern bool is_terminal_state(GameStateJson* state);
extern double evaluate_state(GameStateJson* state);
extern double random_play_until_terminal(GameStateJson* state);

constexpr double EXPLORATION_CONSTANT = 1.414;
//#define EXPLORATION_CONSTANT 2

bool check5 = false;

static std::unique_ptr<MCTSNode> create_node(GameStateJson* state, uint action, MCTSNode* parent) {
  auto node = std::make_unique<MCTSNode>();
  node->state = *state; //*clone_state(state);
  node->action = action;
  node->parent = parent;
  node->visits = 0;
  node->value = 0.0;
  return node;
}

// static void free_node(MCTSNode* node) {
//   free_state(&node->state);
//   free(node->children);
//   free(node);
// }

static MCTSNode* select_best_leaf(MCTSNode* node) {
  double best_value = -std::numeric_limits<double>::infinity();
  MCTSNode* best_child = node;
  for (auto& child : node->children) {
    double uct_value = child->value / (child->visits + 1) +
                       EXPLORATION_CONSTANT * sqrt(log(node->visits + 1) / (child->visits + 1));
    if (uct_value > best_value) {
      best_value = uct_value;
      best_child = child.get();
    }
  }
  return best_child;
}

MCTSNode* mcts_search(GameStateJson* initial_state, uint iterations) {
  auto root = create_node(initial_state, 0, nullptr);
  constexpr uint PRINT_INTERVAL = 500;

  for (uint MCTS_ITERATIONS = 0; MCTS_ITERATIONS < iterations; MCTS_ITERATIONS++) {
    if (MCTS_ITERATIONS % PRINT_INTERVAL == 0) {
      std::cout << "Iteration " << MCTS_ITERATIONS << std::endl;
      print_mcts(root.get());
    }
    // select best leaf from node root
    MCTSNode* node = select_best_leaf(root.get());
    if (!is_terminal_state(&node->state)) {
      expand_node(node);
      node = node->children[static_cast<uint>(rand()) % node->children.size()].get();
    }
    double result = random_play_until_terminal(&node->state);
    // backpropagate
    while (node != nullptr) {
      node->visits++;
      if (node->parent != nullptr && node->parent->state.current_turn % 2 == 0) {
        node->value += result;
      } else {
        node->value += 1 - result;
      }
      node = node->parent;
    }
  }
  return root.release();
}

void expand_node(MCTSNode* node) {
  uint num_actions = 0;
  Actions actions = {0};
  ActionsPtr actionsPtr = &actions;
  //  printf("Expanding node: %d\n", node->action);

  get_possible_actions(&node->state, &num_actions, actionsPtr);
  node->children.reserve(num_actions);
  for (uint i = 0; i < num_actions; i++) {
    GameStateJson* new_state = clone_state(&node->state);
    uint next_action = actions[i];
    apply_action(new_state, next_action);
    node->children.push_back(create_node(new_state, next_action, node));
    // free_state(new_state);
  }
}
template <typename T, std::size_t N>
constexpr void copy_full_array(const std::array<T, N>& src, std::array<T, N>& dest) {
  std::copy(src.begin(), src.end(), dest.begin());
}
//#define COPY_SUB_ARRAY(src, dest, count) std::copy_n((src).begin(), count, (dest).begin());

constexpr uint MAX_ACTION_SEQUENCES = 20;
std::array<Action_Sequence, MAX_ACTION_SEQUENCES> action_sequences = {};
std::array<uint, MAX_ACTION_SEQUENCES> action_sequence_lengths = {0};
std::array<uint, MAX_ACTION_SEQUENCES> action_sequence_visits = {0};
std::array<double, MAX_ACTION_SEQUENCES> action_sequence_values = {0};

// Helper function to update the top 20 action sequences
void update_top_action_sequences(Action_Sequence current_sequence, uint length, double value,
                                 uint visits) {
  for (uint i = 0; i < MAX_ACTION_SEQUENCES; i++) {
    if (visits > action_sequence_visits.at(i)) {
      // Shift lower value sequences down
      for (uint j = MAX_ACTION_SEQUENCES - 1; j > i; j--) {
        action_sequence_values.at(j) = action_sequence_values.at(j - 1);
        action_sequence_lengths.at(j) = action_sequence_lengths.at(j - 1);
        copy_full_array(action_sequences.at(j - 1), action_sequences.at(j));
      }
      // Insert the new sequence
      action_sequence_values.at(i) = value;
      action_sequence_lengths.at(i) = length;
      copy_full_array(current_sequence, action_sequences.at(i));
      break;
    }
  }
}
constexpr uint MAX_DEPTH = 40;
constexpr uint MIN_VISITS = 10000;

// Function to print the MCTS tree and keep track of action sequences
void print_mcts_tree(MCTSNode* node, uint depth, Action_Sequence current_sequence, uint length) {
  if (node == nullptr) {
    return;
  }
  current_sequence[length++] = node->action;
  if (node->children.empty() || depth == MAX_DEPTH) {
    update_top_action_sequences(current_sequence, length, node->value / node->visits, node->visits);
    return;
  }
  bool has_mature_child = false;

  for (const auto& child : node->children) {
    if (child->visits > MIN_VISITS) {
      has_mature_child = true;
      print_mcts_tree(child.get(), depth + 1, current_sequence, length);
    }
  }
  if (!has_mature_child) {
    update_top_action_sequences(current_sequence, length, node->value / node->visits, node->visits);
  }
}
// Function to print the top 20 action sequences
void print_top_action_sequences() {
  constexpr int VISITS_WIDTH = 8;
  for (uint i = 0; i < MAX_ACTION_SEQUENCES; i++) {
    if (action_sequence_values.at(i) > 0) {
      std::cout << "value: " << std::fixed << std::setprecision(4) << action_sequence_values.at(i)
                << " visits: " << std::setw(VISITS_WIDTH) << action_sequence_visits.at(i)
                << " Actions:";
      for (uint j = 1; j < action_sequence_lengths.at(i); j++) {
        std::cout << " " << std::setw(3) << action_sequences.at(i).at(j);
      }
      std::cout << std::endl;
    }
  }
}

void print_mcts_tree3(MCTSNode* node, uint depth) {
  if (node == nullptr) {
    return;
  }
  constexpr uint MAX_PRINT_DEPTH = 40;
  if (depth > MAX_PRINT_DEPTH || node->children.empty()) {
    return;
  }
  if (node->parent != nullptr) {
    std::cout << PLAYERS[node->parent->state.current_turn].color << "Action: " << node->action
              << ", Visits: " << node->visits << ", Value: " << std::fixed << std::setprecision(2)
              << node->value << ", Avg:" << std::fixed << std::setprecision(4)
              << (node->value / node->visits) << "\033[0m" << std::endl;
  }
  // Recursively print the children
  uint best_index = 0;
  double best_value = 0;
  for (uint i = 0; i < node->children.size(); i++) {
    double new_value = node->children[i]->value / node->children[i]->visits;
    if (new_value > best_value) {
      best_value = new_value;
      best_index = i;
    }
  }
  print_mcts_tree3(node->children[best_index].get(), depth + 1);
}

void print_mcts_tree2(MCTSNode* node, uint depth) {
  if (node == nullptr) {
    return;
  }
  if (depth > 3) {
    return;
  }

  // Print the current node
  for (uint i = 0; i < depth; i++) {
    std::cout << "  ";
  }
  std::cout << "Action: " << node->action 
            << ", Visits: " << node->visits 
            << ", Value: " << std::fixed << std::setprecision(2) << node->value 
            << ", Avg:" << std::fixed << std::setprecision(4) << (node->value / node->visits) 
            << std::endl;

  // Recursively print the children
  for (const auto& child : node->children) {
    print_mcts_tree2(child.get(), depth + 1);
  }
}

// Public function to print the MCTS tree starting from the root
void print_mcts(MCTSNode* root) {
  // Action_Sequence current_sequence = {0};
  // for (uint i = 0; i < MAX_ACTION_SEQUENCES; i++) {
  //   action_sequence_values[i] = 0;
  //   action_sequence_visits[i] = 0;
  // }
  // print_mcts_tree(root, 0, current_sequence, 0);
  print_mcts_tree2(root, 0);
  // print_top_action_sequences();
}

uint select_best_action(MCTSNode* root) {
  MCTSNode* best_child = nullptr;
  double best_value = -std::numeric_limits<double>::infinity();
  for (const auto& child : root->children) {
    if (child->value > best_value) {
      best_value = child->value;
      best_child = child.get();
    }
  }
  return best_child->action;
}