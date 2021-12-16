#include <stdio.h>
#include <string.h>
#include "lma_cache.h"

int cache_retrieve_host(char *id, entry_t *entry) {
    int ret = -1;
    FILE* fp;

    fp = fopen("/tmp/wihand.bin", "rb");
    if (fp) {
        while(fread(entry, sizeof(*entry), 1, fp)) {
            if (strcmp(entry->id, id) == 0) {
                ret = entry->expired_at > 0 && entry->created_at + time(NULL) > entry->expired_at;
                ret |= entry->session_timeout > 0 && entry->session_time > entry->session_timeout;
                break;
            }
        }

        fclose(fp);
    }

    return ret;
}

int cache_persist_host(entry_t *entry) {
    int ret = -1, read;
    FILE* fp;

    fp = fopen("/tmp/wihand.bin", "a+b");
    if (fp) {
        read = fwrite(entry, sizeof(*entry), 1, fp);

        fclose (fp);

        ret = read != 1;
    }

    return ret;
}

int cache_update_host(entry_t *entry) {
    int ret = -1;
    FILE* fp;
    int p = 0, found = 0, write;
    entry_t entry_read;

    fp = fopen("/tmp/wihand.bin", "rb+");
    if (fp) {
        while(fread(&entry_read, sizeof(entry_read), 1, fp)) {
            if (strcmp(entry_read.id, entry->id) == 0) {
                p = ftell(fp)-sizeof(entry_read);
                found = 1;
                break;
            }
        }

        if (found) {
            fseek(fp, p, SEEK_SET);
            write = fwrite(entry, sizeof(*entry), 1, fp);
            ret = write != 1;
        }

        fclose(fp);
    }

    return ret;
}
