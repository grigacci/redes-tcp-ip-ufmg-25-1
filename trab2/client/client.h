#ifndef CLIENT_H
#define CLIENT_H

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include "timer.h"

#define SERVER_PORT 54321
#define MAX_LINE 256
#define MAX_MESSSAGES 100000
#define MESSAGE_SIZE 8
#define MAX_TIMEOUT 2000000 // 2 seconds in microseconds


int main(int argc, char* argv[]);

int tcp_socket(int* s, struct sockaddr_in sin);

int tcp_send(int s, char* buf, int len);

int tcp_receive(int s, char* buf, int buf_size);

void generate_random_string(char *s, const int len);

int udp_receive(int s, struct sockaddr_in sin, char* buf, int buf_size);


#endif