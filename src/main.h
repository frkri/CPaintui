#ifndef MAIN_H
#define MAIN_H

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

#endif
