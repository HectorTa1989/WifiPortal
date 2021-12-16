#ifndef _TC_H
#define _TC_H 1

typedef struct {
    int classid;
    unsigned int bps;
} bandclass_t;

int init_bandwidth_stack(char *);
int deinit_bandwidth_stack(char *);
int register_bclass(char *, int, unsigned int, bandclass_t *);
int get_or_instance_bclass(bandclass_t [], int *, unsigned int, char *, bandclass_t **, int *);
int unregister_bclass(char *, bandclass_t);
int limit_down_band(char *, char *, bandclass_t *);
int limit_up_band(char *, char *, unsigned int);
int unlimit_up_band(char *, char *);
int unlimit_down_band(char *, char *);

#endif
