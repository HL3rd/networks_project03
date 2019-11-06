/* command_handler.c */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#include "command_handler.h"
#include "utils.h"

#include <stdio.h>
#include <unistd.h>

#define streq(a, b) (strcmp(a, b) == 0)

int broadcast_message_handler(struct client_list *active_clients, FILE *client_file) {
    printf("BROADCAST\n");

    // send a "ready to receive message" response to the client
    fputs("Cready\n", client_file); fflush(client_file);

    // receive the broadcast message
    char message[BUFSIZ] = {0};
    fgets(message, BUFSIZ, client_file);

    printf("Message received: %s", &message[1]);
    if (message[0] != 'D') {
        fprintf(stderr, "%s:\terror:\treceived unexpected message: %s", __FILE__, message);
        return 1;
    }

    // broadcast the message
    char full_message[BUFSIZ + 1] = {0};
    strcat(full_message, "D");
    strcat(full_message, &message[1]);

    pthread_mutex_lock(&active_clients->mutex);
    struct client_t *current = active_clients->head;
    while (current) {
        if (client_file != current->client_file) {
            int copy_client_fd = dup(fileno(current->client_file));
            FILE *client_file_copy = fdopen(copy_client_fd, "w+");
            fputs(full_message, client_file_copy); fflush(client_file_copy);
        }

        current = current->next;
    }

    pthread_mutex_unlock(&active_clients->mutex);

    // send confirmation to the client that the message was sent
    fputs("Cmessage sent\n", client_file);
    return 0;
}

int private_message_handler(struct client_list *active_clients, FILE *client_file) {

    /*
        Client then prompts for, and sends the message to be sent.
        Server receives the above information and checks to make sure the target user exists/online.
        If the target user exists/online, the server forwards the message to the user, which displays the message. The server should do this by sending the message to the corresponding socket descriptor of the target user.
        Server sends confirmation that the message was sent or that the user did not exist. Note: You can decide the content/format of the confirmation.
        Client receives the confirmation from the server.
        Client and server return to "prompt user for operation" and "wait for operation from client" state, respectively.
    */

    printf("PRIVATE\n");

    // send a list of online users as a response to the client
    // send username to the server
    pthread_mutex_lock(&active_clients->mutex);
    struct client_t *current = active_clients->head;
    int user_count = 1;

    char online_users[BUFSIZ] = {0};
    sprintf(online_users, "%s\n", "COnline Users:");

    printf("Online users v1: %s\n", online_users);

    while (current) {
        if (client_file != current->client_file) {
            // append each name into message
            sprintf(online_users, "%d) %s\n", user_count, current->username);
            user_count += 1;
        }

        current = current->next;
    }
    printf("Online users v2: %s\n", online_users);
    // Send list of online users to the client
    strcat(online_users, "\n");
    fputs(online_users, client_file); fflush(client_file);

    pthread_mutex_unlock(&active_clients->mutex);

    // receive the broadcast message
    char username[BUFSIZ] = {0};
    fgets(username, BUFSIZ, client_file);

    printf("User Name received: %s", &username[1]);    // TODO: Delete this
    if (username[0] != 'D') {
        fprintf(stderr, "%s:\terror:\treceived unexpected message: %s", __FILE__, username);
        return 1;
    }

    // check to see if user is still online
    pthread_mutex_lock(&active_clients->mutex);

    int found = 0;
    current = active_clients->head;
    while (current->username || found == 0) {
        if (current->username == username) {
            found = 1;
            // TODO: Copy the client_file of matching target user
            break;
        }  // user is found
        current = current->next;
    }

    // send confirmation to the client that user is online
    if (found == 0) {
        fputs("user not online\n", client_file); fflush(client_file);
    } else {
        fputs("user is online\n", client_file); fflush(client_file);
    }

    pthread_mutex_unlock(&active_clients->mutex);

    // receive the broadcast message
    char message[BUFSIZ] = {0};
    fgets(message, BUFSIZ, client_file);

    printf("Message received: %s", &message[1]);
    if (message[0] != 'D') {
        fprintf(stderr, "%s:\terror:\treceived unexpected message: %s", __FILE__, message);
        return 1;
    }

    // TODO: Send the message

    // broadcast the message
    // char full_message[BUFSIZ + 1] = {0};
    // strcat(full_message, "D");
    // strcat(full_message, &message[1]);
    //
    // // receive the private message
    // char message[BUFSIZ] = {0};
    // fgets(message, BUFSIZ, client_file);
    //
    // printf("Message received: %s", &message[1]);
    // if (message[0] != 'D') {
    //     fprintf(stderr, "%s:\terror:\treceived unexpected message: %s", __FILE__, message);
    //     return 1;
    // }
    //
    // // broadcast the message
    // char full_message[BUFSIZ + 1] = {0};
    // strcat(full_message, "D");
    // strcat(full_message, &message[1]);
    //
    // pthread_mutex_lock(&active_clients->mutex);
    // struct client_t *current = active_clients->head;
    // while (current) {
    //     if (client_file != current->client_file) {
    //         int copy_client_fd = dup(fileno(current->client_file));
    //         FILE *client_file_copy = fdopen(copy_client_fd, "w+");
    //         fputs(full_message, client_file_copy); fflush(client_file_copy);
    //     }
    //
    //     current = current->next;
    // }
    //
    // pthread_mutex_unlock(&active_clients->mutex);
    //
    // // send confirmation to the client that the message was sent
    // fputs("Cmessage sent\n", client_file);
    return 0;
}

int history_handler(struct client_list *active_clients, FILE *client_file) {
    return 0;
}

int exit_handler(struct client_list *active_clients, FILE *client_file) {
    return 0;
}
