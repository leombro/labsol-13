/** 
 *  \file bris.h
 *  \author Orlando Leombruni
 * 
 *  \brief Implementazione di funzioni su carte.
 *  
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera originale dell'autore.
 * 
 */

#ifndef __BRIS__H
#define __BRIS__H

#include <stdio.h>
/** Tipo Booleano */
typedef enum bool { FALSE, TRUE } bool_t;

/** Possibili valori delle carte */
typedef enum valori{ASSO,DUE,TRE,QUATTRO,CINQUE,SEI,SETTE,FANTE,DONNA,RE} valori_t;

/** Possibili semi delle carte */
typedef enum semi{CUORI,QUADRI,FIORI,PICCHE } semi_t;

/** Una carta ha un valore ed un seme */
typedef struct carta {
  /** Valore */
  valori_t val; 
  /** Seme */
  semi_t seme;  
} carta_t;

/** Numero di carte in un mazzo da briscola */
#define NCARTE 40
/** Un mazzo e' un insieme di \c NCARTE carte in un certo ordine*/
typedef struct mazzo {
  /** Carte mischiate */
  carta_t carte [NCARTE]; 
  /** Indice della prossima carta da pescare */
  unsigned int next ;     
  /** Briscola (ovvero seme dell'ultima carta del mazzo)*/
  semi_t briscola;        
} mazzo_t;

/** Genera un mazzo di carte mischiato.
    
    \note Richiede che la srand() sia stata gia' invocata con un seed adeguato.

    \note ATTENZIONE: questa funzione e' fornita gia' implementata dai docenti 
    nel modulo oggetto newMazzo.o da inserire nella libreria libbris.a

    \retval p puntatore al nuovo mazzo 
    \retval NULL se si e' verificato un errore (setta \c errno)
*/
mazzo_t * newMazzo(void);



/** Stampa una carta nel formato a due caratteri (\c AQ \c 2F etc ...).
    \param fpd stream di uscita
    \param c  puntatore carta da stampare
*/
void printCard(FILE* fpd,carta_t *c);


/** Stampa una carta nel formato a due caratteri (\c AQ \c 2F etc ...) su stringa.
    \param s stringa di uscita (deve essere lunga almeno 3 caratteri)
    \param c  puntatore carta da stampare
    
    \section commentiagg1 Commenti Aggiuntivi    
    Ho scelto di implementare \c cardToString in modo che, se
  	\c c->val o \c c->seme contengono valori non definiti nelle
  	relative \c enum, il carattere stampato è U (Undefined)  
  	Ciò per evitare che la procedura si pianti quando
  	qualcuno particolarmente "cattivo" setta
  	il valore o il seme di una carta a un intero non
  	incluso nel range di \c valori_t e \c semi_t
*/
void cardToString(char* s, carta_t *c);

/** Converte una carta dal formato stringa 2 caratteri a struttura.
    \param str stringa di ingresso (deve essere lunga almeno 2, converte i primi 2 caratteri)
    \retval c  puntatore carta creata (alloca memoria)
    \retval NULL  se si e' verificato un errore (setta \c errno)
    
    \section commentiagg2 Commenti Aggiuntivi
    Ho incluso il caso \c <b>default</b>: se il carattere letto
  	non è nessuno di quelli ammissibili, setto \c errno
  	a "Invalid argument" (\c EINVAL) e la creazione della carta
  	fallisce.
*/
carta_t* stringToCard(char* str) ;

/** Pesca la prossima carta dal mazzo (aggiustando il campo \c next).
    \param m mazzo

    \retval NULL se tutte le carte del mazzo sono gia' state pescate (\c errno = 0)
    \retval NULL se si e' verificato un errore (\c errno != 0)
    \retval pc puntatore alla prossima carta (questa e' una copia della carta, allocata all'interno della funzione)
*/
carta_t* getCard(mazzo_t* m);

/** Stampa un mazzo di carte.
    \param fpd stream di uscita
    \param pm  puntatore al mazzo da stampare
*/
void printMazzo(FILE* fpd, mazzo_t *pm);

/** Dealloca mazzo di carte.
    \param pm  puntatore al mazzo
*/
void freeMazzo(mazzo_t *pm);

/** Confronta due carte data la briscola.
   \param briscola il seme di briscola
   \param ca, cb carte da confrontare (ca giocata prima di cb)
   \retval TRUE se ca batte cb
   \retval FALSE altrimenti
   
   \section commentiagg3 Commenti Aggiuntivi   
   Ho scritto la funzione seguendo questa logica:
 	\arg Se <tt>ca->seme == briscola</tt> e <tt>cb->seme != briscola</tt>, vince A
  	\arg Se <tt>ca->seme != briscola</tt> e <tt>cb->seme == briscola</tt>, vince B
  	\arg Se né A né B sono di seme di briscola e <tt>ca->seme != cb->seme</tt>, vince A
  	\arg Negli altri due casi, viene chiamata \c sameSeedCompare
 */
bool_t compareCard(semi_t briscola, carta_t* ca, carta_t* cb);

/** Calcola l'ammontare complessivo dei punti secondo le regole della briscola.
    \arg ASSO: 11
    \arg TRE:  10
    \arg RE:    4
    \arg DONNA: 3
    \arg FANTE: 2
    \arg DUE, QUATTRO, CINQUE, SEI, SETTE: 0

    \param c array carte da valutare
    \param n lunghezza array

    \retval -1 se si e' verificato un errore (setta \c errno)
    \retval np numero complessivo di punti
 */
int computePoints(carta_t**c, int n);

/** Controlla se una carta è presente nella mano
 * \param card carta da controllare
 * \param hand mano del giocatore
 * 
 * \retval TRUE se la mano del giocatore contiene la carta
 * \retval FALSE altrimenti
 */
bool_t isInHand(carta_t* card, carta_t** hand);

/** Scambia le carte di due mani diverse
 * \param hand1 mano del primo giocatore
 * \param hand2 mano del secondo giocatore
 * 
 * \retval 0 se l'operazione va a buon fine
 * \retval -1 se si verifica un errore (setta \c errno)
 */
int exchangeHands(carta_t* hand1[], carta_t* hand2[]);

/** Rimpiazza una carta della mano con una nuova
 * \param hand mano del giocatore
 * \param new carta pescata
 * \param old carta da sostituire
 * 
 */
void replace(carta_t* hand[], carta_t* new, carta_t* old);

/** Converte un seme in un carattere
 * \param seme seme da convertire
 * 
 * \retval c iniziale del seme ('N' se si verifica un errore)
 */
char semeToChar(semi_t seme);

/** Controlla se una partita è terminata.
 * La partita è terminata se le mani dei due giocatori sono vuote
 * 
 * \param first mano del primo giocatore
 * \param second mano del secondo giocatore
 * 
 * \retval TRUE se la partita è finita (entrambe le mani sono vuote)
 * \retval FALSE se la partita continua
 */
bool_t checkIfFinish (carta_t* first[], carta_t* second[]);
#endif
