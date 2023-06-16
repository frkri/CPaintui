#ifndef UTIL_H
#define UTIL_H

#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *create_buffer(int width, int height, int color, long modified);
char *format_timestamp(long timestamp);

#endif
