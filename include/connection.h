#ifndef CONNECTION_H
#define CONNECTION_H

struct Territory;

typedef struct Connection {
    struct Territory* destination_territory;
    struct Territory* required_territory[2];
} Connection;

#endif
