/** \file commonstrings.h
 * \author Orlando Leombruni
 * 
 * \brief Definizioni di macro per il client e il server
 *  
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera originale dell'autore.
 * 
 */
 
#ifndef __COMMONSTRINGS_H_
#define __COMMONSTRINGS_H_

/* NON MODIFICARE */
/* Stringhe per i nomi dei file */
/** Pathname della socket */
#define SOCKNAME "./tmp/briscola.skt"
/** Pathname per il file checkpoint */
#define CP_NAME "./brs.checkpoint"
/** Template per il nome dei file di log (prefisso) */
#define LOG_NAME_ST "./BRS-"
/** Template per il nome dei file di log (suffisso) */
#define LOG_NAME_END ".log"
/* Opzioni */
/** Modalità server test */
#define TEST_OPTN "-t"
/** Registrazione di un utente */
#define REG_OPTN "-r"
/** Cancellazione di un utente */
#define CANC_OPTN "-c"
/** Disconnessione forzata */
#define DISC_OPTN "-d"
/** Messaggio di attesa */
#define WAIT_MSG "WAIT"

/* Definizione macro per stringhe */

/** Corretto utilizzo del server */
#define SR_RIGHT_WAY "Uso:\tbrsserver file_utenti [-t]"
/** Non è stata fornita una lista di utenti */
#define NO_USRLIST "Errore: devi fornire la lista utenti"
/** Troppi parametri */
#define TOO_MANY_PAR "Errore: troppi parametri"
/** Parametro non corretto */
#define WRONG_PAR "Errore: secondo parametro non corretto"

/** Il server è in modalità test */
#define TESTMODE "-- MODALITA' TEST ATTIVA --"
/** Numero di utenti caricati */
#define LOADED "Caricati %d utenti dal file %s \n"
/** Il server è in chiusura */
#define CLOSING "Chiusura..."
/** Numero di utenti salvati */
#define SAVED "Scritti %d utenti nel file %s \n"

/** Errore nella stringToUser */
#define ERR_STRTOU "Impossibile elaborare le informazioni inserite\n(Es. nome utente o password troppo lunghi)"
/** Errore fatale nella addUser */
#define INSERT_ERROR "Errore nell'inserimento nell'albero utenti"
/** Utente già presente (errore nella addUser) */
#define USR_ALREADY "Utente gia' presente"
/** Utente non presente (errore nella removeUser) */
#define NOUSR_ERROR "Non esiste un utente con questo username"
/** Password errata (errore nella removeUser) */
#define WRPWD_ERROR "La password inserita e' errata"
/** Errore fatale nella removeUser */
#define REMOVE_ERROR "Errore nella rimozione"
/** Utente già connesso */
#define ALR_CONN "Utente già connesso"
/** Funzionalità non supportata */
#define NOT_SUPPORTED "Non supportato al momento"
/** La carta giocata dall'utente non è presente nella sua mano */
#define NOT_IN_DECK "La carta giocata non e' presente nella mano"
/** La stringa inserita dall'utente non corrisponde a una carta */
#define NOT_A_CARD "Formato carta errato"
/** Stringa "pareggio" */
#define DRAW "pareggio"

/** Prima riga del file di log */
#define FIRST_LOG "%s:%s\nBRISCOLA:%c\n"
/** Ultima riga del file di log */
#define LAST_LOG "WINS:%s\nPOINTS:%s\n"

/** Segnale SIGINT ricevuto */
#define TERM_SIGINT "SIGINT -- Terminazione..."
/** Segnale SIGTERM ricevuto */
#define TERM_SIGTERM "SIGTERM -- Terminazione..."
/** Segnale SIGUSR1 ricevuto */
#define CHECK_SIGUSR1 "SIGUSR1 -- Stampa dell'albero su file di checkpoint completata"

/* Client */

/** Server (o altro giocatore) disconnesso */
#define SERVER_KILLED "Errore: il server e' stato terminato o lo sfidante si e' disconnesso\nUscita in corso"

/** Utilizzo del programma */
#define CL_RIGHT_WAY "Uso:\tbrsclient username password [-r | -c | -d]"
/** Numero di argomenti da linea di comando non valido */
#define WR_NUMB_OF_ARGS "Errore: numero di argomenti non valido"
/** Opzione non riconosciuta */
#define WRONG_OPTION "Errore: opzione non riconosciuta"

/** Connessione al server riuscita, scelta dell'avversario */
#define PLAYERSELECT "Connessione al server effettuata! Lista utenti disponibili:\n%s\nScegli un avversario (WAIT per attendere):\n"
/** Il server ha accettato la richiesta di attesa */
#define OK_WAITING "Ok, sei in attesa!"
/** Connessione al server riuscita, nessun giocatore disponibile */
#define NOPLAYERS "Connessione al server effettuata!\nNessun giocatore e' disponibile per una partita (verrai automaticamente messo in attesa)"
/** Connessione al server rifiutata */
#define CONN_REFUSED "Il server ha rifiutato la connessione. Motivazione:\n%s\n"


/** Inizio della stampa del messaggio inviato dal server */
#define SERVER_REPLY "Il server ha inviato la risposta: "
/** Il server ha risposto MSG_OK */
#define OP_OK "Operazione andata a buon fine.\n"
/** Il server ha risposto MSG_NO */
#define OP_NO "Operazione non eseguita.\n"
/** Il server ha risposto MSG_ERR */
#define OP_ERR "Si e' verificato un errore.\n"
/** Spiegazioni addizionali */
#define MORE_INFO "Ulteriori informazioni: "
/** Nessuna spiegazione addizionale */
#define NO_MORE_INFO "nessuna.\n"
/** Tipo di messaggio non atteso */
#define MSG_NOT_EXPECTED "Messaggio inatteso dal server, uscita\n"

/** Primo messaggio di una partita */
#define PLAY_FIRST "Giochiamo con %s\nBriscola: %c\n"
/** Carte della mano */
#define CARDS "Carte: %s\n"
/** Turno del giocatore */
#define YOURTURN "Turno: %s --? "
/** Turno dell'avversario (se gioca per secondo) */
#define ENEMYTURN_2 "Turno: %s --> %s\n"
/** Turno dell'avversario (se gioca per primo) */
#define ENEMYTURN_1 "Turno: %s --> "
/** Messaggio di fine partita (vittoria di uno dei giocatori) */
#define WINMSG "Vince %s con %d punti!\nBye\n"
/** Messaggio di fine partita (pareggio) */
#define DRAWMSG "Pareggio con 60 punti!\nBye\n"




#endif
