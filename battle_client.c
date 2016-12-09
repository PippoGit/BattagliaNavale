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

void init_player_one(player_t *p1)
{
  printf("Inserisci il tuo username:");
  scanf("%s", p1->name_);
  printf("Inserisci la porta di ascolto UDP:");
  scanf("%d", &p1->udp_port_);
}

void connect_to_server(srv_connection_t* c, const player_t pl, const char* ip, const int p)
{
  char buff[DEFAULT_BUFF_SIZE];

  c->srv_socket_ = tcp_socket();
  printf("cerco di connettermi a %s:%d\n", ip, p);
  tcp_connect(c->srv_socket_, &c->address_, ip, p);

  sprintf(buff, "%d %s %d", HELLO, pl.name_, pl.udp_port_);
  tcp_send(c->srv_socket_, buff);
}

void print_playerlist_from_server(srv_connection_t* c, player_t me)
{
  char buffer[DEFAULT_BUFF_SIZE], name[MAX_USERNAME_LEN];
  int list_size=5, i, stat;

  sprintf(buffer, "%d", LIST);
  tcp_send(c->srv_socket_, buffer);

  tcp_recv(c->srv_socket_, buffer);
  sscanf(buffer, "1 %d", &list_size);

  printf("Clienti connessi al server:\n");
  for(i = 0; i<list_size; i++)
  {
    tcp_recv(c->srv_socket_, buffer);
    sscanf(buffer, "1 %s %d", name, &stat);
    if(strncmp(name, me.name_, MAX_USERNAME_LEN))
      printf("\t%s (%s)\n", name, playerstatus_desc(stat));
  }
}

void destroy_battle(srv_connection_t *c, player_t me)
{
  char buffer[DEFAULT_BUFF_SIZE];
  sprintf(buffer, "%d %s", BYE, me.name_);
  tcp_send(c->srv_socket_, buffer);

  //destroy data structure
  exit(0);
}

void connect_to_player(srv_connection_t *c, char *player, player_t me)
{
  char buffer[DEFAULT_BUFF_SIZE];
  int msgt = -1, errort = -1;

  if(strncmp(me.name_, player, MAX_USERNAME_LEN) == 0) //you can't play with yourself, duh
  {
    printf("Non puoi giocare con te stesso!\n");
    return;
  }

  sprintf(buffer, "%d %s %s", PLAY, me.name_, player);
  tcp_send(c->srv_socket_, buffer);

  tcp_recv(c->srv_socket_, buffer);

  sscanf(buffer, "%d", &msgt);
  switch(msgt)
  {
    case ERROR:
      sscanf(buffer, "%d %d", &msgt, &errort);
      if(errort == PLAYER_NOT_EXISTS) printf("Il giocatore richiesto e' inesistente.\n");
      if(errort == PLAYER_OCCUPIED) printf("Il giocatore richiesto e' gia' impegnato in un'altra partita.\n");
      return;

    case PLAY:
      printf("Giocatore esistente. Provo ad inviare la richiesta al player...");
      sprintf(buffer, "%d %s", SET_OCCUPIED, me.name_);
      tcp_send(c->srv_socket_, buffer);
      return;
  }

  //richiedere connessione con il client

  //aspettare che il client accetti

  //se accetta, avviare la partita

  //se declina, pazienza return.
}

void request_from_player(char *msg)
{
  char name[MAX_USERNAME_LEN], pl2_ip[15], resp;
  char *line = (char *)malloc(DEFAULT_BUFF_SIZE * sizeof(char));;
  unsigned short pl2_port = 0;
  int msgt;
  size_t buffsize = DEFAULT_BUFF_SIZE;

  sscanf(msg, "%d %s %s %hd", &msgt, name, pl2_ip, &pl2_port);
  fflush(stdout);
  printf("%s vuole sfidarti. Accetti la sfida? (y/n) ", name);

  //getline(&line, &buffsize, 0);
  //sscanf(line, "%c", &resp);
  free(line);

  resp = 'y';

  switch(resp)
  {
    case 'n':
      return;
    case 'y':
      printf("Apro socket UDP con %s su indirizzo %s:%d\n", name, pl2_ip, pl2_port);
      return;
  }

}

void greetings()
{
  printf("Sono disponibili i seguenti comandi:\n");
  printf("!help --> mostra l'elenco dei comandi disponibili\n");
  printf("!who --> mostra l'elenco dei client connessi al server\n");
  printf("!connect username --> avvia una partita con l'utente username\n");
  printf("!quit --> disconnette il client dal server\n");
}

void print_prompt(enum prg_state s)
{
  switch(s)
  {
    case MENU: printf(">"); return;
    case GAME: printf("#"); return;
  }
}


void handle_cmd(int cmd, const enum prg_state t, char *param, srv_connection_t *srv_conn, const player_t pl_conf)
{
  switch(cmd)
  {
    case REQ_FROM_SOCKET: //request from socket
      request_from_player(param);
      break;

    case HELP:
      greetings();
      break;
    case WHO:
      print_playerlist_from_server(srv_conn, pl_conf);
      break;
    case CONNECT:
      connect_to_player(srv_conn, param, pl_conf);
      break;
    case QUIT:
      destroy_battle(srv_conn, pl_conf);
      break;
    default:
      printf("Comando non riconosciuto...\n");
  }
}

int wait_for_cmd(const enum prg_state t, char *param)
{
  int cmd = -1;
  char buffer[DEFAULT_BUFF_SIZE];

  scanf("%s", buffer);

  if(t == MENU)
  {
    if(strncmp("!help", buffer, 5) == 0) cmd =  HELP;
    else if(strncmp("!who", buffer, 4) == 0) cmd =  WHO;
    else if(strncmp("!connect", buffer, 8) == 0)
    {
      cmd =  CONNECT;
      scanf("%s", param);
    }
    else if(strncmp("!quit", buffer, 5) == 0) cmd =  QUIT;
  }
  else
  {
    if(strncmp("!help", buffer, 5) == 0) cmd =  HELP;
    else if(strncmp("!disconnect", buffer, 11) == 0) cmd =  DISCONNECT;
    else if(strncmp("!shot", buffer, 5) == 0)
    {
      cmd =  SHOT;
      scanf("%s", param);
    }
    else if(strncmp("!show", buffer, 5) == 0) cmd =  SHOW;
  }

  return cmd;
}

int wait_for_cmd_or_socket(const enum prg_state t, srv_connection_t *c, const player_t me)
{
  char param[DEFAULT_BUFF_SIZE];
  fd_set master;
  fd_set read_fds;
  int fdmax, i, cmd;

  FD_SET(0, &master);
  FD_SET(c->srv_socket_, &master);

  fdmax = c->srv_socket_;

  while(1)
  {
    read_fds = master;
    select(fdmax + 1, &read_fds, NULL, NULL, NULL);

    for(i=0; i<=fdmax; i++)
    {
      if(FD_ISSET(i, &read_fds))
      {
        if(i == 0)
        {
          print_prompt(t);
          cmd = wait_for_cmd(t, param);
          handle_cmd(cmd, t, param, c, me);
        }
        else if(i==c->srv_socket_)
        {
          //data from socket
          tcp_recv(i, param);
          handle_cmd(REQ_FROM_SOCKET, t, param, c, me);
        }
      }
    }
  }

  return 0;
}

int main(int argc, char* argv[])
{
  srv_connection_t srv_conn;
  player_t pl_conf;
  enum prg_state current_state = MENU;

  //check parameters
  if(argc < 3)
  {
    printf("Errore nell'inserimento dei parametri! Ti serve un aiuto?\n\t.battle_client.c <host remoto> <porta>\n");
    exit(-1);
  }

  //print greetings
  greetings();

  //init player one
  init_player_one(&pl_conf);

  //connect to the server
  connect_to_server(&srv_conn, pl_conf, argv[1], atoi(argv[2]));

  //connection with server established
  wait_for_cmd_or_socket(current_state, &srv_conn, pl_conf);
}
