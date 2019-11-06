/* client.h */

/* * * * * * * * * * * * * * * *
 * Authors:
 *    Blake Trossen (btrossen)
 *    Horacio Lopez (hlopez1)
 * * * * * * * * * * * * * * * */

#include "client.h"
#include "history_logger.h"

struct client_t *client_init(FILE *client_file, char *username) {
    struct client_t *client = malloc(sizeof(struct client_t));
    if (!client) {
        fprintf(stderr, "%s:\terror: failed to initialize the client: %s", __FILE__, strerror(errno));
        return NULL;
    }

    client->client_file = client_file;
    client->history_log = history_logger_init(username);
    client->username = username;
    client->next = NULL;
    client->prev = NULL;

    return client;
}

void client_destroy(struct client_t *client) {
    fclose(client->client_file);
    fclose(client->history_log);
    free(client->username);
    free(client);
}
