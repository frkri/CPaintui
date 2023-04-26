#include <curses.h>
#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>

int display_modal(char *msg);

int main(void) {
  initscr(); // Initialize ncurses
  cbreak();  // Stops buffering of Chars
  noecho();  // Stops chars from being printed to the screen

  printw("Hello World!\n");
  refresh();
  clear();

  // Prepare Screen for next refresh
  display_modal("Hello Again!");

  getch(); // Wait for user input
  refresh();

  getch();

  // End ncurses
  endwin();

  return 0;
}

int draw_base() { return 0; }

int display_modal(char *msg) {

  int Y = getmaxy(stdscr);
  int X = getmaxx(stdscr);

  printw("X: %i, Y: %i\n", X, Y);

  mvprintw(15, 200, "MSG: %s\n", msg);

  return 0;
}
