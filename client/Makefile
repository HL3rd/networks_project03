
# Makefile

# Authors:
# 	Blake Trossen (btrossen)
# 	Horacio Lopez (hlopez1)

CC		= gcc
CFLAGS	= -Wall -pthread

TARGETS	= chatclient
.PHONY: all clean

all: $(TARGETS)

chatclient: chat_client.c message.c message_queue.c command_handler.c utils.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)
