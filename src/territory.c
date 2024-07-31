#include "territory_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Territory* getTerritoryByName(char* name) {
  for (int i = 0; i < TERRITORIES_COUNT; i++) {
    if (strcmp(Territories[i].name, name) == 0) {
      return &Territories[i];
    }
  }
  printf("Territory name: %s not found\n", name);
  exit(1);
}
