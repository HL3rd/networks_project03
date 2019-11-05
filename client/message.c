/* message.c */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#include "message.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

struct message_t *message_init(char *content) {
    struct message_t *new_message = malloc(sizeof(struct message_t));
    if (!new_message) {
        fprintf(stderr, "%s:\terror:\tfailed to malloc for new struct message_t: %s\n", __FILE__, strerror(errno));
        return NULL;
    }

    char *message_literal = malloc(BUFSIZ);
    if (!message_literal) {
        fprintf(stderr, "%s:\terror:\tfailed to malloc for message literal: %s\n", __FILE__, strerror(errno));
        return NULL;
    }

    strcat(message_literal, content);
    new_message->message = message_literal;
    new_message->next = NULL;
    return new_message;
}

void message_destroy(struct message_t *message) {
    free(message->message);
    free(message);
}
