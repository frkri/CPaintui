#include <ctype.h>
#include <curses.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cursor.h"
#include "fs.h"
#include "logger.h"
#include "main.h"
#include "modal.h"
#include "util.h"
#include "win.h"

int main(int argc, char *argv[]) {
  // Where argv[0] is the program name
  // and argv[1] is the first argument (filename).
  // Rest of the arguments are ignored
  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  // Initialize ncurses
  initscr();
  cbreak();             // Stops buffering of Chars
  noecho();             // Stops chars from being printed to the screen
  keypad(stdscr, TRUE); // Allows for arrow keys to be used
  curs_set(0);          // Hide cursor

  // Enable color
  start_color();
  init_pair(0, COLOR_WHITE, COLOR_WHITE);
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_BLACK, COLOR_RED);
  init_pair(3, COLOR_BLACK, COLOR_YELLOW);
  init_pair(4, COLOR_BLACK, COLOR_GREEN);
  init_pair(5, COLOR_BLACK, COLOR_CYAN);
  init_pair(6, COLOR_BLACK, COLOR_BLUE);
  init_pair(7, COLOR_BLACK, COLOR_MAGENTA);

  struct container *display = setup_display();
  draw_windows(display, 0, 0);
  setup_log(display->children[1]->children[1]->win);

  int w, h;
  get_window_size(display->children[0]->children[0]->win, &w, &h);
  struct canvas_data *canvas_data = safe_load_canvas_from_file(argv[1], w, h);
  if (canvas_data == NULL)
    exit_self(display, canvas_data, NULL,
              "Could not read file, check if file contains a valid canvas");

  // Update the canvas state
  refresh_canvas_state(canvas_data, display);
  struct cursor *cursor =
      setup_cursor(display->children[0]->children[0]->win, canvas_data);

  log_info("Use H for help");
  if (has_colors() == false)
    log_info("Your terminal does not support color!");

  // Input Handler
  while (true) {
    int ch = tolower(getch());
    switch (ch) {
    case ' ':
      cursor->is_active = !cursor->is_active;
      draw_cursor(canvas_data, cursor, 0, 0);
      break;
    case '0' ... '7':
      cursor->color = ch - '0';
      draw_cursor(canvas_data, cursor, 0, 0);
      break;
    case KEY_UP:
      draw_cursor(canvas_data, cursor, 0, -1);
      break;
    case KEY_DOWN:
      draw_cursor(canvas_data, cursor, 0, 1);
      break;
    case KEY_LEFT:
      draw_cursor(canvas_data, cursor, -1, 0);
      break;
    case KEY_RIGHT:
      draw_cursor(canvas_data, cursor, 1, 0);
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
      if (show_quit_modal())
        exit_self(display, canvas_data, cursor, "Exiting...");
      break;
    case KEY_RESIZE:
      clear();
      draw_windows(display, 0, 0);
      setup_log(display->children[1]->children[1]->win);
      setup_cursor(display->children[0]->children[0]->win, canvas_data);
      refresh_canvas_state(canvas_data, display);
      break;
    }
  }
  return 0;
}

void exit_self(struct container *display, struct canvas_data *canvas_data,
               struct cursor *cursor, char *msg) {
  cleanup_log();
  endwin();
  printf("%s\n", msg);

  // Free the cursor
  if (cursor != NULL)
    free(cursor);

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
      "Help",
      "H\t\t- Open help window\n W\t\t- Save canvas to "
      "file\n Arrow Keys\t- Move cursor\n Space\t\t- Switch between cursor"
      "modes\n 0-8\t\t- Select color",
      "Yy",
      true,
      0,
      NULL};
  create_modal(modal_help);
  return 0;
}

bool show_quit_modal(void) {
  struct modal modal_quit = {"Quit",
                             "Are you sure you want to quit?\n Any unsaved "
                             "data will be lost!\n\n\n\n\t\tY - Yes : N - No",
                             "yY",
                             true,
                             0,
                             NULL};
  return create_modal(modal_quit);
}

int refresh_canvas_state(struct canvas_data *canvas_d,
                         struct container *display) {
  WINDOW *canvas = display->children[0]->children[0]->win;
  wclear(canvas);

  for (int x = 0; x < canvas_d->width; x++)
    for (int y = 0; y < canvas_d->height; y++) {
      struct canvas_pixel *pixel = &canvas_d->data[x][y];

      int parent_w, parent_h;
      get_window_size(canvas, &parent_w, &parent_h);

      // Calculate the middle of the parent window
      int x_offset = (parent_w - canvas_d->width) / 2 + x;
      int y_offset = (parent_h - canvas_d->height) / 2 + y;

      wattron(canvas, COLOR_PAIR(pixel->color));
      mvwprintw(canvas, y_offset, x_offset, " ");
      wattroff(canvas, COLOR_PAIR(pixel->color));

      // Copy the window to the virtual screen
      wnoutrefresh(canvas);
    }

  doupdate();
  return 0;
}

/*
  Defines the layout of the windows
  Row 1: Canvas - Info
  Row 2: Tools  - Log
*/
struct container *setup_display(void) {
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
  tools->title = "Colors";
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

  // Append rows to display
  append_container(display, top_row);
  append_container(display, bottom_row);

  // Append panels to top row
  append_container(top_row, canvas);
  append_container(top_row, info);

  // Append panels to bottom row
  append_container(bottom_row, tools);
  append_container(bottom_row, log);

  return display;
}

/*
  Creates & displays the windows from a
  struct representing the layout
*/
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
struct canvas_data *safe_load_canvas_from_file(char *filename, int w, int h) {
  char *buffer = load_file(filename);
  if (buffer == NULL) {
    log_info("File not found, creating new blank canvas");
    buffer = create_buffer(w, h, 4);
    write_file(filename, buffer);
    buffer = load_file(filename);
  }

  struct canvas_data *canvas_d = deserialize_buffer(buffer);
  if (canvas_d == NULL) {
    log_info("Failed to deserialize buffer");
    return NULL;
  }

  free(buffer);
  return canvas_d;
}
