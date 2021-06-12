#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>

#define MAX_CLIENT_COUNT 100

// Client structure
typedef struct{
    struct sockaddr_in address;
    int socketFileDescriptor;
    int clientId;
    char name[32];
} Client;

extern _Atomic unsigned int clientCount;

void addClient(Client *client);
void *clientThread(void *voidClient);

#endif // CLIENT_H
