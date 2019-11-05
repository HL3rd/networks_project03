/* command_handler.h */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <stdio.h>
#include "message_queue.h"

int broadcast_message_handler(FILE *client_file, struct message_queue_t *message_queue);
int private_message_handler(FILE *client_file, struct message_queue_t *message_queue);
int history_handler(FILE *client_file, struct message_queue_t *message_queue);
int exit_handler(FILE *client_file, struct message_queue_t *message_queue);

#endif
