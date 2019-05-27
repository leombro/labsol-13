/**  \file comsock.h
 *   \brief Header libreria di comunicazione socket AF_UNIX
 *
 *   \author Orlando Leombruni
 * 
 * Attenzione, la libreria deve funzionare senza fare assunzioni sul tipo 
 * e sulla lunghezza del messaggio inviato, inoltre deve essere possibile 
 * aggiungere tipi senza dover riprogrammare la libreria stessa.
 * 
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera originale dell'autore o dei docenti del corso di Lab SOL a.a. 2012/2013.
 * 
 */

#ifndef _COMSOCK_H
#define _COMSOCK_H

/* -= TIPI =- */


/** <H3>Messaggio</H3>
 * La struttura \c message_t rappresenta un messaggio 
 * - \c type rappresenta il tipo del messaggio
 * - \c length rappresenta la lunghezza in byte del campo \c buffer
 * - \c buffer del messaggio 
 *
 * <HR>
 */
typedef struct {
  /** Tipo del messaggio */
    char type;           
  /** Lunghezza messaggio in byte */
    unsigned int length; 
  /** Buffer messaggio */
    char *buffer;        
} message_t; 

/** Lunghezza buffer indirizzo \c AF_UNIX */
#define UNIX_PATH_MAX    108

/** Massimo numero di tentativi di connessione specificabili nella \c openConnection */
#define MAXTRIAL 10
/** Massimo numero secondi specificabili nella \c openConnection */
#define MAXSEC 10

/** Numero di tentativi per connessione a un server */
#define NTRIAL 3
/** Numero di secondi fra due connessioni consecutive */
#define NSEC 1

/* Tipi dei messaggi scambiati fra server e client */

/** Messaggio di richiesta di registrazione nuovo utente */
#define MSG_REG        'R' 
/** Messaggio di richiesta di cancellazione utente */
#define MSG_CANC       'Q' 
/** Messaggio di richiesta di disconnessione forzata */
#define MSG_DISC       'D' 
/** Messaggio di richiesta di connessione al servizio */
#define MSG_CONNECT      'C' 
/** Messaggio di attesa di un avversario */
#define MSG_WAIT      'W' 
/** Messaggio di accettazione */
#define MSG_OK           'K' 
/** Messaggio di rifiuto */
#define MSG_NO           'N' 
/** Messaggio di errore */
#define MSG_ERR          'E' 
/** Messaggio di comunicazione inizio gioco: avversario, briscola, prima mano, turno*/
#define MSG_STARTGAME      'S' 
/** Messaggio di comunicazione fine gioco */
#define MSG_ENDGAME      'Z' 
/** Messaggio di comunicazione carta giocata */
#define MSG_PLAY      'P' 
/** Messaggio di comunicazione nuova carta */
#define MSG_CARD      'A' 


/* -= FUNZIONI =- */
/** Crea una socket \c AF_UNIX
 *  \param  path pathname della socket
 *
 *  \retval s    il file descriptor della socket  (\c s > 0)
 *  \retval -1   in altri casi di errore (setta \c errno)
 *               \c errno = \c E2BIG se il nome eccede \c UNIX_PATH_MAX
 *
 *  \note in caso di errore ripristina la situazione inziale: rimuove eventuali socket create e chiude eventuali file descriptor rimasti aperti
 */
int createServerChannel(char* path);

/** Chiude un canale lato server, rimuovendo la socket dal file system 
 *   \param path path della socket
 *   \param s file descriptor della socket
 *
 *   \retval 0  se tutto ok, 
 *   \retval -1  se errore (setta \c errno)
 */
int closeServerChannel(char* path, int s);

/** Accetta una connessione da parte di un client
 *  \param  s socket su cui ci mettiamo in attesa di accettare la connessione
 *
 *  \retval  c il descrittore della socket su cui siamo connessi
 *  \retval  -1 in casi di errore (setta \c errno)
 */
int acceptConnection(int s);

/** Legge un messaggio dalla socket 
 * \note Si richiede che il messaggio sia adeguatamente spacchettato e trasferito nella struttura \c msg
 *  \param  sc  file descriptor della socket
 *  \param msg  indirizzo della struttura che conterra' il messaggio letto
 * 
 * \note la struttura \c message_t deve essere allocata all'esterno della funzione, ma il campo \c buffer è allocato
 * 		all'interno della funzione in base alla lunghezza ricevuta
 *
 *  \retval lung  lunghezza del buffer letto, se l'operazione è andata a buon fine
 *  \retval  -1   in caso di errore (setta \c errno)
 *                 \c errno = \c ENOTCONN se il peer ha chiuso la connessione 
 *                   (non ci sono piu' scrittori sulla socket)
 *  
 * 	\section comagg2 Commenti Aggiuntivi
 * 	La funzione legge dalla socket un messaggio nel formato definito nella sendMessage e lo trasforma in una
 * 	struttura message_t.
 */
int receiveMessage(int sc, message_t * msg);

/** Scrive un messaggio sulla socket 
 *  \note Sono inviati \b solo i byte significativi del campo \c buffer (\c msg->length byte) - il messaggio è impacchettato
 * 		  in modo da garantire l'atomicità della \c write
 * 	
 *   \param  sc file descriptor della socket
 *   \param msg indirizzo della struttura che contiene il messaggio da scrivere 
 *   
 *   \retval  n    il numero di caratteri inviati, se la scrittura è andata a buon fine
 *   \retval -1   in caso di errore (setta \c errno)
 *                 \c errno = \c ENOTCONN se il peer ha chiuso la connessione 
 *                   (non ci sono piu' lettori sulla socket)
 * 
 * 	 \section commagg1 Commenti Aggiuntivi
 * 	 Il messaggio viene incapsulato in una struttura di tipo \c char* secondo questa logica: in posizione 0
 * 	 si trova il carattere che contraddistingue il tipo di messaggio (campo \c type della struttura message_t);
 * 	 nelle posizioni 1-4 si trovano i byte in cui è stato convertito il campo \c length (mediante la intToChar); infine,
 * 	 se \c msg.length > 0, nelle posizioni da 5 a \c msg.length+4 è memorizzato il campo \c buffer.
 */
int sendMessage(int sc, message_t *msg);

/** Crea una connessione alla socket del server. In caso di errore, ritenta \c ntrial volte la connessione (a distanza di \c k secondi l'una dall'altra) prima di ritornare errore.
 *   \param  path  nome della socket su cui il server accetta le connessioni
 *   \param  ntrial numeri di tentativi prima di restituire errore (\c ntrial <= \c MAXTRIAL)
 *   \param  k secondi l'uno dell'altro (\c k <= \c MAXSEC)
 *   
 *   \return fd il file descriptor della connessione
 *            se la connessione ha successo
 *   \retval -1 in caso di errore (setta \c errno)
 *               \c errno = \c E2BIG se il nome eccede \c UNIX_PATH_MAX
 *
 *  \note In caso di errore ripristina la situazione inziale: rimuove eventuali socket create e chiude eventuali file descriptor rimasti aperti.
 */
int openConnection(char* path, int ntrial, int k);

/** Chiude una connessione
 *   \param s file descriptor della socket relativa alla connessione
 *
 *   \retval 0  se tutto ok 
 *   \retval -1  se errore (setta \c errno)
 */
int closeConnection(int s);

/** Crea un messaggio a partire dal tipo e dal buffer
 * 
 * \param msg struttura che conterrà il messaggio
 * \param m_type tipo di messaggio da inviare
 * \param m_buf stringa contenente il messaggio da inviare
 * 
 * \retval 0 se la creazione va a buon fine
 * \retval -1 se la creazione fallisce (setta \c errno)
 *
 */

int createMessage(message_t* msg, char m_type, char* m_buf);

#endif
