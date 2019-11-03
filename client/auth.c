#include "user.h"

#define streq(a, b) (strcmp(a, b) == 0)

// function returns 0 if use properly logs in or signs up
int login_or_signup(int client_fd, char* user_name) {
    FILE *fp = fopen("users.txt", "r");
    if (fp == NULL){
        printf("Error checking users file\n");
        return 1;
    }

    // write the username to the server to begin search
    int res = fputs(user_name, client_fd);

    // receive the password or no-password message from server
    char buffer[BUFSIZ] = {0};
    fgets(buffer, BUFSIZ, stdin);

    if (streq(buffer, "no user")) {
        // ask client for desired password
        int res = create_new_user(user_name);
    } else {
        // ask client for a password attempt to send to server
        int res = password_prompt(buffer, user_name);
    }


    return res;
}

// loop that asks for existing user password
// returns 0 if correct password is enetered
// otherwise, it continues to run
int password_prompt(char* message, char* name) {

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
        // Successful addition to the database

        fclose(user_db);
        return 0;
    }

    return 1;
}

int send_buffer(char* name) {
    // TODO
    int len;
    if ((len = write(client_fd, buffer, size)) == -1) {
        perror("ERROR: Client Send\n");
        exit(1);
    }
    return len;
}
