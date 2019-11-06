/* command_handler.c */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#include "command_handler.h"
#include "history_logger.h"
#include "utils.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define streq(a, b) (strcmp(a, b) == 0)

int broadcast_message_handler(struct client_list *active_clients, FILE *client_file, char *username) {
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
        // record in log
        int copy_history_log_fd = dup(fileno(current->history_log));
        FILE *log_copy = fdopen(copy_history_log_fd, "a");
        rstrip(&message[1]);
        history_logger_add_entry(log_copy, 'B', username, NULL, &message[1]);
        fclose(log_copy);

        // write to client stream if sender != current
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

int private_message_handler(struct client_list *active_clients, FILE *client_file, char *username) {
    printf("HISTORY\n");
    return 0;
}

int history_handler(char *username, FILE *client_file) {
    printf("HISTORY\n");

    // open up the history file
    char filename[BUFSIZ] = {0};
    strcat(filename, username);
    strcat(filename, ".txt");
    FILE *history = fopen(filename, "r");
    if (!history) {
        fprintf(stderr, "%s:\terror:\tfailed to open history file %s: %s", __FILE__, filename, strerror(errno));
        return 1;
    }

    // send the contents to the client
    char buffer[BUFSIZ] = {0};
    buffer[0] = 'C';
    while (fgets(&buffer[1], BUFSIZ - 1, history)) {
        fputs(buffer, client_file); fflush(client_file);
    }

    memset(buffer, 0, BUFSIZ);
    strcat(buffer, "C_EOF\n");
    fputs(buffer, client_file); fflush(client_file);

    // cleanup
    fclose(history);
    return 0;
}

int exit_handler(struct client_list *active_clients, char *username) {
    printf("EXIT\n");
    client_list_remove(active_clients, username);
    return 0;
}
