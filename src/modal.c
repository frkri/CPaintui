#include <ncurses.h>

struct modal {
  WINDOW *win;
  char *title;
  char *text;

  // If box should be drawn around window
  bool box;
};

bool create_dialog_new(struct modal *modal) {
  // Create a new window for the dialog box
  WINDOW *dialog_win = newwin(10, 50, (LINES - 10) / 2, (COLS - 50) / 2);

  if (modal->box)
    box(dialog_win, 0, 0); // Add a border to the window
  if (modal->title)
    mvwprintw(dialog_win, 0, 1, " %s ",
              modal->title); // Add title to the window
  if (modal->text)
    mvwprintw(dialog_win, 2, 2, "%s", modal->text); // Add text to the window

  // Overlay the dialog box on top of the current screen
  overlay(dialog_win, stdscr);

  wrefresh(dialog_win); // Refresh the window

  // Wait for user input
  int ch = getch();

  switch (ch) {
  case 'y':
    return true;
  default:
    return false;
  }

  // Clean up and exit
  delwin(dialog_win);

  refresh();
  return false;
}
