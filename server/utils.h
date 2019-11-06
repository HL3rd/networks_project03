/* utils.h */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#ifndef UTILS_H
#define UTILS_H

void rstrip(char *s);
void rstrip_c(char *s, char c);
int string_in_string_array(char *s, char **arr, int size);

#endif
