#include "user.h"

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

            int pass = password_prompt(password, name);
            return pass;
        }
    }

    fclose(fp);
    int user = create_new_user(user_name);
    return user;
}

// loop that asks for existing user password
// returns 0 if correct password is enetered
// otherwise, it continues to run
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
