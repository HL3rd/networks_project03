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

/* Define Structures and Variables */
struct client_t {
    struct sockaddr *addr;  /* Client remote address */
    int client_fd;          /* Connection file descriptor */
    int uid;                /* Client unique identifier */
    char *name;             /* Client username */

    struct client_t *next;  /* Pointer to next client in linked list */
    struct client_t *prev;  /* Pointer to prev client in linked list */
};

struct client_list {
    struct client_t *head;
    struct client_t *tail;
    pthread_mutex_t mutex;
    int size;
};

/* Define Functions */
struct client_list *client_list_init() {
    struct client_list *list = malloc(sizeof(struct client_list));
    if (!list) {
        fprintf(stderr, "%s:\terror: failed to initialize the client list: %s", __FILE__, strerror(errno));
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    pthread_mutex_init(&list->mutex, NULL);

    return list;
}

void client_list_add(struct client_list *list, struct client_t *client) {
    pthread_mutex_lock(&list->mutex);
    if (!list->head && !list->tail) {
        list->head = client;
        list->tail = client;

    } else {
        client->prev = list->tail;
        list->tail->next = client;
        list->tail = client;
    }

    client->uid = list->size++;
    pthread_mutex_unlock(&list->mutex);
}

void client_list_remove(struct client_list *list, int client_id) {
    struct client_t *client_to_destroy = NULL;
    pthread_mutex_lock(&list->mutex);

    // case: client to remove is at the head of the list
    if (list->head->uid == client_id) {
        client_to_destroy = list->head;
        if (!list->head->next) {
            list->head = NULL:
            list->tail = NULL;
        } else {
            list->head = list->head->next;
            list->head->prev = NULL;
        }
    }

    // case: client to remove is at the tail of the list
    else if (list->tail->uid == client_id) {
        client_to_destroy = list->tail;
        list->tail = list->tail->prev;
        list->tail->next = NULL;
        found_it = 1;
    }

    // case: client to remove is in the middle of the list
    else {
        struct client_t *current = list->head->next;
        while (current) {
            if (current->uid == client_id) {
                client_to_destroy = current;
                current->prev->next = current->next;
                current->next->prev = current->prev;
                break;
            }

            current = current->next;
        }
    }

    if (!client_to_destroy) {   // uid was not in the list of clients
        return;
    }

    client_destroy(client_to_destroy);
    list->size = list->size - 1;
    pthread_mutex_unlock(&list->mutex);
}

void client_list_destroy(struct client_list *list) {
    if (list->size > 0) {
        pthread_mutex_lock(&list->mutex);
        struct client_t *current = list->head;
        struct client_t *next;
        while (current) {
            next = current->next;
            client_destroy(current);
            current = next;
        }

        pthread_mutex_unlock(&list->mutex);
    }

    pthread_mutex_destroy(&list->mutex);
    free(list);
}

struct client_t *client_init(struct sockaddr addr, int client_fd, char *name) {
    struct client_t *client = malloc(sizeof(struct client_t));
    if (!client) {
        fprintf(stderr, "%s:\terror: failed to initialize the client: %s", __FILE__, strerror(errno));
        return NULL;
    }

    client->addr = addr;
    client->client_fd = client_fd;
    client->uid = -1;
    client->name = name;
    client->next = NULL;
    client->prev = NULL;

    return client;
}

void client_destroy(struct client_t *client) {
    free(client->addr);
    free(client->name);
    free(client);
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

    // Create socket
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

        // add client to the list and fork thread
        list_add(cli);
        if (pthread_create(&tid, NULL, &connection_handler, (void*)cli) < 0) {
            perror("Could not create thread\n");
            return 1;
        }

        // in order to reduce CPU usage
        sleep(1);
    }
}
