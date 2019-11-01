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

// function returns 0 if use properly logs in or signs up
int login_sign_up(char* user_name) {
    FILE *fp = fopen("users.txt", "r");
    if (fp == NULL){
        printf("Error checking users file\n");
        return 1;
    }

    char* line = NULL;
    size_t len = 0;

    while ((getline(&line, &len, fp)) != -1) {
        char* name = strtok(line, " ");
        if (streq(name, user_name)) {
            // found the user, prompt for password to login
            char* password = strtok(NULL, " ");
            fclose(fp);
            // Note: if <name> fails, use <user_name>
            int pass = password_prompt(password, name);
            return pass;
        }
    }

    fclose(fp);
    int user = create_new_user(user_name);
    return user;
}

// loop that asks for existing user password
int password_prompt(char* password, char* name) {
    while(1) {
        printf("Welcome back! Enter passsword >> ");

        // accept user input from stdin
        char buffer[BUFSIZ] = {0};
        while (fgets(buffer, BUFSIZ, stdin)) {
            // get the password attempt from the stdin buffer
    		char* attempt = strtok(buffer, " ");

            while (1) {
                if (streq(attempt, password)) {
                    // Successful login
                    printf("Welcome %s!\n", name);
                    return 0;
                } else {
                    printf("Invalid password.\n");
                    printf("Please enter again >> ");
                    continue;
                }
            }

        }
    }
}

// create new user
int create_new_user(char* name) {
    char* new_pair = name;
    concatenate_string(new_pair, " ");

    printf("New user? Create password >> ");
    // accept user input from stdin
    char buffer[BUFSIZ] = {0};
    while (fgets(buffer, BUFSIZ, stdin)) {
        // get the new password
		char* new_password = strtok(buffer, " ");

        // set new 'user password' pair in users.txt
        FILE* user_db = fopen("users.txt", "w");

        concatenate_string(new_pair, new_password);

        int results = fputs(new_pair, user_db);
        if (results == EOF) {
            // Failed to write do error code here.
            fprintf(stderr, "Error creating user: %s\n", name);
            fclose(user_db);
            return 1;
        }
        fclose(user_db);
        return 0;
    }

    return 1;
}

/* Define Functions */

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
    }

    while (1) {
        printf("Enter P for private conversation.\n");
        printf("Enter B for message broadcasting.\n");
        printf("Enter H for chat history.\n");
        printf("Enter X for exit.\n");
        printf(">>");

        // TODO: Place command handling from stdin

    }

}
