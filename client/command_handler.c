/* command_handler.c */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#include <string.h>

#include "command_handler.h"
#include "message.h"
#include "utils.h"

#define streq(a, b) (strcmp(a, b) == 0)

int broadcast_message_handler(FILE *client_file, struct message_queue_t *message_queue) {
    // tell the server that you want to broadcast and receive "okay"
    fputs("CB\n", client_file); fflush(client_file);
    struct message_t *incoming_message;
    do {
        incoming_message = message_queue_pop(message_queue);
    } while (!incoming_message);

    if (!streq(incoming_message->message, "ready")) {
        fprintf(stderr, "%s:\terror:\treceived bad ready status from the server\n", __FILE__);
        return 1;
    }

    // get the user's message to broadcast
    printf("Enter broadcast message >> ");
    char message[BUFSIZ - 1] = {0};
    fgets(message, BUFSIZ, stdin);

    // send the message to the server
    char full_message[BUFSIZ] = {0};
    full_message[0] = 'D';
    strcat(full_message, message);
    fputs(full_message, client_file); fflush(client_file);

    // receive confirmation that message was sent
    do {
        incoming_message = message_queue_pop(message_queue);
    } while (!incoming_message);

    if (!streq(incoming_message->message, "message sent")) {
        fprintf(stderr, "%s:\terror:\treceived bad confirmation status from the server\n", __FILE__);
        return 1;
    }

    printf("Message broadcasted.\n"); fflush(stdout);
    printf("\n");

    return 0;
}

int private_message_handler(FILE *client_file, struct message_queue_t *message_queue) {
    return 0;
}

int history_handler(FILE *client_file, struct message_queue_t *message_queue) {
    return 0;
}

int exit_handler(FILE *client_file, struct message_queue_t *message_queue) {
    return 0;
}
