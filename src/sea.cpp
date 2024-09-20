#include "sea.h"
//  ATLANTIC         BALTIC         ATLANTIC
//  PACIFIC | USA | ATLANTIC | ENG | BALTIC | GER | RUS | JAP | PAC

const SeaStructs SEAS = {{
    {
        "Atlantic",
        3, // sea_conn_count
        2, // land_conn_count
        {0, 1, 2, -1, -1, -1, -1}, // connected_sea_index
        {0, 1, -1, -1, -1, -1} // connected_land_index
    },
    {
        "Pacific",
        2, // sea_conn_count
        3, // land_conn_count
        {0, 2, -1, -1, -1, -1, -1}, // connected_sea_index
        {0, 1, 2, -1, -1, -1} // connected_land_index
    },
    {
        "Indian",
        1, // sea_conn_count
        1, // land_conn_count
        {1, -1, -1, -1, -1, -1, -1}, // connected_sea_index
        {2, -1, -1, -1, -1, -1} // connected_land_index
    }
}};