/* client.h */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#ifndef CLIENT_H
#define CLIENT_H

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

struct client_t {
    FILE        *client_file;   // connection file descriptor
    FILE        *history_log;   // history log
    char        *username;      // client username

    struct client_t *prev;      // pointer to prev client in linked list
    struct client_t *next;      // pointer to next client in linked list
};

// initialize a client
struct client_t *client_init(FILE *client_file, char *username);

// destroy a client
void client_destroy(struct client_t *client);

#endif
