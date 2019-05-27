/** \file brsserver.c
 *  \author Orlando Leombruni
 * 
 *  \brief Server per il gioco della briscola.
 *  
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera originale dell'autore.
 * 
 */
#include <signal.h>
#include <pthread.h>
#include "commonstrings.h"
#include "errors.h"
#include "comsock.h"
#include "bris.h"
#include "users.h"
#include "newMazzo_r.h"

/** Struttura a lista per la gestione dei thread */
typedef struct _tlist {
/** ID del thread */
	pthread_t tid;
/** Elemento successivo */
	struct _tlist* next;
} tlist;

/** Inserimento di un nuovo thread_id in coda nella struttura a lista
 * 
 * \param list_h testa della lista
 * \param list_t coda della lista
 * \param el thread_id da inserire
 * 
 * \retval list_h testa della lista, se l'elemento è stato inserito
 * \retval NULL se non è stato possibile inserire l'elemento (setta \c errno)
 *
 */
tlist* NuovoInCoda(tlist* list_h, pthread_t el)
{
	if (list_h == NULL) {
		tlist* new = NULL;
		if ((new = (tlist*)malloc(sizeof(tlist))) == NULL) return NULL;
		new->tid = el;
		new->next = NULL;
		list_h = new;
	}
	else list_h->next = NuovoInCoda(list_h->next, el);
	return list_h;
}

/* Variabili globali */

/** Mutex per la gestione di \c term_signal */
static pthread_mutex_t sig_mutex = PTHREAD_MUTEX_INITIALIZER;
/** Mutex per l'albero \c generalTree */
static pthread_mutex_t tree_mutex = PTHREAD_MUTEX_INITIALIZER;
/** Mutex per il n. di partite giocate */
static pthread_mutex_t plays_mutex = PTHREAD_MUTEX_INITIALIZER;
/** Segnale di STOP per i thread \c Signaler e \c Dispatcher */
static bool_t term_signal = FALSE;
/** Albero degli utenti, in mutex fra i thread */
static nodo_t* generalTree = NULL;
/** Lista degli ID dei thread Worker (con in coda l'ID del thread attivato più recentemente) */
static tlist* threadList_head = NULL;
/** Opzione di testing (per l'uso della newMazzo thread-safe) */
static bool_t t_option = FALSE;
/** Variabile che conta il numero progressivo di partite */
static int npart = 0;

/** Esegue la free su un puntatore
 * (è una funzione di cleanup per il thread Dispatcher)
 * 
 * \param arg puntatore all'area di memoria da liberare
 */

void freefd(void* arg)
{
	free(arg);
}

/** Attende la terminazione di tutti i thread presenti nella lista
 * 
 * \param arg (non usato)
 * 
 * \section commagg1 Commenti Aggiuntivi
 * La funzione è di cleanup per la cancellazione del thread \c Dispatcher; essa opera facendo la join dei thread \c Worker,
 * a partire dal meno recente (quello in testa alla lista). Ciò permette di liberare più rapidamente le aree di memoria
 * occupate dagli elementi della lista, in quanto i thread in testa sono quelli con maggiore probabilità di essere già
 * terminati.
 */
void joinAllThreads(void* arg)
{
	while (threadList_head != NULL) {
		tlist* temp = threadList_head;
		ec_rv ( pthread_join(threadList_head->tid, NULL) )
		threadList_head = threadList_head->next;
		free(temp);
	}
	return;
	
	EC_CLEANUP_BGN
		return;
	EC_CLEANUP_END
}

/** Check in mutex sulla variabile di terminazione \c term_signal
 * 
 * \retval r stato di \c term_signal
 *
 */
 
bool_t CheckTermSignal()
{
	bool_t r = FALSE;
	ec_rv ( pthread_mutex_lock(&sig_mutex) )
	r = term_signal;
	ec_rv ( pthread_mutex_unlock(&sig_mutex) )
	return r;
	
	EC_CLEANUP_BGN
		pthread_mutex_unlock(&sig_mutex);
		return TRUE;
	EC_CLEANUP_END
}

/**Attivazione in mutex del segnale di terminazione
 * 
 */

void WriteTermSignal()
{
	ec_rv ( pthread_mutex_lock(&sig_mutex) )
	term_signal = TRUE;
	ec_rv ( pthread_mutex_unlock(&sig_mutex) )
	return;
	
	EC_CLEANUP_BGN
		pthread_mutex_unlock(&sig_mutex);
		return;
	EC_CLEANUP_END
}

/** addUser in mutex sull'albero \c globalTree
 * 
 * \param puser utente da inserire
 * 
 * \retval a valore ritornato da addUser
 * \retval -1 se si è verificato un errore
 *
 */

int addUser_Mutex (user_t* puser)
{
	int a;
	ec_rv ( pthread_mutex_lock(&tree_mutex) )
	a = addUser(&generalTree, puser);
	ec_rv ( pthread_mutex_unlock(&tree_mutex) )
	return a;
	
	EC_CLEANUP_BGN
		pthread_mutex_unlock(&tree_mutex);
		return -1;
	EC_CLEANUP_END
}

/** removeUser in mutex sull'albero \c globalTree
 * 
 * \param puser utente da rimuovere
 * 
 * \retval a valore ritornato da removeUser
 * \retval -1 se si è verificato un errore
 *
 */

int removeUser_Mutex (user_t* puser)
{
	int a;
	ec_rv ( pthread_mutex_lock(&tree_mutex) )
	a = removeUser(&generalTree, puser);
	ec_rv ( pthread_mutex_unlock(&tree_mutex) )
	return a;
	
	EC_CLEANUP_BGN
		pthread_mutex_unlock(&tree_mutex);
		return -1;
	EC_CLEANUP_END
}

/** setUserChannel in mutex sull'albero \c globalTree
 * 
 * \param puser utente da modificare
 * \param channel canale da settare
 * 
 * \retval a valore ritornato da setUserChannel
 * \retval -1 se si è verificato un errore
 *
 */

bool_t setUserChannel_Mutex (char* puser, int channel)
{
	bool_t a;
	ec_rv ( pthread_mutex_lock(&tree_mutex) )
	a = setUserChannel(generalTree, puser, channel);
	ec_rv ( pthread_mutex_unlock(&tree_mutex) )
	return a;
	
	EC_CLEANUP_BGN
		pthread_mutex_unlock(&tree_mutex);
		return -1;
	EC_CLEANUP_END
}

/** setUserStatus in mutex sull'albero \c globalTree
 * 
 * \param puser utente da modificare
 * \param st status da settare
 * 
 * \retval a valore ritornato da setUserStatus
 * \retval -1 se si è verificato un errore
 *
 */

bool_t setUserStatus_Mutex (char* puser, status_t st)
{
	bool_t a = FALSE;
	ec_rv ( pthread_mutex_lock(&tree_mutex) )
	a = setUserStatus(generalTree, puser, st);
	ec_rv ( pthread_mutex_unlock(&tree_mutex) )
	return a;
	
	EC_CLEANUP_BGN
		pthread_mutex_unlock(&tree_mutex);
		return -1;
	EC_CLEANUP_END
}

/** isUser in mutex sull'albero \c globalTree
 * 
 * \param puser utente da controllare
 * 
 * \retval a valore ritornato da isUser
 *
 */

bool_t isUser_Mutex (char* puser)
{
	bool_t a = FALSE;
	ec_rv ( pthread_mutex_lock(&tree_mutex) )
	a = isUser(generalTree, puser);
	ec_rv ( pthread_mutex_unlock(&tree_mutex) )
	return a;
	
	EC_CLEANUP_BGN
		pthread_mutex_unlock(&tree_mutex);
		return a;
	EC_CLEANUP_END
}

/** checkPwd in mutex sull'albero \c globalTree
 * 
 * \param puser utente da controllare
 * 
 * \retval a valore ritornato da checkPwd
 *
 */

bool_t checkPwd_Mutex (user_t* puser)
{
	bool_t a = FALSE;
	ec_rv ( pthread_mutex_lock(&tree_mutex) )
	a = checkPwd(generalTree, puser);
	ec_rv ( pthread_mutex_unlock(&tree_mutex) )
	return a;
	
	EC_CLEANUP_BGN
		pthread_mutex_unlock(&tree_mutex);
		return a;
	EC_CLEANUP_END
}

/** getUserChannel in mutex sull'albero \c globalTree
 * 
 * \param puser utente da controllare
 * 
 * \retval a valore ritornato da getUserChannel
 * \retval -1 se si è verificato un errore
 *
 */

int getUserChannel_Mutex (char* puser)
{
	int a;
	ec_rv ( pthread_mutex_lock(&tree_mutex) )
	a = getUserChannel(generalTree, puser);
	ec_rv ( pthread_mutex_unlock(&tree_mutex) )
	return a;
	
	EC_CLEANUP_BGN
		pthread_mutex_unlock(&tree_mutex);
		return -1;
	EC_CLEANUP_END
}

/** getUserStatus in mutex sull'albero \c globalTree
 * 
 * \param puser utente da controllare
 * 
 * \retval a valore ritornato da getUserStatus
 * \retval -1 se si è verificato un errore
 *
 */

status_t getUserStatus_Mutex (char* puser)
{
	status_t a;
	ec_rv ( pthread_mutex_lock(&tree_mutex) )
	a = getUserStatus(generalTree, puser);
	ec_rv ( pthread_mutex_unlock(&tree_mutex) )
	return a;
	
	EC_CLEANUP_BGN
		pthread_mutex_unlock(&tree_mutex);
		return -1;
	EC_CLEANUP_END
}

/** getUserList in mutex sull'albero \c globalTree
 * 
 * \param st status richiesto
 * 
 * \retval a stringa ritornata da getUserChannel
 * \retval NULL se nessun utente ha lo status richiesto (errno == 0) o
 * se si è verificato un errore (errno != 0)
 *
 */

char* getUserList_Mutex (status_t st)
{
	char* a = NULL;
	ec_rv ( pthread_mutex_lock(&tree_mutex) )
	a = getUserList(generalTree, st);
	ec_rv ( pthread_mutex_unlock(&tree_mutex) )
	return a;
	
	EC_CLEANUP_BGN
		pthread_mutex_unlock(&tree_mutex);
		return NULL;
	EC_CLEANUP_END
}

/** Funzione della partita
 * \param fd_p1 file descriptor del giocatore che ha richiesto la sfida
 * \param fd_p2 file descriptor del giocatore che aspettava la sfida
 * \param player1 stringa contenente il nome del giocatore che ha richiesto la sfida
 * \param player2 stringa contenente il nome del giocatore che aspettava la sfida
 * 
 * \retval 0 se la partita si conclude normalmente
 * \retval err se si verificano errori (intero compatibile con le specifiche di \c errno)
 * 
 * E' possibile trovare ulteriori informazioni sulla struttura della partita nella relazione
 */

int Play (int fd_p1, int fd_p2, char* player1, char* player2)
{
	int i, filename_len, fd_first, fd_second, check, P1Number = 0, P2Number = 0, points1, points2, err = 0, winsize;
	bool_t whowins, finished = FALSE;
	FILE *log = NULL;
	mazzo_t* deck = NULL;
	carta_t* FirstPlayerHand[3], *SecondPlayerHand[3], *playedByFirst = NULL, *playedBySecond = NULL, *P1Cards[NCARTE], *P2Cards[NCARTE], *drawn1 = NULL, *drawn2 = NULL, *copia1 = NULL, *copia2 = NULL;
	message_t toFirst, toSecond, fromFirst, fromSecond;
	char* buffer1 = NULL, *buffer2 = NULL, cd[3], *first = NULL, *second = NULL, *filename = NULL, numb[5], winpoints[4], *winner = NULL, *winstring = NULL;
	
	toFirst.buffer = NULL;
	toSecond.buffer = NULL;
	fromSecond.buffer = NULL;
	fromFirst.buffer = NULL;
	
	/* Creazione del file di log (con il nome corretto) */
	ec_rv ( err = pthread_mutex_lock(&plays_mutex) )
	npart++;
	sprintf(numb, "%d", npart);
	ec_rv ( err = pthread_mutex_unlock(&plays_mutex) )
	filename_len = strlen(LOG_NAME_ST) + strlen(LOG_NAME_END) + strlen(numb) + 1;
	ec_null( filename = (char*)malloc((filename_len)*sizeof(char)) )
	strcpy(filename, LOG_NAME_ST);
	strcat(filename, numb);
	strcat(filename, LOG_NAME_END);
	ec_null( log = fopen(filename, "w") )
	free(filename);
	filename = NULL;
	
	/* Generazione del mazzo */
	ec_null ( deck = newMazzo_r(t_option) )
	fprintf(log, FIRST_LOG, player1, player2, semeToChar(deck->briscola));
	
	/* Generazione delle mani, preparazione ed invio dei messaggi MSG_STARTGAME */
	toFirst.length = 10+strlen(player2);
	toSecond.length = 10+strlen(player1);
	ec_null ( buffer1 = (char*)malloc((toFirst.length)*sizeof(char)) )
	ec_null ( buffer2 = (char*)malloc((toSecond.length)*sizeof(char)) )
	buffer1[0] = semeToChar(deck->briscola); buffer1[1] = ':'; buffer1[2] = '\0'; 
	buffer2[0] = semeToChar(deck->briscola); buffer2[1] = ':'; buffer2[2] = '\0';
	for (i = 0; i < 3; i++) {
		FirstPlayerHand[i] = getCard(deck);
		cardToString(cd, FirstPlayerHand[i]);
		strcat(buffer1, cd);
		SecondPlayerHand[i] = getCard(deck);
		cardToString(cd, SecondPlayerHand[i]);
		strcat(buffer2, cd);
	}
	strcat(buffer1, ":"); strcat(buffer1, player2);
	strcat(buffer2, ":"); strcat(buffer2, player1);
	toFirst.buffer = buffer1;
	toSecond.buffer = buffer2;
	toFirst.type = MSG_STARTGAME;
	toSecond.type = MSG_STARTGAME;
	ec_neg1 ( sendMessage(fd_p1, &toFirst) )
	ec_neg1 ( sendMessage(fd_p2, &toSecond) )
	free(buffer1);
	free(buffer2);
	buffer1 = NULL;
	buffer2 = NULL;
	toFirst.buffer = NULL;
	toSecond.buffer = NULL;
	
	/* Assegnazione iniziale dell'ordine dei turni */
	first = player1;
	second = player2;
	fd_first = fd_p1;
	fd_second = fd_p2;
	
	/* Ciclo principale */
	while (!finished) {
		
		/* Ricezione della carta giocata dal primo */
		ec_neg1 ( receiveMessage(fd_first, &fromFirst) )
		playedByFirst = stringToCard(fromFirst.buffer);
		if (playedByFirst == NULL)
			if (errno == EINVAL) check = -1;
			else {
				err = errno;
				return err;
			}
		else check = isInHand(playedByFirst, FirstPlayerHand);
		
		/* Controllo sulla validità della carta giocata */
		while (check != 1) {
			if (check == 0) {
				ec_neg1 ( createMessage(&toFirst, MSG_ERR, NOT_IN_DECK) )
			}
			if (check == -1) {
				ec_neg1 ( createMessage(&toFirst, MSG_ERR, NOT_A_CARD) )
			}
			ec_neg1 ( sendMessage(fd_first, &toFirst) )
			free(fromFirst.buffer);
			fromFirst.buffer = NULL;
			free(toFirst.buffer);
			toFirst.buffer = NULL;
			ec_neg1 ( receiveMessage(fd_first, &fromFirst) )
			playedByFirst = stringToCard(fromFirst.buffer);
			if (playedByFirst == NULL)
				if (errno == EINVAL) check = -1;
				else {
					err = errno;
					return err;
				}
			else check = isInHand(playedByFirst, FirstPlayerHand);
		}
		
		/* Invio delle informazioni al secondo e ricezione della sua carta */
		ec_neg1 ( createMessage(&toSecond, MSG_PLAY, fromFirst.buffer) )
		ec_neg1 ( sendMessage(fd_second, &toSecond) )
		free(toSecond.buffer);
		toSecond.buffer = NULL;
		
		ec_neg1 ( receiveMessage(fd_second, &fromSecond) )
		playedBySecond = stringToCard(fromSecond.buffer);
		if (playedBySecond == NULL) 
			if (errno == EINVAL) check = -1;
			else {
				err = errno;
				return err;
			}
		else check = isInHand(playedBySecond, SecondPlayerHand);
		
		/* Controllo sulla validità della carta giocata */
		while (check != 1) {
			if (check == 0) {
				ec_neg1 ( createMessage(&toSecond, MSG_ERR, NOT_IN_DECK) )
			}
			if (check == -1) {
				ec_neg1 ( createMessage(&toSecond, MSG_ERR, NOT_A_CARD) )
			}
			ec_neg1 ( sendMessage(fd_second, &toSecond) )
			free(fromSecond.buffer);
			fromSecond.buffer = NULL;
			free(toSecond.buffer);
			toSecond.buffer = NULL;
			ec_neg1 ( receiveMessage(fd_second, &fromSecond) )
			playedBySecond = stringToCard(fromSecond.buffer);
			if (playedBySecond == NULL)
				if (errno == EINVAL) check = -1;
				else {
					err = errno;
					return err;
				}
			else check = isInHand(playedBySecond, SecondPlayerHand);
		}
		
		ec_neg1 ( createMessage(&toSecond, MSG_OK, NULL) )
		ec_neg1 ( sendMessage(fd_second, &toSecond) )
		
		ec_neg1 ( createMessage(&toFirst, MSG_PLAY, fromSecond.buffer) )
		ec_neg1 ( sendMessage(fd_first, &toFirst) )
		free(toFirst.buffer);
		toFirst.buffer = NULL;
		
		/* Fine del turno */
		fprintf(log, "%s:%s#%s:%s\n", first, fromFirst.buffer, second, fromSecond.buffer);
		free(fromFirst.buffer);
		fromFirst.buffer = NULL;
		free(fromSecond.buffer);
		fromSecond.buffer = NULL;
		
		ec_null ( copia1 = (carta_t*)malloc(sizeof(carta_t)) )
		ec_null ( copia2 = (carta_t*)malloc(sizeof(carta_t)) )
		copia1->seme = playedByFirst->seme;
		copia1->val = playedByFirst->val;
		copia2->seme = playedBySecond->seme;
		copia2->val = playedBySecond->val;
		
		whowins = compareCard(deck->briscola, playedByFirst, playedBySecond);
		if (whowins) {
			if (strcmp(player1, first) == 0) {
				P1Cards[P1Number] = copia1;
				P1Number++;
				P1Cards[P1Number] = copia2;
				P1Number++;
			}
			else {
				P2Cards[P2Number] = copia1;
				P2Number++;
				P2Cards[P2Number] = copia2;
				P2Number++;
			}
		}
		else {
			if (strcmp(player1, first) == 0) {
				P2Cards[P2Number] = copia1;
				P2Number++;
				P2Cards[P2Number] = copia2;
				P2Number++;
				first = player2;
				second = player1;
				ec_neg1 ( exchangeHands(FirstPlayerHand, SecondPlayerHand) )
				fd_first = fd_p2;
				fd_second = fd_p1;
			}
			else {
				P1Cards[P1Number] = copia1;
				P1Number++;
				P1Cards[P1Number] = copia2;
				P1Number++;
				first = player1;
				second = player2;
				ec_neg1 ( exchangeHands(FirstPlayerHand, SecondPlayerHand) )
				fd_first = fd_p1;
				fd_second = fd_p2;
			}
			
		}
		
		/* Pesca nuove carte dal mazzo */
		if ((drawn1 = getCard(deck)) == NULL) {
			if (errno != 0) EC_CLEANUP_NOW
		}
		if (whowins) replace(FirstPlayerHand, drawn1, playedByFirst);
		else replace(FirstPlayerHand, drawn1, playedBySecond);
		
		if ((drawn2 = getCard(deck)) == NULL) {
			if (errno != 0) EC_CLEANUP_NOW
		}
		if (whowins) replace(SecondPlayerHand, drawn2, playedBySecond);
		else replace (SecondPlayerHand, drawn2, playedByFirst);
		
		finished = checkIfFinish(FirstPlayerHand, SecondPlayerHand);
		
		/* Se la partita non è ancora finita: composizione e invio dei messaggi MSG_CARD */
		if (!finished) {
			if (drawn1 != NULL) cardToString(cd, drawn1);
			else strcpy(cd, "NN");
			sprintf(numb, "t:%s", cd);
			ec_neg1 ( createMessage(&toFirst, MSG_CARD, numb) )
			ec_neg1 ( sendMessage(fd_first, &toFirst) )
			free(toFirst.buffer);
			toFirst.buffer = NULL;
			if (drawn1 != NULL) free(drawn1);
			
			if (drawn2 != NULL) cardToString(cd, drawn2);
			else strcpy(cd, "NN");
			sprintf(numb, "a:%s", cd);
			ec_neg1 ( createMessage(&toSecond, MSG_CARD, numb) )
			ec_neg1 ( sendMessage(fd_second, &toSecond) )
			free(toSecond.buffer);
			toSecond.buffer = NULL;
			if (drawn2 != NULL) free(drawn2);
		}
	}
	
	/* Fine partita: conteggio punti e decretazione vincitore */
	points1 = computePoints(P1Cards, P1Number);
	points2 = computePoints(P2Cards, P2Number);
	if (points1 > points2) {
		winner = player1;
		sprintf(winpoints, "%d", points1);
	}
	else if (points1 < points2) {
		winner = player2;
		sprintf(winpoints, "%d", points2);
	}
	else {
		ec_null ( winner = (char*)malloc(10*sizeof(char)) )
		strcpy(winner, DRAW);
		strcpy(winpoints, "60");
	}
	fprintf(log, LAST_LOG, winner, winpoints);
	ec_eof ( fclose(log) )
	log = NULL;
	
	/* Creazione e invio dei messaggi MSG_STARTGAME */
	winsize = strlen(winner)+strlen(winpoints)+2;
	ec_null ( winstring = (char*)malloc(winsize*sizeof(char)) )
	sprintf(winstring, "%s:%s", winner, winpoints);
	if (strcmp(winner, DRAW) == 0) {
		free(winner);
		winner = NULL;
	}
	ec_neg1 ( createMessage(&toFirst, MSG_ENDGAME, winstring) )
	ec_neg1 ( sendMessage(fd_p1, &toFirst) )
	ec_neg1 ( sendMessage(fd_p2, &toFirst) )
	
	/* Operazioni finali di pulizia */
	for (i = 0; i < P1Number; i++) {
		if (P1Cards[i] != NULL) {
			free(P1Cards[i]);
			P1Cards[i] = NULL;
		}
	}
	for (i = 0; i < P2Number; i++) {
		if (P2Cards[i] != NULL) {
			free(P2Cards[i]);
			P2Cards[i] = NULL;
		}
	}
	freeMazzo(deck);
	free(toFirst.buffer);
	toFirst.buffer = NULL;
	free(winstring);
	winstring = NULL;
	
	return 0;
	
	EC_CLEANUP_BGN
		if (err == 0) err = errno;
		closeConnection(fd_p1);
		closeConnection(fd_p2);
		
		freeMazzo(deck);
		
		if (filename != NULL) free(filename);
		if (buffer1 != NULL) free(buffer1);
		if (buffer2 != NULL) free(buffer2);
		if (playedByFirst != NULL) free(playedByFirst);
		if (playedBySecond != NULL) free(playedBySecond);
		if (fromFirst.buffer != NULL) free(fromFirst.buffer);
		if (toFirst.buffer != NULL) free(toFirst.buffer);
		if (fromSecond.buffer != NULL) free(fromSecond.buffer);
		if (toSecond.buffer != NULL) free(toSecond.buffer);
		if (winner != NULL)
			if(strcmp(winner, DRAW) == 0) free(winner);
		if (winstring != NULL) free(winstring);
		
		if (drawn1 != NULL) free(drawn1);
		if (drawn2 != NULL) free(drawn2);
		if (copia1 != NULL) free(copia1);
		if (copia2 != NULL) free(copia2);
		for (i = 0; i < P1Number; i++) {
			if (P1Cards[i] != NULL) {
				free(P1Cards[i]);
				P1Cards[i] = NULL;
			}
		}
		for (i = 0; i < P2Number; i++) {
			if (P2Cards[i] != NULL) {
				free(P2Cards[i]);
				P2Cards[i] = NULL;
			}
		}
		
		for (i = 0; i < 3; i++) {
			if (FirstPlayerHand[i] != NULL) free(FirstPlayerHand[i]);
			if (SecondPlayerHand[i] != NULL) free(SecondPlayerHand[i]);
		}
		
		setUserStatus_Mutex(player1, DISCONNECTED);
		setUserChannel_Mutex(player1, -1);
		setUserStatus_Mutex(player2, DISCONNECTED);
		setUserChannel_Mutex(player2, -1);
		
		if (log != NULL) fclose(log);
		
		return err;
		
	EC_CLEANUP_END
}

/** Funzione del thread di gestione dei segnali
 * 
 * \param arg thread_id del thread Dispatcher (castato a \c void*)
 * 
 * \retval 0 in caso di terminazione con successo
 * \retval err in caso di errore
 * 
 * \section commagg2 Commenti Aggiuntivi
 * Il thread è deputato alla ricezione di segnali SIGINT, SIGTERM e SIGUSR1. La ricezione è effettuata mediante la SC \c sigwait,
 * posta in un ciclo controllato dalla variabile globale di terminazione \c term_signal. Il segnale SIGUSR1 provoca la stampa dell'albero
 * corrente nel file di checkpoint, operazione effettuata da questo stesso thread; i segnali SIGINT e SIGTERM pongono \c term_signal a 1
 * e cancellano il thread \c Dispatcher (che, prima di terminare, eseguirà la funzione di cleanup \c joinAllThreads).
 */
void* Signaler(void* arg) 
{
	int snumb;
	pthread_t *from_arg, dispatch;
	FILE * out = NULL;
	sigset_t sgs;
	from_arg = (pthread_t*) arg;
	dispatch = *from_arg;
	
	/* Impostazione della maschera per i segnali (1 per SIGINT, SIGTERM e SIGUSR1, 0 per gli altri)
	 * ed applicazione al thread corrente */
	ec_neg1 ( sigemptyset(&sgs) )
	ec_neg1 ( sigaddset(&sgs, SIGINT) )
	ec_neg1 ( sigaddset(&sgs, SIGTERM) )
	ec_neg1 ( sigaddset(&sgs, SIGUSR1) )
	ec_rv ( pthread_sigmask(SIG_SETMASK, &sgs, NULL) )

	while (!CheckTermSignal()) {
		ec_rv ( sigwait(&sgs, &snumb) )
		switch (snumb) {
			case SIGINT:  /* SIGINT o SIGTERM: pone a 1 la variabile di controllo di terminazione e cancella il Dispatcher */
				fprintf(stderr, "%s\n", TERM_SIGINT);
				(void) WriteTermSignal();
				ec_rv ( pthread_cancel(dispatch) )
				break;
			case SIGTERM:
				fprintf(stderr, "%s\n", TERM_SIGTERM);
				(void) WriteTermSignal();
				ec_rv ( pthread_cancel(dispatch) )
				break;
			case SIGUSR1:	/* SIGUSR1: stampa l'albero attuale, non termina */
				ec_null ( out = fopen(CP_NAME, "w") )
				ec_rv ( pthread_mutex_lock(&tree_mutex) )
				ec_neg1 ( storeUsers(out, generalTree) )
				ec_rv ( pthread_mutex_unlock(&tree_mutex) )
				ec_eof ( fclose(out) )
				out = NULL;
				fprintf(stderr, "%s\n", CHECK_SIGUSR1);
				break;
		}
	}
	return NULL;
	
	EC_CLEANUP_BGN
	
		pthread_mutex_unlock(&tree_mutex);
		if (out != NULL) fclose(out);
		return NULL;
	
	EC_CLEANUP_END
}

/** Gestione delle registrazioni (thread Worker): esecuzione della \c addUser e
 * creazione del relativo messaggio di risposta
 * 
 * \param buf buffer contenente le credenziali dell'utente in formato \c username:password
 * 
 * \retval retn struttura messaggio di risposta
 * 
 */

message_t* User_Register(char* buf) 
{
	int msglen;
	message_t* retn = NULL;
	user_t* client_user;
	if ((retn = (message_t*)malloc(sizeof(message_t))) == NULL) return NULL;
	msglen = strlen(buf)+1;
	client_user = stringToUser(buf, msglen);
	if (client_user == NULL) {
		if (createMessage(retn, MSG_ERR, ERR_STRTOU) == -1) {
			free(retn);
			return NULL;
		}	    /* Comunico al client che la stringToUser è fallita, */
	}			/* probabilmente perché user/password troppo lunghi */
	
	else {
		switch (addUser_Mutex(client_user)) {
			case -1:	/* addUser fallita, dealloco la struttura user_t */
				free(client_user);
				if (createMessage(retn, MSG_ERR, INSERT_ERROR) == -1) {
					free(retn);
					return NULL;
				}
				break;
			case 0:		/* addUser eseguita con successo; *NON* dealloco client_user perché entra a far parte dell'albero! */
				if (createMessage(retn, MSG_OK, NULL) == -1) {
					free(retn);
					return NULL;
				}
				break;
			case 1:		/* addUser non eseguita, utente già presente; dealloco la struttura user_t */
				free(client_user); 
				if (createMessage(retn, MSG_NO, USR_ALREADY) == -1) {
					free(retn);
					return NULL;
				}
				break;
		}
	}
	return retn;
}

/** Gestione delle cancellazioni (thread Worker): esecuzione della \c removeUser e
 * creazione del relativo messaggio di risposta
 * 
 * \param buf buffer contenente le credenziali dell'utente in formato \c username:password
 * 
 * \retval retn struttura messaggio di risposta
 * 
 */
message_t* User_Cancel(char* buf) 
{
	int msglen;
	message_t* retn = NULL;
	user_t* client_user;
	if ((retn = (message_t*)malloc(sizeof(message_t))) == NULL) return NULL;
	msglen = strlen(buf)+1;
	client_user = stringToUser(buf, msglen);
	if (client_user == NULL) {
		if (createMessage(retn, MSG_ERR, ERR_STRTOU) == -1) {
			free(retn);
			return NULL;
		}		/* Comunico al client che la stringToUser è fallita, */
	}			/* probabilmente perché user/password troppo lunghi */
	else {			
		switch (removeUser_Mutex(client_user)) {
			case NOUSR:	/* removeUser non eseguita, utente non presente */
				if (createMessage(retn, MSG_NO, NOUSR_ERROR) == -1) {
					free(retn);
					return NULL;
				}
				break;
			case WRPWD:	/* removeUser non eseguita, la password fornita è errata */
				if (createMessage(retn, MSG_NO, WRPWD_ERROR) == -1) {
					free(retn);
					return NULL;
				}
				break;
			case -1:	/* removeUser fallita */
				if (createMessage(retn, MSG_ERR, REMOVE_ERROR) == -1) {
					free(retn);
					return NULL;
				}
				break;
			case 0:		/* removeUser eseguita con successo */
				if (createMessage(retn, MSG_OK, NULL) == -1) {
					free(retn);
					return NULL;
				}
				break;
		}
	}
				
	free(client_user);	/* In qualsiasi caso, dealloco la struttura user_t */
	return retn;
}

/** Gestione delle disconnessioni forzate (thread Worker): esecuzione della \c setUserStatus/Channel e
 * creazione del relativo messaggio di risposta
 * 
 * \param buf buffer contenente le credenziali dell'utente in formato \c username:password
 * 
 * \retval retn struttura messaggio di risposta
 * 
 */
message_t* User_Disconnect(char* buf) 
{
	int msglen;
	message_t* retn = NULL;
	user_t* client_user;
	if ((retn = (message_t*)malloc(sizeof(message_t))) == NULL) return NULL;
	msglen = strlen(buf)+1;
	client_user = stringToUser(buf, msglen);
	if (client_user == NULL) {
		if (createMessage(retn, MSG_ERR, ERR_STRTOU) == -1) {
			free(retn);
			return NULL;
		}		/* Comunico al client che la stringToUser è fallita, */
	}			/* probabilmente perché user/password troppo lunghi */
	else {			
		if (isUser_Mutex(client_user->name)) { /* Controllo credenziali */
			if (checkPwd_Mutex(client_user)) {
				setUserChannel_Mutex(client_user->name, -1);
				setUserStatus_Mutex(client_user->name, DISCONNECTED);
				if (createMessage(retn, MSG_OK, NULL) == -1) {
					free(retn);
					return NULL;
				}
			}
			else {
				if (createMessage(retn, MSG_NO, WRPWD_ERROR) == -1) {
					free(client_user);
					free(retn);
					return NULL;
				}
			}
		}
		else {
			if (createMessage(retn, MSG_NO, NOUSR_ERROR) == -1) {
				free(client_user);
				free(retn);
				return NULL;
			}
		}
	}
				
	free(client_user);	/* In qualsiasi caso, dealloco la struttura user_t */
	return retn;
}

/** Inizializzazione della connessione (thread Worker): controllo credenziali, ricezione lista utenti connessi,
 * preparazione del messaggio
 * 
 * \param buf buffer contenente le credenziali dell'utente in formato \c username:password
 * \param player buffer che conterrà il nome dell'utente
 * \param sock file descriptor della socket del client
 * 
 * \retval retn struttura messaggio di risposta
 * 
 */
message_t* User_Setup(char* buf, char* player, int sock)
{
	int msglen;
	char* player_list = NULL;
	message_t* retn = NULL;
	user_t* client_user;
	if ((retn = (message_t*)malloc(sizeof(message_t))) == NULL) return NULL;
	msglen = strlen(buf)+1;
	client_user = stringToUser(buf, msglen);
	if (client_user == NULL) {
		if (createMessage(retn, MSG_ERR, ERR_STRTOU) == -1) {
			free(retn);
			return NULL;
		}		/* Comunico al client che la stringToUser è fallita */
	}
	else {
		strcpy(player, client_user->name);
		if (isUser_Mutex(client_user->name)) { /* Controllo credenziali */
			if (checkPwd_Mutex(client_user)) {
				status_t check;
				check = getUserStatus_Mutex(client_user->name);
				if (check == WAITING || check == PLAYING) { /* Utente già connesso */
					if (createMessage(retn, MSG_ERR, ALR_CONN) == -1) {
						free(client_user);
						free(retn);
						return NULL;
					}
				}
				else {
					player_list = getUserList_Mutex(WAITING);
					if (player_list == NULL && errno == 0) {  /* Nessun utente in attesa */
						if (createMessage(retn, MSG_WAIT, NULL) == -1) {
							free(client_user);
							free(retn);
							return NULL;
						}
						setUserStatus_Mutex(client_user->name, WAITING);
						setUserChannel_Mutex(client_user->name, sock);
					}
					else if (player_list == NULL && errno != 0) {
						free(client_user);
						return NULL;
					}					
					else {
						if (createMessage(retn, MSG_OK, player_list) == -1) {
							free(client_user);
							free(retn);
							free(player_list);
							return NULL;
						}
						free(player_list);
					}
				}
			}
			else {
				if (createMessage(retn, MSG_NO, WRPWD_ERROR) == -1) {
					free(client_user);
					free(retn);
					return NULL;
				}
			}
		}
		else {
			if (createMessage(retn, MSG_NO, NOUSR_ERROR) == -1) {
				free(client_user);
				free(retn);
				return NULL;
			}
		}
	}
	free(client_user);
	return retn;
}

/** Funzione del thread di connessione al client
 * 
 * \param arg puntatore al file descriptor della socket di connessione
 * 
 * \retval NULL
 * 
 * \section commagg3 Commenti Aggiuntivi
 * Il thread gestisce le operazioni richieste dal client mediante una serie di invii e ricezioni di messaggi.
 * Ogni operazione chiama un'apposita funzione di gestione, che elabora le informazioni richieste e prepara
 * la risposta al client; se il client richiede di iniziare una partita, il thread chiama la funzione Play.
 * Maggiori informazioni sono disponibili nella relazione.
 */

void* Worker(void* arg)
{
	int *sock_p, sock, guest_sock;
	bool_t playing = FALSE, waiting = FALSE;
	message_t *receive = NULL, *send = NULL;
	char player[LUSER+1], guest[LUSER+1];
	sock_p = (int*) arg;
	sock = *sock_p;
	free(arg);
	ec_null ( receive = (message_t*)malloc(sizeof(message_t)) )
		
	/* Ricezione del primo messaggio */
	ec_neg1 ( receiveMessage(sock, receive) )		
		
	switch (receive->type) {
		case MSG_REG:
			ec_null ( send = User_Register(receive->buffer) )	 /* Registrazione */
			break;
		case MSG_CANC:
			ec_null ( send = User_Cancel(receive->buffer) )		/* Cancellazione */
			break;
		case MSG_DISC:
			ec_null ( send = User_Disconnect(receive->buffer) )		/* Disconnessione forzata */
			break;
		case MSG_CONNECT:
			ec_null ( send = User_Setup(receive->buffer, player, sock) )	/* Elaborazione richiesta di connessione */
			if (send->type == MSG_OK) {		/* Connessione andata a buon fine, si può scegliere uno sfidante */
				playing = TRUE;
				setUserChannel_Mutex(player, sock);
			}
			else {
				if (send->type == MSG_WAIT) {	/* Connessione andata a buon fine, nessuno sfidante disponibile */
					waiting = TRUE;
					setUserStatus_Mutex(player, WAITING);
					setUserChannel_Mutex(player, sock);
				}
			}	/* Se la connessione non va a buon fine (errori o utente/psw errati) non devo fare nulla, messaggio già formato */
			break;
		default:
			ec_neg1 ( createMessage(send, MSG_ERR, NOT_SUPPORTED) )
			break;
	}
	
	/* Invio del messaggio */
	ec_neg1 ( sendMessage(sock, send) ) 
	
	/* Nel caso in cui il client possa scegliere un avversario, si attende la risposta */
	if (playing) {
		if (receive->buffer != NULL) free(receive->buffer);
		if (send->buffer != NULL) free(send->buffer);
		receive->buffer = NULL;
		send->buffer = NULL;
		ec_neg1 ( receiveMessage(sock, receive) )
		
		switch (receive->type) {
			case MSG_WAIT:			/* Il client ha deciso di aspettare */
				setUserStatus_Mutex(player, WAITING);
				ec_neg1 ( createMessage(send, MSG_OK, NULL) )
				ec_neg1 ( sendMessage(sock, send) )
				waiting = TRUE;
				break;
			case MSG_OK:			/* Il client ha inviato il nome dell'avversario */
				strcpy(guest, receive->buffer);
				if (isUser_Mutex(guest) && (getUserStatus_Mutex(guest) == WAITING)) {
					guest_sock = getUserChannel_Mutex(guest);
					setUserStatus_Mutex(player, PLAYING);
					setUserStatus_Mutex(guest, PLAYING);
					ec_neg1 ( createMessage(send, MSG_OK, NULL) )
					ec_neg1 ( sendMessage(sock, send) )
					ec_rv ( Play(sock, guest_sock, player, guest) )
					setUserStatus_Mutex(player, DISCONNECTED);
					setUserChannel_Mutex(player, -1);
					setUserStatus_Mutex(guest, DISCONNECTED);
					setUserChannel_Mutex(guest, -1);
					ec_neg1 ( closeConnection(guest_sock) )
				}
				else {
					ec_neg1 ( createMessage(send, MSG_NO, NOUSR_ERROR) )
					ec_neg1 ( sendMessage(sock, send) )
				}
				break;
			default:
				ec_neg1 ( createMessage(send, MSG_ERR, NOT_SUPPORTED) )
				break;
		}
		
	}
		
	/* Operazioni finali di pulizia */	
	if (receive != NULL) {
		if (receive->buffer != NULL)
			free(receive->buffer);
		free(receive);
	}
		
	if (send != NULL) {
		if (send->buffer != NULL) 
			free(send->buffer);
		free(send);
	}
	
	/* Se il client è stato messo in attesa, non devo chiudere la connessione */
	if (!waiting) closeConnection(sock);
	return NULL;
	
	EC_CLEANUP_BGN
		
		if (receive != NULL) {
			if (receive->buffer != NULL)
				free(receive->buffer);
			free(receive);
		}
			
		if (send != NULL) {
			if (send->buffer != NULL) 
				free(send->buffer);
			free(send);
		}
		
		closeConnection(sock);
		
		return NULL;
		
	EC_CLEANUP_END
}

/** Funzione del thread dispatcher
 * 
 * \param arg file descriptor della socket del server (castata a \c void* )
 * 
 * \retval NULL
 * 
 * \section commagg4 Commenti Aggiuntivi
 * Il thread si occupa di accettare le connessioni dai client e di far partire i corrispondenti thread \c Worker;
 * l'ID di ogni nuovo thread viene inserito in lista. Il \c Dispatcher verrà cancellato dal \c Signaler; quando ciò
 * accade, viene chiamata la \c joinAllThreads per consentire una corretta terminazione dei \c Worker.
 */
void* Dispatcher(void* arg) 
{
	int *mainsock_p = NULL, mainsock;
	mainsock_p = (int*) arg;
	mainsock = *mainsock_p;
	
	/* Push della funzione di attesa thread Worker */
	pthread_cleanup_push(&joinAllThreads, NULL);
	
	while (!CheckTermSignal()) {
		pthread_t actual;
		int* fd_c = NULL;
		ec_null ( fd_c = (int*)malloc(sizeof(int)) )
		pthread_cleanup_push(&freefd, fd_c);
	
		/* Attesa della connessione di un client */
		ec_neg1 ( (*fd_c) = acceptConnection(mainsock) )
		/* Avvio di un nuovo thread Worker ed inserimento dell'ID di quest'ultimo in coda alla lista */
		ec_nzero ( pthread_create(&actual, NULL, &Worker, fd_c) )
		threadList_head = NuovoInCoda(threadList_head, actual);
		pthread_cleanup_pop(0);
	}
	pthread_cleanup_pop(1);
	return NULL;
	
	EC_CLEANUP_BGN
	
		return NULL;
	
	EC_CLEANUP_END
}	

int main(int argc, char **argv)
{
	int socket_desc = -1, err = 0, n_users;
	pthread_t signaler = 0, dispatch = 0;
	FILE *utenti_r = NULL;
	sigset_t sgs;
	
	/* Asserzione per il controllo degli errori */
	PTRASSERT
	
	/* Impostazione maschera dei segnali */
	ec_neg1 ( sigemptyset(&sgs) )
	ec_neg1 ( sigaddset(&sgs, SIGINT) )
	ec_neg1 ( sigaddset(&sgs, SIGTERM) )
	ec_neg1 ( sigaddset(&sgs, SIGUSR1) )
	ec_neg1 ( sigaddset(&sgs, SIGPIPE) )
	ec_rv ( err = pthread_sigmask(SIG_SETMASK, &sgs, NULL) )
	
	
	/* Controllo input della riga di comando */
	if (argc == 1) {
		fprintf(stderr, "%s\n", NO_USRLIST);
		fprintf(stderr, "%s\n", SR_RIGHT_WAY);
		exit(EXIT_FAILURE);
	}
	if (argc > 3) {
		fprintf(stderr, "%s\n", TOO_MANY_PAR);
		fprintf(stderr, "%s\n", SR_RIGHT_WAY);
		exit(EXIT_FAILURE);
	}
	if (argc == 3) {
		if (strcmp(argv[2], TEST_OPTN) != 0) {
			fprintf(stderr, "%s\n", WRONG_PAR);
			fprintf(stderr, "%s\n", SR_RIGHT_WAY);
			exit(EXIT_FAILURE);
		}
		else t_option = TRUE;
	}
	
	
	/* Messaggio di attivazione della modalità test */
	if (t_option) {
		fprintf(stdout, "%s\n", TESTMODE);
	}
	
	/* Apertura del file degli utenti e popolazione dell'albero */
	ec_null ( utenti_r = fopen(argv[1], "r") )
	ec_neg1 ( n_users = loadUsers(utenti_r, &generalTree) )
	fprintf(stdout, LOADED, n_users, argv[1]);
	ec_eof ( fclose(utenti_r) )
	utenti_r = NULL;
	
	/* Apertura della socket per l'accettazione delle connessioni */
	ec_neg1 ( socket_desc = createServerChannel(SOCKNAME) )
	
	/* Avvio del thread Signaler e del thread Dispatcher */
	ec_nzero ( err = pthread_create(&dispatch, NULL, &Dispatcher, &socket_desc) )
	ec_nzero ( err = pthread_create(&signaler, NULL, &Signaler, &dispatch) )
	
	/* Attesa della terminazione dei thread */
	ec_nzero ( err= pthread_join(signaler, NULL) )
	ec_nzero ( err= pthread_join(dispatch, NULL) )
	
	fprintf(stdout, "%s\n", CLOSING);
	
	/* Chiusura della socket */
	ec_neg1 ( closeServerChannel(SOCKNAME, socket_desc) )
	
	/* Aggiornamento file utenti */
	ec_null ( utenti_r = fopen(argv[1], "w") )
	ec_neg1 ( n_users = storeUsers(utenti_r, generalTree) )
	fprintf(stdout, SAVED, n_users, argv[1]);
	ec_eof ( fclose(utenti_r) )
	
	freeTree(generalTree);
	return 0;
	
	EC_CLEANUP_BGN
		
		if (err == 0) err = errno;
		
		if (signaler != 0) pthread_cancel(signaler);
		if (dispatch != 0) pthread_cancel(dispatch);
	
		if (utenti_r != NULL)
			fclose(utenti_r);
		
		freeTree(generalTree);
		
		if (socket_desc != -1)
			closeServerChannel(SOCKNAME, socket_desc);
		
		return err;
		
	EC_CLEANUP_END
}

