/** \file errors.h
 * \author Orlando Leombruni
 * 
 * \brief Header contenente macro e definizione di funzioni per la gestione e la stampa di errori
 *  
 * Alcune righe di codice sono prese dal libro di M. Rochkind "Advanced Unix Programming".
 * Si dichiara che il contenuto di questo file, fatta eccezione per le righe di cui sopra, è opera dell'autore.
 * 
 */

#ifndef __ERRORHANDLING_H_
#define __ERRORHANDLING_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#define true 1
#define false 0

/** Macro per l'asserzione della dimensione di un puntatore
 * (serve a garantire che il tipo \c intptr_t possa contenere ogni tipo
 * di puntatore)
 */
#define PTRASSERT assert(sizeof(long) == sizeof(void*));

/** Macro per l'inizio delle sezioni di pulizia di una funzione.
 * La stampa "Cleaning from errors..." è superflua ed è stata inserita per evitare il
 * warning "ec_in_cleanup: variable set but not used" del GCC
 */
#define EC_CLEANUP_BGN\
	fprintf(stderr, "Attenzione, si e' entrati per errore in una funzione di cleanup\n");\
	ec_cleanup_bgn:\
	{\
		bool_t ec_in_cleanup;\
		ec_in_cleanup = true;\
		if (ec_in_cleanup) fprintf(stderr, "Cleaning from errors...\n");

/** Macro per la fine delle sezioni di pulizia di una funzione */
#define EC_CLEANUP_END\
	}

/** Macro principale della gestione degli errori: se \c var == \c val si ha un errore
 * e si salta dunque alle sezioni di pulizia. Le funzioni chiamate con questo codice di errore
 * settano \c errno.
 */
#define ec_cmp(var,val)\
	{\
		assert(!ec_in_cleanup);\
		if ((intptr_t)(var) == (intptr_t)(val)) {\
			errorprint(__func__, __FILE__, __LINE__, #var, errno);\
			goto ec_cleanup_bgn;\
		}\
	}

/** Macro per funzioni che ritornano direttamente un codice di errore e non settano \c errno */
#define ec_rv(var)\
	{\
		int errrtn;\
		assert(!ec_in_cleanup);\
		if ((errrtn = (var)) != 0) {\
			errorprint(__func__, __FILE__, __LINE__, #var, errrtn);\
			goto ec_cleanup_bgn;\
		}\
	}

/** Macro che garantisce l'esecuzione della procedura di errore */
#define EC_FAIL ec_cmp(0, 0)
/** Macro per le funzioni che ritornano -1 e settano \c errno in caso di errore */
#define ec_neg1(x) ec_cmp(x, -1)
/** Macro per le funzioni che ritornano \c NULL e settano \c errno in caso di errore */
#define ec_null(x) ec_cmp(x, NULL)
/** Macro per le funzioni che ritornano \c EOF e settano \c errno in caso di errore */
#define ec_eof(x) ec_cmp(x, EOF)
/** Macro per le funzioni che ritornano un valore diverso da 0 e settano \c errno in caso di errore */
#define ec_nzero(x)\
	{\
		if ((x) != 0) \
			EC_FAIL\
	}

/** Macro per saltare direttamente nella funzione di cleanup */
#define EC_CLEANUP_NOW goto ec_cleanup_bgn;

/** Definizione del tipo booleano */
typedef int bool_e;
/** Definizione del tipo \c intptr_t (puntatore generico) */
typedef long intptr_t;

/** Struttura per convertire valori di \c errno nelle corrispondenti stringhe.
 * Sono stati inseriti solo i valori di \c errno conformi agli standard PosiX
 */
static struct {
	int code;
	char* str;
} errcodes[] =
{
	{ EPERM, "EPERM" },
	{ ENOENT, "ENOENT" },
	{ ESRCH, "ESRCH" },
	{ EINTR, "EINTR" },
	{ EIO, "EIO" },
	{ ENXIO, "ENXIO" },
	{ E2BIG, "E2BIG" },
	{ ENOEXEC, "ENOEXEC" },
	{ EBADF, "EBADF" },
	{ ECHILD, "ECHILD" },
	{ EAGAIN, "EAGAIN" },
	{ ENOMEM, "ENOMEM" },
	{ EACCES, "EACCES" },
	{ EFAULT, "EFAULT" },
	{ EBUSY, "EBUSY" },
	{ EEXIST, "EEXIST" },
	{ EXDEV, "EXDEV" },
	{ ENODEV, "ENODEV" },
	{ ENOTDIR, "ENOTDIR" },
	{ EISDIR, "EISDIR" },
	{ EINVAL, "EINVAL" },
	{ ENFILE, "ENFILE" },
	{ EMFILE, "EMFILE" },
	{ ENOTTY, "ENOTTY" },
	{ EFBIG, "EFBIG" },
	{ ENOSPC, "ENOSPC" },
	{ ESPIPE, "ESPIPE" },
	{ EROFS, "EROFS" },
	{ EMLINK, "EMLINK" },
	{ EPIPE, "EPIPE" },
	{ EDOM, "EDOM" },
	{ ERANGE, "ERANGE" },
	{ EDEADLK, "EDEADLK" },
	{ ENAMETOOLONG, "ENAMETOOLONG" },
	{ ENOLCK, "ENOLCK" },
	{ ENOSYS, "ENOSYS" },
	{ ENOTEMPTY, "ENOTEMPTY" },
	{ ENOTBLK, "ENOTBLK" },
	{ ETXTBSY, "ETXTBSY" },
	{ ENOTSOCK, "ENOTSOCK" },
	{ EDESTADDRREQ, "EDESTADDRREQ" },
	{ EMSGSIZE, "EMSGSIZE" },
	{ EPROTOTYPE, "EPROTOTYPE" },
	{ ENOPROTOOPT, "ENOPROTOOPT" },
	{ EPROTONOSUPPORT, "EPROTONOSUPPORT" },
	{ ESOCKTNOSUPPORT, "ESOCKTNOSUPPORT" },
	{ EOPNOTSUPP, "EOPNOTSUPP" },
	{ EPFNOSUPPORT, "EPFNOSUPPORT" },
	{ EAFNOSUPPORT, "EAFNOSUPPORT" },
	{ EADDRINUSE, "EADDRINUSE" },
	{ EADDRNOTAVAIL, "EADDRNOTAVAIL" },
	{ ENETDOWN, "ENETDOWN" },
	{ ENETUNREACH, "ENETUNREACH" },
	{ ENETRESET, "ENETRESET" },
	{ ECONNABORTED, "ECONNABORTED" },
	{ ECONNRESET, "ECONNRESET" },
	{ ENOBUFS, "ENOBUFS" },
	{ EISCONN, "EISCONN" },
	{ ENOTCONN, "ENOTCONN" },
	{ ESHUTDOWN, "ESHUTDOWN" },
	{ ETOOMANYREFS, "ETOOMANYREFS" },
	{ ETIMEDOUT, "ETIMEDOUT" },
	{ ECONNREFUSED, "ECONNREFUSED" },
	{ ELOOP, "ELOOP" },
	{ EWOULDBLOCK, "EWOULDBLOCK" },
	{ EALREADY, "EALREADY" },
	{ EINPROGRESS, "EINPROGRESS" },
	{ EHOSTDOWN, "EHOSTDOWN" },
	{ EHOSTUNREACH, "EHOSTUNREACH" },
	{ ESTALE, "ESTALE" },
	{ EUSERS, "EUSERS" },
	{ 0, NULL }
};

/** Variabile globale (per evitare cicli di macro) */
extern const bool_e ec_in_cleanup;

/** Dato un valore di \c errno, restituisce una stringa contenente
 * l'abbreviazione PosiX (ad es. se \c errno == \c EINVAL la funzione ritorna
 * la stringa \c "EINVAL")
 * \param errno_arg valore di errno
 * 
 * \retval stringa contenente l'abbreviazione PosiX
 */
const char *errsymbol (int errno_arg);

/** Dato un valore di \c errno, costruisce una stringa esplicativa del tipo:
 * 
 * \c abbreviazionePosiX \c (numeroErrore: \c "spiegazione")
 * 
 * \param buf buffer che conterrà la stringa
 * \param buf_max dimensione del buffer
 * \param errno_arg valore di errno
 * 
 * \retval buf buffer contenente la stringa
 */
char *syserrmsg (char* buf, size_t buf_max, int errno_arg);

/** Stampa a schermo informazioni sull'errore
 * \param fun nome della funzione che ha riportato errore
 * \param file nome del file corrente
 * \param line numero linea della chiamata di funzione che ha riportato errore
 * \param arg linea della chiamata di funzione che ha riportato errore
 * \param errno_arg valore di errno
 */
 
void errorprint (const char *fun, const char *file, int line, const char *arg, int errno_arg);

#endif
