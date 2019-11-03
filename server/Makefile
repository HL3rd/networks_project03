# Makefile

# Authors:
# 	Blake Trossen (btrossen)
# 	Horacio Lopez (hlopez1)

CC		= gcc
CFLAGS	= -Wall -pthread

TARGETS	= chatserver

all: $(TARGETS)

chatserver: chat_server.c user.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)
