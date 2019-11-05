/* auth.c */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#include "auth.h"

#define streq(a, b) (strcmp(a, b) == 0)
#define USERS_FILENAME "users.txt"

char *user_is_registered(char *username) {
    FILE *registry = fopen(USERS_FILENAME, "r");
    if (!registry){
        registry = fopen(USERS_FILENAME, "w");
        return NULL;
    }

    char line[BUFSIZ] = {0};
    while (fgets(line, BUFSIZ, registry)) {
        char *registry_username = strtok(line, " ");
        if (streq(registry_username, username)) {    // found the user
            char *password = strtok(NULL, " ");
            fclose(registry);
            return password;
        }
    }

    fclose(registry);
    return NULL;
}

int user_register(char *username, char *password) {
    FILE *fp = fopen(USERS_FILENAME, "a+");
    if (fp == NULL){
        printf("%s:\terror: failed to open %s: %s\n", __FILE__, USERS_FILENAME, strerror(errno));
        return 1;
    }

    char user_registry[BUFSIZ] = {0};
    sprintf(user_registry, "%s %s\n", username, password);
    fputs(user_registry, fp);
    fclose(fp);
    return 0;
}

int user_login(char *username, char *password) {
    FILE *fp = fopen(USERS_FILENAME, "r");
    if (fp == NULL){
        printf("%s:\terror: failed to open %s: %s\n", __FILE__, USERS_FILENAME, strerror(errno));
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    while ((getline(&line, &len, fp)) != -1) {
        char *entry_username = strtok(line, " ");
        if (streq(entry_username, username)) {      // found the user
            char *entry_password = strtok(NULL, " ");
            if (streq(entry_password, password)) {  // correct password
                return 0;
            }

            return -1;
        }
    }

    fclose(fp);
    return -2;
}
