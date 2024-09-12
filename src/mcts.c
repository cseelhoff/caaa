#include "mcts.h"
#include "game_state.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
    if (MCTS_ITERATIONS % 1000 == 0) {
      printf("Iteration %d\n", MCTS_ITERATIONS);
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
// Function to print the MCTS tree
void print_mcts_tree(MCTSNode* node, uint8_t depth) {
  if (node == NULL) {
    return;
  }
  if (depth > 10) {
    return;
  }

  // Print the current node
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }
  printf("Action: %d, Visits: %d, Value: %.2f, Avg:%.4f\n", node->action, node->visits, node->value,
         node->value / node->visits);

  // Recursively print the children
  double highest_value = 0;
  double second_highest_value = 0;
  for (int i = 0; i < node->num_children; i++) {
    if (node->children[i]->value > highest_value) {
      second_highest_value = highest_value;
      highest_value = node->children[i]->value;
    } else if (node->children[i]->value > second_highest_value) {
      second_highest_value = node->children[i]->value;
    }
  }

  for (int i = 0; i < node->num_children; i++) {
    if (node->children[i]->value >= second_highest_value) {
      print_mcts_tree(node->children[i], depth + 1);
    }
  }
}

// Public function to print the MCTS tree starting from the root
void print_mcts(MCTSNode* root) { print_mcts_tree(root, 0); }