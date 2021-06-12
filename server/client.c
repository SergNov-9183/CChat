#include "client.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

#define BUFFER_SIZE 2048

_Atomic unsigned int clientCount = 0;

Client *clients[MAX_CLIENT_COUNT];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void addClient(Client *client){
    pthread_mutex_lock(&clients_mutex);

    for(int i = 0; i < MAX_CLIENT_COUNT; ++i){
        if(!clients[i]){
            clients[i] = client;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void removeClient(int clientId){
    pthread_mutex_lock(&clients_mutex);

    for(int i = 0; i < MAX_CLIENT_COUNT; ++i){
        if(clients[i]){
            if(clients[i]->clientId == clientId){
                clients[i] = NULL;
                break;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void sendMessageToClients(char *message, int senderClientId){
    pthread_mutex_lock(&clients_mutex);

    for(int i = 0; i < MAX_CLIENT_COUNT; ++i){
        if(clients[i]){
            if(clients[i]->clientId != senderClientId){
                if(write(clients[i]->socketFileDescriptor, message, strlen(message)) < 0){
                    perror("Error sending message to the client");
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void *clientThread(void *voidClient){
    char buffOut[BUFFER_SIZE];
    char name[32];
    int clientLeft = FALSE;

    clientCount++;
    Client *client = (Client *)voidClient;

    // Check name
    if(recv(client->socketFileDescriptor, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){
        printf("Didn't enter the name.\n");
        clientLeft = TRUE;
    } else{
        strcpy(client->name, name);
        sprintf(buffOut, "%s has joined\n", client->name);
        printf("%s", buffOut);
        sendMessageToClients(buffOut, client->clientId);
    }

    bzero(buffOut, BUFFER_SIZE);

    while(TRUE){
        if (clientLeft) {
            break;
        }

        int receive = recv(client->socketFileDescriptor, buffOut, BUFFER_SIZE, 0);
        if (receive > 0){
            if(strlen(buffOut) > 0){
                sendMessageToClients(buffOut, client->clientId);

                removeNewLineSymbol(buffOut, strlen(buffOut));
                printf("%s -> %s\n", buffOut, client->name);
            }
        } else if (receive == 0 || strcmp(buffOut, "exit") == 0){
            sprintf(buffOut, "%s has left\n", client->name);
            printf("%s", buffOut);
            sendMessageToClients(buffOut, client->clientId);
            clientLeft = TRUE;
        } else {
            printf("ERROR: -1\n");
            clientLeft = TRUE;
        }

        bzero(buffOut, BUFFER_SIZE);
    }

    // Delete client from client list and yield thread
    close(client->socketFileDescriptor);
    removeClient(client->clientId);
    free(client);
    clientCount--;
    pthread_detach(pthread_self());

    return NULL;
}

