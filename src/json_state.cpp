#include <boost/hana.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include "game_state.hpp"

namespace hana = boost::hana;
using json = nlohmann::json;

// Define reflection metadata using Boost.Hana
//BOOST_HANA_ADAPT_STRUCT(BitField, bit);
BOOST_HANA_ADAPT_STRUCT(LandState, owner_idx, factory_dmg, factory_max, bombard_max, fighters, bombers, infantry, artillery, tanks, aa_guns);
BOOST_HANA_ADAPT_STRUCT(UnitsSea, fighters, trans_empty, trans_1i, trans_1a, trans_1t, trans_2i, trans_1i_1a, trans_1i_1t, submarines, destroyers, carriers, cruisers, battleships, bs_damaged, bombers);
BOOST_HANA_ADAPT_STRUCT(GameState, player_index, money, builds_left, land_state, units_sea, other_land_units, other_sea_units, flagged_for_combat, skipped_moves);

// Generic to_json function
template <typename T>
void to_json(json& j, const T& obj) {
    hana::for_each(hana::members(obj), [&](const auto& member) {
        j[hana::to<char const*>(hana::first(member))] = hana::second(member);
    });
}

// Generic from_json function
template <typename T>
void from_json(const json& j, T& obj) {
    hana::for_each(hana::members(obj), [&](auto& member) {
        using MemberType = typename std::decay<decltype(hana::second(member))>::type;
        j.at(hana::to<char const*>(hana::first(member))).get_to(hana::second(member));
    });
}

// Function to save GameState to a JSON file
void saveGameStateToFile(const GameState& gState, const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        json j = gState;
        file << j.dump(4); // Pretty print with 4 spaces
        file.close();
    } else {
        std::cerr << "Unable to open file for writing: " << filename << std::endl;
    }
}

// Function to load GameState from a JSON file
GameState loadGameStateFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        json j;
        file >> j;
        file.close();
        return j.get<GameState>();
    }
    std::cerr << "Unable to open file for reading: " << filename << std::endl;
    return GameState{};
}

int main() {
    // Example usage
    GameState gState;
    // Initialize gs with some data...

    // Save to file
    saveGameStateToFile(gState, "game_state.json");

    // Load from file
    GameState loaded_gs = loadGameStateFromFile("game_state.json");

    // Use loaded_gs...
    return 0;
}