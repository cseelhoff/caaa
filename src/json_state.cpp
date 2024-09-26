#include "json_state.hpp"
#include "game_state.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool load_game_state_from_json(const std::string& filename, GameStateJson& gameState) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open the file: " << filename << std::endl;
        return false;
    }

    json jsonData;
    file >> jsonData;
    file.close();

    try {
        from_json(jsonData, gameState);
    } catch (json::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return false;
    }

    return true;
}

// Serialization and deserialization functions for BitField
void to_json(json& jsonData, const BitField& bField) {
    jsonData = json{{"bit", static_cast<int>(bField.bit)}}; // Cast to int for serialization
}

void from_json(const json& jsonData, BitField& bField) {
    bool temp = false;
    jsonData.at("bit").get_to(temp);
    bField.bit = temp; // Assign the value back to the bit-field
}

// Serialization and deserialization functions for LandState
void to_json(json& jsonData, const LandStateJson& lState) {
    jsonData = json{{"owner_idx", lState.owner_idx},
             {"factory_dmg", lState.factory_dmg},
             {"factory_max", lState.factory_max},
             {"bombard_max", lState.bombard_max},
             {"fighters", lState.fighters},
             {"bombers", lState.bombers},
             {"infantry", lState.infantry},
             {"artillery", lState.artillery},
             {"tanks", lState.tanks},
             {"aaguns", lState.aaguns}};
}

void from_json(const json& jsonData, LandStateJson& lState) {
    lState.owner_idx = jsonData.at("owner_idx").get<uint>();
    lState.factory_dmg = jsonData.value("factory_dmg", 0U);
    lState.factory_max = jsonData.value("factory_max", 0U);
    lState.bombard_max = jsonData.value("bombard_max", 0U);
    lState.fighters = jsonData.value("fighters", Fighterstates{0}); // Default array of zeros
    lState.bombers = jsonData.value("bombers", Bomberlandstates{0}); // Default array of zeros
    lState.infantry = jsonData.value("infantry", Infantrystates{0}); // Default array of zeros
    lState.artillery = jsonData.value("artillery", Artillerystates{0}); // Default array of zeros
    lState.tanks = jsonData.value("tanks", Tankstates{0}); // Default array of zeros
    lState.aaguns = jsonData.value("aaguns", AAgunstates{0}); // Default array of zeros
}

// Serialization and deserialization functions for UnitsSea
void to_json(json& jsonData, const SeaState& uSea) {
    jsonData = json({{"fighters", uSea.fighters},       {"transempty", uSea.transempty},
             {"trans1i", uSea.trans1i},       {"trans1a", uSea.trans1a},
             {"trans1t", uSea.trans1t},       {"trans2i", uSea.trans2i},
             {"trans1i1a", uSea.trans1i1a}, {"trans1i1t", uSea.trans1i1t},
             {"submarines", uSea.submarines},   {"destroyers", uSea.destroyers},
             {"carriers", uSea.carriers},       {"cruisers", uSea.cruisers},
             {"battleships", uSea.battleships}, {"bs_damaged", uSea.bs_damaged},
             {"bombers", uSea.bombers}});
}

void from_json(const json& jsonData, SeaState& uSea) {
    uSea.fighters = jsonData.value("fighters", Fighterstates{});
    uSea.transempty = jsonData.value("transempty", TransEmptystates{});
    uSea.trans1i = jsonData.value("trans1i", Trans1istates{});
    uSea.trans1a = jsonData.value("trans1a", Trans1astates{});
    uSea.trans1t = jsonData.value("trans1t", Trans1tstates{});
    uSea.trans2i = jsonData.value("trans2i", Trans2istates{});
    uSea.trans1i1a = jsonData.value("trans1i1a", Trans1i1astates{});
    uSea.trans1i1t = jsonData.value("trans1i1t", Trans1i1tstates{});
    uSea.submarines = jsonData.value("submarines", Submarinestates{});
    uSea.destroyers = jsonData.value("destroyers", Destroyerstates{});
    uSea.carriers = jsonData.value("carriers", Carrierstates{});
    uSea.cruisers = jsonData.value("cruisers", Cruiserstates{});
    uSea.battleships = jsonData.value("battleships", Battleshipstates{});
    uSea.bs_damaged = jsonData.value("bs_damaged", Battleshipstates{});
    uSea.bombers = jsonData.value("bombers", Bomberseastates{});
}

//Serialization and deserialization functions for GameState
void to_json(json& jsonData, const GameStateJson& gameState) {
    jsonData = json{
        {"player_index", gameState.current_turn},
        {"money", gameState.money},
        {"builds_left", gameState.builds_left},
        {"land_state", gameState.land_states},
        {"units_sea", gameState.sea_states},
        {"other_land_units", gameState.idle_land_units},
        {"other_sea_units", gameState.idle_sea_units},
        {"combat_status", gameState.combat_status},
        {"skipped_moves", gameState.skipped_moves}
    };
}

void from_json(const json& jsonData, GameStateJson& gameState) {
    gameState.current_turn = jsonData.value("player_index", 0U);
    gameState.money = jsonData.value("money", PlayersArray{});
    gameState.builds_left = jsonData.value("builds_left", AirArray{});
    gameState.land_states = jsonData.value("land_state", LandStateArray{});
    gameState.sea_states = jsonData.value("units_sea", SeaStateArray{});
    gameState.idle_land_units = jsonData.value("other_land_units", OplayerLandUTArray{});
    gameState.idle_sea_units = jsonData.value("other_sea_units", OplayerSeaUTArray{});
    gameState.combat_status = jsonData.value("combat_status", AirArray{});
    gameState.skipped_moves = jsonData.value("skipped_moves", BfAirAirArray{});
}