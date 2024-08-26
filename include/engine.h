#ifndef ENGINE_H
#define ENGINE_H
#include <stdbool.h>
#include <stdint.h>

#ifdef DEBUG
#define DEBUG_PRINT(msg) printf("DEBUG: %s\n", msg)
#else
#define DEBUG_PRINT(msg)
#endif

#define STRING_BUFFER_SIZE 64
#define MIN_AIR_HOPS 2
#define MAX_AIR_HOPS 6
#define MIN_SEA_HOPS 1
#define MAX_SEA_HOPS 2
#define MIN_LAND_HOPS 1
#define MAX_LAND_HOPS 2
#define AIR_MOVE_SIZE 1 + MAX_AIR_HOPS - MIN_AIR_HOPS
#define SEA_MOVE_SIZE 1 + MAX_SEA_HOPS - MIN_SEA_HOPS
#define LAND_MOVE_SIZE 1 + MAX_LAND_HOPS - MIN_LAND_HOPS
#define DEFENDER_LAND_UNIT_TYPES_COUNT 6
#define ATTACKER_LAND_UNIT_TYPES_COUNT_1 3
#define ATTACKER_LAND_UNIT_TYPES_COUNT_2 2
#define DEFENDER_SEA_UNIT_TYPES_COUNT 13
#define ATTACKER_SEA_UNIT_TYPES_COUNT_1 2
#define ATTACKER_SEA_UNIT_TYPES_COUNT_2 2
#define ATTACKER_SEA_UNIT_TYPES_COUNT_3 8
#define BLOCKADE_UNIT_TYPES_COUNT 5
#define PRINTABLE_GAME_STATUS_SIZE 4096

void initializeGameData();
void generate_total_air_distance();
void generate_total_land_distance();
void generate_total_sea_distance();
void generate_landMoveAllDestination();
void generate_airMoveAllDestination();
void generate_seaMoveAllDestination();
void generate_within_x_moves();
void generate_random_numbers();
void generate_unit_pointers();

void load_game_data();

void play_full_turn();
void refresh_quick_totals();
void refresh_cache();

void setPrintableStatus();
void setPrintableStatusLands();
void setPrintableStatusSeas();

uint8_t getUserInput(uint8_t* valid_moves, int valid_moves_count);
uint8_t getAIInput(uint8_t* valid_moves, int valid_moves_count);

void add_valid_land_move_if_history_allows_1(uint8_t* valid_moves, uint8_t* valid_moves_count,
                                             uint8_t dst_air, uint8_t src_land);
void add_valid_land_move_if_history_allows_2(uint8_t* valid_moves, uint8_t* valid_moves_count,
                                             uint8_t dst_air, uint8_t src_land);
void add_valid_sea_move_if_history_allows_1(uint8_t* valid_moves, uint8_t* valid_moves_count,
                                            uint8_t dst_sea, uint8_t src_sea);
void add_valid_sea_move_if_history_allows_2(uint8_t* valid_moves, uint8_t* valid_moves_count,
                                            uint8_t dst_sea, uint8_t src_sea);
void add_valid_sub_move_if_history_allows_2(uint8_t* valid_moves, uint8_t* valid_moves_count,
                                            uint8_t dst_sea, uint8_t src_sea);
void add_valid_air_move_if_history_allows_X(uint8_t* valid_moves, uint8_t* valid_moves_count,
                                            uint8_t dst_air, uint8_t src_air, uint8_t moves);
void clear_move_history();

void build_landMove2Destination();
void build_landMove1Destination();
void build_landMove1DestinationAlt();
void build_airMove2Destination();
void build_airMove3Destination();
void build_airMove4Destination();
void build_airMove5Destination();
void build_airMove6Destination();

uint8_t get_user_purchase_input(uint8_t src_air, uint8_t* valid_purchases,
                                uint8_t valid_purchases_count);
uint8_t get_user_move_input(uint8_t unit_type, uint8_t src_air, uint8_t* valid_moves,
                            uint8_t valid_moves_count);
void update_move_history(uint8_t user_input, uint8_t src_air, uint8_t* valid_moves,
                         uint8_t* valid_moves_count);
bool load_transport(uint8_t unit_type, uint8_t src_land, uint8_t dst_sea, uint8_t land_unit_state);
void add_valid_land_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_land,
                          uint8_t moves_remaining, uint8_t unit_type);
void add_valid_sea_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_sea,
                         uint8_t moves_remaining);
void add_valid_sub_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_sea,
                         uint8_t moves_remaining);
void stage_transport_units();
void move_fighter_units();
void move_bomber_units();
void conquer_land(uint8_t dst_land);
void move_land_unit_type(uint8_t unit_type);
void move_transport_units();
void move_subs();
void move_destroyers_battleships();
void carry_allied_fighters(uint8_t src_sea, uint8_t dst_sea);
void resolve_sea_battles();
uint8_t ask_to_retreat();
void remove_land_defenders(uint8_t src_land, uint8_t hits);
void remove_land_attackers(uint8_t src_land, uint8_t hits);
void remove_sea_defenders(uint8_t src_sea, uint8_t hits, bool defender_submerged);
void remove_sea_attackers(uint8_t src_sea, uint8_t hits);
void unload_transports();
void resolve_land_battles();
void add_valid_unload_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_sea);
void add_valid_fighter_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_air,
                             uint8_t remaining_moves);
void add_valid_fighter_landing(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_air,
                               uint8_t remaining_moves);
void add_valid_bomber_moves(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_air,
                            uint8_t remaining_moves);
void land_fighter_units();
void add_valid_bomber_landing(uint8_t* valid_moves, uint8_t* valid_moves_count, uint8_t src_air,
                              uint8_t movement_remaining);
void land_bomber_units();
void buy_units();
void crash_air_units();
void reset_units_fully();
void buy_factory();
void collect_money();
void rotate_turns();
double get_score();
void debug_checks();
#endif
