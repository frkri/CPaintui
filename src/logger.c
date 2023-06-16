#include <curses.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "win.h"

// Global logger
struct logger *logger = NULL;

void setup_log(WINDOW *win) {
  // TODO: Fix Func to be more dynamic to multiple recalls
  // On resize, free logger and reallocate
  if (logger != NULL) {
    for (int i = 0; i != logger->max_lines; i++)
      free(logger->msg_context[i]);
    free(logger);
  }

  int w, h;
  get_window_size(win, &w, &h);

  // Fail safe for small windows
  if (h <= 2)
    h = 3;

  // Allocate memory for logger and add space for msg_context
  logger = malloc(sizeof(struct logger) + (h - 2) * 100 * sizeof(char));

  logger->win = win;
  logger->line = 0;
  logger->max_lines = h - 2; // -2 for border

  // Initialize msg_context
  for (int i = 0; i != logger->max_lines; i++) {
    logger->msg_context[i] = calloc(100, sizeof(char));
  }
  print_log();
}

void cleanup_log(void) {
  // Free msg_context
  for (int i = 0; i != logger->max_lines; i++) {
    free(logger->msg_context[i]);
  }
  free(logger);
}

/*
    Logs a info message to the Log window of the UI
*/
void log_info(char *str) {
  // Append new line to log, removing the oldest line
  strcpy(logger->msg_context[logger->line], str);
  logger->line = (logger->line + 1) % logger->max_lines;
  print_log();
}

void print_log(void) {
  werase(logger->win);

  for (int i = 0; i != logger->max_lines; i++) {
    if (logger->msg_context[i][0] == '\0')
      continue;
    wmove(logger->win, i + 1, 1);

    wattron(logger->win, COLOR_PAIR(5));
    wprintw(logger->win, ">%s", logger->msg_context[i]);
    wattroff(logger->win, COLOR_PAIR(5));
  }
  box(logger->win, 0, 0);
  wrefresh(logger->win);
}
