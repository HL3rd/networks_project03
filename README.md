# Program 03

Authors:
* Blake Trossen (btrossen)
* Horacio Lopez (hlopez1)

## Example Commands to Run
To compile and run the server code, from this directory, perform the following commands:
```
$ cd server
$ make
$ ./chatserver 41045
```
Replace __41045__ with a port of your choosing.

To compile and run the client code, from this directory, perform the following commands:
```
$ cd client
$ make
$ ./chatclient student00.cse.nd.edu 41045 btrossen
```
Replace __student00.cse.nd.edu__ with the machine name you ran the server on, replace
__41045__ with the port you chose to run the client on, and replace __btrossen__
with your username.

Multiple clients can log on, as in the demonstration provided in the assignment.

## Server Files
The following is a listing of server files:
```
auth.c              -- implementation file for authentication into the chat service
auth.h              -- header file for "" -- ""
chat_server.c       -- the main chat server code
client_list.c       -- implementation file for the client list to keep track of active clients
client_list.h       -- header file for "" -- ""
client.c            -- implementation file for the client struct representation
client.h            -- header file for "" -- ""
command_handler.c   -- implementation for the command handlers, which handle different client requests
command_handler.h   -- header file for "" -- ""
history_logger.c    -- implementation for the history logger file, which logs history of clients
history_logger.h    -- header file for "" -- ""
Makefile            -- to build executables
users.txt           -- to keep track of users and passwords
utils.c             -- implementation for utility file
utils.h             -- header file for "" -- ""
```

## Client Files
```
chat_client.c       -- the main chat client code
command_handler.c   -- the implementation for the command handlers, which handle different user requests
command_handler.h   -- the header file for "" -- ""
Makefile            -- to build executables
message_queue.c     -- implementation for queue to handle incoming messages
message_queue.h     -- header file for "" -- ""
message.c           -- implementation for message struct representation
message.h           -- header file for "" -- ""
utils.c             -- implementation for utility file
utils.h             -- header file for "" -- ""
```
```
