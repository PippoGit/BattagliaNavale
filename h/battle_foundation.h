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
#include <string.h>

#include <stdlib.h>

#include <poll.h>

enum connection_status {PENDING, CONN_ESTABLISHED, TIMEOUT};
enum map_tile {WATER, SHIP, HIT_SHIP, MISSED_SHIP};
enum player_status {FREE, OCCUPIED};
enum cmd {HELP, WHO, CONNECT, QUIT, GHELP, SHOT, DISCONNECT, SHOW, REQ_FROM_SOCKET, INPUT_TIMEOUT};
enum msg_type {HELLO, LIST, PLAY, ATK, HIT, MISS, I_LOST, SURRENDER, BYE, ERROR, SET_OCCUPIED, SET_FREE, REQ_DECLINED, REQ_ACCEPTED};
enum error_type {PLAY_WITH_YOURSELF, PLAYER_OCCUPIED, PLAYER_NOT_EXISTS, PLAYER_ALREADY_REGISTERED};
enum prg_state {MENU, GAME};

typedef struct sockaddr_in addr_t;

typedef struct player
{
  char name_[MAX_USERNAME_LEN];
  int udp_port_;

  addr_t address_;

  //Server side information (not significant in battle_client.c)
  int socket_;
  enum player_status status_;
} player_t;

typedef struct game {
  int pvp_socket_;
  player_t pl2_;
  //enum connection_status conn_stat_;

  enum map_tile pl1_map_[MAP_SIZE*MAP_SIZE];
  enum map_tile pl2_map_[MAP_SIZE*MAP_SIZE];
  int pl1_history_[MAP_SIZE*MAP_SIZE]; //index: #attempt, value:-1 not done, 0 missed, 1 hit
  int pl2_history_[MAP_SIZE*MAP_SIZE]; //index: #attempt, value:-1 not done, 0 missed, 1 hit
  int my_turn_;
} game_t;

typedef struct srv_connection {
  int srv_socket_;
  addr_t address_;
} srv_connection_t;

#endif
