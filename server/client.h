#ifndef CLIENT_H
#define CLIENT_H

#define MAX_CLIENT_COUNT 100
#define NICKNAME_LENGTH 32
#define FULLNAME_LENGTH 50
#define PASSWORD_LENGTH 30

// Client structure
typedef struct{
    int socketFileDescriptor;
    char nickName[NICKNAME_LENGTH];
    char fullName[FULLNAME_LENGTH];
    char password[PASSWORD_LENGTH];
} Client;

typedef enum { NewClient } ClientCommands;
typedef enum { GetPassword, Welcome, WrongPassword, Registration } ServerCommands;

extern _Atomic int clientCount;

void* clientThread(void* value);
void closeAllClients();

void loadClients();

#endif // CLIENT_H
