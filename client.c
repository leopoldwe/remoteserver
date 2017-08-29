/*
 * Author: Leopold Eckert
 * Filename: client.c
 * CS3411 Assignment #5
 *
 * Description: This file implements the client side of the remote procedure calls.
 */

#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "client.h"

/*
 * Function: rp_open
 * Parameters:
 *   path: the path to the file that should be opened.
 *   flags: the flags that would be passed to open.
 *   mode: the mode the file is in.
 * Return: A file descriptor.
 * Description: Performs a remote procedure call similar to open.
 */

int rp_open(char *path, int flags, int mode) {
	int length;
	char *msg;
	char rsp[8];
	int i = 0;
	int result;
	int error;
	
	length = 1 + 4 + 4 + strlen(path) + 1;//opcode + flags + mode + path
	
	msg = malloc(length);
	if (msg <= 0) {
		perror("malloc");
		return -1;
	}
	
	msg[0] = (char) open_call;
	i++;
	bcopy(&flags, &(msg[i]), sizeof(flags));
	i += sizeof(flags);
	bcopy(&mode, &(msg[i]), sizeof(mode));
	i += sizeof(mode);
	strcpy(&(msg[i]), path);
	i += (strlen(path) + 1);
	write(socketToServer, msg, length);
	i = 0;
	free(msg);
	while(i < 8) {
		i += read(socketToServer, &(rsp[i]), (8 - i));
	}
	bcopy(&result, rsp, 4);
	bcopy(&error, &(rsp[4]), 4);
	errno = error;
	return result;
}//END OF FUNCTION: rp_open

/*
 * Function: rp_read
 * Parameters:
 *   fd: a file descriptor
 *   buff: a buffer to read into
 *   nbyte: the number of bytes to read.
 * Return: the number of bytes read.
 * Description: Performs a remote procedure call similar to read.
 */

int rp_read(int fd, char *buff, int nbyte) {
	int length;
	char *msg;
	char rsp[8];
	int i = 0;
	int result;
	int error;
	
	length = 1 + 4 + 4;
	msg = malloc(length);
	if (msg <= 0) {
		perror("malloc");
		return -1;
	}
	
	msg[0] = (char) read_call;
	i++;
	bcopy(&fd, &(msg[i]), 4);
	i += 4;
	bcopy(&nbyte, &(msg[i]), 4);
	i += 4;
	write(socketToServer, msg, length);
	free(msg);
	i = 0;
	while (i < 8) {
		i += read(socketToServer, &(rsp[i]), (8 - i));
	}
	bcopy(&result, rsp, 4);
	bcopy(&error, &(rsp[4]), 4);
	i = 0;
	while (i < result) {
		i += read(socketToServer, &(rsp[i]), result - i);
	}
	errno = error;
	return result;
} //END OF FUNCTION: rp_read

/*
 * Function: rp_write
 * Parameters:
 *   fd: a file descriptor.
 *   buff: a buffer to write from.
 *   nbyte: the number of bytes to write.
 * Return: the number of bytes written.
 * Description: Performs a remote procedure call similar to write.
 */

int rp_write(int fd, char *buff, int nbyte) {
	int length;
	char *msg;
	char rsp[8];
	int i = 0;
	int result;
	int error;
	
	length = 1 + 4 + 4 + nbyte;
	
	msg = malloc(length);
	
	if (msg <= 0) {
		perror("malloc");
		return -1;
	}
	
	msg[0] = (char) write_call;
	i++;
	
	bcopy(&fd, &(msg[i]), 4);
	i += 4;
	bcopy(&nbyte, &(msg[i]), 4);
	i += 4;
	bcopy(buff, &(msg[i]), nbyte);
	i += nbyte;
	write(socketToServer, msg, length);
	free(msg);
	i = 0;
	while(i < 8) {
		i += read(socketToServer, &(rsp[i]), (8 - i));
	}
	bcopy(&result, rsp, 4);
	bcopy(&error, &(rsp[4]), 4);
	errno = error;
	return result;
}//END OF FUNCTION: rp_write

/*
 * Function: rp_lseek
 * Parameters:
 *   fd: a file descriptor
 *   offset: the offset of bytes to seek
 *   whence: the same whence as lseek
 * Return: the offset from the beginning of the file.
 * Description: Performs a remote procedure call similar to lseek.
 */

int rp_lseek(int fd, int offset, int whence) {
	int length;
	char *msg;
	char rsp[8];
	int i = 0;
	int result;
	int error;
	
	length = 1 + 4 + 4 + 4;
	
	msg = malloc(length);
	
	if (msg <= 0) {
		perror("malloc");
		return -1;
	}
	msg[0] = (char) seek_call;
	i++;
	
	bcopy(&fd, &(msg[i]), 4);
	i += 4;
	bcopy(&offset, &(msg[i]), 4);
	i += 4;
	bcopy(&whence, &(msg[i]), 4);
	i += 4;
	write(socketToServer, msg, length);
	free(msg);
	i = 0;
	while(i < 8) {
		i += read(socketToServer, &(rsp[i]), (8 - i));
	}
	bcopy(&result, rsp, 4);
	bcopy(&error, &(rsp[4]), 4);
	errno = error;
	return result;
}//END OF FUNCTION: rp_lseek

/*
 * Function: rp_close
 * Parameters:
 *   fd: a file descriptor
 * Return: 0 on success, -1 on failure.
 * Description: Performs a remote procedure call similar to close.
 */

int rp_close(int fd) {
	int length;
	char *msg;
	char rsp[8];
	int i = 0;
	int result;
	int error;
	
	length = 1 + 4;
	
	msg = malloc(length);
	
	if (msg <= 0) {
		perror("malloc");
		return -1;
	}
	
	msg[0] = (char) close_call;
	i++;
	bcopy(&fd, &(msg[i]), 4);
	i += 4;
	write(socketToServer, msg, length);
	free(msg);
	i = 0;
	while(i < 8) {
		i += read(socketToServer, &(rsp[i]), (8 - i));
	}
	bcopy(&result, rsp, 4);
	bcopy(&error, &(rsp[4]), 4);
	errno = error;
	return result;
}//END OF FUNCTION: rp_close

/*
 * Function: rp_checksum
 * Parameters:
 *   fd: a file descriptor
 * Return: the checksum of the file
 * Description: Gets a checksum of a file.
 */

int rp_checksum(int fd) {
	int length;
	char *msg;
	char rsp[9];
	int i = 0;
	int result;
	int error;
	unsigned char ret;
	
	length = 1 + 4;
	
	msg = malloc(length);
	
	if (msg <= 0) {
		perror("malloc");
		return -1;
	}
	
	msg[0] = (char) checksum_call;
	i++;
	bcopy(&fd, &(msg[i]), 4);
	i += 4;
	write(socketToServer, msg, length);
	free(msg);
	i = 0;
	while(i < 9) {
		i += read(socketToServer, &(rsp[i]), (9 - i));
	}
	bcopy(&result, rsp, 4);
	bcopy(&error, &(rsp[4]), 4);
	bcopy(&ret, &(rsp[8]), 1);
	errno = error;
	return (unsigned int) ret;
}//END OF FUNCTION: rp_checksum

//END OF FILE: client.c