/** 
 *  \file bris.c
 *  \author Orlando Leombruni
 * 
 *  \brief Implementazione di funzioni su carte.
 *  
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera originale dell'autore.
 * 
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "bris.h"
 
void cardToString(char* s,carta_t *c) {
	switch (c->val) {
		case ASSO: 
			s[0] = 'A';
			break;
		case DUE:
			s[0] = '2';
			break;
		case TRE:
			s[0] = '3';
			break;
		case QUATTRO:
			s[0] = '4';
			break;
		case CINQUE:
			s[0] = '5';
			break;
		case SEI:
			s[0] = '6';
			break;
		case SETTE:
			s[0] = '7';
			break;
		case FANTE:
			s[0] = 'J';
			break;
		case DONNA:
			s[0] = 'Q';
			break;
		case RE:
			s[0] = 'K';
			break;
		default:
			s[0] = 'U';
			break;
	}
	switch (c->seme){
		case CUORI:
			s[1] = 'C';
			break;
		case QUADRI:
			s[1] = 'Q';
			break;
		case FIORI:
			s[1] = 'F';
			break;
		case PICCHE:
			s[1] = 'P';
			break;
		default:
			s[1] = 'U';
			break;
	}
	s[2] = '\0';
}

void printCard(FILE* fpd,carta_t *c) {
	char dest[3];
	cardToString(dest, c);
	fprintf(fpd, "%s", dest);
}

carta_t* stringToCard(char* str) {
	carta_t* c = NULL;
	c = (carta_t*)malloc(sizeof(carta_t));
	if (c == NULL) return NULL;
	else {
		switch(str[0]) {
			case 'A':
				c->val = ASSO;
				break;
			case '2':
				c->val = DUE;
				break;
			case '3':
				c->val = TRE;
				break;
			case '4':
				c->val = QUATTRO;
				break;
			case '5':
				c->val = CINQUE;
				break;
			case '6':
				c->val = SEI;
				break;
			case '7':
				c->val = SETTE;
				break;
			case 'J':
				c->val = FANTE;
				break;
			case 'Q':
				c->val = DONNA;
				break;
			case 'K':
				c->val = RE;
				break;
			default:
				free(c);
				errno = EINVAL;
				return NULL;
		}
		switch (str[1]) {
			case 'C':
				c->seme = CUORI;
				break;
			case 'Q':
				c->seme = QUADRI;
				break;
			case 'F':
				c->seme = FIORI;
				break;
			case 'P':
				c->seme = PICCHE;
				break;
			default:
				errno = EINVAL;
				return NULL;
		}
	}
	return c;
}

carta_t* getCard(mazzo_t* m) {
	if (m->next == NCARTE) {
		errno = 0;
		return NULL;
	}
	else {
		carta_t* pc = NULL;
		pc = (carta_t*)malloc(sizeof(carta_t));
		if (pc == NULL) return NULL;
		else {
			pc->val = m->carte[m->next].val;
			pc->seme = m->carte[m->next].seme;
			m->next++;
		}
		return pc;
	}
}

void printMazzo(FILE* fpd, mazzo_t *pm) {
	int i = 0;
	for (i=0; i<NCARTE; i++) {
		fprintf(fpd, "%2d ", i+1);
		printCard(fpd, &(pm->carte[i]));
		fprintf(fpd, "\n");
	}
}

void freeMazzo(mazzo_t *pm) {
	if (pm != NULL)	free(pm);
}

/** Date due carte dello stesso seme, decreta il vincitore del confronto.
 * 	
 * \param ca, cb carte da confrontare
 * \retval caWins booleano che indica se ca vince su cb
 */

static bool_t sameSeedCompare(carta_t* ca, carta_t* cb) {
	bool_t caWins = FALSE;
	if (ca->val == ASSO) caWins = TRUE;
	if (ca->val == TRE && cb->val != ASSO) caWins = TRUE;
	if (ca->val > cb->val && cb->val != ASSO && cb->val != TRE) caWins = TRUE;
	return caWins;	
}

bool_t compareCard(semi_t briscola, carta_t* ca, carta_t* cb) {
	if (ca->seme == briscola) {
		if (cb->seme == briscola) return sameSeedCompare(ca, cb);
		else return TRUE;
	}
	else {
		if (ca->seme == cb->seme) return sameSeedCompare(ca, cb);
		else if (cb->seme != briscola) return TRUE;
	}
	return FALSE;
}

int computePoints(carta_t** c, int n) {
	if (n < 0 || n > NCARTE || c == NULL) {
		 errno = EINVAL;
		 return -1;
	}
	else {
		int i = 0, np = 0;
		for (i = 0; i<n; i++) {
			switch (c[i]->val) {
				case ASSO:
					np += 11;
					break;
				case TRE:
					np += 10;
					break;
				case RE:
					np += 4;
					break;
				case DONNA:
					np += 3;
					break;
				case FANTE:
					np += 2;
					break;
				case DUE:
					np += 0;
					break;
				case QUATTRO:
					np += 0;
					break;
				case CINQUE:
					np += 0;
					break;
				case SEI:
					np += 0;
					break;
				case SETTE:
					np += 0;
					break;
				default:
					errno = EINVAL;
					return -1;
				}
		}
		return np;
	}
}

bool_t isInHand(carta_t* card, carta_t** hand) 
{
	int i;
	for (i = 0; i < 3; i++) {
		if (hand[i] != NULL) 
			if (card->val == hand[i]->val) 
				if (card->seme == hand[i]->seme) return TRUE;
	}
	return FALSE;
}

int exchangeHands(carta_t* hand1[], carta_t* hand2[]) {
	carta_t* temp[3];
	int i;
	for (i = 0; i < 3; i++) {
		if (hand1[i] == NULL) {
			hand1[i] = hand2[i];
			hand2[i] = NULL;
		}
		else if (hand2[i] == NULL) {
			hand2[i] = hand1[i];
			hand1[i] = NULL;
		}
		else {
			if ((temp[i] = (carta_t*)malloc(sizeof(carta_t))) == NULL) return -1;
			temp[i]->val = hand1[i]->val;
			temp[i]->seme = hand1[i]->seme;
			hand1[i]->val = hand2[i]->val;
			hand1[i]->seme = hand2[i]->seme;
			hand2[i]->val = temp[i]->val;
			hand2[i]->seme = temp[i]->seme;
			free(temp[i]);
		}
	}
	return 0;
}

void replace(carta_t* hand[], carta_t* new, carta_t* old)
{
	int i;
	for (i = 0; i < 3; i++) {
		if (hand[i] != NULL) {
			if ((hand[i]->val == old->val) && (hand[i]->seme == old->seme)) {
				if (new != NULL) {
					hand[i]->val = new->val;
					hand[i]->seme = new->seme;
				}
				else {
					free(hand[i]);
					hand[i] = NULL;
				}
			}
		}
	}
	free(old);
}

char semeToChar(semi_t seme) {
	switch (seme) {
		case CUORI:
			return 'C';
		case QUADRI:
			return 'Q';
		case FIORI:
			return 'F';
		case PICCHE:
			return 'P';
	}
	return 'N';
}

bool_t checkIfFinish (carta_t* first[], carta_t* second[])
{
	bool_t result = TRUE;
	int i;
	for (i = 0; i < 3; i++) {
		if (first[i] != NULL || second[i] != NULL) result = FALSE;
	}
	return result;
}
