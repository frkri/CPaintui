#include <curses.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fs.c"
#include "util.c"

struct container setup_windows(void);

void create_windows(struct container *container, int container_x_offset,
                    int container_y_offset);

void load_canvas_from_file(char *filename, struct canvas_data *canvas_d);

int main(int argc, char *argv[]) {
  // Check if atleast one argument is passed
  // Where argv[0] is the program name
  // and argv[1] is the first argument (filename)
  if (argc < 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  struct canvas_data canvas_d = {0, 0, NULL};

  load_canvas_from_file(argv[1], &canvas_d);

  printf("deserialized file!\n");

  printf("%d\n", canvas_d.width);
  printf("%d\n", canvas_d.height);

  printf("%d\n", canvas_d.data[0][0].color);
  printf("%d\n", canvas_d.data[0][0].last_modified);

  printf("%d\n", canvas_d.data[0][1].color);
  printf("%d\n", canvas_d.data[0][1].last_modified);

  sleep(9999999);

  initscr(); // Initialize ncurses

  // Set Options
  cbreak();             // Stops buffering of Chars
  noecho();             // Stops chars from being printed to the screen
  keypad(stdscr, TRUE); // Allows for arrow keys to be used
  curs_set(0);          // Hide cursor

  // Setup windows
  struct container display = setup_windows();
  struct container *canvas = display.children[0]->children[0];
  struct container *info = display.children[0]->children[1];
  struct container *tools = display.children[1]->children[0];
  struct container *log = display.children[1]->children[1];

  // Wait for input
  int ch;

  while ((ch = getch()) != 'q') {

    switch (ch) {
    case KEY_RESIZE:
      clear();
      display = setup_windows();
    default:
      break;
    }

    // Update the screen
    refresh();
  }

  endwin(); // End ncurses
  return 0;
}

/*
  Defines and draws the layout of the windows
  Row 1: Canvas - Info
  Row 2: Tools  - Log
*/
struct container setup_windows(void) {
  // Main window
  struct container display = {100,    100,  NULL, true, false,
                              stdscr, NULL, NULL, 0};

  // Top row
  struct container top_row = {100, 80, NULL, true, false, NULL, NULL, NULL, 0};
  struct container canvas = {70,   100,  "Canvas", true, false,
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

void load_canvas_from_file(char *filename, struct canvas_data *canvas) {
  // Will be allocated by load_file
  char *buffer = NULL;

  buffer = load_file(filename);
  if (buffer == NULL) {
    printf("Error: Could not load file\n");
    exit(1);
  }

  printf("Buffer: %s\n\n\n\n", buffer);

  deserialize_buffer(canvas, buffer);

  free(buffer);
}
