#include <curses.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

struct container {
  // Percentage of parent window
  int prc_w;
  int prc_h;

  // Title of window
  char *title;

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
int get_window_size(WINDOW *win, int *w, int *h) {
  *w = getmaxx(win);
  *h = getmaxy(win);

  return 0;
}

/*
  Creates a middle aligned window with margin
  Takes in whole margin width and height of window
*/
WINDOW *create_window_centered(WINDOW *win, int margin_w, int margin_h) {
  int w, h;
  get_window_size(win, &w, &h);

  return newwin(h - margin_h, w - margin_w, margin_h / 2, margin_w / 2);
}

/*
  Creates a inner window relative to the size of window
*/
WINDOW *create_window_relative(WINDOW *win, float w_prc, float h_prc) {
  int w, h;
  get_window_size(win, &w, &h);

  int win_w = w * w_prc;
  int win_h = h * h_prc;
  int start_x = w * w_prc - (float)win_w / 2;
  int start_y = h * h_prc - (float)win_h / 2;

  WINDOW *new_win = derwin(win, win_h, win_w, start_y, start_x);

  return new_win;
}

/*
  Appends a child container to a parent container
  This will reallocate memory of the array for the new child container, as we
  don't know how many children a container will have
*/
int append_container(struct container *parent, struct container *child) {
  // Reallocate memory block for new child

  // This will be the size of the old array + 1
  int new_size = sizeof(struct container *) * (parent->children_len + 1);

  parent->children = realloc(parent->children, new_size);
  if (parent->children == NULL) {
    printf("Could not allocate memory for new child container");
    return 1;
  }

  // Append child to second to last index, leaving the last index empty
  parent->children[parent->children_len] = child;

  // Increment length of array
  parent->children_len++;
  return 0;
}
