#include <curses.h>

#ifndef LOGGER_H
#define LOGGER_H

struct logger {
  WINDOW *win;
  int line;
  int max_lines;
  // Variable length array!
  char *msg_context[];
};

void setup_log(WINDOW *win);
void cleanup_log(void);
void print_log(void);
void log_info(char *str);

#endif
