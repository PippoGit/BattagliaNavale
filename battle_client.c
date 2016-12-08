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

void greetings()
{
  printf("Sono disponibili i seguenti comandi:\n");
  printf("!help --> mostra l'elenco dei comandi disponibili\n");
  printf("!who --> mostra l'elenco dei client connessi al server\n");
  printf("!connect username --> avvia una partita con l'utente username\n");
  printf("!quit --> disconnette il client dal server\n");
}

int wait_for_cmd(const enum cmd_type t, char *param)
{
  int cmd = -1; //enum cmd {HELP, WHO, CONNECT, QUIT, SHOT, DISCONNECT, SHOW};

  char buffer[100];

  if(t == MENU)
  {
    printf(">");
    scanf("%s", buffer);

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
    printf("#");
    scanf("%s", buffer);
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

void init_player_one(player_t *p1)
{
  printf("Inserisci il tuo username:");
  scanf("%s", p1->name_);
  printf("Inserisci la porta di ascolto UDP:");
  scanf("%d", &p1->udp_port_);
}

void connect_to_server(srv_connection_t* c, const player_t pl, const char* ip, const int p)
{
  char buff[MAX_USERNAME_LEN+8];

  c->srv_socket_ = tcp_socket();
  printf("cerco di connettermi a %s:%d\n", ip, p);
  tcp_connect(c->srv_socket_, &c->address_, ip, p);

  sprintf(buff, "%d %s %d", HELLO, pl.name_, pl.udp_port_);
  tcp_send(c->srv_socket_, buff);
}

void print_playerlist_from_server(srv_connection_t* c, player_t me)
{
  char buffer[1024], name[MAX_USERNAME_LEN];
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
  char buffer[10];
  sprintf(buffer, "%d %s", BYE, me.name_);
  tcp_send(c->srv_socket_, buffer);

  //destroy data structure
  exit(0);
}

void connect_to_player(srv_connection_t *c, char *player, player_t me)
{
  char buffer[50];
  int msgt = -1, errort=-1;

  if(strncmp(me.name_, player, MAX_USERNAME_LEN) == 0) //you can't play with yourself, duh
  {
    printf("Non puoi giocare con te stesso!\n");
    return;
  }

  sprintf(buffer, "%d %s", PLAY, player);
  tcp_send(c->srv_socket_, buffer);

  //memset(buffer, 0, sizeof(buffer));

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

int main(int argc, char* argv[])
{
  srv_connection_t srv_conn;
  player_t pl_conf;

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

  //suppose to have established conn
  while(1)
  {
    char p[MAX_USERNAME_LEN];
    int a = wait_for_cmd(MENU, p);

    switch(a)
    {
      case HELP:
        greetings();
        break;
      case WHO:
        print_playerlist_from_server(&srv_conn, pl_conf);
        break;
      case CONNECT:
        connect_to_player(&srv_conn, p, pl_conf);
        break;
      case QUIT:
        destroy_battle(&srv_conn, pl_conf);
        break;
      default:
        printf("Comando non riconosciuto...\n");
    }

  }
}
