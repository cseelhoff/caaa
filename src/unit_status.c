#include <stddef.h>
/*
void unit_status_init(UnitStatus* status, UnitHealth unit_health, unsigned char
moves_remaining) { if (status != NULL) { status->unit_health = unit_health;
        status->moves_remaining = moves_remaining;
    }
}

void create_unit_statuses(UnitHealth* unit_healths, int unit_healths_count,
UnitStatus* moved_unit_statuses, int* moved_unit_statuses_count) { int index =
0; for (int i = 0; i < unit_healths_count; ++i) { UnitHealth current_health =
unit_healths[i]; for (unsigned char moves_remaining = 0; moves_remaining <=
current_health.unit_type.max_moves; ++moves_remaining) { if (index <
*moved_unit_statuses_count) { unit_status_init(&moved_unit_statuses[index],
current_health, moves_remaining);
                ++index;
            }
        }
    }
    *moved_unit_statuses_count = index;
}
*/