#ifndef UTILS_H
#define UTILS_H

#include <netinet/in.h>
#include <signal.h>

#define BUFFER_SIZE 2048
#define TRUE 1
#define FALSE 0

typedef char Bool;

extern int listenFileDescriptor;
extern volatile sig_atomic_t run;

void catchCtrlC(int signal);

void removeNewLineSymbol (char* string, int length);
void printClientAddress(struct sockaddr_in address);

void* packInt(int value);
int unpackInt(void* value);

int getValue(char* source, char* destination, int position);

#endif // UTILS_H
