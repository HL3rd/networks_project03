/* message.h */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#ifndef MESSAGE_H
#define MESSAGE_H

struct message_t {
    char *message;
    struct message_t *next;
};

struct message_t *message_init(char *content);
void message_destroy(struct message_t *message);

#endif
