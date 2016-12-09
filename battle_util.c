#include "h/battle_util.h"

const char* playerstatus_desc(enum player_status stat)
{
  switch(stat)
  {
    case FREE: return "libero";
    case OCCUPIED: return "occupato";
  }
}

int select_and_get_line(char *buffer)
{
  fd_set master;
  fd_set read_fds;
  unsigned long size = DEFAULT_BUFF_SIZE;

  FD_SET(0, &master);
  read_fds = master;

  int c = select(1, &read_fds, NULL, NULL, NULL);
  if(c<0) return c;
  return getline(&buffer, &size, 0);
}
