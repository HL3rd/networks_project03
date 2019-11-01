
# Makefile

# Authors:
# 	Blake Trossen (btrossen)
# 	Horacio Lopez (hlopez1)

CC		= gcc
CFLAGS	= -Wall

TARGETS	= myclient

all: $(TARGETS)

myclient: chat_client.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)
