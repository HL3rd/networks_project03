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

#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "user.h"
#include "command_handler.h";

/* Define Macros */
#define streq(a, b) (strcmp(a, b) == 0)

/* Define Structures */

/* Define Utilities */
void concatenate_string(char *original, char *add)
{
   while(*original) {
       original++;
   }

   while(*add) {
       *original = *add;
       add++;
       original++;
   }

   *original = '\0';
}

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

void flush_socket(int fd) {
	FILE *stream = fdopen(fd, "r");
	fflush(stream);
	fclose(stream);
}

int open_socket(char *host, char *port) {
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
        return -1;
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

    if (client_fd < 0) {
        fprintf(stderr, "%s:\terror:\tfailed to make socket to connect to %s:%s: %s\n", __FILE__, host, port, strerror(errno));
    }

    // free the linked list of address results
    freeaddrinfo(results);

    return client_fd;
}

int send_command(int client_fd, char *buffer) {
	const int CMD_SIZE = 4;
	ssize_t bytes_sent = 0;
	do {
		bytes_sent += send(client_fd, buffer, CMD_SIZE, 0);
		if (bytes_sent < 0) {
			fprintf(stderr, "%s:\terror:\tunable to send %s command to the server: %s\n", __FILE__, buffer, strerror(errno));
			return -1;
		}
	} while (bytes_sent < CMD_SIZE);

	return 0;
}

/* Define Functions */
void recv_message_handler() {
    char receiveMessage[LENGTH_SEND] = {};
    while (1) {
        int receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0);
        if (receive > 0) {
            printf("\r%s\n", receiveMessage);
            str_overwrite_stdout();
        } else if (receive == 0) {
            break;
        } else {
            // -1
        }
    }
}

void send_message_handler() {
    char message[LENGTH_MSG] = {};
    while (1) {
        str_overwrite_stdout();
        while (fgets(message, LENGTH_MSG, stdin) != NULL) {
            str_trim_lf(message, LENGTH_MSG);
            if (strlen(message) == 0) {
                str_overwrite_stdout();
            } else {
                break;
            }
        }
        send(sockfd, message, LENGTH_MSG, 0);
        if (strcmp(message, "exit") == 0) {
            break;
        }
    }
    catch_ctrl_c_and_exit(2);
}

/* Main Execution */
int main(int argc, char *argv[]) {
    // parse arguments
	if (argc != 4) {
		fprintf(stderr, "%s:\terror:\tincorrect number of arguments!\n", __FILE__);
		fprintf(stderr, "Usage: %s [host] [port] [name]\n", __FILE__);
		return EXIT_FAILURE;
	}

	char *host = argv[1];
	char *port = argv[2];
    char *user_name = argv[3];

    // open a socket that is connected to the server
    int client_fd = open_socket(host, port);
	if (client_fd < 0) {
        return EXIT_FAILURE;
    }

    // check if the user exists in file
    if (login_sign_up(user_name) != 0) {
        EXIT_FAILURE;
    } else {
        pthread_t send_msg_thread;
        if (pthread_create(&send_msg_thread, NULL, (void *) send_message_handler, NULL) != 0) {
            printf ("Create pthread error!\n");
            exit(EXIT_FAILURE);
        }

        pthread_t recv_msg_thread;
        if (pthread_create(&recv_msg_thread, NULL, (void *) recv_message_handler, NULL) != 0) {
            printf ("Create pthread error!\n");
            exit(EXIT_FAILURE);
        }

        char* user_input;

        while (1) {
            // Recieve first client input
            printf("Enter P for private conversation.\n");
            printf("Enter B for message broadcasting.\n");
            printf("Enter H for chat history.\n");
            printf("Enter X for exit.\n");
            printf(">>");

            fgets(user_input, 50, stdin);

            // Remove endline char
            user_input[strlen(user_input)-1] = '\0';

            // Get chosen operation from user input
            char* operation = strtok(user_input, " \t\n");

            if (streq(operation, "P")) {
                if (private_message_handler() == 0) {
                    continue;
                } else {
                    // ERROR
                }
            } else if (streq(operation, "B")) {
                if (broadcast_message_handler() == 0) {
                    continue;
                } else {
                    // ERROR
                }
            } else if (streq(operation, "H")) {
                if (history_handler() == 0) {
                    continue;
                } else {
                    //ERROR
                }
            } else if (streq(operation, "X")) {
                exit_handler();
            } else {
                printf("Error. Please enter a proper operation.\n");
                continue;
            }
        }
    }
}
