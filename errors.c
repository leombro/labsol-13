/** \file errors.c
 * \author Orlando Leombruni
 * 
 * \brief Implementazione delle funzioni di errors.h
 *  
 * Alcune righe di codice sono prese dal libro di M. Rochkind "Advanced Unix Programming".
 * Si dichiara che il contenuto di questo file, fatta eccezione per le righe di cui sopra, è opera dell'autore.
 * 
 */
#include "errors.h"

const bool_e ec_in_cleanup = false;

const char *errsymbol (int errno_arg)
{
	int i;
	for (i = 0; errcodes[i].str != NULL; i++) {
		if (errcodes[i].code == errno_arg)
			return errcodes[i].str;
	}
	return "[Unknown]";
}

char *syserrmsg (char* buf, size_t buf_max, int errno_arg)
{
	char *errmsg;
	
	if (errno_arg == 0) snprintf(buf, buf_max, "No error code");
	else {
		errmsg = strerror(errno_arg);
		snprintf(buf, buf_max, "\t\t**** %s (%d: \"%s\") ****", errsymbol(errno_arg), errno_arg, errmsg != NULL ? errmsg : "no message string");
	}
	return buf;
}

#define sep1 "["
#define sep2 ":"
#define sep3 "]"

void errorprint (const char *fun, const char *file, int line, const char *arg, int errno_arg)
{
	char buf[200];
	
	fprintf(stderr, "%s %s%s%s%d%s %s\n%s\n", fun, sep1, file, sep2, line, sep3, arg, syserrmsg(buf, 200, errno_arg));
}
