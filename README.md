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

Each player takes turns making a word from the randomly given letters. A word is said to be valid if the word is found in the passed in ditionary and is made up of letters on the board. Players go back and forth until one player submits an invalid word, or the timer runs out. Scores are incremented accordingly and the next round starts. Player 1 starts odd rounds and Player 2 starts even rounds. The game is over after one player reaches 3 round wins.

The game can support multiple two player games at once. The server will continue to listen for connections until it is manually terminated.

To run each program you need to pass in the correct arguements to avoid errors.

Server format
   
    ./server {HOST_PORT} {BOARD_LENGTH} {ROUND_TIMER} {DICTIONARY_PATH} 
    
Client format
   
    ./client {HOST_ADDRESS} {HOST_PORT} 


### Example Game

This example game assumes that you are hosting the server and clients on the same machine.

Start the server

    ./server 3987 8 30 dict.txt
    
Start the two clients

    ./client localhost 3987
    
Below is a screenshot of what the game should look like after both clients connect. The left and middle terminals are clients, while the right terminal is the server.
  
<img width="1336" alt="Screen Shot 2022-05-21 at 9 48 28 PM" src="https://user-images.githubusercontent.com/55816533/169679226-85a8f6e9-0c90-4874-bd12-affac0c93349.png">

