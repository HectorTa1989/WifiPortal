#ifndef _HOST_H
#define _HOST_H 1

#include <time.h>
#include "radius.h"
#include "tc.h"

#define __DEFAULT_IDLE  60
#define __OUTGOING_FLUSH 100
#define __TRAFFIC_IN_FLUSH 110
#define __TRAFFIC_OUT_FLUSH 120
#define __OUTGOING_ADD 130
#define __TRAFFIC_IN_ADD 140
#define __TRAFFIC_OUT_ADD 150
#define __CHECK_AUTH 160
#define __READ_TRAFFIC_IN 170
#define __READ_TRAFFIC_OUT 180
#define __REMOVE_HOST 190
#define __FILTER_GLOBAL_ADD 200
#define __NAT_GLOBAL_ADD 210

typedef struct {
    unsigned int idle_timeout;
    unsigned int session_timeout;
    unsigned int b_up;
    unsigned int b_down;
    unsigned int max_traffic_in;
    unsigned int max_traffic_out;
    unsigned int max_traffic;
} limits_t;

/* Define the host proto */
typedef struct {
    char ip[20];
    char mac[18];
    char username[128];
    char status;
    int staled;
    int pstaled;
    time_t start_time;
    time_t stop_time;
    unsigned long traffic_in;
    unsigned long traffic_out;
    int idle;
    char session[20];
    limits_t limits;
} host_t;

int get_host_by_ip(host_t [], int, char *, host_t **);
void write_hosts_list(host_t *, int);
int authorize_host(char *);
int check_authorized_host(char *);
int update_hosts(host_t *, int, host_t *, int);
int dnat_host(host_t *);
void start_host(host_t *);
int auth_host(host_t *, char *, char *, char *, char *, int, char *, char *, char *, char *, char *, char *, FILE *);
int temporary_session(host_t *host);
int iptables_man(const int, char *, char *);
unsigned long read_traffic_data(char *, const int);
int check_host_limits(const host_t *);

#endif
