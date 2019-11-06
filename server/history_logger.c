/* history_logger.c */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#include "history_logger.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

FILE *history_logger_init(char *username) {
    char filename[BUFSIZ] = {0};
    strcat(filename, username);
    strcat(filename, ".txt");
    FILE *log = fopen(filename, "a+");
    if (!log) {
        printf("%s:\terror: failed to open %s: %s\n", __FILE__, filename, strerror(errno));
        return NULL;
    }

    return log;
}

int history_logger_add_entry(FILE *log, char type, char *src, char *dst, char *msg) {
    time_t t = time(NULL);
    struct tm local = *localtime(&t);
    char entry[BUFSIZ] = {0};
    char action[BUFSIZ] = {0};
    if (type == 'B') {
        strcat(action, "Broadcasted");
    } else if (type == 'P') {
        strcat(action, "Private message");
    }

    char to_from[BUFSIZ] = {0};
    strcat(to_from, "from ");
    strcat(to_from, src);
    if (dst) {
        strcat(to_from, " to ");
        strcat(to_from, dst);
    }

    sprintf(entry, "%02d-%02d-%d %02d:%02d:%02d\t%s %s:\t%s\n", local.tm_mday, local.tm_mon + 1, local.tm_year + 1900, local.tm_hour, local.tm_min, local.tm_sec, action, to_from, msg);
    fputs(entry, log);
    return 0;
}
