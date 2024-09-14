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
#define MAX_ACTIONS 1000
typedef uint8_t Action_Sequence[MAX_ACTIONS];
void print_mcts_tree(MCTSNode* node, uint8_t depth, Action_Sequence current_sequence, int length);
void print_mcts(MCTSNode* root);
void print_top_action_sequences();
static void expand_node(MCTSNode* node);
#endif // MCTS_H