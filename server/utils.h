#ifndef UTILS_H
#define UTILS_H

#include <netinet/in.h>

#define TRUE 1
#define FALSE 0

void removeNewLineSymbol (char* string, int length);
void printClientAddress(struct sockaddr_in address);

#endif // UTILS_H
