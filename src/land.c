#include "land_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Land* getLandByName(char* name) {
  for (int i = 0; i < LANDS_COUNT; i++) {
    if (strcmp(Lands[i].name, name) == 0) {
      return &Lands[i];
    }
  }
  printf("Land name: %s not found\n", name);
  exit(1);
}
