#include "utils.h"

#include <stdio.h>

void removeNewLineSymbol (char* string, int length) {
  for (int i = 0; i < length; ++i) {
    if (string[i] == '\n') {
      string[i] = '\0';
      break;
    }
  }
}

void printClientAddress(struct sockaddr_in address){
    printf("%d.%d.%d.%d",
        address.sin_addr.s_addr & 0xff,
        (address.sin_addr.s_addr & 0xff00) >> 8,
        (address.sin_addr.s_addr & 0xff0000) >> 16,
        (address.sin_addr.s_addr & 0xff000000) >> 24);
}

