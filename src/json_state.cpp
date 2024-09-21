#include "json_state.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool load_game_state_from_json(const std::string& filename, GameState& gameState) {
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
void to_json(json& jsonData, const LandState& lState) {
    jsonData = json{{"owner_idx", lState.owner_idx},
             {"factory_dmg", lState.factory_dmg},
             {"factory_max", lState.factory_max},
             {"bombard_max", lState.bombard_max},
             {"fighters", lState.fighters},
             {"bombers", lState.bombers},
             {"infantry", lState.infantry},
             {"artillery", lState.artillery},
             {"tanks", lState.tanks},
             {"aa_guns", lState.aa_guns}};
}

void from_json(const json& jsonData, LandState& lState) {
    jsonData.at("owner_idx").get_to(lState.owner_idx);
    jsonData.at("factory_dmg").get_to(lState.factory_dmg);
    jsonData.at("factory_max").get_to(lState.factory_max);
    jsonData.at("bombard_max").get_to(lState.bombard_max);
    jsonData.at("fighters").get_to(lState.fighters);
    jsonData.at("bombers").get_to(lState.bombers);
    jsonData.at("infantry").get_to(lState.infantry);
    jsonData.at("artillery").get_to(lState.artillery);
    jsonData.at("tanks").get_to(lState.tanks);
    jsonData.at("aa_guns").get_to(lState.aa_guns);
}

// Serialization and deserialization functions for UnitsSea
void to_json(json& jsonData, const UnitsSea& uSea) {
    jsonData = json({{"fighters", uSea.fighters},       {"trans_empty", uSea.trans_empty},
             {"trans_1i", uSea.trans_1i},       {"trans_1a", uSea.trans_1a},
             {"trans_1t", uSea.trans_1t},       {"trans_2i", uSea.trans_2i},
             {"trans_1i_1a", uSea.trans_1i_1a}, {"trans_1i_1t", uSea.trans_1i_1t},
             {"submarines", uSea.submarines},   {"destroyers", uSea.destroyers},
             {"carriers", uSea.carriers},       {"cruisers", uSea.cruisers},
             {"battleships", uSea.battleships}, {"bs_damaged", uSea.bs_damaged},
             {"bombers", uSea.bombers}});
}

void from_json(const json& jsonData, UnitsSea& uSea) {
    jsonData.at("fighters").get_to(uSea.fighters);
    jsonData.at("trans_empty").get_to(uSea.trans_empty);
    jsonData.at("trans_1i").get_to(uSea.trans_1i);
    jsonData.at("trans_1a").get_to(uSea.trans_1a);
    jsonData.at("trans_1t").get_to(uSea.trans_1t);
    jsonData.at("trans_2i").get_to(uSea.trans_2i);
    jsonData.at("trans_1i_1a").get_to(uSea.trans_1i_1a);
    jsonData.at("trans_1i_1t").get_to(uSea.trans_1i_1t);
    jsonData.at("submarines").get_to(uSea.submarines);
    jsonData.at("destroyers").get_to(uSea.destroyers);
    jsonData.at("carriers").get_to(uSea.carriers);
    jsonData.at("cruisers").get_to(uSea.cruisers);
    jsonData.at("battleships").get_to(uSea.battleships);
    jsonData.at("bs_damaged").get_to(uSea.bs_damaged);
    jsonData.at("bombers").get_to(uSea.bombers);
}

// Serialization and deserialization functions for GameState
void to_json(json& jsonData, const GameState& gameState) {
    jsonData = json{{"player_index", gameState.player_index},
             {"money", gameState.money},
             {"builds_left", gameState.builds_left},
             {"land_state", gameState.land_state},
             {"units_sea", gameState.units_sea},
             {"other_land_units", gameState.other_land_units},
             {"other_sea_units", gameState.other_sea_units},
             {"flagged_for_combat", gameState.flagged_for_combat},
             {"skipped_moves", gameState.skipped_moves}};
}

void from_json(const json& jsonData, GameState& gameState) {
    jsonData.at("player_index").get_to(gameState.player_index);
    jsonData.at("money").get_to(gameState.money);
    jsonData.at("builds_left").get_to(gameState.builds_left);
    jsonData.at("land_state").get_to(gameState.land_state);
    jsonData.at("units_sea").get_to(gameState.units_sea);
    jsonData.at("other_land_units").get_to(gameState.other_land_units);
    jsonData.at("other_sea_units").get_to(gameState.other_sea_units);
    jsonData.at("flagged_for_combat").get_to(gameState.flagged_for_combat);
    jsonData.at("skipped_moves").get_to(gameState.skipped_moves);
}
