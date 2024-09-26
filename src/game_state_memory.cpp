#include "game_state_memory.hpp"
#include "game_cache.hpp"
#include "land.hpp"
#include "player.hpp"
#include "units/units.hpp"
#include <array>
#include <iomanip>
#include <sstream>
#include <sys/types.h>
#include <tuple>

constexpr uint UNIT_NAME_WIDTH = 14;

std::string get_printable_status(GameStateMemory& state) {
  std::ostringstream oss;
  oss << "---\n";
  oss << get_printable_status_lands(state);
  oss << get_printable_status_seas(state) << "\n";
  oss << PLAYERS[state.current_turn].color;
  oss << PLAYERS[state.current_turn].name;
  oss << "\033[0m" << ": " << state.money[0] << " IPC\n";
  return oss.str();
}

void append_land_unit(std::ostringstream& oss, uint player_idx, uint unit_idx, uint land_idx,
                      GameStateMemory& state) {

  uint unit_count = static_cast<const PlayerLandArray*>(get_idle_land_units(state).at(unit_idx))
                        ->val(player_idx, land_idx);
  if (unit_count > 0) {
    oss << PLAYERS[player_idx].color;
    oss << PLAYERS[player_idx].name;
    oss << " ";
    oss << std::left << std::setw(UNIT_NAME_WIDTH) << NAMES_UNIT_LAND[unit_idx];
    oss << std::right << std::setw(3) << unit_count;
    const auto* units_here = static_cast<const std::array<uint, LANDS_COUNT * BOMBER_LAND_STATES>*>(
        get_active_land_units(state).at(unit_idx));
    for (uint cur_state = 0; cur_state < STATES_MOVE_LAND[unit_idx]; cur_state++) {
      oss << std::right << std::setw(3)
          << units_here->at(land_idx * STATES_MOVE_LAND[unit_idx] + cur_state);
    }
    oss << "\n";
  }
}

// #define APPEND_LAND_UNIT(UNIT_TYPE, INDEX)                                                         \
//   append_land_unit(oss, player_idx, INDEX, state.idle_land_##UNIT_TYPE.val(player_idx, land_idx),  \
//                    state.active_land_##UNIT_TYPE.arr(land_idx))
//     APPEND_LAND_UNIT(bombers, FIGHTERS_LAND);
//     APPEND_LAND_UNIT(bombers, BOMBERS_LAND_AIR);
//     APPEND_LAND_UNIT(infantry, INFANTRY);
//     APPEND_LAND_UNIT(artillery, ARTILLERY);
//     APPEND_LAND_UNIT(tanks, TANKS);
//     APPEND_LAND_UNIT(aaguns, AAGUNS);

void get_printable_active_land_units(std::ostringstream& oss, GameStateMemory& state,
                                     GameCache& cache) {
  uint player_idx = state.current_turn;
  const char* my_color = PLAYERS[player_idx].color;
  oss << my_color;
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    if (cache.total_player_units.val(player_idx, land_idx) > 0) {
      for (uint unit_idx = 0; unit_idx < LAND_UNIT_TYPES_COUNT; unit_idx++) {
        append_land_unit(oss, player_idx, unit_idx, land_idx, state);
      }
    }
  }
}

std::string get_printable_status_lands(GameStateMemory& state, GameCache& cache) {
  std::ostringstream oss;
  uint current_player_idx = state.current_turn;
  LandArray& land_owners = state.land_owners;
  LandArray& factory_dmg = state.factory_dmg;
  LandArray& factory_max = state.factory_max;
  oss << "                 |Tot| 0| 1| 2| 3| 4| 5| 6|\n";
  for (uint land_idx = 0; land_idx < LANDS_COUNT; land_idx++) {
    uint land_owner = land_owners[land_idx];
    oss << PLAYERS[land_owner].color << land_idx << " ";
    oss << LANDS[land_idx].name << ": " << PLAYERS[land_owner].name;
    oss << " " << state.builds_left.at(land_idx);
    oss << "/" << factory_dmg[land_idx];
    oss << "/" << factory_max[land_idx];
    oss << "/" << LANDS[land_idx].land_value;
    oss << " Combat: " << state.combat_status.at(land_idx);
    get_printable_active_land_units(oss, state, cache);
    oss << "\033[0m";
    for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      if (current_player_idx == player_idx ||
          cache.total_player_units.val(player_idx, land_idx) == 0) {
        continue;
      }
      oss << PLAYERS[player_idx].color;
      for (uint land_unit_idx = 0; land_unit_idx < LAND_UNIT_TYPES_COUNT; land_unit_idx++) {
        uint unit_count = state.idle_land_fighters.val(player_idx, land_idx);
        if (unit_count > 0) {
          oss << PLAYERS[player_idx].name;
          oss << " ";
          oss << std::left << std::setw(UNIT_NAME_WIDTH) << NAMES_UNIT_LAND[land_unit_idx];
          oss << std::right << std::setw(3) << unit_count;
          oss << "\n";
        }
      }
      oss << "\033[0m";
    }
  }
  oss << "\n";
  return oss.str();
}

std::string get_printable_status_seas(GameStateMemory& state) {
  std::ostringstream oss;
  for (uint sea_index = 0; sea_index < SEAS_COUNT; sea_index++) {
    uint grand_total = 0;
    for (uint player_idx = 0; player_idx < PLAYERS_COUNT; player_idx++) {
      grand_total += total_player_sea_units[player_idx][sea_index];
    }
    if (grand_total == 0) {
      continue;
    }
    oss << (LANDS_COUNT + sea_index) << " " << SEAS[sea_index].name << " Combat:";
    if (state.combat_status.at(sea_index + LANDS_COUNT) > 0) {
      oss << "true\n";
    } else {
      oss << "false\n";
    }
    oss << "                 |Tot| 0| 1| 2| 3| 4| 5| 6|\n";
    oss << PLAYERS[state.current_turn].color;

    if (total_player_sea_units[0][sea_index] > 0) {
      for (uint sea_unit_idx = 0; sea_unit_idx < SEA_UNIT_TYPES_COUNT; sea_unit_idx++) {
        uint unit_count = current_player_sea_unit_types[sea_index][sea_unit_idx];
        if (unit_count > 0) {
          oss << PLAYERS[state.current_turn].name << " ";
          oss << std::left << std::setw(UNIT_NAME_WIDTH) << NAMES_UNIT_SEA[sea_unit_idx];
          oss << std::right << std::setw(3) << unit_count;
          for (uint cur_state = 0; cur_state < STATES_MOVE_SEA[sea_unit_idx]; cur_state++) {
            oss << std::right << std::setw(3)
                << sea_units_state[sea_index][sea_unit_idx][cur_state];
          }
          oss << "\n";
        }
      }
    }
    oss << "\033[0m";
    for (uint player_idx = 1; player_idx < PLAYERS_COUNT; player_idx++) {
      if (total_player_sea_units[player_idx][sea_index] == 0) {
        continue;
      }
      oss << PLAYERS[(state.current_turn + player_idx) % PLAYERS_COUNT].color;
      for (uint sea_unit_idx = 0; sea_unit_idx < SEA_UNIT_TYPES_COUNT; sea_unit_idx++) {
        uint unit_count = total_player_sea_unit_types[player_idx][sea_index][sea_unit_idx];
        if (unit_count > 0) {
          oss << PLAYERS[(state.current_turn + player_idx) % PLAYERS_COUNT].name << " ";
          oss << std::left << std::setw(UNIT_NAME_WIDTH) << NAMES_UNIT_SEA[sea_unit_idx];
          oss << std::right << std::setw(3) << unit_count;
          oss << "\n";
        }
      }
      oss << "\033[0m";
    }
  }
  printableGameStatus = oss.str();
}
