#include "game_state.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Serialization and deserialization functions for BitField
void to_json(json& ret_json, const BitField& bField) {
    ret_json = json{{"bit", static_cast<int>(bField.bit)}}; // Cast to int for serialization
}

void from_json(const json& const_json, BitField& bField) {
    bool temp = false;
    const_json.at("bit").get_to(temp);
    bField.bit = temp; // Assign the value back to the bit-field
}

// Serialization and deserialization functions for LandState
void to_json(json& ret_json, const LandState& lState) {
    ret_json = json{{"owner_idx", lState.owner_idx},
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

void from_json(const json& const_json, LandState& lState) {
    const_json.at("owner_idx").get_to(lState.owner_idx);
    const_json.at("factory_dmg").get_to(lState.factory_dmg);
    const_json.at("factory_max").get_to(lState.factory_max);
    const_json.at("bombard_max").get_to(lState.bombard_max);
    const_json.at("fighters").get_to(lState.fighters);
    const_json.at("bombers").get_to(lState.bombers);
    const_json.at("infantry").get_to(lState.infantry);
    const_json.at("artillery").get_to(lState.artillery);
    const_json.at("tanks").get_to(lState.tanks);
    const_json.at("aa_guns").get_to(lState.aa_guns);
}

// Serialization and deserialization functions for UnitsSea
void to_json(json& ret_json, const UnitsSea& uSea) {
    ret_json = json({{"fighters", uSea.fighters},       {"trans_empty", uSea.trans_empty},
             {"trans_1i", uSea.trans_1i},       {"trans_1a", uSea.trans_1a},
             {"trans_1t", uSea.trans_1t},       {"trans_2i", uSea.trans_2i},
             {"trans_1i_1a", uSea.trans_1i_1a}, {"trans_1i_1t", uSea.trans_1i_1t},
             {"submarines", uSea.submarines},   {"destroyers", uSea.destroyers},
             {"carriers", uSea.carriers},       {"cruisers", uSea.cruisers},
             {"battleships", uSea.battleships}, {"bs_damaged", uSea.bs_damaged},
             {"bombers", uSea.bombers}});
}

void from_json(const json& const_json, UnitsSea& uSea) {
    const_json.at("fighters").get_to(uSea.fighters);
    const_json.at("trans_empty").get_to(uSea.trans_empty);
    const_json.at("trans_1i").get_to(uSea.trans_1i);
    const_json.at("trans_1a").get_to(uSea.trans_1a);
    const_json.at("trans_1t").get_to(uSea.trans_1t);
    const_json.at("trans_2i").get_to(uSea.trans_2i);
    const_json.at("trans_1i_1a").get_to(uSea.trans_1i_1a);
    const_json.at("trans_1i_1t").get_to(uSea.trans_1i_1t);
    const_json.at("submarines").get_to(uSea.submarines);
    const_json.at("destroyers").get_to(uSea.destroyers);
    // Continue with the rest of the function...
}