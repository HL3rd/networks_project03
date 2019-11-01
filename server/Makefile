# Makefile

# Authors:
# 	Blake Trossen (btrossen)
# 	Horacio Lopez (hlopez1)

CC		= gcc
CFLAGS	= -Wall -pthread

TARGETS	= myserver

all: $(TARGETS)

myserver: chat_server.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)
