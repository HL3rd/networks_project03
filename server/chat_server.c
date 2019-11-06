/* chat_server.c */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "client.h"
#include "client_list.h"
#include "auth.h"
#include "command_handler.h"
#include "history_logger.h"
#include "utils.h"

/* Define Macros */
#define streq(a, b) (strcmp(a, b) == 0)

/* Define Structures */
struct thread_arg_t {
    pthread_t *client_thread;
    FILE *client_file;
    struct client_list *active_clients;
};

/* Define Functions */
int open_socket(const char *port) {
    // get linked list of DNS results for corresponding host and port
    struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
    hints.ai_family     = AF_INET;      // return IPv4 choices
    hints.ai_socktype   = SOCK_STREAM;  // use TCP (SOCK_DGRAM for UDP)
    hints.ai_flags      = AI_PASSIVE;   // use all interfaces

    struct addrinfo *results;
    int status;
    if ((status = getaddrinfo(NULL, port, &hints, &results)) != 0) {    // NULL indicates localhost
        fprintf(stderr, "%s:\terror:\tgetaddrinfo failed: %s\n", __FILE__, gai_strerror(status));
        return -1;
    }

    // iterate through results and attempt to allocate a socket, bind, and listen
    int server_fd = -1;
    struct addrinfo *p;
    for (p = results; p != NULL && server_fd < 0; p = p->ai_next) {
        // allocate the socket
        if ((server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            fprintf(stderr, "%s:\terror:\tfailed to make socket: %s\n", __FILE__, strerror(errno));
            continue;
           }

        // bind the socket to the port
        if (bind(server_fd, p->ai_addr, p->ai_addrlen) < 0) {
            fprintf(stderr, "%s:\terror:\tfailed to bind: %s\n", __FILE__, strerror(errno));
            close(server_fd);
            server_fd = -1;
            continue;
        }

        // listen to the socket
        if (listen(server_fd, SOMAXCONN) < 0) {
            fprintf(stderr, "%s:\terror:\tfailed to listen: %s\n", __FILE__, strerror(errno));
            close(server_fd);
            server_fd = -1;
            continue;
        }
    }

    // free the linked list of address results
    freeaddrinfo(results);

    return server_fd;
}

FILE *accept_client(int server_fd) {
    struct sockaddr client_addr;
    socklen_t client_len = sizeof(struct sockaddr);

    // accept the incoming connection by creating a new socket for the client
    int client_fd = accept(server_fd, &client_addr, &client_len);
    if (client_fd < 0) {
        fprintf(stderr, "%s:\terror:\tfailed to accept client: %s\n", __FILE__, strerror(errno));
    }

    FILE *client_file = fdopen(client_fd, "w+");
    if (!client_file) {
        fprintf(stderr, "%s:\terror:\tfailed to fdopen: %s\n", __FILE__, strerror(errno));
        close(client_fd);
    }

    return client_file;
}

void *client_handler(void *arg) {
    struct thread_arg_t *thread_arg = (struct thread_arg_t *) arg;
    FILE *client_file = thread_arg->client_file;
    struct client_list *active_clients = thread_arg->active_clients;

    // receive username and password from the client
    char *username = malloc(BUFSIZ);
    if (!username) {
        fprintf(stderr, "%s:\terror:\tfailed to malloc username: %s\n", __FILE__, strerror(errno));
        fclose(client_file);
        exit(EXIT_FAILURE);
    }

    memset(username, 0, BUFSIZ);
    fgets(username, BUFSIZ, client_file);
    rstrip(username);

    char *password = user_is_registered(username);
    if (!password) {    // user is not registered
        fputs("new user\n", client_file); fflush(client_file);

        // if the client is a new user, save the password
        char new_password[BUFSIZ] = {0};
        fgets(new_password, BUFSIZ, client_file);
        rstrip(new_password);

        int register_status = user_register(username, new_password);
        if (register_status != 0) {
            fprintf(stderr, "%s:\terror:\tfailed to register user: %s\n", __FILE__, strerror(errno));
            fclose(client_file);
            exit(EXIT_FAILURE);
        }
    } else {
        fputs("user is registered\n", client_file); fflush(client_file);

        // if the client is an existing user, check the password credentials
        char password_attempt[BUFSIZ] = {0};
        while (fgets(password_attempt, BUFSIZ, client_file)) {
            int login_status = user_login(username, password_attempt);
            if (login_status == 0) {                // login success
                fputs("login successful\n", client_file); fflush(client_file);
                break;
            } else {
                if (login_status == -1) {           // username found but incorrect password
                    fputs("incorrect password\n", client_file); fflush(client_file);
                } else if (login_status == -2) {    // username not found
                    fputs("username not found\n", client_file); fflush(client_file);
                } else {                            // error opening the users registry file
                    fputs("server error\n", client_file); fflush(client_file);
                }
            }
        }
    }

    // create a struct client_t and add it to the client list
    struct client_t *new_client = client_init(client_file, username);
    if (!new_client) {
        fclose(client_file);
        exit(EXIT_FAILURE);
    }

    client_list_add(active_clients, new_client);

    while (1) {
        char buffer[BUFSIZ] = {0};
        while (fgets(buffer, BUFSIZ, client_file)) {
            if (buffer[0] != 0 && buffer[0] == 'C') {           // command message
                // handle the command appropriately
                if (buffer[1] != 0 && buffer[1] == 'B') {
                    if (broadcast_message_handler(active_clients, client_file, username) != 0) {
                        fprintf(stderr, "%s:\terror:\tfailed to broadcast message\n", __FILE__);
                        continue;
                    }
                } else if (buffer[1] != 0 && buffer[1] == 'P') {
                    if (private_message_handler(active_clients, client_file, username) != 0) {
                        fprintf(stderr, "%s:\terror:\tfailed to private message\n", __FILE__);
                        continue;
                    }
                } else if (buffer[1] != 0 && buffer[1] == 'H') {
                    if (history_handler(username, client_file) != 0) {
                        fprintf(stderr, "%s:\terror:\tfailed to get history\n", __FILE__);
                        continue;
                    }
                } else if (buffer[1] != 0 && buffer[1] == 'X') {
                    if (exit_handler(active_clients, client_file) != 0) {
                        fprintf(stderr, "%s:\terror:\tfailed to exit\n", __FILE__);
                        continue;
                    }
                } else {
                    fprintf(stderr, "%s:\terror:\tunexpected command message received: %s\n", __FILE__, buffer);
                    continue;
                }
            } else if (buffer[0] != 0 && buffer[0] == 'D') {    // data message
                fprintf(stderr, "%s:\terror:\tunexpected data message received\n", __FILE__);
                continue;
            } else {
                fprintf(stderr, "%s:\terror:\tinvalid message format for following message: %s\n", __FILE__, buffer);
                continue;
            }
        }
    }
}

/* Main Execution */
int main(int argc, char *argv[]) {
    // parse arguments
    if (argc != 2) {
        fprintf(stderr, "%s:\terror:\tincorrect number of arguments!\n", __FILE__);
        fprintf(stderr, "usage: %s [port]\n", __FILE__);
        return EXIT_FAILURE;
    }

    char *port = argv[1];
    struct client_list *active_clients = client_list_init();

    // open socket, bind socket to the port, and listen on socket
    int server_fd = open_socket(port);
    if (server_fd < 0) {
        return EXIT_FAILURE;
    }

    printf("Waiting for connections...\n");
    while (1) {
        // accept a client connection
        FILE *client_file = accept_client(server_fd);
        if (!client_file) {
            continue;
        }

        printf("Connection accepted.\n"); fflush(stdout);

        // generate thread
        struct thread_arg_t *arg = malloc(sizeof(struct thread_arg_t));
        if (!arg) {
            fprintf(stderr, "%s:\terror:\tfailed to malloc struct for thread argument: %s", __FILE__, strerror(errno));
            return EXIT_FAILURE;
        }

        pthread_t *thread = malloc(sizeof(pthread_t));
        arg->client_thread = thread;
        arg->client_file = client_file;
        arg->active_clients = active_clients;

        pthread_create(thread, NULL, client_handler, arg);
    }
}
