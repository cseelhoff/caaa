#include "territory.h" // Include the territory header file
#include <string.h>    // For memset

void territory_init(Territory* territory, const char* name,
                    uint8_t land_value, uint8_t original_owner_id,
                    unsigned int index) {}

void territory_build_factory(Territory* territory) {
  territory->factory_max = territory->land_value;
  territory->factory_health = territory->land_value;
}

void territory_reset_factory(Territory* territory) {
  territory->construction_remaining = territory->factory_max;
}