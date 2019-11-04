/* client_list.c */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#include "client_list.h"

struct client_list *client_list_init() {
    struct client_list *list = malloc(sizeof(struct client_list));
    if (!list) {
        fprintf(stderr, "%s:\terror: failed to initialize the client list: %s", __FILE__, strerror(errno));
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    pthread_mutex_init(&list->mutex, NULL);

    return list;
}

void client_list_add(struct client_list *list, struct client_t *client) {
    pthread_mutex_lock(&list->mutex);
    if (!list->head && !list->tail) {
        list->head = client;
        list->tail = client;

    } else {
        client->prev = list->tail;
        list->tail->next = client;
        list->tail = client;
    }

    pthread_mutex_unlock(&list->mutex);
}

void client_list_remove(struct client_list *list, char *client_username) {
    struct client_t *client_to_destroy = NULL;
    pthread_mutex_lock(&list->mutex);

    // case: client to remove is at the head of the list
    if (streq(list->head->username, client_username)) {
        client_to_destroy = list->head;
        if (!list->head->next) {
            list->head = NULL;
            list->tail = NULL;
        } else {
            list->head = list->head->next;
            list->head->prev = NULL;
        }
    }

    // case: client to remove is at the tail of the list
    else if (streq(list->tail->username, client_username)) {
        client_to_destroy = list->tail;
        list->tail = list->tail->prev;
        list->tail->next = NULL;
    }

    // case: client to remove is in the middle of the list
    else {
        struct client_t *current = list->head->next;
        while (current) {
            if (streq(current->username, client_username)) {
                client_to_destroy = current;
                current->prev->next = current->next;
                current->next->prev = current->prev;
                break;
            }

            current = current->next;
        }
    }

    if (!client_to_destroy) {   // username was not in the list of clients
        return;
    }

    client_destroy(client_to_destroy);
    list->size = list->size - 1;
    pthread_mutex_unlock(&list->mutex);
}

void client_list_print(struct client_list *list) {
    printf("Forward Traversal:\n");
    struct client_t *current = list->head;
    int i = 0;
    while (current) {
        printf("Client #%d: %s\n", i++, current->username);
        printf("\tprev: %p  next: %p\n", current->prev, current->next);
        current = current->next;
    }

    printf("Reverse Traversal:\n");
    current = list->tail;
    i = list->size - 1;
    while (current) {
        printf("Client #%d: %s\n", i--, current->username);
        printf("\tprev: %p  next: %p\n", current->prev, current->next);
        current = current->prev;
    }
}

void client_list_destroy(struct client_list *list) {
    if (list->size > 0) {
        pthread_mutex_lock(&list->mutex);
        struct client_t *current = list->head;
        struct client_t *next;
        while (current) {
            next = current->next;
            client_destroy(current);
            current = next;
        }

        pthread_mutex_unlock(&list->mutex);
    }

    pthread_mutex_destroy(&list->mutex);
    free(list);
}
