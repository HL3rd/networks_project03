/* command_handler.h */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "client_list.h"

int broadcast_message_handler(struct client_list *active_clients, FILE *client_file, char *username);
int private_message_handler(struct client_list *active_clients, FILE *client_file, char *username);
int history_handler(char *username, FILE *client_file);
int exit_handler(struct client_list *active_clients, FILE *client_file);

#endif
