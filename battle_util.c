#include "h/battle_util.h"

const char* playerstatus_desc(enum player_status stat)
{
  switch(stat)
  {
    case FREE: return "libero";
    case OCCUPIED: return "occupato";
  }
}

void init_fdset(fd_set *set)
{
  FD_ZERO(set);
}

void set_fdset(fd_set *set, int desc)
{
  FD_SET(desc, set);
}

int select_fdset(fd_set *set, int set_size)
{
  int result = select(set_size+1, set, NULL, NULL, NULL);
  if(result < 0)
    perror("Error during file descriptor select");
  return result;
}

int is_set(fd_set *set, int desc)
{
  return FD_ISSET(desc, set);
}

int scan_input(char** buffer) {
  int result;
  size_t size = 0;

	*buffer = NULL;
  result = getline(buffer, &size, stdin);

	if(result < 0) {
  	return 0;
  }

	remove_newline(*buffer);
  return 1;
}

int remove_newline(char* buffer) {
    char *pos;
    if ((pos = strchr(buffer, '\n')) != NULL)
        *pos = '\0';
    else
        return 0;
    return 1;
}

/*
int scan_input_poll(char** buffer, int sec) {
    struct pollfd mypoll = { 0, POLLIN|POLLPRI };

    if(poll(&mypoll, 1, sec*1000) )
        return scan_input(buffer);
    return 0;
}
*/

void flush_input() {
    int n;
    while ((n = getchar()) != '\n' && n != EOF);
}
