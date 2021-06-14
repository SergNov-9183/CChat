#ifndef CLIENT_H
#define CLIENT_H

#define MAX_CLIENT_COUNT 100

typedef enum { NewClient } ClientCommands;
typedef enum { GetPassword, Welcome, WrongPassword, Registration } ServerCommands;

extern _Atomic int clientCount;

void* clientThread(void* value);
void closeAllClients();

void loadClients();

#endif // CLIENT_H
