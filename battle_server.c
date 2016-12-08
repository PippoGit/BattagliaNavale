#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "h/battle_const.h"
#include "h/battle_foundation.h"
#include "h/net_util.h"
#include "h/battle_util.h"

typedef struct player_list_element
{
  player_t pl;
  struct player_list_element *next;
} player_list_element_t;

player_list_element_t *list = NULL;
int list_size = 0;

void remove_player(const char* name)
{
  player_list_element_t *node = list, *prev = NULL;

  while(node != NULL)
  {
    if(strncmp(node->pl.name_, name, MAX_USERNAME_LEN)==0)
    {
      if(prev == NULL)
        list = node->next;
      else
        prev->next = node->next;
      free(node);
      list_size--;
      return;
    }
    prev = node;
    node = node->next;
  }
}

int search_player(const char* name, addr_t *addr)
{
  player_list_element_t *node = list;
  while(node != NULL)
  {
    if(strncmp(node->pl.name_, name, MAX_USERNAME_LEN)==0)
    {
      *addr = node->pl.address_;
      return (node->pl.status_ == FREE)?node->pl.udp_port_:-2;
    }
    node = node->next;
  }
  return -1;
}

void set_player_status(const char*name, enum player_status s)
{
  player_list_element_t *node = list;
  while(node != NULL)
  {
    if(strncmp(node->pl.name_, name, MAX_USERNAME_LEN)==0)
    {
      node->pl.status_ = s;
      return;
    }
    node = node->next;
  }
}

void set_player_occupied(const char* name)
{
  set_player_status(name, OCCUPIED);
}

void set_player_free(const char* name)
{
  set_player_status(name, FREE);
}

void insert_player(player_t pl)
{
  player_list_element_t *temp = (player_list_element_t*) malloc(sizeof(player_list_element_t));
  temp->pl = pl;
  temp->next = list;
  list = temp;
  list_size++;
}

void print_player_list()
{
  const player_list_element_t *node = list;

  while(node != NULL)
  {
    printf("%s (%s)\n", node->pl.name_, playerstatus_desc(node->pl.status_));
    node = node->next;
  }
}

void new_player_connected(int a_socket, char *msg)
{
  char username[MAX_USERNAME_LEN];
  int udp_port, msg_type;
  unsigned int len;
  player_t p;

  sscanf(msg, "%d %s %d", &msg_type, username, &udp_port);

  printf("Connessione stabilita con il client\n");
  printf("%s si e' connesso\n", username);
  printf("%s e' libero\n", username);

  strncpy(p.name_, username, MAX_USERNAME_LEN);
  p.udp_port_ = udp_port;
  p.status_ = FREE;
  len = sizeof(p.address_);
  getpeername(a_socket, (struct sockaddr *)&p.address_, &len);
  insert_player(p);
}

void send_player_list(int a_socket)
{
  const player_list_element_t *node = list;
  char buffer[50];

  //send list size
  sprintf(buffer, "%d %d", LIST, list_size);
  tcp_send(a_socket, buffer);

  while(node != NULL)
  {
    sprintf(buffer, "%d %s %d", LIST, node->pl.name_, node->pl.status_);
    tcp_send(a_socket, buffer);
    node = node->next;
  }
}

void game_request(int a_socket, char *buffer)
{
  int available, msgt;
  char pl2[MAX_USERNAME_LEN], pl1[MAX_USERNAME_LEN];
  addr_t opponent_address;

  sscanf(buffer, "%d %s %s", &msgt, pl1, pl2);

  //look for asked player
  available = search_player(pl2, &opponent_address);

  switch(available)
  {
    case -1:
      sprintf(buffer, "%d %d", ERROR, PLAYER_NOT_EXISTS);
      tcp_send(a_socket, buffer);
      return;
    case -2:
      sprintf(buffer, "%d %d", ERROR, PLAYER_OCCUPIED);
      tcp_send(a_socket, buffer);
      return;
    default:
      sprintf(buffer, "%d %d %s", PLAY, available, inet_ntoa(opponent_address.sin_addr));
      tcp_send(a_socket, buffer);

      return;
  }

}

void server_func(int *a_socket)
{
  char buffer[50], name[MAX_USERNAME_LEN];
  int msg_type;

  //read cmd
  tcp_recv(*a_socket, buffer);
  //printf("DEBUG: received cmd %s\n", buffer);

  sscanf(buffer, "%d", &msg_type);

  switch(msg_type)
  {
    case HELLO:
      new_player_connected(*a_socket, buffer);
      break;

    case LIST:
      //print_player_list();
      send_player_list(*a_socket);
      break;

    case PLAY:
      game_request(*a_socket, buffer);
      break;

    case BYE:
      sscanf(buffer, "%d %s", &msg_type, name);
      remove_player(name);
      *a_socket = -1;
      break;

    case SET_OCCUPIED:
      sscanf(buffer, "%d %s", &msg_type, name);
      set_player_occupied(name);
      break;

    case SET_FREE:
    default:
      printf("Messaggio sconosciuto... :(\n");
  }
}

int main(int argc, char* argv[]) {
  int socket;

  if(argc < 2)
  {
    printf("Errore nell'inserimento dei parametri! Ti serve un aiuto?\n\t.battle_server.c <porta>\n");
    exit(-1);
  }

  socket = tcp_server(atoi(argv[1]));
  tcp_start_server(socket, server_func);
}
