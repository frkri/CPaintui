#include <curses.h>
#include <ncurses.h>
#include <stdio.h>

#include "util.c"

struct container setup_windows(void);
void create_windows(struct container *container, int xx, int yy);

int main(void) {
  initscr(); // Initialize ncurses

  // Set Options
  cbreak();             // Stops buffering of Chars
  noecho();             // Stops chars from being printed to the screen
  keypad(stdscr, TRUE); // Allows for arrow keys to be used
  curs_set(0);          // Hide cursor

  struct container display = setup_windows();

  wgetch(display.children[0]->children[0]->win);

  endwin(); // End ncurses
  return 0;
}

/*
  Defines and draws the layout of the windows
  Row 1: Canvas - Info
  Row 2: Tools - Log
*/
struct container setup_windows(void) {
  // Main window
  struct container display = {100,    100,  NULL, true, false,
                              stdscr, NULL, NULL, 0};

  // Top row
  struct container top_row = {100, 80, NULL, true, false, NULL, NULL, NULL, 0};
  struct container canvas = {70,   100,  "Canvas", true, true,
                             NULL, NULL, NULL,     0};
  struct container info = {30, 100, "Info", true, true, NULL, NULL, NULL, 0};

  // Bottom row
  struct container bottom_row = {100,  20,   NULL, true, false,
                                 NULL, NULL, NULL, 0};
  struct container tools = {70, 100, "Tools", true, true, NULL, NULL, NULL, 0};
  struct container log = {30, 100, "Log", true, true, NULL, NULL, NULL, 0};

  // Append rows to display
  append_container(&display, &top_row);
  append_container(&display, &bottom_row);

  // Append containers to top row
  append_container(&top_row, &canvas);
  append_container(&top_row, &info);

  // Append containers to bottom row
  append_container(&bottom_row, &tools);
  append_container(&bottom_row, &log);

  // Create windows from layout
  create_windows(&display, 0, 0);

  return display;
}

// Creates the windows for the container and its children
void create_windows(struct container *container, int container_x_offset,
                    int container_y_offset) {

  // Get the size of the parent window
  int parent_w, parent_h;
  get_window_size(container->parent, &parent_w, &parent_h);

  // Calculate the width and height
  const int w = parent_w * container->prc_w / 100;
  const int h = parent_h * container->prc_h / 100;

  // Create the window
  container->win =
      derwin(container->parent, h, w, container_y_offset, container_x_offset);

  // Draw box if needed
  if (container->box)
    box(container->win, 0, 0);

  // Print the title
  mvwprintw(container->win, 0, 1, " %s ", container->title);

  // Copy the window to the virtual screen
  // See: https://www.man7.org/linux/man-pages/man3/curs_refresh.3x.html
  wnoutrefresh(container->win);

  int x_offset = 0;
  int y_offset = 0;

  int x = 0;
  int y = 0;

  // Create the child windows
  for (int i = 0; i < container->children_len; i++) {
    container->children[i]->parent = container->win;

    create_windows(container->children[i], x, y);

    // Add offset for the next child window
    x_offset += container->children[i]->prc_w;
    y_offset += container->children[i]->prc_h;

    if (x_offset >= 100)
      x_offset = 0;
    if (y_offset >= 100)
      y_offset = 0;

    // Calculate the x and y coordinates of the child windows
    x = x_offset * w / 100;
    y = y_offset * h / 100;
  }

  // Finally, update the screen
  doupdate();
}
