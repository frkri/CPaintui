#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Preface: Working with in memory data structures allow for easier and more
// flexible manipulation of data, we only write to the file when we need to

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

  // Open the file
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

  // Close the file
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
      struct canvas_pixel *pixel = &canvas->data[x][y];

      sprintf(buffer, "%s%d,%d,%d,%d;", buffer, x, y, pixel->color,
              pixel->last_modified);
    }

  return 0;
}

/*
    Copies data from buffer to a new canvas data structure

    Requires a char buffer with the following format:
    <width>,<height>;<x>,<y>,<color>,<last_modified>;...
*/
int deserialize_buffer(struct canvas_data *canvas, char *buffer) {
  /*
    strtok allow us to split a string until a delimiter is found
    Given an NULL pointer (after calling strtok with a string), strtok will
    continue from the last string excluding the delimiter

    See: https://cplusplus.com/reference/cstring/strtok/
  */

  // Set width and height
  char *width_str = strtok(buffer, ",");
  char *height_str = strtok(NULL, ";");

  canvas->width = atoi(width_str);
  canvas->height = atoi(height_str);

  printf("Width: %d\n", canvas->width);
  printf("Height: %d\n\n\n", canvas->height);

  canvas->data = (struct canvas_pixel **)malloc(canvas->width *
                                                sizeof(struct canvas_pixel *));

  for (int i = 0; i < canvas->width; i++)
    canvas->data[0] = (struct canvas_pixel *)malloc(
        canvas->height * sizeof(struct canvas_pixel *));

  int x = 0, y = 0;

  char *pixel_str = strtok(NULL, ";");
  while (x != canvas->width && y != canvas->height) {
    printf("\n\nNEW: %s from %s\n", pixel_str, buffer);
    struct canvas_pixel *pixel = malloc(sizeof(struct canvas_pixel));

    char *color_str = strtok(NULL, ",");
    char *last_modified_str = strtok(NULL, ",");

    pixel->color = atoi(color_str);
    pixel->last_modified = atoi(last_modified_str);

    printf("%s from %s\n", pixel_str, buffer);

    printf("%d,%d,%d,%d\n", x, y, pixel->color, pixel->last_modified);

    canvas->data[x][y] = *pixel;

    y++;
    if (y == canvas->height) {
      y = 0;
      x++;
    }

    pixel_str = strtok(buffer, ";");
  }

  return 0;
}
