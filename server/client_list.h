/* client_list.h */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#ifndef CLIENT_LIST_H
#define CLIENT_LIST_H

#include "client.h"
#include <pthread.h>

#define streq(a, b) (strcmp(a, b) == 0)

struct client_list {
    struct client_t *head;
    struct client_t *tail;
    pthread_mutex_t mutex;
    int size;
};

struct client_list *client_list_init();

void client_list_add(struct client_list *list, struct client_t *client);

void client_list_remove(struct client_list *list, char *client_username);

void client_list_print(struct client_list *list);

void client_list_destroy(struct client_list *list);

void get_active_users(struct client_list *list, char **active_users, int size);

#endif
