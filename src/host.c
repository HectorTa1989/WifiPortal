#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "host.h"
#include "iptables.h"
#include "utils.h"
#include "lma_cache.h"

int get_host_by_ip(host_t hosts[], int hosts_len, char *ip, host_t **host) {
    int i = 0;
    host_t *target_host = NULL;

    for (i = 0; i < hosts_len; i++) {
        if (strcmp(hosts[i].ip, ip) == 0) {
            target_host = &hosts[i];
            *host = target_host;
            break;
        }
    }

    return target_host == NULL;
}

void write_hosts_list(host_t *hosts, int len) {
    FILE *status_file = NULL;
    char tbuff[20];
    char ebuff[20];
    int i;
    struct tm *sTm;

    status_file = fopen("/tmp/wihand.status", "w+");

    fprintf(status_file, "MAC\t\t\tStatus\tIdle\tSession Start\t\tSession Stop\t\tTraffic In\tTraffic Out\tSession\n");
    fprintf(status_file, "----------------------------------------------------------------------------------------------------------------------------------------\n");

    for (i = 0; i < len; i++) {
        strcpy(tbuff, "                   ");
        strcpy(ebuff, "                   ");

        if (hosts[i].start_time) {
            sTm = gmtime (&hosts[i].start_time);
            strftime (tbuff, sizeof(tbuff), "%Y-%m-%d %H:%M:%S", sTm);
        }

        if (hosts[i].stop_time) {
            sTm = gmtime (&hosts[i].stop_time);
            strftime (ebuff, sizeof(ebuff), "%Y-%m-%d %H:%M:%S", sTm);
        }

        if (hosts[i].traffic_in == 0 && hosts[i].traffic_out == 0) {
            fprintf(status_file, "%s\t%c\t%d\t%s\t%s\n\n", hosts[i].mac, hosts[i].status, hosts[i].idle, tbuff, ebuff);
        } else {
            fprintf(status_file, "%s\t%c\t%d\t%s\t%s\t\t%lu\t\t%lu\t%s\n",
                    hosts[i].mac,
                    hosts[i].status,
                    hosts[i].idle,
                    tbuff,
                    ebuff,
                    hosts[i].traffic_in,
                    hosts[i].traffic_out,
                    hosts[i].session);
        }
    }

    fclose(status_file);
}

int authorize_host(char *mac)
{
    int ret, ret_tia, ret_toa;

    ret = iptables_man(__OUTGOING_ADD, mac, NULL);
    ret_tia = iptables_man(__TRAFFIC_IN_ADD, mac, NULL);
    ret_toa = iptables_man(__TRAFFIC_OUT_ADD, mac, NULL);

    if (ret == 0 && ret_tia == 0 && ret_toa == 0)
        return 0;
    else
        return -1;
}

int check_authorized_host(char *mac)
{
    int ret;

    ret = iptables_man(__CHECK_AUTH, mac, NULL);

    return ret;
}

int update_hosts(host_t *hosts, int hosts_len, host_t *arp_cache, int arp_cache_len) {
    int i, h, found;
    int new_hosts_len = hosts_len;

    /* Check for new hosts */
    for (i = 0; i < arp_cache_len; i++) {

        /* Check if host exists in the daemon list */
        found = 0;
        for (h = 0; h < new_hosts_len; h++) {
            if (strcmp(hosts[h].mac, arp_cache[i].mac) == 0) {
                hosts[h].staled = arp_cache[i].staled;
                found = 1;
                break;
            }
        }

        if (!found) {
            /* New host found */
            strcpy(hosts[new_hosts_len].ip, arp_cache[i].ip);
            strcpy(hosts[new_hosts_len].mac, arp_cache[i].mac);
            hosts[new_hosts_len].staled = arp_cache[i].staled;

            new_hosts_len++;
        }
    }

    return new_hosts_len;
}

int dnat_host(host_t *host) {
    int ret;

    /* remove host from chains */
    ret = iptables_man(__REMOVE_HOST, host->mac, NULL);

    /* update host status */
    if (ret == 0) {
        host->status = 'D';
        host->stop_time = time(0);
    }

    return ret;
}

void start_host(host_t *host) {
    host->status = 'A';
    host->start_time = time(0);
    host->stop_time = NULL;
    host->idle = 0;
}

int auth_host(host_t *host,
              char *username,
              char *pass,
              char *iface,
              char *mode,
              int lma,
              char *nasid,
              char *called_station,
              char *radhost,
              char *radauthport,
              char *radacctport,
              char *radsecret,
              FILE *log_stream)
{
    int ret = 0;
    reply_t reply = {0, 0, 0, 0, 0, 0, 0};
    char logstr[255];
    entry_t cache_entry;
    limits_t limits;
    int lma_flag = 0;

    /* Try to auth the host */
    if (lma && cache_retrieve_host(host->mac, &cache_entry) == 0) {
        limits = cache_entry.limits;
        limits.session_timeout = cache_entry.session_timeout-cache_entry.session_time;
        lma_flag = 1;
        ret = 0;
    } else {
        if (strcmp(mode, "radius") == 0) {
            ret = radclient(username, pass, nasid, radhost, radauthport, radsecret, &reply);

            if (ret == 0) {
                limits.idle_timeout = (reply.idle > 0 ? reply.idle : __DEFAULT_IDLE);
                limits.session_timeout = reply.session_timeout;
                limits.b_up = reply.b_up;
                limits.b_down = reply.b_down;
                limits.max_traffic_in = reply.traffic_in;
                limits.max_traffic_out = reply.traffic_out;
                limits.max_traffic = reply.traffic_total;
            }
        }
    }

    snprintf(logstr, sizeof logstr, "Auth request %s%s for %s", (ret == 0) ? "AUTHORIZED" : "REJECTED",
            lma_flag ? " (lma)" : "", host->mac);
    writelog(log_stream, logstr);

    if (ret == 0) {
        /* set host status on auth response outcome */
        if (iptables_man(__OUTGOING_ADD, host->mac, NULL) == 0
                && iptables_man(__TRAFFIC_IN_ADD, host->mac, NULL) == 0
                && iptables_man(__TRAFFIC_OUT_ADD, host->mac, NULL) == 0)
        {
            start_host(host);
            snprintf(logstr, sizeof logstr, "Authorize host %s%s", host->mac, lma_flag ? " (lma)" : "");
            writelog(log_stream, logstr);

            /* set host limits */
            host->limits = limits;

            /* set auth username into host */
            strcpy(host->username, username);

            /* Set bandwidth */
            if (limits.b_up > 0) {
                if (limit_up_band(iface, host->ip, limits.b_up) == 0) {
                    snprintf(logstr, sizeof logstr, "Set up bandwidth limit to %d bps for host %s", limits.b_up, host->mac);
                    writelog(log_stream, logstr);
                } else {
                    snprintf(logstr, sizeof logstr, "Error in setting up bandwidth limit for host %s", host->mac);
                    writelog(log_stream, logstr);
                }
            }

            if (limits.b_down > 0) {
                if (limit_down_bandwidth(host->ip, limits.b_down/1000) == 0) {
                    snprintf(logstr, sizeof logstr, "Set down bandwidth limit to %d bps for host %s", limits.b_down, host->mac);
                    writelog(log_stream, logstr);
                } else {
                    snprintf(logstr, sizeof logstr, "Error in set down bandwidth limit for host %s", host->mac);
                    writelog(log_stream, logstr);
                }

            }

            /* execute start acct */
            ret = radacct_start(username,
                                host->mac,
                                called_station,
                                host->session,
                                nasid,
                                radhost,
                                radacctport,
                                radsecret);

            if (ret != 0) {
                snprintf(logstr, sizeof logstr, "Fail to execute radacct start for host %s", host->mac);
                writelog(log_stream, logstr);
            }
        } else {
            host->status = 'D';
        }

    }

    return ret;
}

int temporary_session(host_t *host) {
    limits_t limits;
    int ret = -1;

    limits.idle_timeout = __DEFAULT_IDLE;
    limits.session_timeout = 30; //TODO
    limits.max_traffic_in = 0;
    limits.max_traffic_out = 0;
    limits.max_traffic = 0;

    host->limits = limits;

    if (authorize_host(host->mac) == 0) {
        ret = 0;
    }

    return ret;
}

int iptables_man(const int action, char *mac, char *data) {
    int retcode;

    switch(action) {
        case __OUTGOING_FLUSH:
            retcode = flush_chain("mangle", "wlan0_Outgoing");

            break;
        case __TRAFFIC_IN_FLUSH:
            retcode = flush_chain("filter", "wlan0_Traffic_In");

            break;
        case __TRAFFIC_OUT_FLUSH:
            retcode = flush_chain("filter", "wlan0_Traffic_Out");

            break;
        case __OUTGOING_ADD:
            retcode = add_mac_rule_to_chain("mangle", "wlan0_Outgoing", mac, "MARK --set-mark 2");

            break;
        case __FILTER_GLOBAL_ADD:
            retcode = add_dest_rule("filter", "wlan0_Global", mac, "ACCEPT");

            break;
        case __NAT_GLOBAL_ADD:
            retcode = add_dest_rule("nat", "wlan0_Global", mac, "ACCEPT");

            break;
        case __TRAFFIC_IN_ADD:
            retcode = add_mac_rule_to_chain("filter", "wlan0_Traffic_In", mac, "ACCEPT");

            break;
        case __TRAFFIC_OUT_ADD:
            retcode = add_mac_rule_to_chain("filter", "wlan0_Traffic_Out", mac, "ACCEPT");

            break;
        case __CHECK_AUTH:
            retcode = check_chain_rule("mangle", "wlan0_Outgoing", mac);

            break;
        case __READ_TRAFFIC_IN:
            retcode = read_chain_bytes("filter", "wlan0_Traffic_In", mac, data);

            break;
        case __READ_TRAFFIC_OUT:
            retcode = read_chain_bytes("filter", "wlan0_Traffic_Out", mac, data);

            break;
        case __REMOVE_HOST:
            retcode = remove_rule_from_chain("mangle", "wlan0_Outgoing", mac)
                    || remove_rule_from_chain("filter", "wlan0_Traffic_In", mac)
                    || remove_rule_from_chain("filter", "wlan0_Traffic_Out", mac);
            break;
    }

    return retcode;
}

unsigned long read_traffic_data(char *mac, const int inout) {
    unsigned long res = 0;
    int ret;
    char bytes[64];

    ret = iptables_man(inout, mac, bytes);

    if (ret == 0) {
        if (strcmp(bytes, "") != 0) {
            res = atol(bytes);
        }
    }

    return res;
}

int check_host_limits(const host_t *host) {
    time_t curtime;
    int ret;

    curtime = time(NULL);
    ret = (host->limits.session_timeout > 0 && curtime - host->start_time > host->limits.session_timeout ||
        host->limits.max_traffic_in > 0 && host->traffic_in > host->limits.max_traffic_in ||
        host->limits.max_traffic_out > 0 && host->traffic_out > host->limits.max_traffic_out ||
        host->limits.max_traffic > 0 && host->traffic_in + host->traffic_out > host->limits.max_traffic);

    return ret;
}
