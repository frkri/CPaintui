#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fs.h"
#include "logger.h"

/*
    Loads a file into a buffer.
    Returns 0 on success, 1 on failure
*/
char *load_file(char *filename) {
  // See: https://stackoverflow.com/a/2029227

  FILE *fp = fopen(filename, "r");
  if (fp == NULL)
    return NULL;

  // Get the size of the file
  fseek(fp, 0L, SEEK_END);
  int file_len = ftell(fp);
  // Allocate memory for the file
  char *buffer = malloc(sizeof(char) * (file_len + 1));
  // Reset the file pointer
  fseek(fp, 0L, SEEK_SET);

  // Read the file into the buffer
  fread(buffer, sizeof(char), file_len, fp);

  fclose(fp);
  return buffer;
}

/*
    Writes a char buffer to a file.
    Returns 0 on success, 1 on failure
*/
int write_file(char *filename, char *buffer) {
  FILE *fp = fopen(filename, "w");
  if (fp == NULL)
    return 1;

  // Write the buffer to the file
  fwrite(buffer, sizeof(char), strlen(buffer), fp);
  fclose(fp);
  return 0;
}

/*
    Creates and copies data from canvas to a buffer for writing to a file
*/
char *serialize_buffer(struct canvas_data *canvas) {
  char *buffer = malloc(sizeof(char) * canvas->width * canvas->height * 20);

  // Set the width and height
  sprintf(buffer, "%d,%d;", canvas->width, canvas->height);

  // Loop through each pixel
  for (int x = 0; x < canvas->width; x++)
    for (int y = 0; y < canvas->height; y++) {
      struct canvas_pixel pixel = canvas->data[x][y];
      char pixel_str[8] = {0};

      // Append the pixel data to the buffer
      sprintf(pixel_str, "%d,%ld;", pixel.color, pixel.last_modified);
      strcat(buffer, pixel_str);
    }
  return buffer;
}

/*
    Copies data from buffer to a new canvas data structure.
    Requires a char buffer with the following format:
    <width>,<height>;<color>,<last_modified>;...
*/
struct canvas_data *deserialize_buffer(char *buffer) {
  /*
    strsep is used to split the buffer into tokens, it's meant to be a
    replacement for strtok, but strsep might not be available on all platforms
    See: https://man7.org/linux/man-pages/man3/strsep.3.html
  */
  if (buffer == NULL || strlen(buffer) == 0)
    return NULL;

  // Create a new canvas, needs to be a pointer so we can return it
  struct canvas_data *canvas = malloc(sizeof(struct canvas_data));

  char *width_str = strsep(&buffer, ",");
  char *height_str = strsep(&buffer, ";");
  if (width_str == NULL || height_str == NULL)
    return NULL;

  canvas->width = atoi(width_str);
  canvas->height = atoi(height_str);

  // Allocate memory for each row
  canvas->data = malloc(canvas->width * sizeof(struct canvas_pixel *));

  // Allocate memory for each pixel
  for (int i = 0; i < canvas->width; i++)
    canvas->data[i] = malloc(canvas->height * sizeof(struct canvas_pixel));

  for (int x = 0, y = 0; x < canvas->width; y++) {
    // Get the next pixel data: <color>,<last_modified>;
    char *pixel_str = strsep(&buffer, ";");
    char *color_str = strsep(&pixel_str, ","); // Get first value (color)
    if (color_str == NULL || pixel_str == NULL)
      return NULL;

    int color = atoi(color_str);
    if (color > 7) // We only support 8 colors
      color = 7;
    else if (color < 0)
      color = 0;
    int last_modified = atol(pixel_str);

    // Create a new pixel
    struct canvas_pixel pixel = {color, last_modified};
    canvas->data[x][y] = pixel;

    // If we have reached the end of the row, move to the next row
    if (y == canvas->height - 1) {
      x++;
      y = -1; // y will be 0 on next iteration
    }
  }

  return canvas;
}
