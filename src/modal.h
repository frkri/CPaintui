#ifndef MODAL_H
#define MODAL_H

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

bool create_modal(struct modal modal);

#endif
