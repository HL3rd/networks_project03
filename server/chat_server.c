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
#include <pthread.h>

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
int accept_client(int server_fd, struct sockaddr_in client) {
    socklen_t client_len = sizeof(struct sockaddr);

    // accept the incoming connection by creating a new socket for the client
    int client_fd = accept(server_fd, &client, &client_len);
    if (client_fd < 0) {
        fprintf(stderr, "%s:\tError:\tUnable to accept client: %s\n", __FILE__, strerror(errno));
    }

    return client_fd;
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

// add a client to the queue
void queue_add(client_t *new_client){
    pthread_mutex_lock(&clients_mutex);
    int i;
    for (i = 0; i < MAX_CLIENTS; ++i) {
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
    int i;
    for (i = 0; i < MAX_CLIENTS; ++i) {
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
    int i;
    for (i = 0; i < MAX_CLIENTS; ++i) {
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
    int i;
    for (i = 0; i < MAX_CLIENTS; ++i){
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

    char* port = argv[1];

    int socket_desc, client_fd, c;
    struct sockaddr_in server, client;
    pthread_t tid;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
        //print the error message
        perror("Bind failed\n");
        return 1;
    }

    //Listen
    listen(socket_desc , 3);

    // process incoming connections
    while (1) {
        printf("Waiting for connection...\n");

        // accept client connections with multithreading
        client_fd = accept_client(client_fd, client);

        // print that connection is established
        printf("Connection accepted\n");

        // set a client
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->addr = client;
        cli->client_fd = client_fd;
        cli->uid = uid++;
        sprintf(cli->name, "%d", cli->uid);

        // add client to the queue and fork thread
        queue_add(cli);
        if (pthread_create(&tid, NULL, &connection_handler, (void*)cli) < 0) {
            perror("Could not create thread\n");
            return 1;
        }

        // in order to reduce CPU usage
        sleep(1);
    }
}
