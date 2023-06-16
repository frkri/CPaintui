
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *create_buffer(int width, int height, int color) {
  char *buffer = malloc(sizeof(char) * width * height * 20);
  sprintf(buffer, "%d,%d;", width, height);

  for (int x = 0; x < width; x++)
    for (int y = 0; y < height; y++) {
      char pixel_str[8] = {0};
      sprintf(pixel_str, "%d,%d;", color, 0);
      strcat(buffer, pixel_str);
    }

  return buffer;
}