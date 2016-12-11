#include "h/battle_const.h"
#include "h/battle_foundation.h"
#include "h/net_util.h"
#include "h/battle_util.h"

srv_connection_t srv_conn;
player_t pl_conf;
fd_set fds;
enum prg_state current_state;
game_t current_game;

void destroy_battle()
{
  char buffer[DEFAULT_BUFF_SIZE];
  sprintf(buffer, "%d %s", BYE, pl_conf.name_);
  tcp_send(srv_conn.srv_socket_, buffer);

  //destroy data structure
  close(srv_conn.srv_socket_);
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

void init_map(enum map_tile m[])
{
  int i=0;

  for(i=0; i<MAP_SIZE*MAP_SIZE; i++)
    m[i] = WATER;
}

void print_map(enum map_tile m[])
{
  enum map_tile curr;
  int i=0, j=0;

  for(i=0; i<MAP_SIZE; i++)
  {
    for(j=0; j<MAP_SIZE;j++)
    {
      curr = m[i*MAP_SIZE +j];
      switch(curr)
      {
        case WATER:
          printf("\t%d", i*MAP_SIZE + j);
          break;

        case SHIP:
          printf("\tS");
          break;

        case HIT_SHIP:
          printf("\t#");
          break;

        case MISSED_SHIP:
          printf("\tM");
          break;
      }
    }
    printf("\n");
  }
  printf("\n");
}

int valid_position(enum map_tile m[], int p)
{
  return (p < (MAP_SIZE*MAP_SIZE) && p >= 0 && m[p] == WATER);
}

void init_game(char *name, char *ip, int udp_port, int turn)
{
  char *line = NULL;
  int i=0, tile;
  player_t pl2;
  current_state = GAME;

  memset(&current_game, 0, sizeof(current_game)); //init current GAME

  current_game.my_turn_ = turn;
  strncpy(pl2.name_, name, MAX_USERNAME_LEN);
  pl2.udp_port_ = udp_port;

  current_game.pvp_socket_ = udp_connect(&pl2.address_, ip, pl2.udp_port_, pl_conf.udp_port_);
  current_game.pl2_ = pl2;

  init_map(current_game.pl1_map_);
  init_map(current_game.pl2_map_);

  printf("Posiziona %d caselle (un numero compreso tra 0 e %d)\n", MAX_SHIPS_NUM, (MAP_SIZE*MAP_SIZE)-1);

  for(i=0; i<MAX_SHIPS_NUM; i++)
  {
    scan_input(&line);
    sscanf(line, "%d",&tile);

    if(valid_position(current_game.pl1_map_, tile))
      current_game.pl1_map_[tile] = SHIP;
    else
    {
      printf("Casella non valida, riprova:");
      i--;
    }
  }

  printf("Mappa creata con successo!\n");
  free(line);
}

void connect_to_player(char *player)
{
  char buffer[DEFAULT_BUFF_SIZE], pl2_ip[15];
  int msgt = -1, errort = -1, response=0, pl2_port;

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
      sscanf(buffer, "%*d %*s %s %d", pl2_ip, &pl2_port);

      //wait for response
      tcp_recv(srv_conn.srv_socket_, buffer);
      sscanf(buffer, "%*d %d", &response);

      if(response)
      {
        //request accepted
        printf("Richiesta accettata!\n");
        //invited player always do the first move
        init_game(player, pl2_ip, pl2_port, 0);
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
      //invited player always do the first move
      init_game(name, pl2_ip, pl2_port, 1);
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

void print_man()
{
  printf("Sono disponibili i seguenti comandi:\n");
  printf("!help --> mostra l'elenco dei comandi disponibili\n");
  printf("!shot square --> fai un tentativo con la casella square\n");
  printf("!show --> visualizza griglia di gioco\n");
  printf("!disconnect --> disconnette il client dall'attuale partita\n");
}

int have_i_lost()
{
  int i=0, ship_left=MAX_SHIPS_NUM;
  for(i=0; i<MAP_SIZE*MAP_SIZE;i++)
    if(current_game.pl1_map_[i] == HIT_SHIP)
      ship_left--;
  return ship_left==0;
}

void shot(int position)
{
  int response = -1;
  char buffer[DEFAULT_BUFF_SIZE];
  if(!valid_position(current_game.pl2_map_, position))
  {
    printf("Posizione non valida. Inserisci un comando valido.\n");
    return;
  }

  //send attack
  sprintf(buffer, "%d %d", ATK, position);
  udp_send(current_game.pvp_socket_, buffer);

  //wait for response
  udp_recv(current_game.pvp_socket_, buffer);
  sscanf(buffer, "%d", &response);

  if(response == MISS)
  {
    sprintf(buffer, "%d", MISS);
    current_game.pl2_map_[position] = MISSED_SHIP;
    printf("%s dice: mancato :(\n", current_game.pl2_.name_);
  }
  else
  {
    sprintf(buffer, "%d", HIT);
    current_game.pl2_map_[position] = HIT_SHIP;
    printf("%s dice: colpito! :)\n", current_game.pl2_.name_);
  }

}

void you_win()
{
  char buffer[DEFAULT_BUFF_SIZE];
  printf("La partita e' terminata. Hai vinto!\n\n");

  //Server, i am free again!
  sprintf(buffer, "%d %s", SET_FREE, pl_conf.name_);
  tcp_send(srv_conn.srv_socket_, buffer);

  //shutdown pvp connection
  close(current_game.pvp_socket_);

  current_state = MENU;
}

void update_my_map(char *msg)
{
  char buffer[DEFAULT_BUFF_SIZE];
  int msgt, position;

  sscanf(msg, "%d %d", &msgt, &position);

  if(msgt == I_LOST || msgt == SURRENDER)
    return you_win();

  if(msgt != ATK) return; //this should never happen

  if(current_game.pl1_map_[position] == WATER)
  {
    sprintf(buffer, "%d", MISS);
    current_game.pl1_map_[position] = MISSED_SHIP;
    printf("%s spara in %d: mancato :)\n", current_game.pl2_.name_, position);
  }
  else
  {
    sprintf(buffer, "%d", HIT);
    current_game.pl1_map_[position] = HIT_SHIP;
    printf("%s spara in %d: colpito :(\n", current_game.pl2_.name_, position);
  }

  //send response
  udp_send(current_game.pvp_socket_, buffer);

  if(have_i_lost()) {
    printf("Le navi a tua disposizione sono finite. Hai perso.\n\n");
    //send that i have lost and quit somehow
    sprintf(buffer, "%d", I_LOST);
    udp_send(current_game.pvp_socket_, buffer);

    //shutdown pvp connection
    close(current_game.pvp_socket_);

    //Server, i am free again!
    sprintf(buffer, "%d %s", SET_FREE, pl_conf.name_);
    tcp_send(srv_conn.srv_socket_, buffer);

    current_state = MENU;
  }
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
      printf("La tua mappa:\n");
      print_map(current_game.pl1_map_);
      printf("\n\nMappa del tuo avversario:\n");
      print_map(current_game.pl2_map_);
      break;
    case DISCONNECT:
      printf("ti sei arreso\n\n");

      //////////////////////////////////////////////////// SOLUZIONE TEMPORANEA
      char buffer[DEFAULT_BUFF_SIZE];
      sprintf(buffer, "%d", I_LOST);
      udp_send(current_game.pvp_socket_, buffer);

      close(current_game.pvp_socket_);

      //Server, i am free again!
      sprintf(buffer, "%d %s", SET_FREE, pl_conf.name_);
      tcp_send(srv_conn.srv_socket_, buffer);
      current_state = MENU;
      //////////////////////////////////////////////////////
      break;
    case SHOT:
      printf("hai sparato in posizione %s\n", param);
      shot(atoi(param));
      break;

    default:
      printf("Comando non riconosciuto o non valido...\n");
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
      if(!valid_position(current_game.pl2_map_, atoi(param)))
        cmd = -1;
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
  char param[DEFAULT_BUFF_SIZE], *wait = NULL, buffer[DEFAULT_BUFF_SIZE];

  if(current_game.my_turn_)
  {
    do {
      print_prompt();
      fflush(stdout);
      cmd = fetch_cmd(param); //i should wait here
      handle_cmd(cmd, param);
    } while(cmd < 0 || cmd == SHOW);
  }
  else
  {
    printf("Aspetta il turno dell'avversario...\n");
    udp_recv(current_game.pvp_socket_, buffer);
    printf("DEBUG: ho ricevuto %s\n", buffer);
    update_my_map(buffer);
  }
  current_game.my_turn_ = !current_game.my_turn_;  //cambio il turno
  free(wait);
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
