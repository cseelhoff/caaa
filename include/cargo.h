#ifndef CARGO_H
#define CARGO_H

#include "player.h"
#include "team.h"
#include "unit_type.h"
#include <sys/types.h>

typedef struct {
  UnitTypes* unit_types;
  int count;
} Cargos;

Cargos createCargos(const UnitTypes unitTypes, const Teams teams);
u_int8_t getMaxCargoCapacity(const UnitTypes unitTypes);

#endif
