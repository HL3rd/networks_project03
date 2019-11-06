/* command_handler.c */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
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
        usleep(250);
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
        usleep(250);
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
    // tell the server that you want to send private message
    fputs("CP\n", client_file); fflush(client_file);

    // receive a list of active clients
    printf("Online Users:\n");
    int i = 0;
    struct message_t *incoming_message;
    char *online_users[BUFSIZ] = {0};
    do {
        incoming_message = message_queue_pop(message_queue);
        if (!incoming_message) {
            continue;
        }

        rstrip(incoming_message->message);
        if (streq(incoming_message->message, "_EOF"))  {
            printf("\n");
            break;
        }

        printf("%d) ", i + 1);
        online_users[i++] = incoming_message->message;
        fputs(incoming_message->message, stdout); printf("\n"); fflush(stdout);
        usleep(250);
    } while (1);

    // determine who the client wants to send to and send this to the server
    char target_user[BUFSIZ] = {0};
    printf("Enter user name >> ");
    fgets(target_user, BUFSIZ, stdin);
    rstrip(target_user);
    while (string_in_string_array(target_user, online_users, BUFSIZ) != 0) {
        fprintf(stderr, "User does not exist.\n");
        printf("Please re-enter user name >> ");
        memset(target_user, 0, BUFSIZ);
        fgets(target_user, BUFSIZ, stdin);
        rstrip(target_user);
    }

    char target_user_full[BUFSIZ + 1] = {0};
    target_user_full[0] = 'C';
    strcat(target_user_full, target_user);
    strcat(target_user_full, "\n");
    fputs(target_user_full, client_file); fflush(client_file);

    // get the message to be sent and send it to the server
    char private_message[BUFSIZ] = {0};
    printf("Enter private message >> ");
    fgets(private_message, BUFSIZ, stdin);
    char private_message_full[BUFSIZ + 1] = {0};
    private_message_full[0] = 'D';
    strcat(private_message_full, private_message);
    fputs(private_message_full, client_file); fflush(client_file);

    // receive server confirmation
    do {
        incoming_message = message_queue_pop(message_queue);
        usleep(250);
    } while (!incoming_message);

    if (!streq(incoming_message->message, "message sent")) {
        if (streq(incoming_message->message, "invalid user")) {
            fprintf(stderr, "%s:\terror:\tuser is no longer online\n", __FILE__);
        } else {
            fprintf(stderr, "%s:\terror:\treceived bad confirmation status from the server\n", __FILE__);
        }

        return 1;
    }

    printf("Message sent.\n"); fflush(stdout);
    printf("\n");

    return 0;
}

int history_handler(FILE *client_file, struct message_queue_t *message_queue) {
    fputs("CH\n", client_file); fflush(client_file);
    printf(" ####################### Chat History: #######################\n");
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
    return 0;
}

int exit_handler(FILE *client_file, struct message_queue_t *message_queue) {
    fputs("CX\n", client_file); fflush(client_file);
    fclose(client_file);
    return 0;
}
