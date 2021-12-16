#ifndef _RADIUS_H
#define _RADIUS_H 1

#include <time.h>

/* Replies */
typedef struct {
    unsigned int idle;
    unsigned int session_timeout;
    unsigned int b_down;
    unsigned int b_up;
    unsigned int traffic_in;
    unsigned int traffic_out;
    unsigned int traffic_total;
} reply_t;

int radclient(char *, char *, char *, char *, char *, char *, reply_t *);
int radacct_start(char *, char *, char *, char *, char *, char *, char *, char *);
int radacct_stop(char *, time_t, unsigned long, unsigned long, char *, char *, char *, char *, char *);
int radacct_interim_update(char *, time_t, unsigned long, unsigned long, char *, char *, char *, char *, char *);

#endif
