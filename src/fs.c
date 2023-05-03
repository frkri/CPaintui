#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct canvas_data {
  int width;
  int height;

  // 2D Array of pixels
  // Allows easy access to pixels using canvas.data[x][y]
  struct canvas_pixel **data;
};

struct canvas_pixel {
  int color;
  int last_modified;
};

/*
    Loads a file into a buffer

    Returns 0 on success, 1 on failure
*/
char *load_file(char *filename) {
  // See: https://stackoverflow.com/a/2029227

  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s", filename);
  }

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
    Writes a char buffer to a file

    Returns 0 on success, 1 on failure
*/
int write_file(char *filename, char *buffer) {
  // Open the file
  FILE *fp = fopen(filename, "w");
  if (fp == NULL) {
    printf("Could not open file %s", filename);
    return 1;
  }

  // Write the buffer to the file
  fwrite(buffer, sizeof(char), strlen(buffer), fp);

  // Close the file
  fclose(fp);

  return 0;
}

/*
    Creates and copies data from canvas to a buffer for writing to a file
*/
int serialize_buffer(struct canvas_data *canvas, char *buffer) {
  sprintf(buffer, "%d,%d;", canvas->width, canvas->height);

  // x and y pixel coordinates are also stored in the buffer for easier
  // readability when manually editing file
  for (int x = 0; x < canvas->width; x++)
    for (int y = 0; y < canvas->height; y++) {
      struct canvas_pixel pixel = canvas->data[x][y];

      char pixel_str[5] = {0};

      // Append the pixel data to the buffer
      sprintf(pixel_str, "%d,%d;", pixel.color, pixel.last_modified);
      strcat(buffer, pixel_str);
    }

  return 0;
}

/*
    Copies data from buffer to a new canvas data structure

    Requires a char buffer with the following format:
    <width>,<height>;<color>,<last_modified>;...
*/
struct canvas_data *deserialize_buffer(char *buffer) {
  /*
    strsep is used to split the buffer into tokens, it's meant to be a
    replacement for strtok, but strsep might not be available on all platforms

    See: https://man7.org/linux/man-pages/man3/strsep.3.html
  */

  // TODO: Consider using memory arenas
  // TODO: Consider using strtok_r instead of strsep for better support

  // Create a new canvas, needs to be a pointer so we can return it
  struct canvas_data *canvas = malloc(sizeof(struct canvas_data));

  // Set width and height
  char *width_str = strsep(&buffer, ",");
  char *height_str = strsep(&buffer, ";");

  canvas->width = atoi(width_str);
  canvas->height = atoi(height_str);

  // Allocate memory for each row
  canvas->data = malloc(canvas->width * sizeof(struct canvas_pixel *));

  // Allocate memory for each column
  for (int i = 0; i < canvas->width; i++)
    canvas->data[i] = malloc(canvas->height * sizeof(struct canvas_pixel));

  for (int x = 0, y = 0; x < canvas->width; y++) {

    // Get the next pixel data
    char *pixel_str = strsep(&buffer, ";");

    // Get first value (color), leave the rest in pixel_str (last_modified)
    char *color_str = strsep(&pixel_str, ",");

    int color = atoi(color_str);
    int last_modified = atoi(pixel_str);

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
