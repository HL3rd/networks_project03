# Makefile

# Authors:
# 	Blake Trossen (btrossen)
# 	Horacio Lopez (hlopez1)

CC		= gcc
CFLAGS	= -Wall

TARGETS	= mychat

all: $(TARGETS)

myftpd: chat_server.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)
