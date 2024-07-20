#include <stdio.h>
#include "unit_type.h"

int main() {
    printf("Starting CAAA\n");
    printf("Reading JSON files\n");
    //create an array of UnitType of size 1
    UnitType* unit_Types = create_two_unit_types();

    return 0;
}

UnitType* create_two_unit_types() {
    // Allocate memory for two UnitType instances
    UnitType* unitTypeArray = UnitType[2];
    if (unitTypeArray == NULL) {
        // Allocation failed
        return NULL;
    }

    // Initialize the two UnitType instances if necessary
    // For example, setting 'id' fields to 0 and 1
    unitTypeArray[0].id = 0; // Assuming UnitType has an 'id' field
    unitTypeArray[1].id = 1;

    return unitTypeArray;
}