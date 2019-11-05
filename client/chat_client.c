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

#include "utils.h"

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

// int send_command(int client_fd, char *buffer) {
// 	const int CMD_SIZE = 4;
// 	ssize_t bytes_sent = 0;
// 	do {
// 		bytes_sent += send(client_fd, buffer, CMD_SIZE, 0);
// 		if (bytes_sent < 0) {
// 			fprintf(stderr, "%s:\terror:\tunable to send %s command to the server: %s\n", __FILE__, buffer, strerror(errno));
// 			return -1;
// 		}
// 	} while (bytes_sent < CMD_SIZE);
//
// 	return 0;
// }
//
// // function returns 0 if use properly logs in or signs up
// int login_sign_up(char* user_name) {
//     FILE *fp = fopen("users.txt", "r");
//     if (fp == NULL){
//         printf("Error checking users file\n");
//         return 1;
//     }
//
//     char* line = NULL;
//     size_t len = 0;
//
//     while ((getline(&line, &len, fp)) != -1) {
//         char* name = strtok(line, " ");
//         if (streq(name, user_name)) {
//             // found the user, prompt for password to login
//             char* password = strtok(NULL, " ");
//             fclose(fp);
//             // Note: if <name> fails, use <user_name>
//             int pass = password_prompt(password, name);
//             return pass;
//         }
//     }
//
//     fclose(fp);
//     int user = create_new_user(user_name);
//     return user;
// }
//
// // loop that asks for existing user password
// int password_prompt(char* password, char* name) {
//     while(1) {
//         printf("Welcome back! Enter passsword >> ");
//
//         // accept user input from stdin
//         char buffer[BUFSIZ] = {0};
//         while (fgets(buffer, BUFSIZ, stdin)) {
//             // get the password attempt from the stdin buffer
//     		char* attempt = strtok(buffer, " ");
//
//             while (1) {
//                 if (streq(attempt, password)) {
//                     // Successful login
//                     printf("Welcome %s!\n", name);
//                     return 0;
//                 } else {
//                     printf("Invalid password.\n");
//                     printf("Please enter again >> ");
//                     continue;
//                 }
//             }
//
//         }
//     }
// }
//
// // create new user
// int create_new_user(char* name) {
//     char* new_pair = name;
//     concatenate_string(new_pair, " ");
//
//     printf("New user? Create password >> ");
//     // accept user input from stdin
//     char buffer[BUFSIZ] = {0};
//     while (fgets(buffer, BUFSIZ, stdin)) {
//         // get the new password
// 		char* new_password = strtok(buffer, " ");
//
//         // set new 'user password' pair in users.txt
//         FILE* user_db = fopen("users.txt", "w");
//
//         concatenate_string(new_pair, new_password);
//
//         int results = fputs(new_pair, user_db);
//         if (results == EOF) {
//             // Failed to write do error code here.
//             fprintf(stderr, "Error creating user: %s\n", name);
//             fclose(user_db);
//             return 1;
//         }
//         fclose(user_db);
//         return 0;
//     }
//
//     return 1;
// }

/* Define Functions */
void prompt() {
    printf("Enter P for private conversation.\n");
    printf("Enter B for message broadcasting.\n");
    printf("Enter H for chat history.\n");
    printf("Enter X for exit.\n");
    printf(">> ");
    fflush(stdout);
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

    // prompt user for password
    char server_response[BUFSIZ] = {0};
    fgets(server_response, BUFSIZ, client_file);
    rstrip(server_response);
    if (streq(server_response, "new user")) {
        printf("New user? Create password >> ");
        char new_password[BUFSIZ] = {0};
        fgets(new_password, BUFSIZ, stdin);
        fputs(new_password, client_file); fflush(client_file);
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

    while (1) {
        char command[BUFSIZ] = {0};
        fgets(command, BUFSIZ, stdin);

        // handle command appropriately

        fputs(command, stdout); fflush(client_file);
    }

    return EXIT_SUCCESS;
}
