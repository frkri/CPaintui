#include <curses.h>
#include <ncurses.h>
#include <stdio.h>

#include "util.c"

int display_modal(char *msg);

int main(void) {
  initscr(); // Initialize ncurses

  // Set Options
  cbreak();             // Stops buffering of Chars
  noecho();             // Stops chars from being printed to the screen
  keypad(stdscr, TRUE); // Allows for arrow keys to be used
  curs_set(0);          // Hide cursor

  // Create Windows
  WINDOW *draw = newwin(20, 30, 0, 0);
  WINDOW *menu = newwin(20, 30, 0, 30);

  box(draw, 0, 0);
  box(menu, 0, 0);

  wprintw(draw, "Draw Window");
  wprintw(menu, "Menu Window");

  wrefresh(draw);
  wrefresh(menu);

  wgetch(draw); // Wait for user input on draw window

  endwin(); // End ncurses

  return 0;
}

int draw_base() { return 0; }

int display_modal(char *msg) {

  int Y = getmaxy(stdscr);
  int X = getmaxx(stdscr);

  printw("X: %i, Y: %i\n", X, Y);

  mvprintw(15, 15, "MSG: %s\n", msg);

  return 0;
}