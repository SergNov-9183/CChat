#include "server.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "client.h"

Bool init(char* strPort) {
    char* ip = "127.0.0.1";
    int port = atoi(strPort);
    int option = 1;
    struct sockaddr_in serverAddress;

    // Socket settings
    listenFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ip);
    serverAddress.sin_port = htons(port);

    if(setsockopt(listenFileDescriptor, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
        perror("Listening socket configuration error");
        return FALSE;
    }

    // Bind
    if(bind(listenFileDescriptor, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Socket connection error");
        return FALSE;
    }

    // Listen
    if (listen(listenFileDescriptor, 10) < 0) {
        perror("Error setting socket to listening state");
        return FALSE;
    }
    return TRUE;
}

void execute() {
    while(run == TRUE){
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int socketFileDescriptor = accept(listenFileDescriptor, (struct sockaddr*)&clientAddress, &clientAddressLength);

        if (socketFileDescriptor < 0) {
            break;
        }

        // Check if max clients is reached
        if((clientCount + 1) == MAX_CLIENT_COUNT){
            printf("Max clients reached. Rejected: ");
            printClientAddress(clientAddress);
            printf(":%d\n", clientAddress.sin_port);
            close(socketFileDescriptor);
            continue;
        }

        pthread_t threadId = 0;
        pthread_create(&threadId, NULL, &clientThread, packInt(socketFileDescriptor));

        // Reduce CPU usage
        sleep(1);
    }

    printf("\nMain thread is terminated\n");
}
