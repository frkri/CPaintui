#include "stdlib.h"

#include "cursor.h"
#include "fs.h"
#include "logger.h"
#include "win.h"
#include <stdio.h>
#include <time.h>

struct cursor *setup_cursor(WINDOW *canvas, struct canvas_data *canvas_data) {
  int w, h;
  get_window_size(canvas, &w, &h);

  struct cursor *cursor = malloc(sizeof(struct cursor));
  cursor->canvas = canvas;
  cursor->is_active = false;

  cursor->x = w / 2;
  cursor->y = h / 2;
  cursor->color = 0;

  cursor->prev_x = w / 2;
  cursor->prev_y = h / 2;
  // Get 'pixel' at cursor position
  chtype ch = mvwinch(cursor->canvas, cursor->y, cursor->x);
  cursor->prev_color = PAIR_NUMBER(ch & A_COLOR); // Bitwise AND to get color

  draw_cursor(canvas_data, cursor, 0, 0);
  return cursor;
}

bool draw_cursor(struct canvas_data *canvas_data, struct cursor *cursor,
                 int x_move, int y_move) {
  int w, h;
  get_window_size(cursor->canvas, &w, &h);

  // Window bounds, area where the canvas begins and ends
  int x_win_begin;
  int y_win_begin;
  int x_win_end;
  int y_win_end;

  // In case the canvas is smaller than the window
  int x_win_begin_offset;
  int y_win_begin_offset;

  if (canvas_data->width < w) {
    x_win_begin = (w - canvas_data->width) / 2;
    x_win_end = x_win_begin + canvas_data->width;
    x_win_begin_offset = 0;
  } else {
    x_win_begin = 0;
    x_win_end = w;

    x_win_begin_offset = (canvas_data->width - w) / 2;
  }

  if (canvas_data->height < h) {
    y_win_begin = (h - canvas_data->height) / 2;
    y_win_end = y_win_begin + canvas_data->height;
    y_win_begin_offset = 0;
  } else {
    y_win_begin = 0;
    y_win_end = h;
    y_win_begin_offset = (canvas_data->height - h) / 2;
  }

  // Out of bounds check
  if (cursor->x + x_move >= x_win_end || cursor->x + x_move < x_win_begin ||
      cursor->y + y_move >= y_win_end || cursor->y + y_move < y_win_begin)
    return false;

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

  // Set new cursor position
  cursor->x += x_move;
  cursor->y += y_move;

  // Update previous cursor position
  cursor->prev_x = cursor->x;
  cursor->prev_y = cursor->y;
  // Get 'pixel' at cursor position
  chtype ch = mvwinch(cursor->canvas, cursor->y, cursor->x);
  cursor->prev_color = PAIR_NUMBER(ch & A_COLOR); // Bitwise AND to get color

  // Move cursor to position on canvas
  wattron(cursor->canvas, COLOR_PAIR(cursor->color));
  if (cursor->is_active) {
    mvwprintw(cursor->canvas, cursor->y, cursor->x, "*");
    canvas_data
        ->data[cursor->x - x_win_begin + x_win_begin_offset]
              [cursor->y - y_win_begin + y_win_begin_offset]
        .color = cursor->color;
    canvas_data
        ->data[cursor->x - x_win_begin + x_win_begin_offset]
              [cursor->y - y_win_begin + y_win_begin_offset]
        .last_modified = time(NULL);
  } else
    mvwprintw(cursor->canvas, cursor->y, cursor->x, "~");
  wattroff(cursor->canvas, COLOR_PAIR(cursor->color));

  wrefresh(cursor->canvas);
  return true;
}
