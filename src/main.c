#include <curses.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs.c"
#include "modal.c"
#include "util.c"

// TODO: Move to .h file
struct canvas_data *load_canvas_from_file(char *filename);
struct container setup_windows(void);
void create_windows(struct container *container, int container_x_offset,
                    int container_y_offset);
int refresh_canvas_state(struct canvas_data *canvas_d,
                         struct container display);
int show_help_modal(void);
int show_canvas_creation_modal(void);

int main(int argc, char *argv[]) {
  // Check if atleast one argument is passed
  // Where argv[0] is the program name
  // and argv[1] is the first argument (filename)
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  struct canvas_data *canvas_data = load_canvas_from_file(argv[1]);

  initscr(); // Initialize ncurses

  // Set Options
  cbreak();             // Stops buffering of Chars
  noecho();             // Stops chars from being printed to the screen
  keypad(stdscr, TRUE); // Allows for arrow keys to be used
  curs_set(0);          // Hide cursor

  // TODO: Check for color support and make this configurable
  start_color(); // Enable color
  init_pair(0, COLOR_BLACK, COLOR_BLACK);
  init_pair(1, COLOR_RED, COLOR_RED);
  init_pair(2, COLOR_GREEN, COLOR_GREEN);
  init_pair(3, COLOR_YELLOW, COLOR_YELLOW);
  init_pair(4, COLOR_BLUE, COLOR_BLUE);
  init_pair(5, COLOR_MAGENTA, COLOR_MAGENTA);
  init_pair(6, COLOR_CYAN, COLOR_CYAN);
  init_pair(7, COLOR_WHITE, COLOR_WHITE);

  struct container display = setup_windows();
  refresh_canvas_state(canvas_data, display);

  // Wait for input
  int ch;

  while ((ch = getch()) != 'q') {
    mvwprintw(stdscr, 3, 2, "char: %i", ch);

    switch (ch) {
    case KEY_RESIZE:
      clear();
      setup_windows();
      refresh_canvas_state(canvas_data, display);
      // Update the screen
      refresh();
      break;
    case 'h':
      show_help_modal();
      break;
    case 'w':
      show_canvas_creation_modal();
    default:
      break;
    }
  }

  endwin(); // End ncurses

  // Free the containers
  free(display.children);

  // Free the canvas data
  for (int i = 0; i < canvas_data->width; i++)
    free(canvas_data->data[i]);
  free(canvas_data->data);
  free(canvas_data);

  printf("Done\n");

  return 0;
}

int show_help_modal(void) {
  struct modal modal_help = {
      "Help", "h - Open help window\n w - Write current canvas to file",
      "yY",   true,
      0,      NULL};

  if (create_modal_new(&modal_help))
    printf("User acknowledged\n");
  else
    printf("User did not acknowledge\n");

  return 0;
}

int show_canvas_creation_modal(void) {
  // TODO: Add validation and handle multiple inputs
  // Maybe add new array just for input results?
  struct modal modal_canvas = {
      "Create new canvas", "Enter size of canvas (X-Y)", "", true, 15, NULL};

  create_modal_new(&modal_canvas);
  printf("Received input: %s\n", modal_canvas.input);

  refresh();
  return 0;
}

int refresh_canvas_state(struct canvas_data *canvas_d,
                         struct container display) {
  WINDOW *canvas = display.children[0]->children[0]->win;
  // This fails
  // mvprintw(0, 0, "Canvas: %s\n", display.children[0]->children[0]->title);

  for (int x = 0; x < canvas_d->width; x++)
    for (int y = 0; y < canvas_d->height; y++) {
      struct canvas_pixel *pixel = &canvas_d->data[x][y];

      // Get the size of the parent window
      int parent_w, parent_h;
      get_window_size(canvas, &parent_w, &parent_h);

      // Calculate the middle of the parent window
      int x_offset = (parent_w - canvas_d->width) / 2 + x;
      int y_offset = (parent_h - canvas_d->height) / 2 + y;

      mvprintw(0, 0, " %d %d ", parent_w, parent_h);
      mvwprintw(canvas, 1, 0, " %d %d ", x_offset, y_offset);

      // Set the color attribute
      wattron(canvas, COLOR_PAIR(pixel->color));

      // Draw the pixel
      mvwprintw(canvas, y_offset, x_offset, " ");

      // Reset the color attribute
      wattroff(canvas, COLOR_PAIR(pixel->color));

      // Copy the window to the virtual screen
      wnoutrefresh(canvas);
    }

  // Finally, update the screen
  doupdate();
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
  struct container canvas = {80,   100,  "Canvas", true, false,
                             NULL, NULL, NULL,     0};
  struct container info = {20, 100, "Info", true, true, NULL, NULL, NULL, 0};

  // Bottom row
  struct container bottom_row = {100,  20,   NULL, true, false,
                                 NULL, NULL, NULL, 0};
  struct container tools = {80, 100, "Tools", true, true, NULL, NULL, NULL, 0};
  struct container log = {20, 100, "Log", true, true, NULL, NULL, NULL, 0};

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

struct canvas_data *load_canvas_from_file(char *filename) {
  char *buffer = load_file(filename);
  if (buffer == NULL) {
    printf("Error: Could not load file\n");
    exit(1);
  }

  struct canvas_data *canvas_d = deserialize_buffer(buffer);

  free(buffer);

  return canvas_d;
}
