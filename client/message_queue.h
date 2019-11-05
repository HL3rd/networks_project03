/* message_queue.h */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "message.h"

struct message_queue_t {
    struct message_t *head;
    struct message_t *tail;
};

struct message_queue_t *message_queue_init();
void message_queue_push(struct message_queue_t *queue, struct message_t *message);
struct message_t *message_queue_pop(struct message_queue_t *queue);
void message_queue_destroy(struct message_queue_t *queue);

#endif
