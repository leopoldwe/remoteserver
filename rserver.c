/*
 * Author: Leopold Eckert
 * Filename: rserver.c
 * CS3411 Assignment #5
 *
 * Description: This file implements the server side of the remote procedure calls.
 */

#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include "rserver.h"

extern int errno;

/*
 * Function: main
 * Params: argc and argv, ignored
 * Return: The exit value
 * Description: Listens on a port for connections.
 */

int main(int argc, char** argv) {
	int savedErr;
	int cpid, rslt, listeningSocket;
	fd_set FD;
	int l;
	char host_id[80];
	struct sockaddr_in soc, soc2;
	int res;
	int addrlen;
	char buff[256];
	
	listeningSocket = socket(AF_INET,SOCK_STREAM,6);
	if (listeningSocket < 0) {
		perror("Socket");
		exit(-1);
	}
	soc.sin_family = AF_INET;
	soc.sin_addr.s_addr = INADDR_ANY;
	soc.sin_port = htons(6784);
	addrlen=sizeof(soc);
    
    l=1;
	res = setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&l, sizeof(l));
	if (res < 0) {
		perror("setsockopt");
		exit(-1);
	}
	
	res=bind(listeningSocket,(struct sockaddr *)&soc,addrlen);
	if (res < 0) {
		perror("Bind");
		exit(-1);
	}
	sprintf(buff, "Listening on port number: %d\n", ntohs(soc.sin_port));
	write(1, buff, strlen(buff));
	getsockname(listeningSocket,(struct sockaddr *)&soc,(socklen_t *)&addrlen);
	gethostname(host_id,80);

	l=1;
	res=setsockopt(listeningSocket,SOL_SOCKET,SO_REUSEADDR,(char *)&l,sizeof(l));
	if (res < 0) {
		perror("setsockopt");
		return -1;
	}

	res=listen(listeningSocket,7);
	if (res < 0) {
		perror("Listen");
		return -1;
	}
	
	while(1) {
		FD_ZERO(&FD);
		FD_SET(listeningSocket, &FD);
		
		rslt=select(listeningSocket + 1, &FD, 0, 0, 0);
		if (rslt < 0) {
			savedErr = errno;
			perror("Select");
			exit(-1);
		}
		
		if (FD_ISSET(listeningSocket, &FD)) {
			addrlen=sizeof(soc2);
			socketToClient = accept(listeningSocket, (struct sockaddr *)&soc2, (socklen_t *)&addrlen);
			if (res < 0) {
				perror("accept");
				continue;
			}
			cpid = fork();
			if (cpid < 0) {
				perror("fork");
				close(socketToClient);
				continue;
			}
			else if (cpid == 0) {
				serveConnection();
			}
			else {
				close(socketToClient);
				continue;
			}
		}
	}
	
}//END OF FUNCTION: main

/*
 * Function: serveConnection
 * Description: Called when a client connects and needs to be served.
 */

void serveConnection() {
	int res;
	char opcode = 0;
	while (1) {
		res = read(socketToClient, &opcode, 1);
		if (res == 0) exit(0);
		if (res < 0) {
			perror("read");
			exit(-1);
		}
		switch(opcode) {
			case open_call:
				res = rp_open();
				if (res == -1) {
					perror("write");
					exit(-1);
				}
				break;
			case read_call:
				res = rp_read();
				if (res == -1) {
					perror("write");
					exit(-1);
				}
				break;
			case write_call:
				res = rp_write();
				if (res == -1) {
					perror("write");
					exit(-1);
				}
				break;
			case close_call:
				res = rp_close();
				if (res == -1) {
					perror("write");
					exit(-1);
				}
				break;
			case seek_call:
				res = rp_lseek();
				if (res == -1) {
					perror("write");
					exit(-1);
				}
				break;
			case checksum_call:
				res = rp_checksum();
				if (res == -1) {
					perror("write");
					exit(-1);
				}
				break;
			default :
				printf("Illegal opcode %d\n", opcode);
				exit(-1);
		}
	}
}

/*
 * Function: rp_open
 * Return: The result of the write to the client socket
 * Description: Marshals the arguments for open from the socket, then sends the result
 *              to the client.
 */

int rp_open() {
	int length, flags, mode, ret, err;
	int i;
	char buff[1024];
	char *msg;
	char rsp[8];
	int tmpResult;
	
	length = 8;
	msg = malloc(length);
	if (msg <= 0) {
		err = errno;
		bcopy(&msg, rsp, 4);
		bcopy(&err, &(rsp[4]), 4);
		return write(socketToClient, rsp, 8);
	}
	i = 0;
	while (i < length) {
		tmpResult = read(socketToClient, &(msg[i]), length - i);
		if (tmpResult < 0) {
			err = errno;
			bcopy(&tmpResult, rsp, 4);
			bcopy(&err, &(rsp[4]), 4);
			return write(socketToClient, rsp, 8);
		}
		i += tmpResult;
	}
	i = 0;
	bcopy(&(msg[i]), &flags, 4);
	i += 4;
	bcopy(&(msg[i]), &mode, 4);
	i += 4;
	free(msg);
	i = 0;
	i += read(socketToClient, buff, sizeof(buff));
	if (i < 0) {
		err = errno;
		bcopy(&i, rsp, 4);
		bcopy(&err, &(rsp[4]), 4);
		return write(socketToClient, rsp, 8);
	}
	while (buff[i]) {
		tmpResult = read(socketToClient, &(buff[i]), sizeof(buff) - i);
		if (tmpResult < 0) {
			err = errno;
			bcopy(&tmpResult, rsp, 4);
			bcopy(&err, &(rsp[4]), 4);
			return write(socketToClient, rsp, 8);
		}
		i += tmpResult;
	}
	ret = open(buff, flags, mode);
	err = errno;
	i = 0;
	bcopy(&ret, &(rsp[i]), 4);
	i += 4;
	bcopy(&err, &(rsp[i]), 4);
	i += 4;
	return write(socketToClient, rsp, 8);
}//END OF FUNCTION: rp_open

/*
 * Function: rp_read
 * Return: The result of the write to the client socket
 * Description: Marshals the arguments for read from the socket, then sends the result
 *              to the client.
 */

int rp_read() {
	int length, fd, nbytes, ret, err;
	int i;
	char *msg;
	char *readIn;
	char *rsp;
	char failureResponse[8];
	int tmpResult;
	
	length = 4 + 4;
	msg = malloc(length);
	if (msg <= 0) {
		err = errno;
		bcopy(&msg, failureResponse, 4);
		bcopy(&err, &(failureResponse[4]), 4);
		return write(socketToClient, failureResponse, 8);
	}
	i = 0;
	while (i < length) {
		tmpResult = read(socketToClient, &(msg[i]), length - i);
		if (tmpResult < 0) {
			err = errno;
			bcopy(&tmpResult, failureResponse, 4);
			bcopy(&err, &(failureResponse[4]), 4);
			return write(socketToClient, failureResponse, 8);
		}
		i += tmpResult;
	}
	i = 0;
	bcopy(&(msg[i]), &fd, 4);
	i += 4;
	bcopy(&(msg[i]), &nbytes, 4);
	i += 4;
	free(msg);
	readIn = malloc(nbytes);
	i = 0;
	ret = write(fd, readIn, nbytes);
	err = errno;
	length = 4 + 4 + ret;
	rsp = malloc(length);
	i = 0;
	bcopy(&ret, &(rsp[i]), 4);
	i += 4;
	bcopy(&err, &(rsp[i]), 4);
	i += 4;
	bcopy(readIn, &(rsp[i]), ret);
	free(readIn);
	tmpResult = write(socketToClient, rsp, length);
	free(rsp);
	return tmpResult;
}//END OF FUNCTION: rp_read

/*
 * Function: rp_write
 * Return: The result of the write to the client socket
 * Description: Marshals the arguments for write from the socket, then sends the result
 *              to the client.
 */

int rp_write() {
	int length, fd, nbytes, ret, err;
	int i;
	char *msg;
	char *writeIn;
	char rsp[8];
	int tmpResult;
	
	length = 4 + 4;
	msg = malloc(length);
	if (msg <= 0) {
		err = errno;
		bcopy(&msg, rsp, 4);
		bcopy(&err, &(rsp[4]), 4);
		return write(socketToClient, rsp, 8);
	}
	i = 0;
	while (i < length) {
		tmpResult = read(socketToClient, &(msg[i]), length - i);
		if (tmpResult < 0) {
			err = errno;
			bcopy(&tmpResult, rsp, 4);
			bcopy(&err, &(rsp[4]), 4);
			return write(socketToClient, rsp, 8);
		}
		i += tmpResult;
	}
	i = 0;
	bcopy(&(msg[i]), &fd, 4);
	i += 4;
	bcopy(&(msg[i]), &nbytes, 4);
	i += 4;
	free(msg);
	writeIn = malloc(nbytes);
	i = 0;
	i += read(socketToClient, writeIn, nbytes);
	if (i < 0) {
		err = errno;
		bcopy(&tmpResult, rsp, 4);
		bcopy(&err, &(rsp[4]), 4);
		return write(socketToClient, rsp, 8);
	}
	while (i < nbytes) {
		tmpResult = read(socketToClient, &(writeIn[i]), nbytes - i);
		if (tmpResult < 0) {
			err = errno;
			bcopy(&tmpResult, rsp, 4);
			bcopy(&err, &(rsp[4]), 4);
			return write(socketToClient, rsp, 8);
		}
		i += tmpResult;
	}
	ret = write(fd, writeIn, nbytes);
	err = errno;
	free(writeIn);
	i = 0;
	bcopy(&ret, &(rsp[i]), 4);
	i += 4;
	bcopy(&err, &(rsp[i]), 4);
	i += 4;
	return write(socketToClient, rsp, 8);
}//END OF FUNCTION: rp_write

/*
 * Function: rp_lseek
 * Return: The result of the write to the client socket
 * Description: Marshals the arguments for lseek from the socket, then sends the result
 *              to the client.
 */

int rp_lseek() {
	int length, fd, offset, whence, ret, err;
	int i;
	char *msg;
	char rsp[8];
	int tmpResult;
	
	length = 4 + 4 + 4;
	msg = malloc(length);
	if (msg <= 0) {
		err = errno;
		bcopy(&msg, rsp, 4);
		bcopy(&err, &(rsp[4]), 4);
		return write(socketToClient, rsp, 8);
	}
	i = 0;
	while (i < length) {
		tmpResult = read(socketToClient, &(msg[i]), length - i);
		if (tmpResult < 0) {
			err = errno;
			bcopy(&tmpResult, rsp, 4);
			bcopy(&err, &(rsp[4]), 4);
			return write(socketToClient, rsp, 8);
		}
		i += tmpResult;
	}
	i = 0;
	bcopy(&(msg[i]), &fd, 4);
	i += 4;
	bcopy(&(msg[i]), &offset, 4);
	i += 4;
	bcopy(&(msg[i]), &whence, 4);
	i += 4;
	free(msg);
	ret = lseek(fd, offset, whence);
	err = errno;
	i = 0;
	bcopy(&ret, &(rsp[i]), 4);
	i += 4;
	bcopy(&err, &(rsp[i]), 4);
	i += 4;
	return write(socketToClient, rsp, 8);
}//END OF FUNCTION: rp_lseek

/*
 * Function: rp_close
 * Return: The result of the write to the client socket
 * Description: Marshals the arguments for close from the socket, then sends the result
 *              to the client.
 */

int rp_close() {
	int length, fd, ret, err;
	int i;
	char *msg;
	char rsp[8];
	int tmpResult;
	
	length = 4;
	msg = malloc(length);
	if (msg <= 0) {
		err = errno;
		bcopy(&msg, rsp, 4);
		bcopy(&err, &(rsp[4]), 4);
		return write(socketToClient, rsp, 8);
	}
	i = 0;
	while (i < length) {
		tmpResult = read(socketToClient, &(msg[i]), length - i);
		if (tmpResult < 0) {
			err = errno;
			bcopy(&tmpResult, rsp, 4);
			bcopy(&err, &(rsp[4]), 4);
			return write(socketToClient, rsp, 8);
		}
		i += tmpResult;
	}
	i = 0;
	bcopy(&(msg[i]), &fd, 4);
	i += 4;
	free(msg);
	ret = close(fd);
	err = errno;
	i = 0;
	bcopy(&ret, &(rsp[i]), 4);
	i += 4;
	bcopy(&err, &(rsp[i]), 4);
	i += 4;
	return write(socketToClient, rsp, 8);
}//END OF FUNCTION: rp_close

/*
 * Function: rp_checksum
 * Return: The result of the write to the client socket
 * Description: Marshals the arguments for checksum from the socket, then sends the result
 *              to the client.
 */

int rp_checksum() {
	int length;
	int i;
	int pos;
	int fd;
	int err;
	int result = 0;
	char *msg;
	char readIn;
	char rsp[9];
	int tmpResult;
	
	length = 4; //32-bit int
	msg = malloc(length);
	if (msg <= 0) {
		err = errno;
		bcopy(&msg, rsp, 4);
		bcopy(&err, &(rsp[4]), 4);
		bcopy(&result, &(rsp[8]), 1);
		return write(socketToClient, rsp, 9);
	}
	
	i = 0;
	while (i < length) {
		tmpResult = read(socketToClient, &(msg[i]), length - i);
		if (tmpResult < 0) {
			err = errno;
			bcopy(&tmpResult, rsp, 4);
			bcopy(&err, &(rsp[4]), 4);
			bcopy(&result, &(rsp[8]), 1);
			return write(socketToClient, rsp, 9);
		}
		i += tmpResult;
	}
	i = 0;
	bcopy(&(msg[i]), &fd, 4);
	free(msg);
	//Get current position:
	pos = lseek(fd, 0, SEEK_CUR);
	if (pos < 0) {
		err = errno;
		bcopy(&pos, rsp, 4);
		bcopy(&err, &(rsp[4]), 4);
		bcopy(&result, &(rsp[8]), 1);
		return write(socketToClient, rsp, 9);
	}
	//Seek to beginning
	tmpResult = lseek(fd, 0, SEEK_SET);
	if (tmpResult < 0) {
		err = errno;
		bcopy(&tmpResult, rsp, 4);
		bcopy(&err, &(rsp[4]), 4);
		bcopy(&result, &(rsp[8]), 1);
		return write(socketToClient, rsp, 9);
	}
	tmpResult = read(fd, &readIn, sizeof(char));
	if (tmpResult < 0) {
		err = errno;
		bcopy(&tmpResult, rsp, 4);
		bcopy(&err, &(rsp[4]), 4);
		bcopy(&result, &(rsp[8]), 1);
		return write(socketToClient, rsp, 9);
	}
	while (tmpResult) {
		result = result ^ readIn;
		tmpResult = read(fd, &readIn, sizeof(char));
		if (tmpResult < 0) {
			err = errno;
			bcopy(&tmpResult, rsp, 4);
			bcopy(&err, &(rsp[4]), 4);
			bcopy(&result, &(rsp[8]), 1);
			return write(socketToClient, rsp, 9);
		}
	}
	tmpResult = lseek(fd, pos, SEEK_SET);
	if (tmpResult < 0) {
		err = errno;
		bcopy(&tmpResult, rsp, 4);
		bcopy(&err, &(rsp[4]), 4);
		bcopy(&result, &(rsp[8]), 1);
		return write(socketToClient, rsp, 9);
	}
	err = errno;
	i = 0;
	bcopy(&tmpResult, &(rsp[i]), 4);
	i += 4;
	bcopy(&err, &(rsp[i]), 4);
	i += 4;
	bcopy(&result, &(rsp[i]), 1);
	return write(socketToClient, rsp, 9);
}//END OF FUNCTION: rp_checksum
//END OF FILE: server.c