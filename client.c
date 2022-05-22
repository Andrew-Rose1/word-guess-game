/* demo_client.c - code for example client program that uses TCP */

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/*------------------------------------------------------------------------
* Program: demo_client
*
* Purpose: allocate a socket, connect to a server, and print all output
*
* Syntax: ./demo_client server_address server_port
*
* server_address - name of a computer on which server is executing
* server_port    - protocol port number server is using
*
*------------------------------------------------------------------------
*/

sigjmp_buf context;
volatile int alarm_occurred = 0;
void alarm_handler(int signum) {
	// int one = 8;
	// int two = 9;
    alarm_occurred = 1;
	// printf("NO INPUT");
	// send(sd, &one, 1, 0);
    siglongjmp(context, -1);
}


int main( int argc, char **argv) {
	struct hostent *ptrh; /* pointer to a host table entry */
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold an IP address */
	int sd; /* socket descriptor */
	int port; /* protocol port number */
	char *host; /* pointer to host name */
	char buf[1000]; /* buffer for data from the server */

	char bufPlayer;
	char isActive;
	char playerGuess[100];
	uint8_t boardSize;
	uint8_t roundTimer;
	uint8_t isValid;
	uint8_t opponentWordLength;
	int myScore = 0;
	int round = 1;
	int newRound = 1;
	int opponentScore = 0;
	int selectOutcome;

	fd_set readfds;
	FD_ZERO(&readfds);

	struct timeval timeout;
	timeout.tv_sec = 10; 
    timeout.tv_usec = 0; 


	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./client server_address server_port\n");
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[2]); /* convert to binary */
	if (port > 0) { /* test for legal value */
		sad.sin_port = htons((u_short)port);
	}
	else {
		fprintf(stderr,"Error: bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}

	host = argv[1]; /* if host argument specified */

	/* Convert host name to equivalent IP address and copy to sad. */
	ptrh = gethostbyname(host);
	if ( ptrh == NULL ) {
		fprintf(stderr,"Error: Invalid host: %s\n", host);
		exit(EXIT_FAILURE);
	}

	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	/* Map TCP transport protocol name to protocol number. */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket. */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	if (connect(sd, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		fprintf(stderr,"connect failed\n");
		exit(EXIT_FAILURE);
	}

	
	signal(SIGALRM, alarm_handler);

	recv(sd, &bufPlayer, sizeof(bufPlayer), 0);
	recv(sd, &boardSize, 1, 0);
	recv(sd, &roundTimer, 1, 0);
	char board[boardSize];
	if (bufPlayer == '1') {
		printf("You are Player %c... the game will begin when Player 2 joins...\n", bufPlayer);
	} else {
		printf("You are Player %c...\n", bufPlayer);
	}
	printf("Board size: %d\n", boardSize);
	printf("Seconds per turn: %d\n\n", roundTimer);


	// Game loop
	while (1) {
		// Handle new round
		if (newRound == 1) {
			printf("Round %d...\n", round);
			printf("Score is: %d-%d\n", myScore, opponentScore);
			recv(sd, board, sizeof(board), 0);
			printf("Board:");
			for (int i=0; i < boardSize; i++) {
				printf(" %c", board[i]);
			}
			printf("\n");
			newRound = 0;
		}

		memset(playerGuess, 0, sizeof(playerGuess));
		timeout.tv_sec = roundTimer;

		recv(sd, &isActive, 1, 0);
		if (isActive == 'Y') {
			if (sigsetjmp(context, 1) == 0) {
				alarm(roundTimer);
				printf("Your turn, enter word: ");
				fgets(playerGuess, sizeof(playerGuess), stdin);
			}
			send(sd, &playerGuess, sizeof(playerGuess), 0);
			
			recv(sd, &isValid, 1, 0);
			if (isValid == 1) {
				printf("Valid Word!\n");
			} else {
				printf("Invalid Word\n\n");
				opponentScore++;
				round++;
				newRound = 1;
				if (opponentScore >= 3) {
					printf("You Lost!\n");
					break;
				}
			}
		} else {
			printf("Please wait for opponent to enter a word...\n");
			
			recv(sd, &isValid, 1, 0);
			if (isValid == 1) {
				recv(sd, &opponentWordLength, 1, 0);
				char opponentWord[opponentWordLength];
				recv(sd, opponentWord, sizeof(opponentWord), 0);
				opponentWord[opponentWordLength] = '\0';
				printf("Opponent entered \"%s\"\n", opponentWord);
			} else {
				printf("Opponent lost the round\n\n");
				myScore++;
				round++;
				newRound = 1;
				if (myScore >= 3) {
					printf("You Won!\n");
					break;
				}
			}
		}
	}
	close(sd);
	exit(EXIT_SUCCESS);
}

