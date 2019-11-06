/* history_logger.h */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#ifndef HISTORY_LOGGER_H
#define HISTORY_LOGGER_H

#include <stdio.h>

FILE *history_logger_init(char *username);
int history_logger_add_entry(FILE *log, char type, char *src, char *dst, char *msg);

#endif
