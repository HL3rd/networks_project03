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
#include <stdbool.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "utils.h"
#include "../shared.h"

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

void* receive_messages(void* socket) {

    int client_file = *(int *)socket;

    char messageBuffer[BUFSIZ];
    fgets(messageBuffer, sizeof(messageBuffer), client_file);
    rstrip(messageBuffer);

    printf("recieved something within listening thread\n");
    // Print message
    printf("%s", messageBuffer);
}

/* Define Functions */
bool isValidOperation(char * op){

  if (streq(op, "B") || streq(op, "P") != 0 || streq(op, "H") != 0 || streq(op, "X") != 0) {
    return true;
  }

  return false;
}

char* receiveOperation(){
    // Recieve command input
    printf("Enter P for private conversation.\n");
    printf("Enter B for message broadcasting.\n");
    printf("Enter H for chat history.\n");
    printf("Enter X for exit.\n");
    printf(">> ");

    // Prepare max buffer
    char opBuffer[BUFSIZ];
    bzero(opBuffer, sizeof(opBuffer));

    if ((fgets(opBuffer, sizeof(opBuffer), stdin)) < 0) {
        printf("Failed to get user input");
    }

    // Remove new line character to avoid errors
    // opBuffer[strlen(opBuffer) - 1] = '\0';
    rstrip(opBuffer);

    // Continue to prompt for operation until a valid operation is recieved
    while (!isValidOperation(opBuffer)) {
        printf("Error: Please a valid operation <B, P, H, X>:  \n");

        bzero(opBuffer, sizeof(opBuffer));

        if ((fgets(opBuffer, sizeof(opBuffer), stdin)) < 0) {
            printf("Failed to get user input");
        }

        // Remove new line
        opBuffer[strlen(opBuffer) - 1] = '\0';
    }

    if (strcmp(opBuffer, "B") == 0) {
        return "Cbroadcast";
    } else if (strcmp(opBuffer, "P") == 0) {
        return "Cprivate";
    } else if (strcmp(opBuffer, "H") == 0) {
        return "Chistory";
    } else if (strcmp(opBuffer, "X") == 0) {
        return "exit";
    } else {
        printf("Received invalid operation somehow.");
        exit(1);
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

    // ***** Create or Login a User *****//
    // send username to the server
    char full_username[BUFSIZ] = {0};
    sprintf(full_username, "%s\n", username);
    fputs(full_username, client_file);

    // prompt user for password for login or sign up
    char server_response[BUFSIZ] = {0};
    fgets(server_response, BUFSIZ, client_file);
    rstrip(server_response);

    if (streq(server_response, "new user")) {
        printf("New user? Create password >> ");
        char new_password[BUFSIZ] = {0};
        fgets(new_password, BUFSIZ, stdin);

        // Send the new password to the server
        fputs(new_password, client_file);
        fflush(client_file);
    } else {
        printf("Welcome back, %s!\n", username);
    }

    // Set up client and buffer for stdin
    pthread_t client_thread;
    pthread_create(&client_thread, NULL, receive_messages, (void*) &client_file);

    char stdinBuffer[BUFSIZ];
    bzero(stdinBuffer, sizeof(stdinBuffer));

    while(1) {
        // Get operation from user
        // this function contains the printf prompt
        char* operation = receiveOperation();

        if (streq(operation, "exit")) {
            printf("Exiting...\n");
            break;
        }

        // Send operation to server, these will have "C" at position 0
        fputs(operation, client_file);
        fflush(client_file);

        // Receive succesful ready confirmation or error from server
        char* commandConf[BUFSIZ];
        fgets(commandConf, BUFSIZ, client_file);
        rstrip(commandConf);

        if(streq(commandConf, "readyBroadcast")) {
            bzero(commandConf, BUFSIZ);
            printf("Broadcasting\n");

            // Get message from user input
            bzero(stdinBuffer, sizeof(stdinBuffer));
            printf("Please enter your broadcast: "); // TODO: make this match the demo video

            if ((fgets(stdinBuffer, sizeof(stdinBuffer), stdin)) < 0) {
              printf("Failed to get user input");
            }

            fputs(stdinBuffer, client_file);
            fflush(client_file);

            // Receive confirmation from server that broadcast was executed
            fgets(commandConf, BUFSIZ, client_file);
            rstrip(commandConf);

            if (streq(commandConf, "Broadcast success")) {
                bzero(commandConf, BUFSIZ);
                // TODO: Change to match demo
                printf("Successfully sent broadcast\n");
            } else {
                printf("Error. Did not send broadcast successfully\n");
            }
            continue;
        } else if (commandConf, "readyPrivate") {
            printf("Private messaging...\n");
            char* confirmation[BUFSIZ];
            continue;
        } else if (commandConf, "readyHistory") {
            printf("executing history...\n");
            break;
        } else {
            printf("Did not receive a valid ready state from the server\n");
        }

        bzero(commandConf, BUFSIZ);
    }

    return EXIT_SUCCESS;
}
