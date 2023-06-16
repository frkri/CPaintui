#include <ctype.h>
#include <curses.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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
  cbreak();             // Stops buffering of chars
  noecho();             // Stops chars from being printed to the screen
  keypad(stdscr, true); // Allows for arrow keys to be used
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
              "Could not read/parse file due to invalid format");

  // Update the canvas state
  refresh_canvas_state(canvas_data, display);
  struct cursor *cursor =
      setup_cursor(display->children[0]->children[0]->win, canvas_data);

  // Log stuff
  log_info("Use H for help");
  if (has_colors() == false)
    log_info("Your terminal does not support color!");
  if (canvas_data->width > w || canvas_data->height > h)
    log_info("Terminal is too small for canvas, consider resizing it");

  // Input handler
  while (true) {
    refresh_info(display->children[1]->children[0]->win, argv[1], canvas_data,
                 cursor);
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
    case 'r':
      refresh_canvas_state(canvas_data, display);
      log_info("Canvas refreshed");
      break;
    case 'h':
      show_help_modal();
      refresh_canvas_state(canvas_data, display);
      break;
    case 'w':
      write_file(argv[1], serialize_buffer(canvas_data));
      log_info("Canvas saved to file");
      break;
    case 'q':
      if (show_quit_modal())
        exit_self(display, canvas_data, cursor, "Exiting...");
      refresh_canvas_state(canvas_data, display);
      break;
    case KEY_RESIZE:
      // Perform a full rerender of the screen
      clear();
      draw_windows(display, 0, 0);
      setup_log(display->children[1]->children[1]->win);
      if (cursor != NULL)
        free(cursor);
      cursor =
          setup_cursor(display->children[0]->children[0]->win, canvas_data);
      int w, h;
      get_window_size(display->children[0]->children[0]->win, &w, &h);
      if (canvas_data->width > w || canvas_data->height > h)
        log_info("Terminal is too small for canvas, consider resizing it");
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

void refresh_info(WINDOW *aside, char *filename,
                  struct canvas_data *canvas_data, struct cursor *cursor) {
  int w, h;
  get_window_size(aside, &w, &h);

  werase(aside);
  box(aside, 0, 0);
  mvwprintw(aside, 0, 1, " Info ");
  mvwprintw(aside, 1, 1, "Filename:\t%.*s", w - 17, filename);
  mvwprintw(aside, 2, 1, "Canvas:\t%ix%i", canvas_data->width,
            canvas_data->height);
  mvwprintw(aside, 3, 1, "Mode:\t\t%s", cursor->is_active ? "Draw" : "Move");
  mvwprintw(aside, 4, 1, "Position:\tx%i y%i", cursor->x, cursor->y);
  mvwprintw(
      aside, 5, 1, "Modified:\t%.*s", w - 17,
      format_timestamp(canvas_data->data[cursor->x][cursor->y].last_modified));
  mvwprintw(aside, 6, 1, "Color:\t\t");
  wattron(aside, COLOR_PAIR(cursor->color));
  wprintw(aside, "%i", cursor->color);
  wattroff(aside, COLOR_PAIR(cursor->color));

  wrefresh(aside);
}

int show_help_modal(void) {
  struct modal modal_help = {
      "Help",
      "H\t\t- Open help window\n W\t\t- Save canvas to "
      "file\n R\t\t- Refresh canvas\n Arrow Keys\t- Move "
      "cursor\n Space\t\t- Switch modes"
      "\n 0-7\t\t- Select color",
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
                             "data will be lost!\n\n\n\n\t   Y - Yes : N - No",
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

  struct container *first_column = malloc(sizeof(struct container));
  display->type = win;
  first_column->prc_w = 75;
  first_column->prc_h = 100;
  first_column->title = NULL;
  first_column->box = false;
  first_column->visible = true;
  first_column->parent = NULL;
  first_column->children = NULL;
  first_column->children_len = 0;

  struct container *canvas = malloc(sizeof(struct container));
  display->type = win;
  canvas->prc_w = 100;
  canvas->prc_h = 100;
  canvas->title = NULL;
  canvas->box = false;
  canvas->visible = true;
  canvas->parent = NULL;
  canvas->children = NULL;
  canvas->children_len = 0;

  struct container *second_column = malloc(sizeof(struct container));
  display->type = win;
  second_column->prc_w = 25;
  second_column->prc_h = 100;
  second_column->title = NULL;
  second_column->box = true;
  second_column->visible = true;
  second_column->parent = NULL;
  second_column->children = NULL;

  struct container *info = malloc(sizeof(struct container));
  display->type = win;
  info->prc_w = 100;
  info->prc_h = 70;
  info->title = "Info";
  info->box = true;
  info->visible = true;
  info->parent = NULL;
  info->children = NULL;
  info->children_len = 0;

  struct container *log = malloc(sizeof(struct container));
  display->type = pad;
  log->prc_w = 100;
  log->prc_h = 30;
  log->title = "Log";
  log->box = true;
  log->visible = true;
  log->parent = NULL;
  log->children = NULL;
  log->children_len = 0;

  // Append rows to display
  append_container(display, first_column);
  append_container(display, second_column);

  // Append panels to top row
  append_container(first_column, canvas);

  // Append panels to bottom row
  append_container(second_column, info);
  append_container(second_column, log);

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
    buffer = create_buffer(w, h, 0, time(NULL));
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
