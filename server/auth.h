/* auth.h */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#ifndef AUTH_H
#define AUTH_H

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

char * user_is_registered(char *username);
int user_register(char *username, char *password);
int user_login(char *username, char *password);

#endif
