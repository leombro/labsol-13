/** 
 *  \file users.c
 *  \author Orlando Leombruni
 * 
 *  \brief Implementazione di funzioni su utenti e albero di utenti.
 *  
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera originale dell'autore.
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "bris.h"
#include "users.h"

user_t* stringToUser(char* r, unsigned int l){
	user_t* p = NULL;
	char* pointer = NULL;
	int i = 0, j = 0;
	p = (user_t*)malloc(sizeof(user_t));
	if (p == NULL) return NULL;
	pointer = r;
	while (pointer[i] != ':' && i < l) {
		i++;
	}
	if (i == 0 || i > LUSER || i == l) {
		free(p);
		errno = EINVAL;
		return NULL;
	}
	strncpy(p->name, r, i);
	p->name[i] = '\0';
	i++;
	while (pointer[i+j] != '\0' && i+j < l) j++;
	if (j == 0 || j > LPWD || i+j == l) {
		free(p);
		errno = EINVAL;
		return NULL;
	}
	strcpy(p->passwd, pointer+i);
	return p;
}

char* userToString(user_t* puser) {
	char* p = NULL;
	int k = 0, m = 0;
	if (puser == NULL) {
		errno = EINVAL;
		return NULL;
	}
	k = strlen(puser->name);
	m = strlen(puser->passwd);
	p = (char*)malloc((k+m+2)*sizeof(char));
	if (p == NULL) return NULL;
	strcpy(p, puser->name);
	strcat(p, ":");
	strcat(p, puser->passwd);
	return p;
}

void printTree(nodo_t* r) {
	if (r != NULL) {
		char* user_val;
		printTree(r->left);
		user_val = userToString(r->user);
		if (user_val != NULL) {
			fprintf(stdout, "Utente: %s,", user_val);
			free(user_val);
			fprintf(stdout, " stato: ");
			if (r->status == DISCONNECTED) fprintf(stdout, "disconnesso,");
			if (r->status == WAITING) fprintf(stdout, "in attesa di partita,");
			if (r->status == PLAYING) fprintf(stdout, "sta giocando,");
			fprintf(stdout, " canale: %d\n", r->channel);
		}
		printTree(r->right);
	}
}

int addUser(nodo_t** r, user_t* puser) {
	if (r != NULL) {
		if ((*r) == NULL) {
			nodo_t* new = NULL;
			new = (nodo_t*)malloc(sizeof(nodo_t));
			if (new == NULL) return -1;
			new->user = puser;
			new->status = DISCONNECTED;
			new->channel = -1;
			new->left = NULL;
			new->right = NULL;
			(*r) = new;
			return 0;
		}
		else {
			if (strcmp((*r)->user->name, puser->name) > 0) return addUser(&((*r)->left), puser);
			else if (strcmp((*r)->user->name, puser->name) < 0) return addUser(&((*r)->right), puser);
			else return 1;
		}
	}
}

bool_t checkPwd(nodo_t* r, user_t* user) {
	if (r != NULL) {
		if (strcmp(r->user->name, user->name) == 0) {
			if (strcmp(r->user->passwd, user->passwd) == 0) return TRUE;
			else return FALSE;
		}
		else {
			if (strcmp(r->user->name, user->name) > 0) return checkPwd(r->left, user);
			else return checkPwd(r->right, user);
		}
	}
	else return FALSE;
}

int removeUser(nodo_t** r, user_t* puser) {
	nodo_t *prec = NULL, *corr = NULL, *temp1 = NULL, *temp2 = NULL;
	bool_t found = FALSE;
	if (r == NULL || puser == NULL) {
		errno = EINVAL;
		return -1;
	}
	corr = *r; 
	while (corr != NULL && !found) {
		if (strcmp(corr->user->name, puser->name) == 0) {
			if (strcmp(corr->user->passwd, puser->passwd) == 0) found = TRUE;
			else return WRPWD;
		}
		else {
			prec = corr;
			if (strcmp(corr->user->name, puser->name) > 0) corr = corr->left;
			else corr = corr->right;
		}
	}
	if (!found) return NOUSR;
	else {
		 if (corr->left == NULL && corr->right == NULL) {
			if (prec != NULL) {
				if (strcmp(prec->user->name, corr->user->name) > 0) prec->left = NULL;
				else prec->right = NULL;
			}
			else (*r) = NULL;
			free(corr->user);
			free(corr);
		}
		else if (corr->left != NULL && corr->right == NULL) {
			if (prec != NULL) {
				if (strcmp(prec->user->name, corr->user->name) > 0) prec->left = corr->left;
				else prec->right = corr->left;
			}
			else (*r) = corr->left;
			free(corr->user);
			free(corr);
		}
		else if (corr->left == NULL && corr->right != NULL) {
			if (prec != NULL) {
				if (strcmp(prec->user->name, corr->user->name) > 0) prec->left = corr->right;
				else prec->right = corr->right;
			}
			else (*r) = corr->right;
			free(corr->user);
			free(corr);
		}
		else if (corr->left != NULL && corr->right != NULL) {
			temp1 = corr;
			temp2 = corr->left;
			while (temp2->right != NULL) {
				temp1 = temp2;
				temp2 = temp2->right;
			}
			if (temp2->left != NULL && temp1 != corr) {
				temp1->right = temp2->left;
			}
			else temp1->right = NULL;
			if (prec != NULL) {
				if (strcmp(prec->user->name, corr->user->name) > 0) prec->left = temp2;
				else prec->right = temp2;
			}
			else (*r) = temp2;
			temp2->left = corr->left;
			temp2->right = corr->right;
			free(corr->user);
			free(corr);
		}
		return 0;
	}
}

void freeTree(nodo_t* r) {
	if (r != NULL) {
		freeTree(r->left);
		freeTree(r->right);
		free(r->user);
		free(r);
	}
}

int loadUsers(FILE* fin, nodo_t** r) {
	int n = 0, res = 0, len = 0;
	char temp1[LUSER+LPWD+3], temp2[LUSER+LPWD+3], *firstseq = NULL, *scorri = NULL;
	user_t* tmp = NULL;
	scorri = fgets(temp1, LUSER+LPWD+3, fin);
	while (scorri != NULL) {
		len = strlen(temp1);
		firstseq = strchr(temp1, '\n');
		if (firstseq == NULL) {
			errno = EINVAL;
			return -1;
		}
		len = firstseq - temp1;
		strncpy(temp2, temp1, len);
		temp2[len] = '\0';
		tmp = stringToUser(temp2, LUSER+LPWD+3);
		if (tmp == NULL) return -1;
		res = addUser(r, tmp);
		if (res == -1) return -1;
		n++;
		scorri = fgets(temp1, LUSER+LPWD+3, fin);
	}
	return n;
}

int storeUsers(FILE* fout, nodo_t* r) {
	if (r != NULL) {
		int a = 0, b = 0;
		a = storeUsers(fout, r->left);
		if (r->user != NULL) fprintf(fout, "%s:%s\n", r->user->name, r->user->passwd);
		else {
			errno = EINVAL;
			return -1;
		}
		b = storeUsers(fout, r->right);
		if (a == -1 || b == -1) return -1;
		return 1+a+b;
	}
	else return 0;
}

/** 
 *  Cerca un utente nell'albero e restituisce il puntatore
 *  al nodo che lo contiene. La funzione è statica rispetto
 *  ad users.c, ed è stata inclusa in users.h solo per una corretta documentazione Doxygen
 * 
 *  \param r radice dell'albero
 *  \param u utente da cercare
 * 
 *  \retval x puntatore nodo che contiene l'utente
 *  \retval NULL se l'albero è vuoto o l'utente non è presente
 */

static nodo_t* searchUser(nodo_t* r, char* u) {
	if (r != NULL && u != NULL) {
		if (strcmp(r->user->name, u) == 0) return r;
		if (strcmp(r->user->name, u) > 0) return searchUser(r->left, u);
		if (strcmp(r->user->name, u) < 0) return searchUser(r->right, u);
	}
	return NULL;
}

status_t getUserStatus(nodo_t* r, char* u) {
	nodo_t* s = NULL;
	s = searchUser(r, u);
	if (s == NULL) return NOTREG;
	else return s->status;
}

int getUserChannel(nodo_t* r, char* u) {
	nodo_t* ch = NULL;
	ch = searchUser(r, u);
	if (ch == NULL) return NOTREG;
	else return ch->channel;
}

bool_t setUserStatus(nodo_t* r, char* u, status_t s) {
	nodo_t* tmp = NULL;
	tmp = searchUser(r, u);
	if (tmp == NULL) return FALSE;
	else {
		tmp->status = s;
		return TRUE;
	}
}

bool_t setUserChannel(nodo_t* r, char* u, int ch) {
	nodo_t* tmp = NULL;
	tmp = searchUser(r, u);
	if (tmp == NULL) return FALSE;
	else {
		tmp->channel = ch;
		return TRUE;
	}
}

bool_t isUser(nodo_t* r, char* u) {
	nodo_t* tmp = NULL;
	tmp = searchUser(r, u);
	if (tmp == NULL) return FALSE;
	else return TRUE;
}

char* getUserList(nodo_t* r, status_t st) {
	if (r != NULL) {
		char *left_string = NULL, *right_string = NULL, *s = NULL;
		int a=0, b=0, c=0;
		left_string = getUserList(r->left, st);
		if (left_string != NULL) a = strlen(left_string) + 1;
		right_string = getUserList(r->right, st);
		if (right_string != NULL) b = strlen(right_string) + 1;
		if (r->status == st) c = strlen(r->user->name) + 1 ;
		s = (char*)malloc((a+b+c)*sizeof(char));
		if (left_string == NULL && right_string == NULL) {
			if (r->status == st) strcpy(s,r->user->name);
			else {
				free(s);
				s = NULL;
			}
		}
		else if (left_string != NULL && right_string == NULL) {
			strcpy(s, left_string);
			if (r->status == st) {
				strcat(s, ":");
				strcat(s, r->user->name);
			}
		}
		else if (left_string == NULL && right_string != NULL) {
			if (r->status == st) {
				strcpy(s, r->user->name);
				strcat(s, ":");
				strcat(s, right_string);
			}
			else strcpy(s, right_string);
		}
		else if (left_string != NULL && right_string != NULL) {
			strcpy(s, left_string);
			if (r->status == st) {	
				strcat(s, ":");
				strcat(s, r->user->name);
			}
			strcat(s, ":");
			strcat(s, right_string);
		}
		if (left_string != NULL) free(left_string);
		if (right_string != NULL) free(right_string);
		return s;
	}
	else return NULL;
}
