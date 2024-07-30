#include "cargo.h"
#include "team.h"
#include <sys/types.h>

Cargos createCargos(const UnitTypes unitTypes, const Teams teams) {
  Cargos cargos;
  u_int8_t maxCargoCapacity = getMaxCargoCapacity(unitTypes);
  return cargos;
}

u_int8_t getMaxCargoCapacity(const UnitTypes unitTypes) {
  u_int8_t maxCargoCapacity = 0;
  for (int i = 0; i < unitTypes.count; i++) {
    if (unitTypes.array[i].max_land > maxCargoCapacity) {
      maxCargoCapacity = unitTypes.array[i].max_land;
    }
  }
  return maxCargoCapacity;
}