CC       = gcc
EXE_FILE = rserver rclient1 rclient2
OBJ_FILE = server.o client.o rclient1.o rclient2.o

all: ${EXE_FILE}

rserver: rserver.o
	${CC} -Wall -g rserver.o -o rserver

rclient1: rclient1.o client.o
	${CC} -Wall -g client.o rclient1.o -o rclient1

rclient2: rclient2.o client.o
	${CC} -Wall -g client.o rclient2.o -o rclient2

rserver.o: rserver.c rserver.h
	${CC} -c -g -Wall rserver.c

rclient1.o: rclient1.c client.h client.c
	${CC} -c -g -Wall rclient1.c

rclient2.o: rclient2.c client.h client.c
	${CC} -c -g -Wall rclient2.c

client.o: client.c client.h
	${CC} -c -g -Wall client.c				

clean:
	rm -f ${OBJ_FILE} ${EXE_FILE}
