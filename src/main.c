#include <curses.h>
#include <ncurses.h>
#include <stdio.h>

#include "util.c"

WINDOW *create_win_pad(int pad);
WINDOW *create_win_relative(WINDOW *win, float w_prc, float h_prc);

int main(void) {
  initscr(); // Initialize ncurses

  // Set Options
  cbreak();             // Stops buffering of Chars
  noecho();             // Stops chars from being printed to the screen
  keypad(stdscr, TRUE); // Allows for arrow keys to be used
  curs_set(0);          // Hide cursor

  WINDOW *display = create_win_pad(10);

  WINDOW *sub = create_win_relative(display, 0.5, 0.5);

  // Use pad for menu win!

  box(display, 0, 0);
  box(sub, 0, 0);

  wrefresh(display);
  wrefresh(sub);

  wgetch(display);

  refresh(); // Refresh the stdscreen

  endwin(); // End ncurses

  return 0;
}

/*
  Creates a window relative to the size of window
*/
WINDOW *create_win_relative(WINDOW *win, float w_prc, float h_prc) {

  int x, y;
  get_win_size(win, &x, &y);

  // TOOD: calc relative cords
  WINDOW *new_win = derwin(win, y * h_prc, x * w_prc, 0, 0);

  return new_win;
}

/*
  Creates a middle centered window with margin
*/
WINDOW *create_win_pad(int mar) {
  int x, y;
  get_win_size(stdscr, &x, &y);

  return newwin(y - mar, x - mar, 5, 5);
}
