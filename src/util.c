
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "util.h"

char *create_buffer(int width, int height, int color, long modified) {
  char *buffer = malloc(sizeof(char) * width * height * 20);
  sprintf(buffer, "%d,%d;", width, height);

  for (int x = 0; x < width; x++)
    for (int y = 0; y < height; y++) {
      char pixel_str[14] = {0};
      sprintf(pixel_str, "%d,%ld;", color, modified);
      strcat(buffer, pixel_str);
    }

  return buffer;
}

char *format_timestamp(long timestamp) {
  struct tm *timeinfo;
  char *out = malloc(sizeof(char) * 80);

  timeinfo = localtime(&timestamp);
  // Format the date and time as "YYYY:MM:DD H:M:S"
  strftime(out, 20, "%Y:%m:%d %H:%M:%S", timeinfo);
  return out;
}
