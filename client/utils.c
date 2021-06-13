#include "utils.h"

#include <stdio.h>

// Global variables
volatile sig_atomic_t run = TRUE;

void catch_ctrl_c_and_exit(int signal) {
    run = FALSE;
}

void str_overwrite_stdout() {
  printf("%s", "> ");
  fflush(stdout);
}

void removeNewLineSymbol (char* string, int length) {
  int i;
  for (i = 0; i < length; i++) {
    if (string[i] == '\n') {
      string[i] = '\0';
      break;
    }
  }
}

