# Word Guessing Co-op Game
A word guessing game between two players connecting via a single server. This project utilizes the c socket library and can support multiple 2 players games at once.

## Getting Started

By following these steps you should be able to get a server up and running with two clients connected.


### Prerequisites
- GNU Compiler Collection


### Installing

Compile both server and client c files

    gcc -o server server.c
    gcc -o client client.c


## Playing the game

To run each program you need to pass in the correct arguements to avoid errors.

Server format
   
    ./server {HOST_ADDRESS} {HOST_PORT} {BOARD_LENGTH} {ROUND_TIMER} {DICTIONARY_PATH} 
    
Client format
   
    ./client {HOST_ADDRESS} {HOST_PORT} 


### Example Game

This example game assumes that you are hosting the server and clients on the same machine.

Start the server

    ./server localhost 3987 8 30 dict.txt
    
Start the two clients

    ./client localhost 3987
