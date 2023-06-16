#ifndef FS_H
#define FS_H

struct canvas_data {
  int width;
  int height;

  // 2D Array of pixels
  // Allows easy access to pixels using canvas.data[x][y]
  struct canvas_pixel **data;
};

struct canvas_pixel {
  int color;
  long last_modified;
};

char *load_file(char *filename);
int write_file(char *filename, char *buffer);
char *serialize_buffer(struct canvas_data *canvas);
struct canvas_data *deserialize_buffer(char *buffer);

#endif
