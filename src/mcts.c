#include "mcts.h"
#include "game_state.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t Actions[AIRS_COUNT];
typedef Actions* ActionsPtr;
int MCTS_ITERATIONS = 0;

// Function prototypes for game-specific logic
extern GameState* clone_state(GameState* state);
extern void free_state(GameState* state);
extern void get_possible_actions(GameState* state, uint8_t* num_actions, ActionsPtr actions);
extern void apply_action(GameState* state, uint8_t action);
extern bool is_terminal_state(GameState* state);
extern double evaluate_state(GameState* state);
extern double random_play_until_terminal(GameState* state);

#define EXPLORATION_CONSTANT 1.414

static MCTSNode* create_node(GameState* state, uint8_t action, MCTSNode* parent) {
  MCTSNode* node = (MCTSNode*)malloc(sizeof(MCTSNode));
  node->state = *clone_state(state);
  node->action = action;
  node->parent = parent;
  node->children = NULL;
  node->num_children = 0;
  node->visits = 0;
  node->value = 0.0;
  return node;
}

static void free_node(MCTSNode* node) {
  free_state(&node->state);
  free(node->children);
  free(node);
}

static MCTSNode* select_node(MCTSNode* node) {
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

static void expand_node(MCTSNode* node) {
  uint8_t num_actions;
  Actions actions = {0};
  ActionsPtr actionsPtr = &actions;
  get_possible_actions(&node->state, &num_actions, actionsPtr);
  node->children = (MCTSNode**)malloc((unsigned long)num_actions * sizeof(MCTSNode*));
  for (int i = 0; i < num_actions; i++) {
    GameState* new_state = clone_state(&node->state);
    uint8_t next_action = actions[i];
    apply_action(new_state, next_action);
    node->children[i] = create_node(new_state, next_action, node);
    free_state(new_state);
  }
  node->num_children = num_actions;
}

static double simulate(GameState* state) {
  double result = random_play_until_terminal(state);
  return result;
}

static void backpropagate(MCTSNode* node, double result) {
  while (node != NULL) {
    node->visits++;
    node->value += result;
    node = node->parent;
  }
}

MCTSNode* mcts_search(GameState* initial_state, int iterations) {
  MCTSNode* root = create_node(initial_state, 0, NULL);
  for (MCTS_ITERATIONS = 0; MCTS_ITERATIONS < iterations; MCTS_ITERATIONS++) {
    if (MCTS_ITERATIONS % 50000 == 0) {
      printf("Iteration %d\n", MCTS_ITERATIONS);
      Action_Sequence current_sequence = {0};
      print_mcts_tree(root, 0, current_sequence, 0);
      print_top_action_sequences();
    }
    MCTSNode* node = select_node(root);
    if (!is_terminal_state(&node->state)) {
      expand_node(node);
      node = node->children[rand() % node->num_children];
    }
    double result = simulate(&node->state);
    backpropagate(node, result);
  }
  return root;
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

#define MAX_ACTION_SEQUENCES 20
Action_Sequence action_sequences[MAX_ACTION_SEQUENCES] = {0};
int action_sequence_lengths[MAX_ACTION_SEQUENCES] = {0};
int action_sequence_visits[MAX_ACTION_SEQUENCES] = {0};
double action_sequence_values[MAX_ACTION_SEQUENCES] = {0};

// Helper function to update the top 20 action sequences
void update_top_action_sequences(Action_Sequence current_sequence, int length, double value, int visits) {
  for (int i = 0; i < MAX_ACTION_SEQUENCES; i++) {
    if (value > action_sequence_values[i]) {
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
#define MAX_DEPTH 20
#define MIN_VISITS 100
// Function to print the MCTS tree and keep track of action sequences
void print_mcts_tree(MCTSNode* node, uint8_t depth, Action_Sequence current_sequence, int length) {
  if (node == NULL) {
    return;
  }

  // Add the current node's action to the sequence
  current_sequence[length++] = node->action;

  // Print the current node
  //for (int i = 0; i < depth; i++) {
  //  printf("  ");
  //}
  //printf("Action: %d, Visits: %d, Avg:%.4f\n", node->action, node->visits,
  //       node->value / node->visits);

  // If the node is a leaf, update the top action sequences
  if (node->num_children == 0 || depth == MAX_DEPTH || node->visits < MIN_VISITS) {
    update_top_action_sequences(current_sequence, length, node->value / node->visits, node->visits);
    return;
  }

  // Recursively print the children
  for (int i = 0; i < node->num_children; i++) {
    print_mcts_tree(node->children[i], depth + 1, current_sequence, length);
  }
}

// Function to print the top 20 action sequences
void print_top_action_sequences() {
  for (int i = 0; i < MAX_ACTION_SEQUENCES; i++) {
    if (action_sequence_values[i] > 0) {
      printf("value: %.4f visits: %8d Actions:", action_sequence_values[i], action_sequence_visits[i]);
      for (int j = 1; j < action_sequence_lengths[i]; j++) {
        printf(" %2d", action_sequences[i][j]);
      }
      printf("\n");
    }
  }
}

// Public function to print the MCTS tree starting from the root
void print_mcts(MCTSNode* root) {
  Action_Sequence current_sequence = {0};
  print_mcts_tree(root, 0, current_sequence, 0);
  print_top_action_sequences();
}