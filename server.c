#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h> 
#include <unistd.h>

#define QLEN 6 /* size of request queue */


int main(int argc, char **argv) {
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold server's address */
	struct sockaddr_in cad; /* structure to hold client's address */
	int sd, sd2, sd3; /* socket descriptors */
	int port; /* protocol port number */
	int alen; /* length of address */
	int optval = 1; /* boolean value when we set socket option */
	char buf[1000]; /* buffer for string the server sends */
	char buf2[1000]; /* buffer for string the server sends */

	struct sockaddr_in cad_;
	int alen_; 
	char player1 = '1';
	char player2 = '2';
	char yesActive = 'Y';
	char noActive = 'N';
	int activePlayer;
	char playerGuess[100];
	uint8_t lengthOfGuessedWord;
	uint8_t isFound;
	uint8_t validLetters;
	uint8_t boardSize = atoi(argv[2]);
	char board[boardSize];
	uint8_t roundTimer = atoi(argv[3]);
	int round = 1;
	int endRecv = 1;
	int newRound = 1;
	int selectOutcome;
	pid_t pid;

	FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
	int inTime;

	fd_set write_sd, read_sd;
	FD_ZERO(&write_sd);
	FD_ZERO(&read_sd);

	struct timeval timeout;
	timeout.tv_sec = roundTimer; 
    timeout.tv_usec = 0; 


	if( argc != 5 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server server_port board_size round_timer word_dictionary\n");
		exit(EXIT_FAILURE);
	}

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */

	sad.sin_family = AF_INET;
	
	sad.sin_addr.s_addr = INADDR_ANY;
     
	port = atoi(argv[1]); /* convert argument to binary */
	if (port > 0) { /* test for illegal value */
		sad.sin_port = htons(port);
	} else { /* print error message and exit */
		fprintf(stderr,"Error: Bad port number %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}

	/* Map TCP transport protocol name to protocol number */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}


	sd = socket(AF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Allow reuse of port - avoid "Bind failed" issues */
	if( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		exit(EXIT_FAILURE);
	}

	if (bind(sd, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}

	if (listen(sd, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}


	/* Main server loop - accept and handle requests */
	while (1) {
		// Wait for first player 
		alen = sizeof(cad);
		if ((sd2 = accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
			fprintf(stderr, "Error: Accept failed\n");
			exit(EXIT_FAILURE);
		}

		// Send player number
		send(sd2, &player1, sizeof(player1), 0);
		// Send board size
		send(sd2, &boardSize, 1, 0);
		// Send round timer
		send(sd2, &roundTimer, 1, 0);


		// Wait for second player
		alen_ = sizeof(cad_);
		if ((sd3 = accept(sd, (struct sockaddr *)&cad_, &alen_)) < 0) {
			fprintf(stderr, "Error: Accept failed\n");
			exit(EXIT_FAILURE);
		}

		// Send player number
		send(sd3, &player2, sizeof(player2), 0);
		// Send board size
		send(sd3, &boardSize, 1, 0);
		// Send round timer
		send(sd3, &roundTimer, 1, 0);
	

		if ((pid = fork()) == 0) {
			// Closes connections after game loop is broken
			close(sd);

			// Main Game Loop
			activePlayer = 1;
			while (1) {
				// Handle new round
				if (newRound == 1) {
					int size = boardSize;
					const char charset[] = "abcdefghijklmnopqrstuvwxyz";
					if (size) {
						--size;
						for (size_t n = 0; n < size; n++) {
							int key = rand() % (int) (sizeof charset - 1);
							board[n] = charset[key];
						}
						board[size] = '\0';
					}
					send(sd2, board, sizeof(board), 0);
					send(sd3, board, sizeof(board), 0);
					newRound = 0;
				}

				// Clear buffer and fd sets
				FD_ZERO(&write_sd);
				FD_ZERO(&read_sd);
				memset(buf, 0, sizeof(buf));

				if (activePlayer == 1) {
					FD_SET(sd3, &write_sd);
					FD_SET(sd2, &read_sd);
					send(sd2, &yesActive, 1, 0);
					send(sd3, &noActive, 1, 0);
					if ((selectOutcome = select(FD_SETSIZE, &read_sd, NULL, NULL, &timeout)) < 0){
						perror("Error");
						exit(EXIT_FAILURE);
					} else if (selectOutcome == 0) {
						isFound = 0;
					} else {
						for (int i=0; i < FD_SETSIZE; i++) {
							if (FD_ISSET(i, &read_sd)) {
								if ((endRecv = recv(i, buf, sizeof(buf), 0)) == 0) {
									break;
								};
							}
						}
					}
				} else {
					FD_SET(sd2, &write_sd);
					FD_SET(sd3, &read_sd);
					send(sd2, &noActive, 1, 0);
					send(sd3, &yesActive, 1, 0);
					if (select(FD_SETSIZE, &read_sd, NULL, NULL, &timeout) < 0){
						perror("Error HERE");
						exit(EXIT_FAILURE);
					} else if (selectOutcome == 0) {
						isFound = 0;
					} else {
						for (int i=0; i < FD_SETSIZE; i++) {
							if (FD_ISSET(i, &read_sd)) {
								if ((endRecv = recv(i, buf, sizeof(buf), 0)) == 0) {
									break;
								};
							}
						}
					}
				}


				buf[strcspn(buf, "\n")] = 0;
				lengthOfGuessedWord = strlen(buf);
				
				// CHeck if word is made of valid letters
				validLetters = 1;
				for (int j=0; j < lengthOfGuessedWord; j++) {
					if(strchr(board, buf[j]) == NULL) {
						validLetters = 0;
						break;
					}
				}

				// If word has valid characters - Check if word is in dictionary
				isFound = 0;
				if (validLetters == 1) {
					fp = fopen(argv[4], "r");
					if (fp == NULL) {
						exit(EXIT_FAILURE);
					}
					while ((read = getline(&line, &len, fp)) != -1) {
						line[strcspn(line, "\n")] = 0;
						if (strcmp(line, buf) == 0) {
							isFound = 1;
							break;
						} 
					}
					fclose(fp);
				}


				send(sd2, &isFound, 1, 0);
				send(sd3, &isFound, 1, 0);
				if (isFound == 1) {
					char guessedWord[lengthOfGuessedWord];
					strcpy(guessedWord, buf);
					if (activePlayer == 1) {
						send(sd3, &lengthOfGuessedWord, 1, 0);
						send(sd3, &guessedWord, strlen(buf), 0);
					} else {
						send(sd2, &lengthOfGuessedWord, 1, 0);
						send(sd2, &guessedWord, strlen(buf), 0);
					}

					if (activePlayer == 1) {
						activePlayer = 0;
					} else {
						activePlayer = 1;
					}

				} else {
					round++;
					newRound = 1;
					if ((round % 2) == 0) {
						activePlayer = 0;
					} else {
						activePlayer = 1;
					}
				}
			}
		}
	}
	close(sd2);
	close(sd3);
}

