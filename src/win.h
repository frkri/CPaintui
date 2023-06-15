#include <ncurses.h>

#ifndef WIN_H
#define WIN_H
// Window type enum
enum window_type { win, pad };

struct container {
  // Percentage of parent window
  int prc_w;
  int prc_h;

  // Title of window
  char *title;

  // Type of window
  enum window_type type;

  // If window should be visible
  bool visible;

  // If box should be drawn around window
  bool box;

  // Parent window
  WINDOW *parent;

  // Window
  WINDOW *win;

  // Array of child containers
  struct container **children;
  int children_len;
};

/*
  Gets the size of a window
*/
int get_window_size(WINDOW *win, int *w, int *h);

/*
  Creates a middle aligned window with margin
  Takes in whole margin width and height of window
*/
WINDOW *create_window_centered(WINDOW *win, int margin_w, int margin_h);

/*
  Creates a inner window relative to the size of window
*/
WINDOW *create_window_relative(WINDOW *win, float w_prc, float h_prc);

/*
  Appends a child container to a parent container
  This will reallocate memory of the array for the new child container, as we
  don't know how many total children a container will have
*/
int append_container(struct container *parent, struct container *child);

#endif
