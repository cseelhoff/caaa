#include "engine.h"
#include <stdio.h>

int main() {
  printf("Starting CAAA \n");
  initializeGameData();
  load_game_data("game_data.json");
  while(true)
    play_full_turn();
  return 0;
}