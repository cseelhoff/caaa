#include "territory.h" // Include the territory header file
#include <string.h> // For memset

void territory_init(Territory* territory, const char* name, unsigned char land_value, unsigned char original_owner_id, unsigned int index) {
    territory->name = name;
    territory->land_value = land_value;
    territory->is_water = false;
    territory->original_owner_id = original_owner_id;
    memset(territory->connections, 0, sizeof(territory->connections)); // Initialize arrays to 0
    memset(territory->water_connections, 0, sizeof(territory->water_connections));
    memset(territory->land_connections, 0, sizeof(territory->land_connections));
    territory->index = index;
    territory->owner = "";
    territory->owner_id = 0;
    territory->is_owned = false;
    territory->is_ally_owned = 0;
    territory->factory_max = 0;
    territory->factory_health = 0;
    territory->construction_remaining = 0;
    territory->recently_conquered = 0;
    territory->my_unit_stacks[0] = 1; // Example initialization
    // Initialize inactive_armies
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 1; j++) {
            territory->inactive_armies[i][j].unit_health = 0; // Assuming this is how you define it
            territory->inactive_armies[i][j].quantity = 0;
            // Initialize other fields of InactiveUnitStack as necessary
        }
    }
}

void territory_build_factory(Territory* territory) {
    territory->factory_max = territory->land_value;
    territory->factory_health = territory->land_value;
}

void territory_reset_factory(Territory* territory) {
    territory->construction_remaining = territory->factory_max;
}