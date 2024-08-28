#include "mcts.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Function prototypes for game-specific logic
extern GameState* clone_state(GameState* state);
extern void free_state(GameState* state);
extern uint8_t* get_possible_actions(GameState* state, int* num_actions);
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
    int num_actions;
    uint8_t* actions = get_possible_actions(&node->state, &num_actions);
    node->children = (MCTSNode**)malloc(num_actions * sizeof(MCTSNode*));
    for (int i = 0; i < num_actions; i++) {
        GameState* new_state = clone_state(&node->state);
        apply_action(new_state, actions[i]);
        node->children[i] = create_node(new_state, actions[i], node);
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
    for (int i = 0; i < iterations; i++) {
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
static void print_mcts_tree(MCTSNode* node, int depth) {
    if (node == NULL) return;

    // Print the current node
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    printf("Action: %d, Visits: %d, Value: %.2f\n", node->action, node->visits, node->value);

    // Recursively print the children
    for (int i = 0; i < node->num_children; i++) {
        print_mcts_tree(node->children[i], depth + 1);
    }
}

// Public function to print the MCTS tree starting from the root
void print_mcts(MCTSNode* root) {
    print_mcts_tree(root, 0);
}