#pragma once

#include "game_state.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Serialization and deserialization functions for GameState
bool load_game_state_from_json(const std::string& filename, GameState& gameState);
void to_json(json& jsonData, const GameState& gameState);
void from_json(const json& jsonData, GameState& gameState);
