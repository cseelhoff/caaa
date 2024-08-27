#include "engine.h"
#include <stdio.h>
#include <sys/types.h>

int main() {
  printf("Starting CAAA \n");
  initializeGameData();
  for (ushort seed = 1507; seed < 2000; seed++) {
    set_seed(seed);
    load_game_data("game_data.json");
    int turns = 100;
    while (turns--) {
      play_full_turn();
      double score = get_score();
      printf("Score: %f\n", score);
    }
  }
  return 0;
}