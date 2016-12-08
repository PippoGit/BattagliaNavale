#include "h/battle_util.h"

const char* playerstatus_desc(enum player_status stat)
{
  switch(stat)
  {
    case FREE: return "libero";
    case OCCUPIED: return "occupato";
  }
}
