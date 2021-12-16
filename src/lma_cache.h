#ifndef _LMA_CACHE_H
#define _LMA_CACHE_H 1

#include "radius.h"
#include "host.h"

typedef struct {
    char id[20];
    unsigned int created_at;
    unsigned int expired_at;
    unsigned int session_timeout;
    unsigned int session_time;
    limits_t limits;
} entry_t;

int cache_retrieve_host(char *, entry_t *);
int cache_persist_host(entry_t *);
int cache_update_host(entry_t *);

#endif
