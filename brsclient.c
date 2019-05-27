/** \file brsclient.c
 * \author Orlando Leombruni
 * 
 * \brief Client per il gioco della briscola.
 *  
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera originale dell'autore.
 * 
 */
#include "commonstrings.h"
#include "errors.h"
#include "comsock.h"
#include "bris.h"
#include "users.h"
#include <time.h>

/** Definizione macro per l'uscita dalla receiveMessage */
#define receive(a, b) \
			if (receiveMessage(a, b) == -1) { \
					if (errno == ENOTCONN) { \
						fprintf(stderr, "%s\n", SERVER_KILLED); \
						EC_CLEANUP_NOW\
					} \
					else { \
						EC_FAIL\
					} \
				}
				
/** Funzione che stampa a schermo i messaggi ricevuti dal server
 * 
 * \param msg messaggio da stampare a schermo
 * 
 */
void explainMsg_rc(message_t msg)
{
	fprintf(stdout, SERVER_REPLY);
	switch(msg.type) {
		case MSG_OK:
			fprintf(stdout, OP_OK);
			break;
		case MSG_NO:
			fprintf(stdout, OP_NO);
			break;
		case MSG_ERR:
			fprintf(stdout, OP_ERR);
			break;
	}
	fprintf(stdout, MORE_INFO);
	if (msg.length == 0)
		fprintf(stdout, NO_MORE_INFO);
	else
		fprintf(stdout, "\"%s\".\n", msg.buffer);
}

/** Funzione che sostituisce una carta di una mano con quella appena pescata.
 * Sia le carte che la mano sono in formato \c char* (array di caratteri).
 * 
 * \param hand mano del giocatore
 * \param new carta pescata
 * \param old carta da scartare
 */
void replace_string (char* hand, char* new, char* old) {
	int i;
	if (strcmp(new, "NN") == 0) {
		if ((hand[0] == old[0]) && (hand[1] == old[1])) {
			hand[0] = hand[3];
			hand[1] = hand[4];
			hand[3] = hand[6];
			hand[4] = hand[7];
		}
		if (((hand[0] == old[0]) && (hand[1] == old[1])) || ((hand[3] == old[0]) && (hand[4] == old[1]))) {
			hand[3] = hand[6];
			hand[4] = hand[7];
		}
		hand[6] = '\0';
	}
	else {
		for (i = 0; i < 8; i++) {
			if ((hand[i] == old[0]) && (hand[i+1] == old[1])) {
				hand[i] = new[0];
				hand[i+1] = new[1];
			}
		}
	}
}

/** Funzione della partita (lato client)
 * \param fd_s file descriptor della socket
 * \param player stringa contenente il proprio username
 * \param first booleano che indica se si gioca per primi nel primo turno
 * 
 * Ulteriori informazioni sono disponibili nella relazione.
 */

void Play (int fd_s, char* player, bool_t first)
{
	int winlen, winpoints;
	bool_t finished = FALSE;
	char mano[9], enemy[LUSER+1], briscola, played[3], winner[LUSER+1], *temp = NULL;
	message_t received, sent;
	received.buffer = NULL;
	sent.buffer = NULL;
	
	/* Ricezione del messaggio di inizio partita */
	receive(fd_s, &received)
	if (received.type != MSG_STARTGAME) {
		fprintf(stdout, MSG_NOT_EXPECTED);
		EC_CLEANUP_NOW
	}
	
	/* Creazione della prima mano */
	mano[2] = ' '; mano[5] = ' '; mano[8] = '\0';
	briscola = received.buffer[0];
	mano[0] = received.buffer[2];
	mano[1] = received.buffer[3];
	mano[3] = received.buffer[4];
	mano[4] = received.buffer[5];
	mano[6] = received.buffer[6];
	mano[7] = received.buffer[7];
	strcpy(enemy, received.buffer+9);
	free(received.buffer);
	received.buffer = NULL;
	fprintf(stdout, PLAY_FIRST, enemy, briscola);
	
	/* Ciclo principale della partita */
	while (!finished) {
		bool_t goodtype = FALSE;
		fprintf(stdout, CARDS, mano);
		if (first) {
			while (!goodtype) {
				fprintf(stdout, YOURTURN, player);
				fscanf(stdin, "%s", played);
				ec_neg1 ( createMessage(&sent, MSG_PLAY, played) )
				ec_neg1 ( sendMessage(fd_s, &sent) )
				free(sent.buffer);
				sent.buffer = NULL;
				receive(fd_s, &received)
				switch (received.type) {
					case MSG_ERR:
						fprintf(stdout, "%s\n", received.buffer);
						break;
					case MSG_PLAY:
						fprintf(stdout, ENEMYTURN_2, enemy, received.buffer);
						goodtype = TRUE;
						break;
					case MSG_ENDGAME:
						goodtype = TRUE;
						finished = TRUE;
						break;
					default:
						EC_CLEANUP_NOW
				}
			}
		}
		else {
			fprintf(stdout, ENEMYTURN_1, enemy); fflush(stdout);
			receive(fd_s, &received)
			fprintf(stdout, "%s\n", received.buffer);
			while (!goodtype) {
				fprintf(stdout, YOURTURN, player);
				fscanf(stdin, "%s", played);
				ec_neg1( createMessage(&sent, MSG_PLAY, played) )
				ec_neg1( sendMessage(fd_s, &sent) )
				free(sent.buffer);
				sent.buffer = NULL;
				receive(fd_s, &received)
				switch (received.type) {
					case MSG_ERR:
						fprintf(stdout, "%s\n", received.buffer);
						break;
					case MSG_OK:
						goodtype = TRUE;
						break;
					case MSG_ENDGAME:
						finished = TRUE;
						break;
					default:
						EC_CLEANUP_NOW
				}
			}
		}
		if (received.buffer != NULL) {
			free(received.buffer);
			received.buffer = NULL;
		}
		
		/* Fine turno/partita */
		receive(fd_s, &received)
		switch (received.type) {
			case MSG_CARD:
				if (received.buffer[0] == 't') first = TRUE;
				else first = FALSE;
				replace_string(mano, received.buffer+2, played);
				break;
			case MSG_ENDGAME:
				finished = TRUE;
				break;
			default:
				EC_CLEANUP_NOW
		}
	}
	
	/* Creazione della stringa vincitore */
	temp = strchr(received.buffer, ':');
	winlen = temp - received.buffer;
	strncpy(winner, received.buffer, winlen);
	winner[winlen] = '\0';
	temp++;
	winpoints = atoi(temp);
	if (strcmp(winner, DRAW) != 0) fprintf(stdout, WINMSG, winner, winpoints);
	else fprintf(stdout, DRAWMSG);
	if (received.buffer != NULL) {
		free(received.buffer);
		received.buffer = NULL;
	}
	if (sent.buffer != NULL) {
		free(sent.buffer);
		sent.buffer = NULL;
	}
	ec_neg1 ( closeConnection(fd_s) )
	return;
	
	EC_CLEANUP_BGN
	
		if (received.buffer != NULL) free(received.buffer);
		if (sent.buffer != NULL) free(sent.buffer);
		closeConnection(fd_s);
		
		exit(1);
		
	EC_CLEANUP_END
}

int main(int argc, char **argv)
{
	int fd;
	bool_t c_option = FALSE, r_option = FALSE, d_option = FALSE, playing = FALSE, first = FALSE;
	char* buf = NULL, player[LUSER+1];
	message_t toSend, toReceive;
	toSend.buffer = NULL;
	toReceive.buffer = NULL;
	
	/* Asserzione per il controllo degli errori */
	PTRASSERT

	/* Controllo input della riga di comando */
	if (argc == 1 || argc == 2 || argc > 4) {
		fprintf(stderr, "%s\n", WR_NUMB_OF_ARGS);
		fprintf(stderr, "%s\n", CL_RIGHT_WAY);
		exit(EXIT_FAILURE);
	}
	if (argc == 4) {
		if (strcmp(argv[3], REG_OPTN) == 0) r_option = TRUE;
		else if (strcmp(argv[3], CANC_OPTN) == 0) c_option = TRUE;
		else if (strcmp(argv[3], DISC_OPTN) == 0) d_option = TRUE;
		else {
			fprintf(stderr, "%s\n", WRONG_OPTION);
			fprintf(stderr, "%s\n", CL_RIGHT_WAY);
			exit(EXIT_FAILURE);
		}
	}
	
	/* Apertura della connessione al server */
	ec_neg1( fd = openConnection(SOCKNAME, NTRIAL, NSEC) )
	/* Richiesta di registrazione */
	if (r_option) {
		toSend.type = MSG_REG;
	}
	else if (c_option) {		/* Richiesta di cancellazione */
		toSend.type = MSG_CANC;
	}
	else if (d_option) {
		toSend.type = MSG_DISC;	/* Richiesta di disconnessione forzata */
	}
	else toSend.type = MSG_CONNECT;
	
	/* Creazione del primo messaggio */
	toSend.length = (strlen(argv[1]) + strlen(argv[2])) + 3;
	ec_null ( buf = (char*)malloc((toSend.length)*sizeof(char)) )
	
	strcpy(buf, argv[1]);
	strcat(buf, ":");
	strcat(buf, argv[2]);
	toSend.buffer = buf;
	ec_neg1( sendMessage(fd, &toSend) )
	receive(fd, &toReceive)
	
	if (c_option || r_option || d_option) { /* Caso registrazione/rimozione/disconnessione */
		explainMsg_rc(toReceive);
		if (toReceive.buffer != NULL) {
			free(toReceive.buffer);
			toReceive.buffer = NULL;
		}
		if (toSend.buffer != NULL) {
			free(toSend.buffer);
			toSend.buffer = NULL;
		}
		ec_neg1 ( closeConnection(fd) )
		return 0;
	}
	
	/* Caso connessione */
	free(toSend.buffer);
	toSend.buffer = NULL;
	switch(toReceive.type) {
		case MSG_OK:
			fprintf(stdout, PLAYERSELECT, toReceive.buffer);
			fscanf(stdin, "%s", player);
			if (strcmp(player, WAIT_MSG) == 0) {
				toSend.type = MSG_WAIT;
				toSend.buffer = NULL;
				toSend.length = 0;
				ec_neg1( sendMessage(fd, &toSend) )
				free(toReceive.buffer);
				toReceive.buffer = NULL;
				receive(fd, &toReceive)
				if (toReceive.type != MSG_OK) {
					fprintf(stdout, MSG_NOT_EXPECTED);
					EC_CLEANUP_NOW
				}
				else fprintf(stdout, "%s\n", OK_WAITING);
				playing = TRUE;
			}
			else {
				toSend.type = MSG_OK;
				toSend.buffer = player;
				toSend.length = strlen(toSend.buffer) + 1;
				ec_neg1 ( sendMessage(fd, &toSend) )
				free(toReceive.buffer);
				toReceive.buffer = NULL;
				receive(fd, &toReceive)
				if (toReceive.type != MSG_OK) {
					explainMsg_rc(toReceive);
					EC_CLEANUP_NOW
				}
				playing = TRUE;
				first = TRUE;
			}
			break;
		case MSG_WAIT:
			fprintf(stdout, "%s", NOPLAYERS);
			playing = TRUE;
			break;
		case MSG_NO:
			fprintf(stdout, CONN_REFUSED, toReceive.buffer);
			break;
		case MSG_ERR:
			fprintf(stdout, "%s\n", OP_ERR);
			fprintf(stdout, MORE_INFO);
			if (toReceive.length == 0)
				fprintf(stdout, NO_MORE_INFO);
			else
				fprintf(stdout, "%s\n", toReceive.buffer);
			break;
	}
	
	if (playing) Play(fd, argv[1], first);	
	
	return 0;
	
	EC_CLEANUP_BGN
		
		if (toReceive.buffer != NULL) free(toReceive.buffer);
		if (toSend.buffer != NULL) free(toSend.buffer);
		closeConnection(fd);
		
		return 1;
	EC_CLEANUP_END
}
