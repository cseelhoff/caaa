#ifndef MCTS_H
#define MCTS_H

#include <stdint.h>
#include <stdbool.h>
#include "game_state.h"

typedef struct MCTSNode {
    GameState state;
    uint8_t action;
    struct MCTSNode* parent;
    struct MCTSNode** children;
    int num_children;
    int visits;
    double value;
} MCTSNode;
extern int MCTS_ITERATIONS;// = 0;
MCTSNode* mcts_search(GameState* initial_state, int iterations);
uint8_t select_best_action(MCTSNode* root);
void print_mcts_tree(MCTSNode* node, uint8_t depth);
void print_mcts(MCTSNode* root);
#endif // MCTS_H