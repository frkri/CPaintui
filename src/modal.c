#include <ctype.h>
#include <curses.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "modal.h"
#include "win.h"

bool create_modal(struct modal modal) {
  int w, h;
  get_window_size(stdscr, &w, &h);

  // Create a new window for the dialog box
  WINDOW *dialog_win = newwin(12, 40, h / 2 - 6, w / 2 - 20);

  if (modal.text)
    mvwprintw(dialog_win, 2, 1, "%s", modal.text); // Add text to the window
  if (modal.box)
    box(dialog_win, 0, 0); // Add a border to the window, this will overwrite
                           // the text found on the edges
  if (modal.title)
    mvwprintw(dialog_win, 0, 1, " %s ",
              modal.title); // Add title to the window

  // Overlay the dialog box on top of the current screen
  overwrite(dialog_win, stdscr);
  wrefresh(dialog_win);

  // If the modal should capture input
  if (modal.capture_count > 0) {
    modal.input = malloc(modal.capture_count * sizeof(char));
    // Wait until newline is entered
    getnstr(modal.input, modal.capture_count);
  }

  // If we should run the confirm check
  bool has_confirmed = false;
  if (strlen(modal.confirm_chars) > 0) {
    // Wait for user input
    int ch = getch();

    // Check if the user accepted
    for (int i = 0; modal.confirm_chars[i] != '\0'; i++) {
      if (ch == modal.confirm_chars[i])
        has_confirmed = true;
    }
  }

  delwin(dialog_win);
  refresh();

  // If none match return false
  return has_confirmed;
}
