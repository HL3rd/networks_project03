# Makefile

# Authors:
# 	Blake Trossen (btrossen)
# 	Horacio Lopez (hlopez1)

CC		= gcc
CFLAGS	= -Wall -pthread

TARGETS	= chatserver
.PHONY: all clean

all: $(TARGETS)

chatserver: chat_server.c client.c client_list.c auth.c command_handler.c history_logger.c utils.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)
