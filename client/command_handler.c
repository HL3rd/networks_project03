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

    // tell the server that you want to send a private message
    // receive the list of online users
    fputs("CP\n", client_file); fflush(client_file);

    struct message_t *incoming_message;

    do {
        incoming_message = message_queue_pop(message_queue);
        if (!incoming_message) {
            continue;
        }

        rstrip(incoming_message->message);
        if (streq(incoming_message->message, "_EOF"))  {
            break;
        }

        fputs(incoming_message->message, stdout); printf("\n"); fflush(stdout);
    } while (1);

    printf("\n");

    // get the target username
    printf("Enter User Name >> ");
    char usernameBuff[BUFSIZ - 1] = {0};
    fgets(usernameBuff, BUFSIZ, stdin);

    // send desired username to the server
    char full_message[BUFSIZ] = {0};
    full_message[0] = 'D';
    strcat(full_message, usernameBuff);
    fputs(full_message, client_file); fflush(client_file);

    // receive confirmation that message was sent
    do {
        incoming_message = message_queue_pop(message_queue);
    } while (!incoming_message);

    if (!streq(incoming_message->message, "user is online")) {
        fprintf(stderr, "%s:\terror:\tuser does not exist or is not online\n", __FILE__);
        return 1;
    }

    // get the user's message to other user
    printf("\nEnter Private Message >> ");
    char message[BUFSIZ - 1] = {0};
    fgets(message, BUFSIZ, stdin);

    // reinitialize the message buffer
    // then send the private message to the server
    bzero(full_message, BUFSIZ);
    full_message[0] = 'D';
    strcat(full_message, message);
    printf("Sending message --> %s\n", full_message);
    fputs(full_message, client_file); fflush(client_file);

    // receive confirmation that message was sent
    do {
        incoming_message = message_queue_pop(message_queue);
    } while (!incoming_message);

    if (!streq(incoming_message->message, "message sent")) {
        fprintf(stderr, "%s:\terror:\treceived bad confirmation status from the server\n", __FILE__);
        return 1;
    }

    return 0;
}

int history_handler(FILE *client_file, struct message_queue_t *message_queue) {
    return 0;
}

int exit_handler(FILE *client_file, struct message_queue_t *message_queue) {
    return 0;
}
