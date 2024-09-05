#include "mcts.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "typedefs.h"

// Function prototypes for game-specific logic
extern GameState* clone_state(GameState* state);
extern void free_state(GameState* state);
extern ActionArray* get_possible_actions(GameState* state, ActionCount* num_actions);
extern void apply_action(GameState* state, Action action);
extern bool is_terminal_state(GameState* state);
extern double evaluate_state(GameState* state);
extern double random_play_until_terminal(GameState* state);

#define EXPLORATION_CONSTANT 1.414

static MCTSNode* create_node(GameState* state, Action action, MCTSNode* parent) {
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
        for (int child_idx = 0; child_idx < node->num_children; child_idx++) {
            MCTSNode* child = node->children[child_idx];
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
    ActionCount num_actions;
    ActionArray* actions = get_possible_actions(&node->state, &num_actions);
    node->children = (MCTSNode**)malloc(num_actions * sizeof(MCTSNode*));
    for (int action_idx = 0; action_idx < num_actions; action_idx++) {
        GameState* new_state = clone_state(&node->state);
        Action action = (*actions)[action_idx];
        apply_action(new_state, action);
        node->children[action_idx] = create_node(new_state, action, node);
        free_state(new_state);
    }
    node->num_children = num_actions;
    //free(actions);
}

static double simulate(GameState* state) {
    //GameState* sim_state = clone_state(state);
    /*
    while (!is_terminal_state(sim_state)) {
        int num_actions;
        uint8_t* actions = get_possible_actions(sim_state, &num_actions);
        if (num_actions == 0) break;
        int action_index = rand() % num_actions;
        apply_action(sim_state, &actions[action_index]);
        free(actions);
    }
    */

    //double result = evaluate_state(sim_state);
    double result = random_play_until_terminal(state);
    //free_state(sim_state);
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
    for (int iteration = 0; iteration < iterations; iteration++) {
        printf("Iteration %d\n", iteration);
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

Action select_best_action(MCTSNode* root) {
    MCTSNode* best_child = NULL;
    double best_value = -INFINITY;
    for (int child_idx = 0; child_idx < root->num_children; child_idx++) {
        MCTSNode* child = root->children[child_idx];
        if (child->value > best_value) {
            best_value = child->value;
            best_child = child;
        }
    }
    return best_child->action;
}
// Function to print the MCTS tree
static void print_mcts_tree(MCTSNode* node, int depth) {
    if (node == NULL) return;

    // Print the current node
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    printf("Action: %d, Visits: %d, Value: %.2f\n", node->action, node->visits, node->value);

    // Recursively print the children
    for (int child_idx = 0; child_idx < node->num_children; child_idx++) {
        print_mcts_tree(node->children[child_idx], depth + 1);
    }
}

// Public function to print the MCTS tree starting from the root
void print_mcts(MCTSNode* root) {
    print_mcts_tree(root, 0);
}