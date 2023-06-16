#ifndef CURSOR_H
#define CURSOR_H

#include "fs.h"
#include <curses.h>

struct cursor {
  WINDOW *canvas;
  bool is_active;

  int x;
  int y;
  int color;

  int prev_x;
  int prev_y;
  int prev_color;
};

struct cursor *setup_cursor(WINDOW *canvas, struct canvas_data *canvas_d);
bool draw_cursor(struct canvas_data *canvas_data, struct cursor *cursor,
                 int x_offset, int y_offset);

#endif
