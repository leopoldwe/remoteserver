/*
 * Author: Leopold Eckert
 * Filename: client.h
 * CS3411 Assignment #5
 *
 * Description: Header file for client.c
 */

#define open_call 1
#define close_call 2
#define read_call 3
#define write_call 4
#define seek_call 5
#define checksum_call 6

int socketToServer;

int rp_open(char *path, int flags, int mode);

int rp_read(int fd, char *buff, int nbyte);

int rp_write(int fd, char *buff, int nbyte);

int rp_lseek(int fd, int offset, int whence);

int rp_close(int fd);

int rp_checksum(int fd);