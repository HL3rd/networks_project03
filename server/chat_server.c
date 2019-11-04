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

/* Define Macros */
#define streq(a, b) (strcmp(a, b) == 0)

/* Define Structures */

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

// handle connection for each client
void *connection_handler(void *socket_desc) {
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];

    //Send some messages to the client
    message = "Greetings! I am your connection handler\n";
    write(sock , message , strlen(message));

    message = "Now type something and i shall repeat what you type \n";
    write(sock , message , strlen(message));

    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 ) {
        //end of string marker
		client_message[read_size] = '\0';

		//Send the message back to client
        write(sock , client_message , strlen(client_message));

		//clear the message buffer
		memset(client_message, 0, 2000);
    }

    if(read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if(read_size == -1) {
        perror("recv failed");
    }

    return 0;
}

// function to send message to all clients except sender
// void broadcast_message(char *s, int uid){
//     pthread_mutex_lock(&clients_mutex);
//     int i;
//     for (i = 0; i < MAX_CLIENTS; ++i) {
//         if (clients[i]) {
//             if (clients[i]->uid != uid) {
//                 if (write(clients[i]->client_fd, s, strlen(s)) < 0) {
//                     perror("Write to descriptor failed");
//                     break;
//                 }
//             }
//         }
//     }
//     pthread_mutex_unlock(&clients_mutex);
// }
//
// // Send message to specific client
// void send_private_message(char *s, int uid){
//     pthread_mutex_lock(&clients_mutex);
//     int i;
//     for (i = 0; i < MAX_CLIENTS; ++i){
//         if (clients[i]) {
//             if (clients[i]->uid == uid) {
//                 if (write(clients[i]->client_fd, s, strlen(s))<0) {
//                     perror("Write to descriptor failed");
//                     break;
//                 }
//             }
//         }
//     }
//     pthread_mutex_unlock(&clients_mutex);
// }

void * client_handler(void *arg) {
    struct client_t *client = (struct client_t *) arg;
    while (1) {
        char buffer[BUFSIZ];
        while (fgets(buffer, BUFSIZ, client->client_file)) {
            // determine if the message is a data message or a command message
            if (buffer[0] != 0 && buffer[0] == 'D') {            // data message

            } else if (buffer[0] != 0 && buffer[0] == 'C') {     // command message

            }

            fputs(buffer, stdout);
            fputs(buffer, client->client_file);
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

    printf("Waiting for connection...\n");
    while (1) {
        // accept a client connection
        FILE *client_file = accept_client(server_fd);
        if (!client_file) {
            continue;
        }

        // receive username and password from the client
        char username[BUFSIZ] = {0};
        fgets(username, BUFSIZ, client_file);

        char *password = user_is_registered(username);
        if (!password) {    // user is not registered
            fputs("no user", client_file);

            // if the client is a new user, save the password
            char new_password[BUFSIZ] = {0};
            fgets(new_password, BUFSIZ, client_file);
            int register_status = user_register(username, new_password);
            if (register_status != 0) {
                fprintf(stderr, "%s:\terror:\tfailed to register user: %s", __FILE__, strerror(errno));
                fclose(client_file);
                continue;
            }
        } else {
            fputs("user is registered", client_file);

            // if the client is an existing user, check the password credentials
            char password_attempt[BUFSIZ] = {0};
            while (fgets(password_attempt, BUFSIZ, client_file)) {
                if (streq(password_attempt, "control-c")) {
                    break;
                }

                int login_status = user_login(username, password_attempt);
                if (login_status == 0) {                // login success
                    fputs("pass", client_file);
                    break;
                } else {
                    if (login_status == -1) {           // username found but incorrect password
                        fputs("incorrect password", client_file);
                    } else if (login_status == -2) {    // username not found
                        fputs("username not found", client_file);
                    } else {                            // error opening the users registry file
                        fputs("server error", client_file);
                    }
                }
            }

            if (streq(password_attempt, "control-c")) {
                fclose(client_file);
                continue;
            }
        }

        // create a struct client_t and add it to the client list
        struct client_t *new_client = client_init(client_file, username);
        if (!new_client) {
            fclose(client_file);
            continue;
        }

        client_list_add(active_clients, new_client);

        // check to see if it is a new or existing user
        pthread_create(&new_client->thread, NULL, client_handler, new_client);
    }
}
