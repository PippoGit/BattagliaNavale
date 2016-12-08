#ifndef NETUTIL_HEADER
#define NETUTIL_HEADER

#include "battle_foundation.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


int tcp_socket();
int tcp_connect(int socket, addr_t *a, const char* ip, const int p);
int tcp_send(int socket, const char *msg);
int tcp_recv(int socket, char* msg);
int tcp_server(int port);
int tcp_start_server(int socket, void (*func)(int*));

#endif
