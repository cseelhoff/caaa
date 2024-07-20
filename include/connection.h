#ifndef CONNECTION_H
#define CONNECTION_H

#include "territory.h" 

typedef struct Connection {
    Territory source_territory;
    Territory destination_territory;
    Territory required_territories[2];
} Connection;

#endif