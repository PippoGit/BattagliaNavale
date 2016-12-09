#ifndef FOUNDATION_HEADER
#define FOUNDATION_HEADER

#include "battle_const.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>


enum connection_status {PENDING, CONN_ESTABLISHED, TIMEOUT};
enum map_tile {WATER, SHIP};
enum player_status {FREE, OCCUPIED};
enum cmd {HELP, WHO, CONNECT, QUIT, GHELP, SHOT, DISCONNECT, SHOW, REQ_FROM_SOCKET};
enum msg_type {HELLO, LIST, PLAY, ATT, SURRENDER, BYE, ERROR, SET_OCCUPIED, SET_FREE};
enum error_type {PLAY_WITH_YOURSELF, PLAYER_OCCUPIED, PLAYER_NOT_EXISTS};
enum prg_state {MENU, GAME};

typedef struct sockaddr_in addr_t;

typedef struct player
{
  char name_[MAX_USERNAME_LEN];
  int udp_port_;

  addr_t address_;
  int socket_;
  enum player_status status_;
} player_t;

typedef struct pvp_connection {
  int pvp_socket_;

  addr_t opponent_address_;
  enum player_status pl_stat_;
} pvp_connection_t;

typedef struct game {
  pvp_connection_t opponent_;
  enum connection_status conn_stat_;
} game_t;

typedef struct srv_connection {
  int srv_socket_;
  addr_t address_;
} srv_connection_t;

#endif
