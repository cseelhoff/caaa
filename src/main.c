#include "engine.h"
#include <stdio.h>
#include <sys/types.h>

int main() {
  printf("Starting CAAA \n");
  initializeGameData();
  ushort start_seed = 1;
  for (ushort seed = start_seed; seed < start_seed + 50; seed++) {
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