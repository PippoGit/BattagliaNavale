#include "h/net_util.h"

int tcp_socket()
{
  int s = -1;

  s = socket(AF_INET, SOCK_STREAM, 0);
  if(s < 0) {
    perror("Error during socket()");
    exit(-1);
  }

  return s;
}

int udp_socket()
{
  int s = -1;

  s = socket(AF_INET, SOCK_DGRAM, 0);
  if(s < 0) {
    perror("Error during socket()");
    exit(-1);
  }

  return s;
}

int udp_connect(addr_t *a, char *ip, int udp_port, int my_port)
{
  int ret = -1, socket;
  addr_t my_addr;

  //create udp socket
  socket = udp_socket();

  //init my address
  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(my_port);
  my_addr.sin_addr.s_addr = INADDR_ANY;

  ret = bind(socket, (struct sockaddr *) &my_addr, sizeof(my_addr));
  if(ret < 0)
  {
    perror("Error during UDP bind()");
    exit(-1);
  }

  //init opponent address
  memset(a, 0, sizeof(*a));
  a->sin_family = AF_INET;
  a->sin_port = htons(udp_port);
  inet_pton(AF_INET, ip, &a->sin_addr);

  ret = connect(socket, (struct sockaddr*)a, sizeof(*a));
  if(ret < 0)
  {
    perror("Error during UDP connect()");
    exit(-1);
  }

  return socket;
}

int tcp_start_server(int socket, void (*func)(int*))
{
  addr_t cl_addr;
  unsigned int len = sizeof(cl_addr);

  fd_set master;
  fd_set read_fds;
  int new_sd, fdmax, i, j;

  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  FD_SET(socket, &master);
  fdmax = socket;

  while(1)
  {
    read_fds = master;
    select(fdmax + 1, &read_fds, NULL, NULL, NULL);

    for(i=0; i<=fdmax; i++)
    {
      if(FD_ISSET(i, &read_fds))
      {
        if(i == socket)
        {
          new_sd = accept(socket, (struct sockaddr*)&cl_addr, &len);
          FD_SET(new_sd, &master);
          if(new_sd > fdmax) fdmax = new_sd;
        }
        else
        {
          j=i;
          func(&j);
          if(j < 0)
          {
            close(i);
            FD_CLR(i, &master);
          }
        }
      }
    }
  }
  return 0;
}

int tcp_server(int port)
{
  int ret, sd;
  addr_t my_addr;


  printf("Costruisco il socket ascolto...");
  sd = tcp_socket();
  printf("OK!\n");

  printf("Effettuo bind indirizzo...");
  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_port = htons(port);
  my_addr.sin_addr.s_addr = INADDR_ANY;

  ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr));
  if(ret < 0)
  {
    perror("Error during bind()");
    exit(-1);
  }
  printf("OK!\n");

  printf("Costruisco sala d'attesa con capienza %d...", TCP_SERVER_LISTENQ);
  ret = listen(sd, TCP_SERVER_LISTENQ);
  if(ret < 0)
  {
    perror("Error during listen()");
    exit(-1);
  }
  printf("OK!\n");

  printf("\n\nServer avviato.\nIndirizzo: %s (Porta: %d)\n\n", inet_ntoa(my_addr.sin_addr), ntohs(my_addr.sin_port));

  return sd;
}

int tcp_connect(int socket, addr_t *a, const char* ip, const int p)
{
  int ret;
  memset(a, 0, sizeof(*a));
  a->sin_family = AF_INET;
  a->sin_port = htons(p);
  inet_pton(AF_INET, ip, &a->sin_addr);

  ret = connect(socket, (struct sockaddr*)a, sizeof(*a));
  if(ret < 0) {
    perror("Error during connect()");
    exit(-1);
  }

  return ret;
}

int tcp_recv(int socket, char *msg)
{
  int msg_len, ret;

  //recv msg len (how many bytes)
  ret = recv(socket, (void *) &msg_len, sizeof(int), 0);
  //printf("DEBUG: %d/%lu Byte ricevuti\n", ret, sizeof(int));

  if(ret < (int) sizeof(int)) //timeout
    return -1;

  msg_len = ntohl(msg_len);

  //recv buffer msg
  ret = recv(socket, (void *) msg, msg_len, 0);
  //printf("DEBUG: %d/%d Byte ricevuti\n", ret, msg_len);

  if(ret < msg_len)
    ret = -1;

  return ret;
}

int tcp_send(int socket, const char *msg)
{
  int ret, real_len, len;
  real_len = strlen(msg)+1;
  len = htonl(real_len);

  ret = send(socket, (void *) &len, sizeof(int), 0);
  if(ret < sizeof(len))
  {
    //Errore
    printf("Errore nella send dei byte da inviare...\n");
    //exit(-1);
  }

  ret = send(socket, (void *) msg, real_len, 0);
  if(ret < real_len)
  {
    //Errore
    printf("Errore nella send...\n");
    return -1;
    //exit(-1);
  }
  return ret;
}

int udp_recv(int socket, char*msg)
{
  return tcp_recv(socket, msg);
}

int udp_send(int socket, const char*msg)
{
  return tcp_send(socket, msg);
}

int udp_recv_timeout(int socket, char*msg)
{
    int byte_rec;
    set_timeout(socket);
    byte_rec = udp_recv(socket, msg);
    reset_timeout(socket);

    return byte_rec;
}

void set_timeout(int sock) {
    struct timeval tv;
    tv.tv_sec = INPUT_TIMEOUT_SEC;
    tv.tv_usec = 0;

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
}

void reset_timeout(int sock) {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
}

int valid_ip_addr(char* addr) {
    struct sockaddr_in addr_in;
    int result = inet_pton(AF_INET, addr, &(addr_in.sin_addr));
    return result != 0;
}
