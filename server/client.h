#ifndef CLIENT_H
#define CLIENT_H

#define MAX_CLIENT_COUNT 100

extern _Atomic int clientCount;

void* clientThread(void* value);
void closeAllClients();

void loadClients();

#endif // CLIENT_H
