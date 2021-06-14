#ifndef CLIENT_H
#define CLIENT_H

#define MAX_CLIENT_COUNT 100

extern _Atomic int clientCount;

void* clientThread(void* value);
void closeAllClients();

void loadClients();
void loadRooms();

#endif // CLIENT_H
