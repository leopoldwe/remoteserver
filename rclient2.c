/*
 * Author: Leopold Eckert
 * Filename: rclient2.c
 * CS3411 Assignment #5
 *
 * Description: This file implements an example client to test the server's functions
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
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "client.h"

int entry(int argc, char** argv);

int main(int argc, char** argv) {
	int l;
	int port;
	int result;
	struct sockaddr_in sin;
	struct hostent *ph;
	char *arg0;
	char *arg1;
	char *arg2 = 0;
	char *newArgs[3];
	char buff[256];
	if (argc < 4) {
		printf("Usage: ./rclient1 <hostname> <portnumber> <file>");
		exit(-1);
	}
	port = atoi(argv[2]);
	
	ph = gethostbyname(argv[1]);
	if (ph == 0) {
		perror("gethostbyname");
		exit(-1);
	}
	sin.sin_family = ph->h_addrtype;
	bcopy (ph->h_addr, (char *) &sin.sin_addr, ph->h_length);
	sin.sin_port = htons(port);
	socketToServer = socket(AF_INET,SOCK_STREAM, 0);
	if (socketToServer < 0) {
		perror("socket");
		exit(-1);
	}
	//O_F=fcntl(socketToServer,F_GETFL);
	//fcntl(socketToServer,F_SETFL,O_NDELAY | O_F);
	l=65535;
	setsockopt(socketToServer,SOL_SOCKET,SO_RCVBUF,(char *)&l,sizeof(l));
	l=65535;
	setsockopt(socketToServer,SOL_SOCKET,SO_SNDBUF,(char *)&l,sizeof(l));
	connect (socketToServer, (struct sockaddr *) &sin, sizeof (sin));
	if (errno == EINPROGRESS) 
         {
            strcpy(buff,"Trying ");
            strcat(buff,argv[1]);
            printf("%s ...",buff);
            fflush(stdout);
             
         }
	printf("Connected to server!\n");
	printf("%d\n", socketToServer);
	arg0 = argv[0];
	arg1 = argv[3];
	newArgs[0] = arg0;
	newArgs[1] = arg1;
	newArgs[2] = arg2;
	result = entry(argc - 2, newArgs);
	exit(result);
}

int entry(int argc, char** argv) {
	int inputfd;
	int outputfd;
	int result, result2;
	char buff[256];
	printf("About to open local file!\n");
	outputfd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0666);
	if (outputfd < 0) {
		perror("open");
		return -1;
	}
	printf("Opened local file!\n");
	printf("About to open remote file!\n");
	inputfd = rp_open(argv[1], O_RDONLY, 0666);
	if (inputfd < 0) {
		perror("open");
		return -1;
	}
	printf("remote fd: %d", inputfd);
	result = rp_lseek(inputfd, 10, SEEK_SET);
	if (result < 0) {
		perror("lseek");
		return -1;
	}
	result = rp_read(inputfd, buff, sizeof(buff));
	if (result < 0) {
		perror("read");
		return -1;
	}
	while (result > 0) {
		result2 = rp_write(outputfd, buff, result);
		if (result2 < 0) {
			perror("write");
			return -1;
		}
		result = read(inputfd, buff, sizeof(buff));
	}
	if (result < 0) {
		perror("read");
		return -1;
	}
	result = close(inputfd);
	if (result < 0) {
		perror("close");
		return -1;
	}
	result2 = rp_close(outputfd);
	if (result2 < 0) {
		perror("close");
		return -1;
	}
	return 0;
}