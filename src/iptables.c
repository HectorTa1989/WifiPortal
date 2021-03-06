#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int flush_chain(const char *table, const char *chain) {
    char cmd[255];
    int retcode;

    snprintf(cmd, sizeof cmd, "iptables -w 2 -t %s -F %s", table, chain);
    retcode = system(cmd);

    return retcode;
}

int add_mac_rule_to_chain(const char *table, const char *chain, const char *mac, const char *policy) {
    char cmd[255];
    int retcode;

    snprintf(cmd, sizeof cmd, "iptables -w 2 -t %s -A %s -m mac --mac-source \"%s\" -j %s", table, chain, mac, policy);
    retcode = system(cmd);

    return retcode;
}

int add_dest_rule(const char* table, const char * chain, const char *dest, const char *policy) {
    char cmd[255];
    int retcode;

    snprintf(cmd, sizeof cmd, "iptables -w 2 -t %s -A %s -d %s -j %s", table, chain, dest, policy);
    retcode = system(cmd);

    return retcode;
}

int check_chain_rule(const char *table, const char *chain, const char *str) {
    char cmd[255];
    int retcode;

    snprintf(cmd, sizeof cmd, "iptables -w 2 -t %s -nvL %s | grep %s > /dev/null 2>&1", table, chain, str);
    retcode = system(cmd);

    return retcode;
}

int check_chain_exists(const char *chain) {
    char cmd[255];
    int retcode;

    snprintf(cmd, sizeof cmd, "iptables -w 2 -n --list %s > /dev/null 2>&1", chain);
    retcode = system(cmd);

    return retcode;
}

int read_chain_bytes(const char *table, const char *chain, const char *str, char *data) {
    char cmd[255];
    char pres[64] = "";
    int retcode = -1;
    FILE *fp;

    /* retrieve the chain bytes */
    snprintf(cmd,
            sizeof cmd, "iptables -w 2 -t %s -nxvL %s | grep %s | awk '{ print $2 }' 2> /dev/null",
            table,
            chain,
            str);

    fp = popen(cmd, "r");

    if (fp) {
        fgets(pres, sizeof(pres)-1, fp);
        retcode = pclose(fp);

        if (retcode == 0) {
            strcpy(data, pres);
        }
    }

    return retcode;
}

int remove_rule_from_chain(const char *table, const char * chain, const char* str) {
    char cmd[255];
    char pres[64] = "";
    int retcode = -1;
    FILE *fp;

    /* search the rule to delete */
    snprintf(cmd,
            sizeof cmd, "iptables -w 2 -t %s -nvL %s --line-numbers | grep %s | tail -n 1 | awk '{ print $1 }'",
            table,
            chain,
            str);

    fp = popen(cmd, "r");

    if (fp) {
        fgets(pres, sizeof(pres)-1, fp);
        retcode = pclose(fp);

        if (retcode == 0 && atoi(pres) > 0) {
            /* rule found, delete it */
            snprintf(cmd, sizeof cmd, "iptables -w 2 -t %s -D %s %s", table, chain, pres);
            retcode = system(cmd);
        }
    }

    return retcode;
}

int add_bandwidth_class_chain(int kbps) {
    char cmd[255];
    int retcode;
    char chain[255];

    snprintf(chain, sizeof chain, "wihan_bclass_%dkbps", kbps);

    snprintf(cmd, sizeof cmd, "iptables -w 2 -N %s", chain);
    retcode = system(cmd);

    snprintf(cmd, sizeof cmd, "iptables -w 2 -A %s --match hashlimit --hashlimit-mode srcip --hashlimit-name dband_limit --hashlimit-upto %d/sec --hashlimit-burst 10 -j ACCEPT",
            chain, kbps/10);
    retcode |= system(cmd);

    snprintf(cmd, sizeof cmd, "iptables -w 2 -A %s -j DROP", chain);
    retcode |= system(cmd);

    return retcode;
}


/*
 * High level functions for throttling
 */

int limit_down_bandwidth(const char *ip, int kbps) {
    char chain[255];
    char str[255];
    int retcode = 0;

    snprintf(chain, sizeof chain, "wihan_bclass_%dkbps", kbps);

    /* check if chain exists */
    if (check_chain_exists(chain) != 0) {
        retcode = add_bandwidth_class_chain(kbps);
    }

    snprintf(str, sizeof str, "%s.*%s", chain, ip);

    if (retcode == 0 && check_chain_rule("filter", "FORWARD", str) != 0) {
        retcode = add_dest_rule("filter", "FORWARD", ip, chain);
    }

    return retcode;
}

int unlimit_down_bandwidth(const char *ip) {
    int retcode;

    retcode = remove_rule_from_chain("filter", "FORWARD", ip);

    return retcode;
}
