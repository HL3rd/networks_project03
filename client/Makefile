
# Makefile

# Authors:
# 	Blake Trossen (btrossen)
# 	Horacio Lopez (hlopez1)

CC		= gcc
CFLAGS	= -Wall

TARGETS	= chatclient
.PHONY: all clean

all: $(TARGETS)

chatclient: chat_client.c utils.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)
