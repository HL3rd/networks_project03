/* utils.c */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#include <string.h>

#include "utils.h"

#define streq(a, b) (strcmp(a, b) == 0)

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

int string_in_string_array(char *s, char **arr, int size) {
    int i = 0;
    while (i < size && arr[i]) {
        if (streq(arr[i++], s)) {
            return 0;
        }
    }

    return 1;
}
