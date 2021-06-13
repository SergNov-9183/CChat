#ifndef CLIENT_H
#define CLIENT_H

#include "utils.h"

Bool init(char* strPort);
Bool createSendingThread();
Bool createReceivingThread();
void execute();

#endif // CLIENT_H
