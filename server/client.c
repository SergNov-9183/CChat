#include "client.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

#define BUFFER_SIZE 2048
#define CLIENTS_FILE_NAME "clients.txt"

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

void disconnectClient(int clientId) {
    pthread_mutex_lock(&clients_mutex);
    clients[clientId]->socketFileDescriptor = 0;
    pthread_mutex_unlock(&clients_mutex);
}

int findClient(int socketFileDescriptor, char* nickName) {
    int clientId = -1;
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < clientCount; ++i){
        if(strcmp(clients[i]->nickName, nickName) == 0) {
            clients[i]->socketFileDescriptor = socketFileDescriptor;
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

        Client* client = addClient(nickName, fullName, password, TRUE);
        if (client) {
            client->socketFileDescriptor = socketFileDescriptor;
            return findClient(socketFileDescriptor, nickName);
        }
    }
    perror("Invalid registration answer");
    return -1;
}

Bool getPassword(int clientId) {
    pthread_mutex_lock(&clients_mutex);
    Client* client = clients[clientId];
    pthread_mutex_unlock(&clients_mutex);

    if (client) {
        int attemptsLeft = 3;
        while (attemptsLeft > 0) {
            char message[BUFFER_SIZE] = { 0 };
            sprintf(message, "#GET_PASSWORD#%d#\n", attemptsLeft);
            if(sendMessage(client->socketFileDescriptor, message) == FALSE){
                return FALSE;
            }
            char data[BUFFER_SIZE] = { 0 };
            if(recv(client->socketFileDescriptor, data, BUFFER_SIZE, 0) <= 0){
                perror("Can't get client data");
                return FALSE;
            }
            removeNewLineSymbol(data, strlen(data));
            if(strcmp(client->password, data) == 0) {
                sendMessage(client->socketFileDescriptor, "#WELCOME#");
                return TRUE;
            }
            --attemptsLeft;
        }
        sendMessage(client->socketFileDescriptor, "#WRONG_PASSWORD#");
    }
    return FALSE;
}

void* clientThread(void* value){
    int socketFileDescriptor = unpackInt(value);

    // Check name
    char nickName[NICKNAME_LENGTH] = { 0 };
    if(recv(socketFileDescriptor, nickName, NICKNAME_LENGTH, 0) <= 0 || strlen(nickName) <  2 || strlen(nickName) >= NICKNAME_LENGTH-1){
        printf("Didn't enter the nickname.\n");
    }
    else {
        int clientId = findClient(socketFileDescriptor, nickName);
        if (clientId < 0) {
            clientId = registration(socketFileDescriptor, nickName);
        }
        if (clientId < 0) {
            printf("Client connection error\n");
        }
        else if (getPassword(clientId) == TRUE){
            char buffOut[BUFFER_SIZE];
            sprintf(buffOut, "%s has joined\n", nickName);
            printf("%s", buffOut);
            sendMessageToClients(buffOut, clientId);


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
                    sprintf(buffOut, "%s has left\n", nickName);
                    disconnectClient(clientId);
                    printf("%s", buffOut);
                    sendMessageToClients(buffOut, clientId);
                    break;
                } else {
                    printf("ERROR: -1\n");
                    break;
                }
            }
        }
        else {
            printf("Wrong password, client %s disconnected\n", nickName);
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
