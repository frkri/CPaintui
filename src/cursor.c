#include <curses.h>

#include "fs.h"
#include "logger.h"
#include "win.h"

bool draw_mode = false;
int prev_cursor_x = 2;
int prev_cursor_y = 2;
int prev_cursor_color = 0;

bool move_cursor(WINDOW *canvas, struct canvas_data *canvas_data, int x, int y,
                 int color) {
  int parent_w, parent_h;
  get_window_size(canvas, &parent_w, &parent_h);

  // Calculate the middle of the parent window
  int x_offset = (parent_w - canvas_data->width) / 2 + x;
  int y_offset = (parent_h - canvas_data->height) / 2 + y;

  // Out of bounds check
  int w, h;
  get_window_size(canvas, &w, &h);
  if (x < 0 || x >= w || y < 0 || y >= h)
    return false;

  // Restore last color of cursor
  if (draw_mode) {
    wattron(canvas, COLOR_PAIR(color));
    mvwprintw(canvas, prev_cursor_y, prev_cursor_x, " ");
    wattroff(canvas, COLOR_PAIR(color));
  } else {
    wattron(canvas, COLOR_PAIR(prev_cursor_color));
    mvwprintw(canvas, prev_cursor_y, prev_cursor_x, " ");
    wattroff(canvas, COLOR_PAIR(prev_cursor_color));
  }

  // Update cursor position
  prev_cursor_x = x;
  prev_cursor_y = y;
  chtype ch = mvwinch(canvas, y, x);
  prev_cursor_color = PAIR_NUMBER(ch & A_COLOR);

  // Move cursor to position on canvas
  wattron(canvas, COLOR_PAIR(color));
  if (draw_mode) {
    mvwprintw(canvas, y, x, "X");
    canvas_data->data[x][y].color = color;
  } else
    mvwprintw(canvas, y, x, "O");
  wattroff(canvas, COLOR_PAIR(color));

  wrefresh(canvas);
  return true;
}

bool toggle_draw_mode() {
  draw_mode = !draw_mode;
  log_info("Draw mode aa");
  return draw_mode;
}
