#ifndef _UTILS_H
#define _UTILS_H 1

void uppercase (char *);
void gen_random(char *, const int);
int replacechar(char *, char, char);
int get_mac(char *, char *);
void trim(char *);
void get_last_octects(char *, char *);
void writelog(FILE *, char *);

#endif
