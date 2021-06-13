#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int listenFileDescriptor = 0;
volatile sig_atomic_t run = TRUE;

void catchCtrlC(int signal) {
    close(listenFileDescriptor);
    run = FALSE;
}
void removeNewLineSymbol (char* string, int length) {
  for (int i = 0; i < length; ++i) {
    if (string[i] == '\n' || string[i] == '\r') {
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

void* packInt(int value) {
    int* result = (int*)malloc(sizeof(int));
    *result = value;
    return (void*)result;
}

int unpackInt(void* value) {
    int result = *(int*)value;
    free(value);
    return result;
}

int getValue(char *source, char *destination, int position) {
    int idx = 0;
    while (source[position] != '#') {
        destination[idx] = source[position];
        ++idx;
        ++position;
    }
    ++position;
    return position;
}
