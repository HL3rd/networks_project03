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

    int port = atoi(argv[1]);

    // Initiilize variables
    int sock_fd, client_addr_len;
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    pthread_t tid;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // Establish the server's own ip address
    char host_buffer[256];

    printf("Getting hostname\n");
    if (gethostname(host_buffer, sizeof(host_buffer)) < 0) {
        fprintf(stderr, "%s: Failed to get current host name\n", argv[0]);
        exit(-1);
    };

    // printf("About to IP\n");
    // char* IPbuffer = inet_ntoa(*((struct in_addr*) gethostbyname(host_buffer)->h_addr_list[0]));
    // printf("Next\n");
    // server_addr.sin_addr.s_addr = inet_addr(IPbuffer);

    printf("About to socket\n");
    /* Initialize server to accept incoming connections */
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        fprintf(stderr, "%s: Failed to call socket\n", argv[0]);
        exit(-1);
    }

    printf("About to bind\n");
    if (bind(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 ) {
        fprintf(stderr, "%s: Failed to bind socket: %s\n", argv[0], strerror(errno));
        exit(-1);
    }

    printf("About to listen\n");
    if((listen(sock_fd, SOMAXCONN)) < 0) {
        fprintf(stderr, "%s: Failed to listen: %s\n", argv[0], strerror(errno));
        exit(-1);
    }

    printf("Waiting for connection...\n");

    client_addr_len = sizeof(client_addr);

    // accept client connections with multithreading
    // process incoming connections
    int client_fd;
    while ((client_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_addr_len))) {

        if (client_fd < 0) {
            fprintf(stderr, "%s: Failed to accept: %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }

        printf("Connection accepted\n");

        // set a client
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->addr = client_addr;
        cli->client_fd = client_fd;
        cli->uid = uid++;
        sprintf(cli->name, "%d", cli->uid);

        // char buf[MAX_BUFFER_SIZE];
        // memset(buf, 0, sizeof(buf));
        // socklen_t addr_len = sizeof(client_addr);
        // int n_received = receive_buffer(clientSocket, buf, 4);
        // print that connection is established

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
