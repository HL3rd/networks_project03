/* chat_client.c */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "message.h"
#include "message_queue.h"
#include "command_handler.h"
#include "utils.h"

/* Define Macros */
#define streq(a, b) (strcmp(a, b) == 0)

/* Define Structures */
struct thread_arg_t {
    FILE *client_file;
    struct message_queue_t *message_queue;
};

/* Define Functions */
FILE *open_socket(char *host, char *port) {
	// get linked list of DNS results for corresponding host and port
    struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
    hints.ai_family     = PF_INET;      // return IPv4 and IPv6 choices
    hints.ai_socktype   = SOCK_STREAM;  // use TCP (SOCK_DGRAM for UDP)
    hints.ai_flags      = AI_PASSIVE;   // use all interfaces

	struct addrinfo *results;
    int status;
    if ((status = getaddrinfo(host, port, &hints, &results)) != 0) {    // NULL indicates localhost
        fprintf(stderr, "%s:\terror:\tgetaddrinfo failed: %s\n", __FILE__, gai_strerror(status));
        return NULL;
    }

    // iterate through results and attempt to allocate a socket and connect
    int client_fd = -1;
    struct addrinfo *p;
    for (p = results; p != NULL && client_fd < 0; p = p->ai_next) {
		// allocate the socket
        if ((client_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            fprintf(stderr, "%s:\terror:\tunable to make socket: %s\n", __FILE__, strerror(errno));
            continue;
    	}

		// connect to the host
   		if (connect(client_fd, p->ai_addr, p->ai_addrlen) < 0) {
   		    close(client_fd);
   		    client_fd = -1;
   		    continue;
   		}
    }

    // free the linked list of address results
    freeaddrinfo(results);

    if (client_fd < 0) {
        fprintf(stderr, "%s:\terror:\tfailed to make socket to connect to %s:%s: %s\n", __FILE__, host, port, strerror(errno));
        return NULL;
    }

    /* Open file stream from socket file descriptor */
    FILE *client_file = fdopen(client_fd, "w+");
    if (client_file == NULL) {
        fprintf(stderr, "%s:\terror:\tfailed to fdopen: %s\n", __FILE__, strerror(errno));
        close(client_fd);
        return NULL;
    }

    return client_file;
}

void prompt() {
    printf("Enter P for private conversation.\n");
    printf("Enter B for message broadcasting.\n");
    printf("Enter H for chat history.\n");
    printf("Enter X for exit.\n");
    printf(">> ");
    fflush(stdout);
}

void *client_listener(void *arg_init) {
    struct thread_arg_t *arg = (struct thread_arg_t *) arg_init;
    char message[BUFSIZ] = {0};

    // open a nonblocking stream for the client file
    int copy_client_fd = dup(fileno(arg->client_file));
    int flags = fcntl(copy_client_fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(copy_client_fd, F_SETFL, flags);
    FILE *client_file_nonblocking = fdopen(copy_client_fd, "w+");
    if (!client_file_nonblocking) {
        fprintf(stderr, "%s:\terror:\tfailed to fdopen: %s\n", __FILE__, strerror(errno));
        close(copy_client_fd);
        return NULL;
    }

    while (1) {
        char *test = fgets(message, BUFSIZ, client_file_nonblocking);
        if (!test && errno == EWOULDBLOCK) {
            usleep(250);
            continue;
        }

        if (message[0] == 'D') {
            fputs("\n\n ####################### New Message: ", stdout);
            rstrip(&message[1]);
            fputs(&message[1], stdout);
            fputs(" #######################\n\n>> ", stdout); fflush(stdout);
        } else {
            rstrip(&message[1]);
            struct message_t *m = message_init(&message[1]);
            message_queue_push(arg->message_queue, m);
        }
    }
}

/* Main Execution */
int main(int argc, char *argv[]) {
    // parse arguments
	if (argc != 4) {
		fprintf(stderr, "%s:\terror:\tincorrect number of arguments!\n", __FILE__);
		fprintf(stderr, "usage: %s [host] [port] [username]\n", __FILE__);
		return EXIT_FAILURE;
	}

	char *host = argv[1];
	char *port = argv[2];
    char *username = argv[3];

    // open a socket that is connected to the server
    FILE *client_file = open_socket(host, port);
	if (!client_file) {
        return EXIT_FAILURE;
    }

    // send username to the server
    char full_username[BUFSIZ] = {0};
    sprintf(full_username, "%s\n", username);
    fputs(full_username, client_file); fflush(client_file);

    // prompt user for password and validate credentials
    char server_response[BUFSIZ] = {0};
    fgets(server_response, BUFSIZ, client_file);
    rstrip(server_response);
    if (streq(server_response, "new user")) {
        printf("New user? Create password >> ");
        char new_password[BUFSIZ] = {0};
        fgets(new_password, BUFSIZ, stdin);
        fputs(new_password, client_file); fflush(client_file);
        printf("Welcome %s! Registration complete.\n", username); fflush(stdout);
    } else {
        printf("Welcome back! Enter password >> ");
        char password_attempt[BUFSIZ] = {0};
        fgets(password_attempt, BUFSIZ, stdin);
        fputs(password_attempt, client_file); fflush(client_file);
        memset(server_response, 0, BUFSIZ);
        while (fgets(server_response, BUFSIZ, client_file)) {
            rstrip(server_response);
            if (streq(server_response, "login successful")) {
                printf("Welcome %s!\n", username); fflush(stdout);
                break;
            } else if (streq(server_response, "incorrect password")) {
                printf("Invalid password.\n");
                printf("Please enter again >> ");
                memset(password_attempt, 0, BUFSIZ);
                fgets(password_attempt, BUFSIZ, stdin);
                fputs(password_attempt, client_file); fflush(client_file);
            } else {
                printf("Server error: %s\n", server_response); fflush(stdout);
                return EXIT_FAILURE;
            }
        }
    }

    // create a thread to listen for incoming messages from other users
    struct thread_arg_t *arg = malloc(sizeof(struct thread_arg_t));
    if (!arg) {
        fprintf(stderr, "%s:\terror:\tfailed to malloc thread argument structure: %s\n", __FILE__, strerror(errno));
        return EXIT_FAILURE;
    }

    struct message_queue_t *message_queue = message_queue_init();
    if (!message_queue) {
        fprintf(stderr, "%s:\terror:\tfailed to create message queue: %s\n", __FILE__, strerror(errno));
        return EXIT_FAILURE;
    }

    arg->client_file = client_file;
    arg->message_queue = message_queue;
    pthread_t thread;
    pthread_create(&thread, NULL, client_listener, arg);

    // receive and handle client commands
    while (1) {
        prompt();
        char command[BUFSIZ] = {0};
        fgets(command, BUFSIZ, stdin);

        // remove endline char
        rstrip(command);
        rstrip_c(command, ' ');

        // handle command appropriately
        if (streq(command, "B")) {
            if (broadcast_message_handler(client_file, message_queue) != 0) {
                fprintf(stderr, "%s:\terror:\tfailed to send broadcast message.\n", __FILE__);
                continue;
            }
        } else if (streq(command, "P")) {
            if (private_message_handler(client_file, message_queue) != 0) {
                fprintf(stderr, "%s:\terror:\tfailed to send private message.\n", __FILE__);
                continue;
            }
        } else if (streq(command, "H")) {
            if (history_handler(client_file, message_queue) != 0) {
                fprintf(stderr, "%s:\terror:\tfailed to retrieve history.\n", __FILE__);
                continue;
            }
        } else if (streq(command, "X")) {
            if (exit_handler(client_file, message_queue) != 0) {
                fprintf(stderr, "%s:\terror:\tfailed to exit appropriately.\n", __FILE__);
                return EXIT_FAILURE;
            }
        } else {
            fprintf(stderr, "%s:\terror:\tplease enter a valid command.\n", __FILE__);
            continue;
        }
    }

    return EXIT_SUCCESS;
}
