#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "game_state.h"

typedef struct MCTSNode {
    GameState state;
    Action action;
    struct MCTSNode* parent;
    struct MCTSNode** children;
    int num_children;
    int visits;
    double value;
} MCTSNode;

MCTSNode* mcts_search(GameState* initial_state, int iterations);
Action select_best_action(MCTSNode* root);
static void print_mcts_tree(MCTSNode* node, int depth);
void print_mcts(MCTSNode* root);
