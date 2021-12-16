#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <syslog.h>
#include <errno.h>

void uppercase ( char *sPtr )
{
    while ( *sPtr != '\0' ) {
        *sPtr = toupper ( ( unsigned char ) *sPtr );
        ++sPtr;
    }
}

void gen_random(char *s, const int len) {
    int i;
    static const char alphanum[] =
        "0123456789"
        "abcdef";

    for (i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}

int replacechar(char *str, char orig, char rep) {
    char *ix = str;
    int n = 0;

    while ((ix = strchr(ix, orig)) != NULL) {
        *ix++ = rep;
        n++;
    }

    return n;
}

int get_mac(char *iface, char *mac) {
    char path[60];
    FILE *f;
    int ret = -1, chread;

    snprintf(path, sizeof path, "/sys/class/net/%s/address", iface);

    f = fopen(path, "r");

    if (f) {
        chread = fscanf(f, "%s", mac);

        if (chread > 0) {
            ret = 0;
        }

        fclose(f);
    }

    return ret;
}

void trim(char *str)
{
  char *end;

  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)
    return;

  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  *(end+1) = 0;
}

void get_last_octects(char *ip, char *str) {
    char *token;
    char *buf;
    int i;

    buf = strdup(ip);

    token = strtok(buf, ".");

    for (i = 0; i < 3; i++) {
        token = strtok(NULL, ".");
    }

    strcpy(str, token);

    free(buf);
}

void writelog(FILE *log_stream, char *msg) {
    struct tm *sTm;
    char buff[20];
    int ret;

    time_t now = time (0);
    sTm = gmtime (&now);
    strftime (buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);
    ret = fprintf(log_stream, "[%s] %s\n", buff, msg);

    if (ret < 0) {
        syslog(LOG_ERR, "Can not write to log stream: %s", strerror(errno));
        return;
    }
    ret = fflush(log_stream);
    if (ret != 0) {
        syslog(LOG_ERR, "Can not fflush() log stream: %s", strerror(errno));
        return;
    }
}
