#include <curses.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct modal {
  char *title;
  char *text;

  // Confirm chars that will return true when pressed, if 0 then no confirm
  // check will be run
  char *confirm_chars;

  // If box should be drawn around window
  bool box;

  // How many chars to capture, if 0 then capture none
  int capture_count;
  char *input;
};

bool create_modal_new(struct modal *modal) {
  int w, h;

  get_window_size(stdscr, &w, &h);

  // Create a new window for the dialog box
  WINDOW *dialog_win = newwin(10, 50, h / 2 - 5, w / 2 - 25);

  if (modal->text)
    mvwprintw(dialog_win, 2, 1, "%s", modal->text); // Add text to the window
  if (modal->box)
    box(dialog_win, 0, 0); // Add a border to the window, this will overwrite
                           // the text found on the edges
  if (modal->title)
    mvwprintw(dialog_win, 0, 1, " %s ",
              modal->title); // Add title to the window

  // Overlay the dialog box on top of the current screen
  overwrite(dialog_win, stdscr);
  wrefresh(dialog_win);

  // If the modal should capture input
  if (modal->capture_count > 0) {
    modal->input = malloc(modal->capture_count * sizeof(char));
    getnstr(modal->input, modal->capture_count);
  }

  bool confirmed = false;

  // If we should run the confirm check
  if (strlen(modal->confirm_chars) > 0) {
    // Wait for user input
    int ch = getch();

    // Check if the user accepted
    for (int i = 0; modal->confirm_chars[i] != '\0'; i++) {
      if (ch == modal->confirm_chars[i])
        confirmed = true;
    }
  }

  delwin(dialog_win);

  // If none match return false
  return confirmed;
}
