#include "stdlib.h"

#include "cursor.h"
#include "fs.h"
#include "logger.h"
#include "win.h"
#include <stdio.h>

struct cursor *setup_cursor(WINDOW *canvas, struct canvas_data *canvas_data) {
  int w, h;
  get_window_size(canvas, &w, &h);

  struct cursor *cursor = malloc(sizeof(struct cursor));
  cursor->canvas = canvas;
  cursor->is_active = false;

  cursor->x = w / 2;
  cursor->y = h / 2;
  cursor->color = 5;

  cursor->prev_x = 0;
  cursor->prev_y = 0;
  cursor->prev_color = 0;

  draw_cursor(canvas_data, cursor, 0, 0);

  char temp[100];
  sprintf(temp, "Cursor created at (%d, %d)", cursor->x, cursor->y);
  log_info(temp);

  return cursor;
}

bool draw_cursor(struct canvas_data *canvas_data, struct cursor *cursor,
                 int x_offset, int y_offset) {
  // Out of bounds check
  int w, h;
  get_window_size(cursor->canvas, &w, &h);
  if (cursor->x + x_offset >= canvas_data->width || cursor->x + x_offset < 0 ||
      cursor->y + y_offset >= canvas_data->height || cursor->y + y_offset < 0)
    return false;

  // Set new cursor position
  cursor->x += x_offset;
  cursor->y += y_offset;

  // Restore last color of cursor
  if (cursor->is_active) {
    wattron(cursor->canvas, COLOR_PAIR(cursor->color));
    mvwprintw(cursor->canvas, cursor->prev_y, cursor->prev_x, " ");
    wattroff(cursor->canvas, COLOR_PAIR(cursor->color));
  } else {
    wattron(cursor->canvas, COLOR_PAIR(cursor->prev_color));
    mvwprintw(cursor->canvas, cursor->prev_y, cursor->prev_x, " ");
    wattroff(cursor->canvas, COLOR_PAIR(cursor->prev_color));
  }

  // Update cursor position
  cursor->prev_x = cursor->x;
  cursor->prev_y = cursor->y;
  chtype ch = mvwinch(cursor->canvas, cursor->y, cursor->x);
  cursor->prev_color = PAIR_NUMBER(ch & A_COLOR);

  // Move cursor to position on canvas
  wattron(cursor->canvas, COLOR_PAIR(cursor->color));
  if (cursor->is_active) {
    mvwprintw(cursor->canvas, cursor->y, cursor->x, "*");
    canvas_data->data[cursor->x][cursor->y].color = cursor->color;
  } else
    mvwprintw(cursor->canvas, cursor->y, cursor->x, "~");
  wattroff(cursor->canvas, COLOR_PAIR(cursor->color));

  wrefresh(cursor->canvas);
  return true;
}
