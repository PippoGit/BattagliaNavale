#ifndef UTIL_HEADER
#define UTIL_HEADER

#include "battle_foundation.h"

const char* playerstatus_desc(enum player_status stat);
int select_and_getline(char **buffer, int s);

void init_fdset(fd_set *set);
void set_fdset(fd_set *set, int desc);
int select_fdset(fd_set *set, int set_size);
int select_and_getline(char **buffer, int s);
int is_set(fd_set *set, int desc);

int scan_input(char** buffer);
int remove_newline(char* buffer);
int scan_input_poll(char** buffer, int sec);
void flush_input();

#endif
