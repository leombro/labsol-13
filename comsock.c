/** 
 *  \file comsock.c
 *  \author Orlando Leombruni
 * 
 *  \brief Implementazione libreria di comunicazione su socket AF_UNIX
 *  
 * Si dichiara che il contenuto di questo file è in ogni sua parte opera originale dell'autore.
 * 
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include "comsock.h"

/** Converte un intero senza segno in una rappresentazione di 4 byte separati da memorizzare in un array di caratteri
 * 
 * \param n intero da convertire
 * \param container array di caratteri che conterrà la rappresentazione
 * 
 */

static void intToChar (unsigned int n, char* container) 
{
	container[3] = n & 0xFF;
	container[2] = (n >> 8) & 0xFF;
	container[1] = (n >> 16) & 0xFF;
	container[0] = (n >> 24) & 0xFF;
}

/** Converte una sequenza di 4 byte, memorizzata in un array di caratteri, in un intero senza segno
 * 
 * \param buffer array di caratteri contenenti la rappresentazione del numero
 * 
 * \retval n numero in formato \c unsigned \c int
 *
 */

static unsigned int charToInt(char buffer[]) 
{
	unsigned int n,
		n1 = buffer[3],
		n2 = buffer[2],
		n3 = buffer[1],
		n4 = buffer[0];
		
	n = n1 + (n2 << 8) + (n3 << 16) + (n4 << 24);
	return n;
}

int createServerChannel(char* path) 
{
	int s = 0, temperrno;
	struct sockaddr_un mysocket;
	
	if (path == NULL) {
		errno = EINVAL;
		return -1;
	}
	
	if (strlen(path) == 0) {
		errno = EINVAL;
		return -1;
	}
	
	if (strlen(path) > UNIX_PATH_MAX) {
		errno = E2BIG;
		return -1;
	}
	mysocket.sun_family = AF_UNIX;
	strncpy(mysocket.sun_path, path, UNIX_PATH_MAX);
	
	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)  
		return -1;
		
	if (bind(s, (struct sockaddr*) &mysocket, sizeof(mysocket)) == -1) {
		temperrno = errno;
		close(s);
		errno = temperrno;
		return -1;
	}
	
	if (listen(s, SOMAXCONN) == -1) {
		temperrno = errno;
		close(s);
		unlink(path);
		errno = temperrno;
		return -1;
	}
		
	return s;
}

int closeServerChannel(char* path, int s)
{
	
	if (close(s) == -1)
		return -1;
	
	if (unlink(path) == -1)
		return -1;
	
	return 0;
}

int acceptConnection(int s)
{
	int c;
	
	if ((c = accept(s, NULL, 0)) == -1)
		return -1;
	
	return c;
}

int receiveMessage(int sc, message_t * msg)
{
	unsigned int r_type, r_size, r_buff, lung;
	char t, buflen[4];
	char* text = NULL;
	
	switch (r_type = read(sc, &t, 1)) {
		case -1:
			return -1;
		case 0:
			errno = ENOTCONN;
			return -1;
		case 1:
			break;
		default:
			return -1;
	}
	
	switch (r_size = read(sc, buflen, 4)) {
		case -1:
			return -1;
		case 0:
			errno = ENOTCONN;
			return -1;
		case 4:
			break;
		default:
			return -1;
	}
	
	r_buff = charToInt(buflen);
	
	if (r_buff > 0) {
		text = (char*)malloc(r_buff*sizeof(char));
		if (text == NULL) return -1;
		
		switch (lung = read(sc, text, r_buff)) {
			case -1:
				return -1;
			case 0:
				errno = ENOTCONN;
				return -1;
			default:
				break;
		}
		msg->type = t;
		msg->length = lung;
		msg->buffer = text;
	}
	else {
		msg->type = t;
		msg->length = 0;
		msg->buffer = NULL;
	}
	
	return r_buff;
}

int sendMessage(int sc, message_t *msg)
{	
	int i, n, size;
	char *receiver = NULL, intcontainer[4];
	if ((receiver = (char*)malloc((5+(msg->length))*sizeof(char))) == NULL)
		return -1;
	intToChar(msg->length, intcontainer);
	
	receiver[0] = msg->type;
	for (i = 0; i<4; i++) receiver[i+1] = intcontainer[i];
	strncpy((receiver+5), msg->buffer, msg->length);
	size = msg->length + 5;
	
	switch(n = write(sc, receiver, size)) {
		case -1:
			free(receiver);
			return -1;
		case 0:
			free(receiver);
			errno = ENOTCONN;
			return -1;
		default:
			break;
	}
	
	free(receiver);
	return n;
}

int openConnection(char* path, int ntrial, int k)
{
	int fd, times = 0, connected = 0, status, temp_errno;
	struct sockaddr_un client_socket;
	
	if (ntrial > MAXTRIAL || k > MAXSEC || ntrial < 0 || k <= 0) {
		errno = EINVAL;
		return -1;
	}
	
	if (strlen(path) > UNIX_PATH_MAX) {
		errno = E2BIG;
		return -1;
	}
	client_socket.sun_family = AF_UNIX;
	strncpy(client_socket.sun_path, path, UNIX_PATH_MAX);
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	
	while (!connected && times <= ntrial) {
		status = connect(fd, (struct sockaddr*) &client_socket, sizeof(client_socket));
		if (status == -1) {
			temp_errno = errno;
			times++;
			sleep(k);
		}
		else connected = 1;
	}
	
	if (times > ntrial) {
		if (close(fd) == -1) return -1;
		if (unlink(path) == -1) return -1;
		errno = temp_errno;
		return -1;
	}
	
	return fd;
}

int closeConnection(int s) 
{
	if (close(s) == -1)
		return -1;
	
	return 0;
}	

int createMessage(message_t* msg, char m_type, char* m_buf)
{
	msg->type = m_type;
	if (m_buf == NULL) {
		msg->length = 0;
		msg->buffer = NULL;
	}
	else {
		msg->length = strlen(m_buf)+1;
		if ((msg->buffer = (char*)malloc(msg->length*sizeof(char))) == NULL) return -1;
		strcpy(msg->buffer, m_buf);
	}
	return 0;
}
