/** 
 *  \file users.h
 *  \author Orlando Leombruni
 * 
 *  \brief Implementazione di funzioni su utenti e albero di utenti.
 *  
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera originale dell'autore.
 * 
 */

#ifndef __USERS__H
#define __USERS__H

#include <stdio.h>
#include "bris.h"

/** Lunghezza massima username */
#define LUSER 20
/** Lunghezza massima password */
#define LPWD  8
/** Errore utente inesistente */
#define NOUSR -2
/** Errore password errata */
#define WRPWD -3

/** Tipo connessione...
   DISCONNECTED disconnesso
   WAITING in attesa di sfida
   PLAYING impegnato in una partita
*/
typedef enum connection { DISCONNECTED, WAITING, PLAYING } status_t;

/** Dati utente **/
typedef struct user {
  /** Username */
  char name[LUSER + 1]; 
  /** Password */
  char passwd[LPWD + 1];
} user_t;

/** Nodo dell'albero che rappresenta l'utente registrato al servizio,
    ogni utente puo' essere connesso o disconnesso e se connesso puo'
    essere in attesa di una sfida o coinvolto in una partita -- 
    l'albero e' un albero di ricerca ordinato lessicograficamente (strcmp()) 
    rispetto al campo user */
typedef struct nodo {
  /** Dati utente */
  user_t* user;
  /** Stato di connessione in partita */
  status_t status;  
  /** Canale di comunicazione, significativo solo se connesso altrimenti (-1)*/
  int channel;
  /** Figlio destro */  
  struct nodo* left;       
  /** Figlio sinistro */ 
  struct nodo* right;      
} nodo_t;

/** A partire da una stringa \c nome_user:password crea una nuova struttura utente allocando la memoria
    corrispondente ed inserendo utente e password nei rispettivi campi.
    
    \param r buffer da convertire
    \param l lunghezza del buffer (numero max di caratteri che si
    possono leggere senza incorrere in overrun)

  \retval p (puntatore alla nuova struttura) 
  \retval NULL se si e' verificato un errore (setta \c errno)
 */
user_t* stringToUser(char* r, unsigned int l);

/** A partire da una struttura utente viene creata una stringa \c nome_user:password.
    La funzione alloca la memoria necessaria e termina correttamente
    la stringa risultato con il terminatore di stringa.

     \param puser struttura da convertire


     \retval p (puntatore alla nuova stringa) 
     \retval NULL se si e' verificato un errore (setta \c errno)
 */
char* userToString(user_t* puser);


/** Stampa l'albero su stdout.
    \param r radice dell'albero da stampare
 */
void printTree(nodo_t* r);

/** Aggiunge un nuovo utente all'albero mantenendolo ordinato lessicograficamente rispetto al campo users (ordinamento di strcmp). Se l'utente e' gia' presente non viene inserito.

 \param r puntatore alla radice dell'albero
 \param puser puntatore utente da inserire

 \retval 0 se l'inserzione e' andata a buon fine
 \retval 1 se l'utente e' gia' presente
 \retval -1 se si e' verificato un errore (setta \c errno)
 */
int addUser(nodo_t** r, user_t* puser);


/** Controlla la password di un utente.
 \param r radice dell'albero
 \param user  utente di cui controllare la passwd

 \retval TRUE  se l'utente e' presente e la password corrisponde
 \retval FALSE se non e' presente o la password e' errata
 */ 
bool_t checkPwd(nodo_t* r, user_t* user);

/** 
Rimuove le informazioni relative ad un certo utente (se l'utente
e' presente e la password coincide).

\param r puntatore alla radice dell'albero
\param puser puntatore utente da rimuovere

\retval NOUSR se l'utente non e' presente
\retval WRPWD se la password e' errata
\retval 0 se la rimozione e' avvenuta correttamente
\retval -1 se si e' verificato un errore (setta \c errno)
\section commentiagg Commenti Aggiuntivi
\subsection parte1 Parte 1: ricerca del nodo da rimuovere
Mediante due puntatori e un booleano, scorro l'albero
fino a trovare il nodo da rimuovere. Se lo trovo e le
password coincidono, setto \c found a \c <b>TRUE</b>; se le password
non coincidono, restituisco \c <b>WRPWD</b>. Se alla fine della visita 
dell'albero \c found è \c <b>FALSE</b>, l'utente non esiste nell'albero e 
dunque restituisco \c <b>NOUSR</b>. Se l'utente esiste e <tt>found == <b>TRUE</b></tt>, 
\c corr punta all'elemento da rimuovere e \c prec è il suo predecessore.

\snippet removeuser-snippets.c Parte1

\n

\subsection parte2 Parte 2: rimozione effettiva del nodo 
In linea generale, per rimuovere un nodo e' necessario trovare un altro
nodo (discendente di quello da rimuovere) che, sostituito al primo,
mantiene le proprietà di ricerca dell'albero. Tale processo e' semplificato
se il nodo da rimuovere è una foglia oppure ha un solo sottoalbero.
\subsubsection p2caso1 Caso 1: il nodo da rimuovere è una foglia
Controllo se il predecessore è \c <b>NULL</b>; se sì, vuol dire che il
nodo da rimuovere è proprio la radice. 
Se <tt>prec != <b>NULL</b></tt>, controllo se \c corr è figlio sinistro
o destro di \c prec e aggiorno i puntatori a \c <b>NULL</b> di
conseguenza.

\snippet removeuser-snippets.c Parte2Sez1

\subsubsection p2caso2a Caso 2a: il nodo da rimuovere ha solo il figlio sinistro
Se <tt>prec == <b>NULL</b></tt>, \c corr è la radice e dunque basta porre
\c corr->left come nuova radice.
Se <tt>prec != <b>NULL</b></tt>, controllo se \c corr è figlio sinistro o
destro di \c prec, dopodiché aggiorno il puntatore giusto
in modo che punti a \c corr->left.

\snippet removeuser-snippets.c Parte2Sez2a
\subsubsection p2caso2b Caso 2b: il nodo da rimuovere ha solo il figlio destro
Analogo al precedente.

\snippet removeuser-snippets.c Parte2Sez2b

\subsubsection p2caso3 Caso 3: il nodo da rimuovere ha entrambi i figli
Con due nuovi puntatori, \c temp1 e \c temp2, visito il
sottoalbero sinistro di \c corr fino a trovarne il massimo
(foglia più a destra del sottoalbero radicato in \c corr->left ).
Scambiando tale foglia con il nodo da rimuovere si manterrà
la proprietà di ricerca dell'albero.
Tale massimo esiste sempre (al limite è proprio il figlio sinistro
di \c corr ); dunque alla fine della visita avrò che \c temp2 è il massimo
del sottoalbero sinistro di \c corr e \c temp1 è il suo predecessore.
Procedo dunque ad aggiornare i puntatori, "agganciando" il sottoalbero
destro di \c corr a \c temp2 e ponendo \c temp2 come nuovo figlio sinistro/destro
di \c prec (se <tt>prec != <b>NULL</b></tt>, altrimenti \c temp2 è la nuova radice).

\snippet removeuser-snippets.c Parte2Sez3
\note La scelta di prendere il massimo del sottoalbero sinistro
come nodo che mantiene le proprietà di ricerca è arbitraria, infatti
anche il minimo del sottoalbero sinistro (foglia più a sinistra del
sottoalbero radicato in \c corr->right ) mantiene la stessa proprietà.
*/
int removeUser(nodo_t** r, user_t* puser);



/** Dealloca l'albero.

 \param r radice dell'albero da deallocare
*/
void freeTree(nodo_t* r);

/** Legge il file che contiene gli utenti ammessi e le loro password e
 *  li aggiunge all'albero di ricerca passato come parametro. 

 Gli utenti sono memorizzati su file nel formato nome_utente:password e separati da un '\\n' 
 
 \param fin il file di ingresso
 \param r il puntatore al puntatore alla radice dell'albero

 \retval n il numero di utenti letti ed inseriti nell'albero se tutto e' andato a buon fine
 \retval -1 se si e' verificato un errore (setta \c errno)
 */
int loadUsers(FILE* fin, nodo_t** r);

/** Scrive tutti i permessi nell'albero su file in ordine
    lessicografico. Ogni utente e' scritto nel formato \c nome_utente:password
 separati da '\\n'
 
 \param fout il file su cui scrivere 
 \param r il puntatore alla radice dell'albero
 
 \retval n il numero di utenti registrati nel file
 \retval -1 se si e' verificato un errore   (setta \c errno)
 */
int storeUsers(FILE* fout, nodo_t* r);

/** utente non registrato */
#define NOTREG (-10)

/** Restituisce lo stato di un utente (se esiste).
 \param r radice dell'albero
 \param u  utente da cercare

 \retval s stato dell'utente se l'utente e' presente
 \retval NOTREG se non e' presente
*/
status_t getUserStatus(nodo_t* r, char* u);

/** Restituisce il canale di un utente (se esiste).
 \param r radice dell'albero
 \param u  utente da cercare

 \retval ch canale su cui e' connesso l'utente se l'utente e' collegato
 \retval NOTREG se non e' presente
*/
int getUserChannel(nodo_t* r, char* u);

/** Setta lo stato di un utente (se esiste).
 \param r radice dell'albero
 \param u  utente da cercare
 \param s stato da settare 

 \retval TRUE se l'utente e' presente
 \retval FALSE se non e' presente
*/
bool_t setUserStatus(nodo_t* r, char* u, status_t s);

/** Setta il canale di un utente (se esiste).
 \param r radice dell'albero
 \param u  utente da cercare
 \param ch canale da settare 

 \retval TRUE se l'utente e' presente
 \retval FALSE se non e' presente
*/
bool_t setUserChannel(nodo_t* r, char* u, int ch);

/** Controlla se un utente e' registrato nell'albero.
 \param r radice dell'albero
 \param u  utente da cercare

 \retval TRUE se l'utente e' presente
 \retval FALSE se non e' presente
*/
bool_t isUser(nodo_t* r, char* u);

/** 
Fornisce la lista degli utenti connessi sull'albero con <tt>stato == st</tt>.
\param r radice dell'albero
\param st stato da ricercare

\retval s la stringa con gli utenti secondo il formato 
              user1:user2:...:userN
\retval NULL se non ci sono utenti nello stato richiesto

\section commentiagg2 Commenti Aggiuntivi
Ho implementato la funzione ricorsivamente, strutturandola come una
visita posticipata; la funzione opera poi concatenando
tra loro la stringa generata dalla visita del sottoalbero sinistro
(se esiste), lo username dell'utente del nodo corrente (se il suo status è
\c st) e la stringa generata dalla visita del sottoalbero destro (se
esiste).
 */
char *  getUserList(nodo_t* r, status_t st);
#endif
