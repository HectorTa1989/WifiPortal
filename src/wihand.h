#ifndef _WIHAND_H
#define _WIHAND_H 1

#define __MAIN_INTERVAL 1
#define __ACCT_INTERVAL 300

typedef struct {
    char *iface;
    char *iface_network_ip;
    char *called_station;
    char *wan;
    char *allowed_garden;
    char *captiveurl;
    char *logfile;
    char *aaa_method;
    int macauth;
    char *radius_host;
    char *radius_authport;
    char *radius_acctport;
    char *radius_secret;
    char *nasidentifier;
    int lma;
    char *wai_port;
    char *ssl_cert;
    char *ssl_key;
} config_t;

#endif
