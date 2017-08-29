/*
 * Author: Leopold Eckert
 * Filename: rserver.h
 * CS3411 Assignment #5
 *
 * Description: Header file for server.c
 */

#define open_call 1
#define close_call 2
#define read_call 3
#define write_call 4
#define seek_call 5
#define checksum_call 6

int socketToClient;

void serveConnection();

int rp_open();

int rp_read();

int rp_write();

int rp_lseek();

int rp_close();

int rp_checksum();