#include "active_stack.h"
#include "unit_health.h"
#include "player.h"
#include <stdlib.h>
#include <string.h>

ActiveStacks createActiveStacks(const Players players, const UnitHealths unitHealths) {
    ActiveStacks activeStacks = malloc(sizeof(struct ActiveStacks));
    activeStacks->players = players;
    activeStacks->unitHealths = unitHealths;
    activeStacks->stacks = malloc(sizeof(ActiveStack) * players->count);
    for (int i = 0; i < players->count; i++) {
        activeStacks->stacks[i] = createActiveStack(unitHealths->count);
    }
    return activeStacks;
}