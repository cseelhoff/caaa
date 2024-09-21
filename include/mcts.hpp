#pragma once
#include "game_state.hpp"
#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

#define ALIGNMENT 128
struct MCTSNode {
    GameState state;
    uint action;
    MCTSNode* parent;
    std::vector<std::unique_ptr<MCTSNode>> children;
    uint visits;
    double value;
} __attribute__((aligned(ALIGNMENT)));
extern uint MCTS_ITERATIONS;// = 0;
MCTSNode* mcts_search(GameState* initial_state, uint iterations);
uint select_best_action(MCTSNode* root);
constexpr uint MAX_ACTIONS = 1000;
using Action_Sequence = std::array<uint, MAX_ACTIONS>;
void print_mcts_tree(MCTSNode* node, uint depth, Action_Sequence current_sequence, uint length);
void print_mcts(MCTSNode* root);
void print_top_action_sequences();
void expand_node(MCTSNode* node);
