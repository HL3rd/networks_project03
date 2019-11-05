/* message_queue.c */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#include "message_queue.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

struct message_queue_t *message_queue_init() {
    struct message_queue_t *queue = malloc(sizeof(struct message_queue_t));
    if (!queue) {
        fprintf(stderr, "%s:\terror: failed to initialize the message queue: %s", __FILE__, strerror(errno));
        return NULL;
    }

    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

void message_queue_push(struct message_queue_t *queue, struct message_t *message) {
    if (!queue->head && !queue->tail) {
        queue->head = message;
        queue->tail = message;
    } else {
        queue->tail->next = message;
        queue->tail = message;
    }
}

struct message_t *message_queue_pop(struct message_queue_t *queue) {
    if (!queue->head) {
        return NULL;
    }

    struct message_t *popped = queue->head;
    if (!queue->head->next) {
        queue->head = NULL;
        queue->tail = NULL;
    } else {
        queue->head = queue->head->next;
    }

    return popped;
}

void message_queue_destroy(struct message_queue_t *queue) {
    struct message_t *current = queue->head;
    struct message_t *next;
    while (current) {
        next = current->next;
        message_destroy(current);
        current = next;
    }

    free(queue);
}
