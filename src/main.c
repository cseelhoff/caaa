#include "engine.h"
#include <stdio.h>

int main() {
  printf("Starting CAAA \n");
  initializeGameData();
  load_game_data("game_data.json");
  set_seed(2);
  int turns = 100;
  while(turns--) {
    play_full_turn();
    double score = get_score();
    printf("Score: %f\n", score);
  }
  return 0;
}