#include "h/battle_const.h"
#include "h/battle_foundation.h"
#include "h/net_util.h"
#include "h/battle_util.h"

srv_connection_t srv_conn;
player_t pl_conf;
fd_set fds;
enum prg_state current_state;

void destroy_battle()
{
  char buffer[DEFAULT_BUFF_SIZE];
  sprintf(buffer, "%d %s", BYE, pl_conf.name_);
  tcp_send(srv_conn.srv_socket_, buffer);

  //destroy data structure
  exit(0);
}

void init_player_one()
{
  printf("Inserisci il tuo username:");
  scanf("%s", pl_conf.name_);
  printf("Inserisci la porta di ascolto UDP:");
  scanf("%d", &pl_conf.udp_port_);
}

void connect_to_server(const char* ip, const int p)
{
  char buff[DEFAULT_BUFF_SIZE];
  int response = 0;

  srv_conn.srv_socket_ = tcp_socket();
  tcp_connect(srv_conn.srv_socket_, &srv_conn.address_, ip, p);

  sprintf(buff, "%d %s %d", HELLO, pl_conf.name_, pl_conf.udp_port_);
  tcp_send(srv_conn.srv_socket_, buff);

  tcp_recv(srv_conn.srv_socket_, buff);
  sscanf(buff, "%d", &response);

  if(response == ERROR)
  {
    printf("Il nome scelto risulta gia registrato.\n");
    exit(-1);
  }
  //if everything is ok
  printf("Connessione al server %s (porta %d) effettuata con successo!\n\n", ip, p);
}

void print_playerlist_from_server()
{
  char buffer[DEFAULT_BUFF_SIZE], name[MAX_USERNAME_LEN];
  int list_size=5, i, stat;

  sprintf(buffer, "%d", LIST);
  tcp_send(srv_conn.srv_socket_, buffer);

  tcp_recv(srv_conn.srv_socket_, buffer);
  sscanf(buffer, "1 %d", &list_size);

  printf("Clienti connessi al server:\n");
  for(i = 0; i<list_size; i++)
  {
    tcp_recv(srv_conn.srv_socket_, buffer);
    sscanf(buffer, "1 %s %d", name, &stat);
    if(strncmp(name, pl_conf.name_, MAX_USERNAME_LEN))
      printf("\t%s (%s)\n", name, playerstatus_desc(stat));
  }
}

void connect_to_player(char *player)
{
  char buffer[DEFAULT_BUFF_SIZE];
  int msgt = -1, errort = -1, response=0;

  if(strncmp(pl_conf.name_, player, MAX_USERNAME_LEN) == 0) //you can't play with yourself, duh
  {
    printf("Non puoi giocare con te stesso!\n");
    return;
  }

  sprintf(buffer, "%d %s %s", PLAY, pl_conf.name_, player);
  tcp_send(srv_conn.srv_socket_, buffer);

  tcp_recv(srv_conn.srv_socket_, buffer);

  sscanf(buffer, "%d", &msgt);
  switch(msgt)
  {
    case ERROR:
      sscanf(buffer, "%d %d", &msgt, &errort);
      if(errort == PLAYER_NOT_EXISTS) printf("Il giocatore richiesto e' inesistente.\n");
      if(errort == PLAYER_OCCUPIED) printf("Il giocatore richiesto e' gia' impegnato in un'altra partita.\n");
      return;

    case PLAY:
      printf("Invio richiesta al giocatore...\n");

      //wait for response
      tcp_recv(srv_conn.srv_socket_, buffer);
      sscanf(buffer, "%*d %d", &response);

      if(response)
      {
        //request accepted
        printf("Richiesta accettata!\n");
        current_state = GAME;
      }
      else
      {
        printf("Richiesta rifiutata.\n");
      }
  }

}

void request_from_player(char *msg)
{
  char name[MAX_USERNAME_LEN], pl2_ip[15], buffer[DEFAULT_BUFF_SIZE];
  char *line;
  unsigned short pl2_port = 0;

  //fetch request from server socket
  sscanf(msg, "%*d %s %s %hd", name, pl2_ip, &pl2_port);

  //ask player 1 to join the game
  printf("\n%s vuole sfidarti. Accetti la sfida? (y/n) ", name);
  fflush(stdout);
  scan_input(&line);

  switch(line[0])
  {
    case 'n':
    case 'N':
      printf("\nRichiesta rifiutata.\n");
      sprintf(buffer, "%d %s %s", REQ_DECLINED, pl_conf.name_, name);
      tcp_send(srv_conn.srv_socket_, buffer);
      break;

    case 'y':
    case 'Y':
      sprintf(buffer, "%d %s %s", REQ_ACCEPTED, pl_conf.name_, name);
      tcp_send(srv_conn.srv_socket_, buffer);

      printf("\nRichiesta accettata...\n");
      current_state = GAME;
  }
  free(line);
}

void greetings()
{
  printf("Sono disponibili i seguenti comandi:\n");
  printf("!help --> mostra l'elenco dei comandi disponibili\n");
  printf("!who --> mostra l'elenco dei client connessi al server\n");
  printf("!connect username --> avvia una partita con l'utente username\n");
  printf("!quit --> disconnette il client dal server\n");
}

void print_prompt()
{
  switch(current_state)
  {
    case MENU: printf(">"); return;
    case GAME: printf("#"); return;
  }
}

//game functions
void print_man()
{
  printf("Sono disponibili i seguenti comandi:\n");
  printf("!help --> mostra l'elenco dei comandi disponibili\n");
  printf("!shot square --> fai un tentativo con la casella square\n");
  printf("!show --> visualizza griglia di gioco\n");
  printf("!disconnect --> disconnette il client dall'attuale partita\n");
}

void handle_cmd(int cmd, char *param)
{
  switch(cmd)
  {
    //socket cmd
    case REQ_FROM_SOCKET: //request from socket
      request_from_player(param);
      break;

    //menu cmd
    case HELP:
      greetings();
      break;
    case WHO:
      print_playerlist_from_server();
      break;
    case CONNECT:
      connect_to_player(param);
      break;
    case QUIT:
      destroy_battle();
      break;

    //game cmd
    case GHELP:
      print_man();
      break;
    case SHOW:
      printf("mostro le mappe\n");
      break;
    case DISCONNECT:
      printf("ti sei arreso\n");
      current_state = MENU;
      break;
    case SHOT:
      printf("hai sparato in posizione %s\n", param);
      break;

    default:
      printf("Comando non riconosciuto...\n");
  }
}

int fetch_cmd(char *param)
{
  int cmd = -1;
  char *buffer = NULL;

  scan_input(&buffer);

  if(current_state == MENU)
  {
    if(strncmp("!help", buffer, 5) == 0) cmd =  HELP;
    else if(strncmp("!who", buffer, 4) == 0) cmd =  WHO;
    else if(strncmp("!connect", buffer, 8) == 0)
    {
      cmd =  CONNECT;
      sscanf(buffer, "%*s %s", param);
    }
    else if(strncmp("!quit", buffer, 5) == 0) cmd =  QUIT;
  }
  else
  {
    if(strncmp("!help", buffer, 5) == 0) cmd =  GHELP;
    else if(strncmp("!disconnect", buffer, 11) == 0) cmd =  DISCONNECT;
    else if(strncmp("!shot", buffer, 5) == 0)
    {
      cmd =  SHOT;
      sscanf(buffer, "%*s %s", param);
    }
    else if(strncmp("!show", buffer, 5) == 0) cmd =  SHOW;
  }

  free(buffer);
  return cmd;
}

int wait_for_cmd_or_socket()
{
  char param[DEFAULT_BUFF_SIZE];
  fd_set read_fds;

  int fdmax, cmd;
  fdmax = srv_conn.srv_socket_;

  print_prompt();
  fflush(stdout);

  read_fds = fds;
  if(select_fdset(&read_fds, fdmax) < 0)
    exit(-1);

  if(is_set(&read_fds, 0))
  {
    cmd = fetch_cmd(param);
  }

  if(is_set(&read_fds, srv_conn.srv_socket_))
  {
    //data from socket
    tcp_recv(srv_conn.srv_socket_, param);
    cmd = REQ_FROM_SOCKET;
  }

  handle_cmd(cmd, param);
  return 0;
}

void game_mode()
{
  int cmd = -1;
  int my_turn = 1;
  char param[DEFAULT_BUFF_SIZE];

  print_prompt();
  fflush(stdout);

  if(my_turn)
  {
    cmd = fetch_cmd(param);
    handle_cmd(cmd, param);
  }
  else
  {
    printf("Aspetta il turno dell'avversario...\n");
  }
}

int main(int argc, char* argv[])
{
  current_state = MENU;

  //check parameters
  if(argc < 3)
  {
    printf("Errore nell'inserimento dei parametri! Ti serve un aiuto?\n\t.battle_client.c <host remoto> <porta>\n");
    exit(-1);
  }

  //init player one
  init_player_one();

  //connect to the server
  connect_to_server(argv[1], atoi(argv[2]));

  //print greetings
  greetings();

  //init fd set
  init_fdset(&fds);
  set_fdset(&fds, 0);
  set_fdset(&fds, srv_conn.srv_socket_);

  //clear input from rubbish
  flush_input();

  //server connection established, let's do some stuff
  while(1)
  {
    if(current_state == MENU)
      wait_for_cmd_or_socket();
    else
      game_mode();
  }
}

/*
GAME

pl1:
  start_handshake();

  if(CONN)
  {
    while(GAME)
    {
      move();
      wait_for_opponent_update();
      update();
      wait_for_opponent_move();
      update();
    }
  }

pl2:
  wait_for_handshake();

  if(CONN)
  {
    while(GAME)
    {
      wait_for_opponent_move();
      update();
      move();
      wait_for_opponent_update();
      update();
    }
  }
*/
