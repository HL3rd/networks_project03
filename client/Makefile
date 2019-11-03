
# Makefile

# Authors:
# 	Blake Trossen (btrossen)
# 	Horacio Lopez (hlopez1)

CC		= gcc
CFLAGS	= -Wall -pthread

TARGETS	= chatclient

all: $(TARGETS)

chatclient: chat_client.c user.c command_handler.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)
