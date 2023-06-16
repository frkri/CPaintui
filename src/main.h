#ifndef MAIN_H
#define MAIN_H

#include <curses.h>
struct canvas_data *safe_load_canvas_from_file(char *filename, int w, int h);
struct container *setup_display(void);
void draw_windows(struct container *container, int container_x_offset,
                  int container_y_offset);
int refresh_canvas_state(struct canvas_data *canvas_d,
                         struct container *display);
int show_help_modal(void);
bool show_quit_modal(void);
void exit_self(struct container *display, struct canvas_data *canvas_data,
               struct cursor *cursor, char *msg);
void refresh_info(WINDOW *aside, char *filename,
                  struct canvas_data *canvas_data, struct cursor *cursor);

#endif
