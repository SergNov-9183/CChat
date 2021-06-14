#include "client.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

#define BUFFER_SIZE 2048
#define CLIENTS_FILE_NAME "clients.txt"
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

_Atomic int clientCount = 0;

FILE* clientsFile = NULL;

Client* clients[MAX_CLIENT_COUNT];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

Bool sendMessage(int socketFileDescriptor, char* message) {
    if(socketFileDescriptor < 1 || write(socketFileDescriptor, message, strlen(message)) < 0){
        perror("Error sending message to the client");
        return FALSE;
    }
    return TRUE;
}

Client* addClient(char* nickName, char* fullName, char* password, Bool writeToFile){
    Client* client = (Client*)malloc(sizeof(Client));
    if (client) {
        client->socketFileDescriptor = 0;
        strcpy(client->nickName, nickName);
        strcpy(client->fullName, fullName);
        strcpy(client->password, password);

        pthread_mutex_lock(&clients_mutex);
        clients[clientCount] = client;
        ++clientCount;
        if (writeToFile == TRUE && clientsFile != NULL) {
            char data[BUFFER_SIZE] = { 0 };
            sprintf(data, "#%s#%s#%s#\n", client->nickName, client->fullName, client->password);
            fputs(data, clientsFile);
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    return client;
}

void sendMessageToClients(char* message, int senderClientId){
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < clientCount; ++i){
        if(i != senderClientId && clients[i]->socketFileDescriptor > 0){
            sendMessage(clients[i]->socketFileDescriptor, message);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

int findClient(char* nickName) {
    int clientId = -1;
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < clientCount; ++i){
        if(strcmp(clients[i]->nickName, nickName) == 0) {
            clientId = i;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return clientId;
}

int registration(int socketFileDescriptor, char* nickName) {
    if (sendMessage(socketFileDescriptor, "#NOT_REGISTERED#") == FALSE) {
        return -1;
    }
    char data[BUFFER_SIZE] = { 0 };
    if(recv(socketFileDescriptor, data, BUFFER_SIZE, 0) <= 0 || strlen(data) <  17){
        perror("Can't get client data");
        return -1;
    }

    char command[BUFFER_SIZE] = { 0 };
    int idx = getValue(data, command, 1);
    if (strcmp(command, "NEW_CLIENT") == 0) {
        char fullName[FULLNAME_LENGTH] = { 0 };
        idx = getValue(data, fullName, idx);
        char password[PASSWORD_LENGTH] = { 0 };
        getValue(data, password, idx);

        addClient(nickName, fullName, password, TRUE);
        return findClient(nickName);
    }
    perror("Invalid registration answer");
    return -1;
}

void buildWelcome(char* message, int clientId) {
    int activeClientCount = 0;
    char activeClients[BUFFER_SIZE] = { 0 };
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < clientCount; ++i){
        if(i != clientId && clients[i]->socketFileDescriptor > 0) {
            ++activeClientCount;
            char client[BUFFER_SIZE] = { 0 };
            sprintf(client, "%s#", clients[i]->nickName);
            strcat(activeClients, client);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    sprintf(message, "#WELCOME#%d#%s\n", activeClientCount, activeClients);
}

Bool getPassword(int socketFileDescriptor, int clientId) {
    pthread_mutex_lock(&clients_mutex);
    Client* client = clients[clientId];
    pthread_mutex_unlock(&clients_mutex);

    if (client) {
        int attemptsLeft = 3;
        while (attemptsLeft > 0) {
            char message[BUFFER_SIZE] = { 0 };
            sprintf(message, "#GET_PASSWORD#%d#\n", attemptsLeft);
            if(sendMessage(socketFileDescriptor, message) == FALSE){
                return FALSE;
            }
            char data[BUFFER_SIZE] = { 0 };
            if(recv(socketFileDescriptor, data, BUFFER_SIZE, 0) <= 0){
                perror("Can't get client data");
                return FALSE;
            }
            removeNewLineSymbol(data, strlen(data));
            if(strcmp(client->password, data) == 0) {
                char welcomeMessage[BUFFER_SIZE] = { 0 };
                buildWelcome(welcomeMessage, clientId);
                sendMessage(socketFileDescriptor, welcomeMessage);
                return TRUE;
            }
            --attemptsLeft;
        }
        sendMessage(socketFileDescriptor, "#WRONG_PASSWORD#");
    }
    return FALSE;
}

void clienJoined(int clientId, int socketFileDescriptor) {
    pthread_mutex_lock(&clients_mutex);
    Client* client = clients[clientId];
    pthread_mutex_unlock(&clients_mutex);

    char message[BUFFER_SIZE];
    sprintf(message, "#CLIENT_JOINED#%s#\n", client->nickName);
    printf("%s", message);
    client->socketFileDescriptor = socketFileDescriptor;
    sendMessageToClients(message, clientId);
}

void clienLeft(int clientId) {
    pthread_mutex_lock(&clients_mutex);
    Client* client = clients[clientId];
    pthread_mutex_unlock(&clients_mutex);

    char message[BUFFER_SIZE];
    sprintf(message, "#CLIENT_LEFT#%s#\n", client->nickName);
    printf("%s", message);
    client->socketFileDescriptor = 0;
    sendMessageToClients(message, clientId);
}

int signIn(int socketFileDescriptor, char* nickName) {
    int clientId = -1;
    if(recv(socketFileDescriptor, nickName, NICKNAME_LENGTH, 0) <= 0 || strlen(nickName) <  2 || strlen(nickName) >= NICKNAME_LENGTH-1){
        printf("Didn't enter the nickname.\n");
    }
    else {
        clientId = findClient(nickName);
        if (clientId < 0) {
            clientId = registration(socketFileDescriptor, nickName);
        }
        if (clientId < 0) {
            printf("Client connection error\n");
        }
        else if (getPassword(socketFileDescriptor, clientId) == FALSE){
            printf("Wrong password, client %s disconnected\n", nickName);
            clientId = -1;
        }
    }
    return clientId;
}

void* clientThread(void* value){
    int socketFileDescriptor = unpackInt(value);
    char nickName[NICKNAME_LENGTH] = { 0 };
    int clientId = signIn(socketFileDescriptor, nickName);
    if (clientId >= 0) {
        clienJoined(clientId, socketFileDescriptor);

        char buffOut[BUFFER_SIZE];
        while(TRUE){
            bzero(buffOut, BUFFER_SIZE);
            int receive = recv(socketFileDescriptor, buffOut, BUFFER_SIZE, 0);
            if (receive > 0){
                if(strlen(buffOut) > 0){
                    sendMessageToClients(buffOut, clientId);

                    removeNewLineSymbol(buffOut, strlen(buffOut));
                    printf("%s -> %s\n", buffOut, nickName);
                }
            } else if (receive == 0 || strcmp(buffOut, "exit") == 0){
                clienLeft(clientId);
                break;
            } else {
                printf("ERROR: -1\n");
                break;
            }
        }
    }

    // Delete client from client list and yield thread
    close(socketFileDescriptor);
    pthread_detach(pthread_self());

    return NULL;
}


void closeAllClients() {
    char* message = "#SERVER_CLOSE#";

    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < clientCount; ++i){
        if (clients[i]->socketFileDescriptor > 0){
            write(clients[i]->socketFileDescriptor, message, strlen(message));
            close(clients[i]->socketFileDescriptor);
            printf("%s thread is terminated\n", clients[i]->nickName);
            free(clients[i]);
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    if (clientsFile != NULL) {
        fclose(clientsFile);
    }
}

void loadClients() {
    clientsFile = fopen(CLIENTS_FILE_NAME, "a+t");
    if (clientsFile != NULL) {
        while (!feof(clientsFile)) {
            char data[BUFFER_SIZE] = { 0 };
            if (fgets(data, BUFFER_SIZE, clientsFile)) {
                char nickName[NICKNAME_LENGTH] = { 0 };
                int idx = getValue(data, nickName, 1);
                char fullName[FULLNAME_LENGTH] = { 0 };
                idx = getValue(data, fullName, idx);
                char password[PASSWORD_LENGTH] = { 0 };
                getValue(data, password, idx);

                addClient(nickName, fullName, password, FALSE);
            }
        }
        return;
    }
    printf("Error opening clients file");
}
