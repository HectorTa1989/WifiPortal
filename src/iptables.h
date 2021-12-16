#ifndef _IPTABLES_H
#define _IPTABLES_H 1

int remove_rule_from_chain(const char *, const char *, const char *);
int read_chain_bytes(const char *, const char *, const char *, char *);
int check_chain_rule(const char *, const char *, const char *);
int check_chain_exists(const char *);
int add_mac_rule_to_chain(const char *, const char *, const char *, const char *);
int add_dest_rule(const char*, const char *, const char *, const char *);
int flush_chain(const char *, const char *);
int add_bandwidth_class_chain(int);
int limit_down_bandwidth(const char *, int);
int unlimit_down_bandwidth(const char *);

#endif
