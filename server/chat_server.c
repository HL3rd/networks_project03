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

#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>

/* Define Macros */
#define streq(a, b) (strcmp(a, b) == 0)
#define MAX_BUFFER_SIZE 4096
#define MAX_CLIENTS 50
#define MIN(a,b) (((a)<(b)) ? (a) : (b))

/* Define Structures and Variables */
typedef struct {
    struct sockaddr_in addr; /* Client remote address */
    int client_fd;              /* Connection file descriptor */
    int uid;                 /* Client unique identifier */
    char name[32];           /* Client name */
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

static int uid = 10;
static int client_count = 0;

/* Define Utilities */
void rstrip(char a, char *s) {
    if (!s || strlen(s) == 0) {
        return;
    }

    int i = strlen(s) - 1;
    while (i >= 0 && *(s + i) == a) {
        *(s + i) = 0;
        i--;
    }
}

// accept client connection
int accept_client(int server_fd) {
    struct sockaddr client_addr;
    socklen_t client_len = sizeof(struct sockaddr);

    // accept the incoming connection by creating a new socket for the client
    int client_fd = accept(server_fd, &client_addr, &client_len);
    if (client_fd < 0) {
        fprintf(stderr, "%s:\terror:\tunable to accept client: %s\n", __FILE__, strerror(errno));
    }
}

// add a client to the queue
void queue_add(client_t *new_client){
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (!clients[i]) {
            clients[i] = new_client;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// remove client from the queue
void queue_delete(int uid){
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid == uid) {
                clients[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

/* Define Functions */
// function to send message to all clients except sender
void broadcast_message(char *s, int uid){
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid != uid) {
                if (write(clients[i]->client_fd, s, strlen(s)) < 0) {
                    perror("Write to descriptor failed");
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Send message to specific client
void send_private_message(char *s, int uid){
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i){
        if (clients[i]) {
            if (clients[i]->uid == uid) {
                if (write(clients[i]->client_fd, s, strlen(s))<0) {
                    perror("Write to descriptor failed");
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
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

    // open socket, bind socket to the port, and listen on socket
    int server_fd = open_socket(port);
    if (server_fd < 0) {
        return EXIT_FAILURE;
    }

    // process incoming connections
    while (1) {
        printf("Waiting for connection...\n");

        // accept client connections with multithreading
        client_fd = accecpt_client(server_fd);

        // print that connection is established
        printf("Connection established\n");

        // set a client
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->addr = cli_addr;
        cli->client_fd = client_fd;
        cli->uid = uid++;
        sprintf(cli->name, "%d", cli->uid);

        // add client to the queue and fork thread
        queue_add(cli);
        pthread_create(&tid, NULL, &handle_client, (void*)cli);

        // in order to reduce CPU usage
        sleep(1);
    }
}
