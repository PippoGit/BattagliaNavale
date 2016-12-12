#ifndef NETUTIL_HEADER
#define NETUTIL_HEADER

#include "battle_foundation.h"

int tcp_socket();
int tcp_connect(int socket, addr_t *a, const char* ip, const int p);
int tcp_send(int socket, const char *msg);
int tcp_recv(int socket, char* msg);
int tcp_server(int port);
int tcp_start_server(int socket, void (*func)(int*));

int udp_connect(addr_t *a, char *ip, int udp_port, int my_port);
int udp_socket();
int udp_recv(int socket, char*msg);
int udp_send(int socket, const char*msg);
int udp_recv_timeout(int socket, char*msg);

void set_timeout(int sock);
void reset_timeout(int sock);

#endif
