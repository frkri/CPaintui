#include <ctype.h>
#include <curses.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fs.c"
#include "logger.c"
#include "modal.c"
#include "win.c"

// TODO: Move to .h file
struct canvas_data *load_canvas_from_file(char *filename);
struct container *setup_display(void);
void draw_windows(struct container *container, int container_x_offset,
                  int container_y_offset);
int refresh_canvas_state(struct canvas_data *canvas_d,
                         struct container *display);
int show_help_modal(void);
int show_creation_modal(void);
bool show_quit_modal(void);
void exit_self(struct container *display, struct canvas_data *canvas_data,
               char *msg);

int main(int argc, char *argv[]) {
  // Check if atleast one argument is passed
  // Where argv[0] is the program name
  // and argv[1] is the first argument (filename)
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  // Initialize ncurses
  initscr();
  // Set Options
  cbreak();             // Stops buffering of Chars
  noecho();             // Stops chars from being printed to the screen
  keypad(stdscr, TRUE); // Allows for arrow keys to be used
  curs_set(0);          // Hide cursor

  // Enable colorwwwww
  start_color();
  init_pair(0, COLOR_BLACK, COLOR_BLACK);
  init_pair(1, COLOR_RED, COLOR_RED);
  init_pair(2, COLOR_GREEN, COLOR_GREEN);
  init_pair(3, COLOR_YELLOW, COLOR_YELLOW);
  init_pair(4, COLOR_BLUE, COLOR_BLUE);
  init_pair(5, COLOR_MAGENTA, COLOR_MAGENTA);
  init_pair(6, COLOR_CYAN, COLOR_CYAN);
  init_pair(7, COLOR_WHITE, COLOR_WHITE);

  // Init
  struct container *display = setup_display();
  // Create windows from layout
  draw_windows(display, 0, 0);
  setup_log(display->children[1]->children[1]->win);
  // Prompt user for canvas size if file does not exist
  struct canvas_data *canvas_data = load_canvas_from_file(argv[1]);
  if (canvas_data == NULL)
    exit_self(display, canvas_data,
              "Could not read file, check if file contains a valid canvas");

  // Update the canvas state
  refresh_canvas_state(canvas_data, display);
  log_info("Use H for help");

  // Input Handler
  while (true) {
    int ch = tolower(getch());
    switch (ch) {
    case KEY_RESIZE:
      clear();
      draw_windows(display, 0, 0);
      setup_log(display->children[1]->children[1]->win);
      refresh_canvas_state(canvas_data, display);
      break;
    case 'h':
      show_help_modal();
      log_info("Help window opened");
      refresh();
      break;
    case 'w':
      write_file(argv[1], serialize_buffer(canvas_data));
      log_info("Canvas saved to file");
      break;
    case 'q':
      if (show_quit_modal()) {
        log_info("Quit confirmed");
        exit_self(display, canvas_data, "Exiting...");
      }
      break;
    }
  }
}

void exit_self(struct container *display, struct canvas_data *canvas_data,
               char *msg) {
  // Free the logger
  cleanup_log();

  // Cleanup ncurses and data
  endwin();
  printf("%s\n", msg);

  if (display != NULL) {
    // Free the containers, this is maybe not needed, as the
    // OS will do this for us on exit
    free(display->children[0]->children);
    free(display->children[1]->children);
    free(display->children[0]);
    free(display->children[1]);
    free(display);
  }

  // Free the canvas data
  if (canvas_data != NULL) {
    for (int i = 0; i < canvas_data->width; i++)
      free(canvas_data->data[i]);
    free(canvas_data->data); // Free the parent array
    free(canvas_data);       // Free the struct
  }

  exit(0);
}

int show_help_modal(void) {
  struct modal modal_help = {
      "Help", "h - Open help window\n w - Save canvas to file", "Yy", true, 0,
      NULL};
  create_modal(modal_help);
  return 0;
}

int show_creation_modal(void) {
  // TODO: Add validation and handle multiple inputs
  // Maybe add new array just for input results?
  struct modal modal_canvas = {
      "Create new canvas", "Enter size of canvas (X-Y)", "", true, 15, NULL};
  create_modal(modal_canvas);
  printf("Received input: %s\n", modal_canvas.input);

  free(modal_canvas.input);
  return 0;
}

bool show_quit_modal(void) {
  // TODO: Check if canvas has been saved, if not prompt user to save
  struct modal modal_quit = {
      "Quit", "Are you sure you want to quit?\n\n\n\n\n\t\tY - Yes : N - No",
      "yY",   true,
      0,      NULL};
  return create_modal(modal_quit);
}

int refresh_canvas_state(struct canvas_data *canvas_d,
                         struct container *display) {
  WINDOW *canvas = display->children[0]->children[0]->win;
  wclear(canvas);

  for (int x = 0; x < canvas_d->width; x++)
    for (int y = 0; y < canvas_d->height; y++) {
      struct canvas_pixel *pixel = &canvas_d->data[x][y];

      // Get the size of the parent window
      int parent_w, parent_h;
      get_window_size(canvas, &parent_w, &parent_h);

      // Calculate the middle of the parent window
      int x_offset = (parent_w - canvas_d->width) / 2 + x;
      int y_offset = (parent_h - canvas_d->height) / 2 + y;

      // Set the color attribute
      wattron(canvas, COLOR_PAIR(pixel->color));

      // Draw the pixel
      mvwprintw(canvas, y_offset, x_offset, " ");

      // Reset the color attribute
      wattroff(canvas, COLOR_PAIR(pixel->color));

      // Copy the window to the virtual screen
      wnoutrefresh(canvas);
    }

  // Update the screen
  doupdate();
  return 0;
}

/*
  Defines and draws the layout of the windows
  Row 1: Canvas - Info
  Row 2: Tools  - Log
*/
struct container *setup_display(void) {
  // Main window
  struct container *display = malloc(sizeof(struct container));
  display->type = win;
  display->prc_w = 100;
  display->prc_h = 100;
  display->parent = stdscr;
  display->box = false;
  display->visible = true;
  display->title = NULL;
  display->children = NULL;
  display->children_len = 0;

  struct container *top_row = malloc(sizeof(struct container));
  display->type = win;
  top_row->prc_w = 100;
  top_row->prc_h = 75;
  top_row->title = NULL;
  top_row->box = false;
  top_row->visible = true;
  top_row->parent = NULL;
  top_row->children = NULL;
  top_row->children_len = 0;

  struct container *canvas = malloc(sizeof(struct container));
  display->type = win;
  canvas->prc_w = 80;
  canvas->prc_h = 100;
  canvas->title = NULL;
  canvas->box = false;
  canvas->visible = true;
  canvas->parent = NULL;
  canvas->children = NULL;
  canvas->children_len = 0;

  struct container *info = malloc(sizeof(struct container));
  display->type = win;
  info->prc_w = 20;
  info->prc_h = 100;
  info->title = "Info";
  info->box = true;
  info->visible = true;
  info->parent = NULL;
  info->children = NULL;
  info->children_len = 0;

  struct container *bottom_row = malloc(sizeof(struct container));
  display->type = win;
  bottom_row->prc_w = 100;
  bottom_row->prc_h = 25;
  bottom_row->title = NULL;
  bottom_row->box = true;
  bottom_row->visible = true;
  bottom_row->parent = NULL;
  bottom_row->children = NULL;

  struct container *tools = malloc(sizeof(struct container));
  display->type = win;
  tools->prc_w = 80;
  tools->prc_h = 100;
  tools->title = "Tools";
  tools->box = true;
  tools->visible = true;
  tools->parent = NULL;
  tools->children = NULL;
  tools->children_len = 0;

  struct container *log = malloc(sizeof(struct container));
  display->type = pad;
  log->prc_w = 20;
  log->prc_h = 100;
  log->title = "Log";
  log->box = true;
  log->visible = true;
  log->parent = NULL;
  log->children = NULL;
  log->children_len = 0;

  // Append children to display
  append_container(display, top_row);
  append_container(display, bottom_row);

  // Append children to top row
  append_container(top_row, canvas);
  append_container(top_row, info);

  // Append children to bottom row
  append_container(bottom_row, tools);
  append_container(bottom_row, log);

  return display;
}

// Creates the windows for the container and its children
void draw_windows(struct container *container, int container_x_offset,
                  int container_y_offset) {

  if (container->visible == false)
    return;

  // Get the size of the parent window
  int parent_w, parent_h;
  get_window_size(container->parent, &parent_w, &parent_h);

  // Calculate the width and height
  const int w = parent_w * container->prc_w / 100;
  const int h = parent_h * container->prc_h / 100;

  if (container->type == win) {
    // Create the window
    container->win =
        derwin(container->parent, h, w, container_y_offset, container_x_offset);
  } else {
    // Create the pad window
    container->win =
        derwin(container->parent, h, w, container_y_offset, container_x_offset);
  }

  // Draw box if needed
  if (container->box)
    box(container->win, 0, 0);

  // Print the title
  if (container->title != NULL)
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

    draw_windows(container->children[i], x, y);

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

/*
  Will load a canvas from a file, if the file does not exist, it will create a
  new blank canvas file
*/
struct canvas_data *load_canvas_from_file(char *filename) {
  char *buffer = load_file(filename);
  if (buffer == NULL) {
    log_info("Failed to load file");

    // Create empty canvas data, by asigning the buffer to a default value
    buffer =
        "5,5;1,1;2,2;5,1;2,2;1,1;2,2;8,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,"
        "1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;"
        "1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,"
        "2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;"
        "2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,"
        "1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;"
        "1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,2;1,1;2,"
        "2;1,1;2,2;1,1;2,2;1,1;2,2;";

    // Save the new empty canvas to the file
    write_file(filename, buffer);
    sleep(2);
  }
  struct canvas_data *canvas_d = deserialize_buffer(buffer);
  if (canvas_d == NULL) {
    log_info("Failed to deserialize buffer");
    return NULL;
  }

  free(buffer);

  return canvas_d;
}
