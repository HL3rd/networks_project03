/* utils.c */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#include <string.h>

#include "utils.h"

void rstrip(char *s) {
    if (!s || strlen(s) == 0) {
        return;
    }

    int i = strlen(s) - 1;
    while (i >= 0 && *(s + i) == '\n') {
        *(s + i) = 0;
        i--;
    }
}

void rstrip_c(char *s, char c) {
    if (!s || strlen(s) == 0) {
        return;
    }

    int i = strlen(s) - 1;
    while (i >= 0 && *(s + i) == c) {
        *(s + i) = 0;
        i--;
    }
}
