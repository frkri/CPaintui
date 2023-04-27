#include <curses.h>
#include <ncurses.h>

int get_win_size(WINDOW *win, int *x, int *y) {
  *x = getmaxx(win);
  *y = getmaxy(win);

  return 0;
}
