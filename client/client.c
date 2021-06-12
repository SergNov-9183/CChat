<<<<<<< HEAD
#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH 2048

static pthread_t sendMessageThread = 0;
static pthread_t recvMessageThread = 0;

static int socketFileDescriptor = 0;
static char name[32];

void sendingThread() {
    char message[LENGTH] = {};
    char buffer[LENGTH + 32] = {};

    while(TRUE) {
        str_overwrite_stdout();
        fgets(message, LENGTH, stdin);
        removeNewLineSymbol(message, LENGTH);

        if (strcmp(message, "exit") == 0) {
            break;
        } else {

            sprintf(buffer, "%s: %s\n", name, message);
            
            send(socketFileDescriptor, buffer, strlen(buffer), 0);
        }

        bzero(message, LENGTH);
        bzero(buffer, LENGTH + 32);
    }
    catch_ctrl_c_and_exit(2);
}

void receivingThread() {
    char message[LENGTH] = {};
    while (TRUE) {
        int receive = recv(socketFileDescriptor, message, LENGTH, 0);
        if (receive > 0) {
            printf("%s", message);
            str_overwrite_stdout();
        } else if (receive == 0) {
            break;
        } else {
            // -1
        }
        memset(message, 0, sizeof(message));
    }
}

Bool init(char *strPort) {
    char *ip = "127.0.0.1";
    int port = atoi(strPort);

    printf("Please enter your name: ");
    fgets(name, 32, stdin);
    removeNewLineSymbol(name, strlen(name));


    if (strlen(name) > 32 || strlen(name) < 2){
        printf("Name must be less than 30 and more than 2 characters.\n");
        return FALSE;
    }

    struct sockaddr_in serverAddress;

    // Socket settings
    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ip);
    serverAddress.sin_port = htons(port);


    // Connect to Server
    int error = connect(socketFileDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (error == -1) {
        printf("Can't connect to server\n");
        return FALSE;
    }

    // Send name
    send(socketFileDescriptor, name, 32, 0);
    return TRUE;
}

Bool createSendingThread() {
    if (pthread_create(&sendMessageThread, NULL, (void *) sendingThread, NULL) == 0) {
        return TRUE;
    }
    printf("Can't create a sending thread\n");
    return FALSE;
}

Bool createReceivingThread() {
    if (pthread_create(&recvMessageThread, NULL, (void *) receivingThread, NULL) == 0) {
        return TRUE;
    }
    printf("Can't create a receiving thread\n");
    return FALSE;
}

void execute() {
    while (run == TRUE);
    printf("\nBye\n");
    close(socketFileDescriptor);

