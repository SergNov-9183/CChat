#include "client.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

#define CLIENTS_FILE_NAME "clients.txt"
#define ROOMS_FILE_NAME "rooms.txt"
#define MAINROOM_NAME "mainRoom"
#define MAX_ROOM_COUNT 10
#define FILENAME_LENGTH 255
#define COMMAND_LENGTH 32
#define NICKNAME_LENGTH 32
#define FULLNAME_LENGTH 50
#define PASSWORD_LENGTH 30
#define ROOMNAME_LENGTH 32

typedef enum { Message, Private, Exit, GotoRoom, AddRoom, InviteClient } ClientCommands;

typedef char Room[ROOMNAME_LENGTH];

// Client structure
typedef struct{
    int id;
    int socketFileDescriptor;
    char nickName[NICKNAME_LENGTH];
    char fullName[FULLNAME_LENGTH];
    char password[PASSWORD_LENGTH];
    int room;
} Client;

_Atomic int clientCount = 0;
_Atomic int roomCount = 0;

FILE* clientsFile = NULL;

Room rooms[MAX_ROOM_COUNT];
Client* clients[MAX_CLIENT_COUNT];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t room_mutex = PTHREAD_MUTEX_INITIALIZER;

void buildPrivateFileName(char* fileName, int id1, int id2) {
    if (id1 < id2) {
        sprintf(fileName, "private%dand%d.txt", id1, id2);
    }
    else {
        sprintf(fileName, "private%dand%d.txt", id2, id1);
    }
}

Bool sendMessage(int socketFileDescriptor, char* message) {
    if(socketFileDescriptor < 1 || write(socketFileDescriptor, message, strlen(message)) < 0){
        perror("Error sending message to the client");
        return FALSE;
    }
    return TRUE;
}

void sendServiceMessageToClients(char* message, Client* client) {
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < clientCount; ++i){
        if(clients[i]->id != client->id && clients[i]->socketFileDescriptor > 0){
            sendMessage(clients[i]->socketFileDescriptor, message);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

Client* addClient(char* nickName, char* fullName, char* password, Bool writeToFile){
    Client* client = (Client*)malloc(sizeof(Client));
    if (client) {
        client->socketFileDescriptor = 0;
        strcpy(client->nickName, nickName);
        strcpy(client->fullName, fullName);
        strcpy(client->password, password);
        client->room = 0;

        pthread_mutex_lock(&clients_mutex);
        clients[clientCount] = client;
        ++clientCount;
        client->id = clientCount;
        if (writeToFile == TRUE && clientsFile != NULL) {
            char data[BUFFER_SIZE] = { 0 };
            sprintf(data, "#%s#%s#%s#\n", client->nickName, client->fullName, client->password);
            fputs(data, clientsFile);
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    return client;
}

Client* findClient(char* nickName) {
    Client* result = NULL;
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < clientCount; ++i){
        if(strcmp(clients[i]->nickName, nickName) == 0) {
            result = clients[i];
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return result;
}

Client* findClientById(int id) {
    Client* result = NULL;
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < clientCount; ++i){
        if(clients[i]->id == id) {
            result = clients[i];
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return result;
}

int findRoom(char* roomName) {
    int result = 0;
    pthread_mutex_lock(&room_mutex);
    for(int i = 0; i < roomCount; ++i){
        if(strcmp(rooms[i], roomName) == 0) {
            result = i;
        }
    }
    pthread_mutex_unlock(&room_mutex);
    return result;
}

void saveMessageFromClient(char* message, char* fileName) {
    pthread_mutex_lock(&file_mutex);
    FILE* file = fopen(fileName, "at");
    if (file) {
        fputs(message, file);
        fclose(file);
    }
    pthread_mutex_unlock(&file_mutex);
}

void loadMessagesForClient(Client* client, char* fileName) {
    //printf("__________00: =%s=%d=%d=%s=\n", client->nickName, client->id, client->socketFileDescriptor, fileName);
    pthread_mutex_lock(&file_mutex);
    FILE* file = fopen(fileName, "rt");
    if (file) {
        while (!feof(file)) {
            char message[BUFFER_SIZE] = { 0 };
            if (fgets(message, BUFFER_SIZE, file)) {
                sendMessage(client->socketFileDescriptor, message);
            }
        }
        fclose(file);
    }
    pthread_mutex_unlock(&file_mutex);
}

void sendMessageToClients(char* message, Client* client){
    char msg[BUFFER_SIZE] = { 0 };
    sprintf(msg, "%s->%s", client->nickName, message);
    if (client->room < 0) {
        Client* partner = findClientById(-client->room);
        if (partner) {
            char fileName[FILENAME_LENGTH] = { 0 };
            buildPrivateFileName(fileName, client->id, partner->id);
            saveMessageFromClient(msg, fileName);
            if (partner->socketFileDescriptor > 0) {
                if (partner->room == -client->id) {
                    sendMessage(partner->socketFileDescriptor, msg);
                }
                else {
                    char infoMessage[BUFFER_SIZE] = { 0 };
                    sprintf(infoMessage, "%s sent you a message\n", client->nickName);
                    sendMessage(partner->socketFileDescriptor, infoMessage);
                }
            }
        }
    }
    else {
        char fileName[FILENAME_LENGTH] = { 0 };
        pthread_mutex_lock(&room_mutex);
        sprintf(fileName, "%s.txt", rooms[client->room]);
        pthread_mutex_unlock(&room_mutex);
        saveMessageFromClient(msg, fileName);
        pthread_mutex_lock(&clients_mutex);
        for(int i = 0; i < clientCount; ++i){
            if(clients[i]->id != client->id && clients[i]->socketFileDescriptor > 0){
                if (clients[i]->room == client->room) {
                    sendMessage(clients[i]->socketFileDescriptor, msg);
                }
                else if (client->room == 0) {
                    char infoMessage[BUFFER_SIZE] = { 0 };
                    sprintf(infoMessage, "%s sent message in main room\n", client->nickName);
                    sendMessage(clients[i]->socketFileDescriptor, infoMessage);
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    removeNewLineSymbol(message, strlen(message));
    printf("Client %s send message: %s\n", client->nickName, message);
}

Client* registration(int socketFileDescriptor, char* nickName) {
    if (sendMessage(socketFileDescriptor, "#NOT_REGISTERED#") == FALSE) {
        return NULL;
    }
    char data[BUFFER_SIZE] = { 0 };
    if(recv(socketFileDescriptor, data, BUFFER_SIZE, 0) <= 0 || strlen(data) <  17){
        perror("Can't get client data");
        return NULL;
    }
    char command[COMMAND_LENGTH] = { 0 };
    int idx = getValue(data, command, 0);

    if (strcmp(command, "NEW_CLIENT") == 0) {
        char fullName[FULLNAME_LENGTH] = { 0 };
        idx = getValue(data, fullName, idx);
        char password[PASSWORD_LENGTH] = { 0 };
        getValue(data, password, idx);

        return addClient(nickName, fullName, password, TRUE);
    }
    perror("Invalid registration answer");
    return NULL;
}

void buildWelcome(char* message, int clientId) {
    int activeClientCount = 0;
    char activeClients[BUFFER_SIZE] = { 0 };
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < clientCount; ++i){
        if(clients[i]->id != clientId && clients[i]->socketFileDescriptor > 0) {
            ++activeClientCount;
            char client[BUFFER_SIZE] = { 0 };
            sprintf(client, "%s#", clients[i]->nickName);
            strcat(activeClients, client);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    sprintf(message, "#WELCOME#%d#%s\n", activeClientCount, activeClients);
}

Bool getPassword(int socketFileDescriptor, Client* client) {
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
                buildWelcome(welcomeMessage, client->id);
                sendMessage(socketFileDescriptor, welcomeMessage);
                return TRUE;
            }
            --attemptsLeft;
        }
        sendMessage(socketFileDescriptor, "#WRONG_PASSWORD#");
    }
    return FALSE;
}

void clienJoined(Client* client, int socketFileDescriptor) {
    if (client) {
        char message[BUFFER_SIZE];
        sprintf(message, "#CLIENT_JOINED#%s#\n", client->nickName);
        client->socketFileDescriptor = socketFileDescriptor;
        char fileName[FILENAME_LENGTH] = { 0 };
        sprintf(fileName, "%s.txt", MAINROOM_NAME);
        loadMessagesForClient(client, fileName);
        sendServiceMessageToClients(message, client);
    }
}

void clienLeft(Client* client) {
    if (client) {
        char message[BUFFER_SIZE];
        sprintf(message, "#CLIENT_LEFT#%s#\n", client->nickName);
        client->socketFileDescriptor = 0;
        sendServiceMessageToClients(message, client);
    }
}

Client* signIn(int socketFileDescriptor) {
    Client* client = NULL;
    char nickName[NICKNAME_LENGTH] = { 0 };
    if(recv(socketFileDescriptor, nickName, NICKNAME_LENGTH, 0) <= 0 || strlen(nickName) <  2 || strlen(nickName) >= NICKNAME_LENGTH-1){
        printf("Didn't enter the nickname.\n");
    }
    else {
        client = findClient(nickName);
        if (!client) {
            client = registration(socketFileDescriptor, nickName);
        }
        if (!client) {
            printf("Client connection error\n");
        }
        else if (getPassword(socketFileDescriptor, client) == FALSE){
            printf("Wrong password, client %s disconnected\n", nickName);
            client = NULL;
        }
    }
    return client;
}

void executePrivate(Client* client, char* nickName) {
    Client* partner = findClient(nickName);
    if (client && partner) {
        char fileName[FILENAME_LENGTH] = { 0 };
        buildPrivateFileName(fileName, client->id, partner->id);
        loadMessagesForClient(client, fileName);
        client->room = -partner->id;
    }
}

void addRoom(char* roomName) {
    pthread_mutex_lock(&room_mutex);
    strcpy(rooms[roomCount], roomName);
    ++roomCount;

    FILE* file = fopen(ROOMS_FILE_NAME, "at");
    if (file) {
        fputs(roomName, file);
        fclose(file);
    }

    pthread_mutex_unlock(&room_mutex);
}

void gotoRoom(Client* client, char* roomName) {
    int roomId = findRoom(roomName);
    if (client) {
        client->room = roomId;
        char fileName[FILENAME_LENGTH] = { 0 };
        sprintf(fileName, "%s.txt", roomName);
        loadMessagesForClient(client, fileName);
    }
}

void executeInviteClient(Client* client, char* nickName) {
    Client* partner = findClient(nickName);
    if (client && partner) {
        char message[BUFFER_SIZE];
        pthread_mutex_lock(&room_mutex);
        sprintf(message, "#INVITATION_TO_CLIENT#%s#%s#", client->nickName, rooms[client->room]);
        pthread_mutex_unlock(&room_mutex);
        sendMessage(partner->socketFileDescriptor, message);
    }
}

ClientCommands getCommand(char* message, char* nickName, char* roomName) {
    ClientCommands result = Message;
    char command[COMMAND_LENGTH] = { 0 };
    int idx = getValue(message, command, 0);
    if (strcmp(command, "PRIVATE") == 0) {
        getValue(message, nickName, idx);
        result = Private;
    }
    else if (strcmp(command, "EXIT") == 0) {
        result = Exit;
    }
    else if (strcmp(command, "ADD_ROOM") == 0) {
        getValue(message, roomName, idx);
        result = AddRoom;
    }
    else if (strcmp(command, "GO_TO_ROOM") == 0) {
        getValue(message, roomName, idx);
        result = GotoRoom;
    }
    else if (strcmp(command, "INVITE_CLIENT") == 0) {
        getValue(message, nickName, idx);
        result = InviteClient;
    }
    return result;
}

void messageHandler(char* message, Client* client) {
    char nickName[NICKNAME_LENGTH] = { 0 };
    char roomName[ROOMNAME_LENGTH] = { 0 };
    ClientCommands command = getCommand(message, nickName, roomName);
    switch (command) {
    case Private:
        executePrivate(client, nickName);
        break;
    case Exit:
        gotoRoom(client, MAINROOM_NAME);
        break;
    case AddRoom:
        addRoom(roomName);
        break;
    case GotoRoom:
        gotoRoom(client, roomName);
        break;
    case InviteClient:
        executeInviteClient(client, nickName);
        break;
    default:
        sendMessageToClients(message, client);
        break;
    }
}

void* clientThread(void* value){
    int socketFileDescriptor = unpackInt(value);
    Client* client = signIn(socketFileDescriptor);
    if (client) {
        clienJoined(client, socketFileDescriptor);

        while(TRUE){
            char buffer[BUFFER_SIZE] = { 0 };
            int receive = recv(socketFileDescriptor, buffer, BUFFER_SIZE, 0);
            if (receive > 0){
                if(strlen(buffer) > 0){
                    messageHandler(buffer, client);
                }
            } else if (receive == 0 || strcmp(buffer, "exit") == 0){
                clienLeft(client);
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
                int idx = getValue(data, nickName, 0);
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

void loadRooms() {
    pthread_mutex_lock(&room_mutex);
    strcpy(rooms[roomCount], MAINROOM_NAME);
    ++roomCount;
    FILE* file = fopen(ROOMS_FILE_NAME, "rt");
    if (file) {
        while (!feof(file)) {
            char roomName[ROOMNAME_LENGTH] = { 0 };
            if (fgets(roomName, ROOMNAME_LENGTH, file)) {
                strcpy(rooms[roomCount], roomName);
                ++roomCount;
            }
        }
        fclose(file);
    }
    pthread_mutex_unlock(&room_mutex);
}
