#ifndef CARGO_H
#define CARGO_H

#include "team.h"
#include "unit_type.h"
#include <stdint.h>

typedef struct {
  UnitType unit_types[MAX_CARGO_COUNT];
  int count;
} Cargos;

Cargos createCargos();

#endif
