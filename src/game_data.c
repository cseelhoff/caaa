#include "game_data.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>

// Existing gameDataInstance definition
GameData gameDataInstance = {0};

void createDummyGameState() {
    int dummyGameState[] = {
        0, // Current Player is allied with Next Player T/F
        0, // Player Money
        0, // Next Player is allied with Next,Next Player T/F
        0, // Next Player Money
        0, // Territory1 Land Value
        0, // T1 is water T/F
        0, // T1 original owner (player_index offset from current - unsign char)
        0, //
    };
}

// Function to read the entire content of a file into a string
char* readFileToString(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = (char*)malloc(length + 1);
    if (content) {
        fread(content, 1, length, file);
        content[length] = '\0'; // Null-terminate the string
    }

    fclose(file);
    return content;
}

// Modified initializeGameData function
GameData* initializeGameData() {
    char* jsonString = readFileToString("/data/unit_types.json");
    if (jsonString == NULL) {
        printf("Failed to read unit types JSON file\n");
        return &gameDataInstance;
    }

    cJSON* json = cJSON_Parse(jsonString);
    if (json == NULL) {
        printf("Failed to parse unit types JSON\n");
        free(jsonString);
        return &gameDataInstance;
    }

    cJSON* unitTypesArray = cJSON_GetObjectItemCaseSensitive(json, "unitTypes");
    if (!cJSON_IsArray(unitTypesArray)) {
        printf("unitTypes is not an array\n");
        cJSON_Delete(json);
        free(jsonString);
        return &gameDataInstance;
    }

    int index = 0;
    cJSON* unitType;
    cJSON_ArrayForEach(unitType, unitTypesArray) {
        if (index >= UNIT_TYPE_COUNT) break; // Prevent overflow

        cJSON* id = cJSON_GetObjectItemCaseSensitive(unitType, "id");
        cJSON* name = cJSON_GetObjectItemCaseSensitive(unitType, "name");
        // Assuming UnitType struct has id and name fields
        if (cJSON_IsNumber(id) && cJSON_IsString(name)) {
            gameDataInstance.unitTypes[index].id = id->valueint;
            strncpy(gameDataInstance.unitTypes[index].name, name->valuestring, sizeof(gameDataInstance.unitTypes[index].name) - 1);
            gameDataInstance.unitTypes[index].name[sizeof(gameDataInstance.unitTypes[index].name) - 1] = '\0'; // Ensure null-termination
        }
        index++;
    }

    cJSON_Delete(json);
    free(jsonString);

    return &gameDataInstance;
}
