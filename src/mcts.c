#include "mcts.h"
#include "game_state.h"
#include "serialize_data.h"
#include <cjson/cJSON.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int MCTS_ITERATIONS = 0;

// Function prototypes for game-specific logic
extern GameState* clone_state(GameState* state);
extern void free_state(GameState* state);
extern void get_possible_actions(GameState* state, uint8_t* num_actions, ActionsPtr actions);
extern void apply_action(GameState* state, uint8_t action);
extern bool is_terminal_state(GameState* state);
extern double evaluate_state(GameState* state);
extern double random_play_until_terminal(GameState* state);

//#define EXPLORATION_CONSTANT 1.414
#define EXPLORATION_CONSTANT 3

bool check5 = false;

static MCTSNode* create_node(GameState* state, uint8_t action, MCTSNode* parent) {
  MCTSNode* node = (MCTSNode*)malloc(sizeof(MCTSNode));
  node->state = *state; //*clone_state(state);
  node->action = action;
  node->parent = parent;
  node->children = NULL;
  node->num_children = 0;
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
  while (node->num_children > 0) {
    double best_value = -INFINITY;
    MCTSNode* best_child = NULL;
    for (int i = 0; i < node->num_children; i++) {
      MCTSNode* child = node->children[i];
      double uct_value = child->value / (child->visits + 1) +
                         EXPLORATION_CONSTANT * sqrt(log(node->visits + 1) / (child->visits + 1));
      if (uct_value > best_value) {
        best_value = uct_value;
        best_child = child;
      }
    }
    node = best_child;
  }
  return node;
}

MCTSNode* mcts_search(GameState* initial_state, int iterations) {
  MCTSNode* root = create_node(initial_state, 0, NULL);
  for (MCTS_ITERATIONS = 0; MCTS_ITERATIONS < iterations; MCTS_ITERATIONS++) {
    if (MCTS_ITERATIONS % 1 == 0) {
      printf("Iteration %d\n", MCTS_ITERATIONS);
      print_mcts(root);
    }
    // select best leaf from node root
    MCTSNode* node = select_best_leaf(root);
    // if (MCTS_ITERATIONS == 6575) {
    //   write_json_to_file("debug_save.json", serialize_game_data_to_json(&node->state));
    // }
    // printf("Action Chain: <-%d", node->action);
    // MCTSNode* parent = node->parent;
    // while(parent != NULL) {
    //   printf("<-%d", parent->action);
    //   parent = parent->parent;
    // }
    // printf("\n");
    if (!is_terminal_state(&node->state)) {
      expand_node(node);
      node = node->children[rand() % node->num_children];
    }
    // if (MCTS_ITERATIONS == 7549) {
    //   write_json_to_file("debug_save.json", serialize_game_data_to_json(&node->state));
    // }

    // simulate
    
    double result = random_play_until_terminal(&node->state);
    // backpropagate
    while (node != NULL) {
      node->visits++;
      node->value += result;
      node = node->parent;
      result = 1 - result;
    }
  }
  return root;
}

static void expand_node(MCTSNode* node) {
  uint8_t num_actions = 0;
  Actions actions = {0};
  ActionsPtr actionsPtr = &actions;
  //  printf("Expanding node: %d\n", node->action);

  get_possible_actions(&node->state, &num_actions, actionsPtr);
  node->children = (MCTSNode**)malloc((unsigned long)num_actions * sizeof(MCTSNode*));
  node->num_children = num_actions;
  for (int i = 0; i < num_actions; i++) {
    GameState* new_state = clone_state(&node->state);
    uint8_t next_action = actions[i];
    //  printf("  Applying Action: %d\n", next_action);
    // if (next_action == 3) {
    //   if (node->action == 255) {
    //     MCTSNode* parent = node->parent;
    //     if (parent->action == 4) {
    //       parent = parent->parent;
    //       if (parent->action == 255) {
    //         parent = parent->parent;
    //         if (parent->action == 3) {
    //           parent = parent->parent;
    //           if (parent->action == 0) {
    //             int break_here = 1;
    //             check5 = true;
    //           }
    //         }
    //       }
    //     }
    //   }
    // }
    apply_action(new_state, next_action);
    if(node->state.player_index == new_state->player_index) {
      printf("Error: Player index did not change after applying action %d\n", next_action);
      exit(1);
    }
    node->children[i] = create_node(new_state, next_action, node);
    // free_state(new_state);
  }
}

#define MAX_ACTION_SEQUENCES 20
Action_Sequence action_sequences[MAX_ACTION_SEQUENCES] = {0};
int action_sequence_lengths[MAX_ACTION_SEQUENCES] = {0};
int action_sequence_visits[MAX_ACTION_SEQUENCES] = {0};
double action_sequence_values[MAX_ACTION_SEQUENCES] = {0};

// Helper function to update the top 20 action sequences
void update_top_action_sequences(Action_Sequence current_sequence, int length, double value,
                                 int visits) {
  for (int i = 0; i < MAX_ACTION_SEQUENCES; i++) {
    if (visits > action_sequence_visits[i]) {
      // Shift lower value sequences down
      for (int j = MAX_ACTION_SEQUENCES - 1; j > i; j--) {
        action_sequence_values[j] = action_sequence_values[j - 1];
        action_sequence_visits[j] = action_sequence_visits[j - 1];
        action_sequence_lengths[j] = action_sequence_lengths[j - 1];
        memcpy(action_sequences[j], action_sequences[j - 1], sizeof(Action_Sequence));
      }
      // Insert the new sequence
      action_sequence_values[i] = value;
      action_sequence_visits[i] = visits;
      action_sequence_lengths[i] = length;
      memcpy(action_sequences[i], current_sequence, sizeof(Action_Sequence));
      break;
    }
  }
}
#define MAX_DEPTH 40
#define MIN_VISITS 100000
// Function to print the MCTS tree and keep track of action sequences
void print_mcts_tree(MCTSNode* node, uint8_t depth, Action_Sequence current_sequence, int length) {
  if (node == NULL) {
    return;
  }
  current_sequence[length++] = node->action;
  if ((node->num_children == 0 || depth == MAX_DEPTH)) {
    update_top_action_sequences(current_sequence, length, node->value / node->visits, node->visits);
    return;
  }
  bool has_mature_child = false;

  for (int i = 0; i < node->num_children; i++) {
    if (node->children[i]->visits > MIN_VISITS) {
      has_mature_child = true;
      print_mcts_tree(node->children[i], depth + 1, current_sequence, length);
    }
  }
  if (!has_mature_child) {
    update_top_action_sequences(current_sequence, length, node->value / node->visits, node->visits);
  }
}

// Function to print the top 20 action sequences
void print_top_action_sequences() {
  for (int i = 0; i < MAX_ACTION_SEQUENCES; i++) {
    if (action_sequence_values[i] > 0) {
      printf("value: %.4f visits: %8d Actions:", action_sequence_values[i],
             action_sequence_visits[i]);
      for (int j = 1; j < action_sequence_lengths[i]; j++) {
        printf(" %3d", action_sequences[i][j]);
      }
      printf("\n");
    }
  }
}

void print_mcts_tree2(MCTSNode* node, int depth) {
  if (node == NULL)
    return;
  if (depth > 3) {
    return;
  }

  // Print the current node
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }
  printf("Action: %d, Visits: %d, Value: %.2f, Avg:%.4f\n", node->action, node->visits, node->value,
         node->value / node->visits);

  // Recursively print the children
  for (int i = 0; i < node->num_children; i++) {
    print_mcts_tree2(node->children[i], depth + 1);
  }
}
// Public function to print the MCTS tree starting from the root
void print_mcts(MCTSNode* root) {
  Action_Sequence current_sequence = {0};
  for (uint8_t i = 0; i < MAX_ACTION_SEQUENCES; i++) {
    action_sequence_values[i] = 0;
    action_sequence_visits[i] = 0;
  }
  print_mcts_tree(root, 0, current_sequence, 0);
  // if (root->num_children > 4) {
  //   MCTSNode* child1 = root->children[4];
  //   if (child1->num_children > 3) {
  //     MCTSNode* child2 = child1->children[3];
  //     print_mcts_tree2(child2, 0);
  //   }
  // }
  print_mcts_tree2(root, 0);
  print_top_action_sequences();
}

uint8_t select_best_action(MCTSNode* root) {
  MCTSNode* best_child = NULL;
  double best_value = -INFINITY;
  for (int i = 0; i < root->num_children; i++) {
    MCTSNode* child = root->children[i];
    if (child->value > best_value) {
      best_value = child->value;
      best_child = child;
    }
  }
  return best_child->action;
}
