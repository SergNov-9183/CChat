#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "client.h"
#include "utils.h"

static int newClientId = 10;

int main(int argc, char **argv){
    if(argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);
    int option = 1;
    int listenFileDescriptor = 0;
    int socketFileDescriptor = 0;
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    pthread_t threadId;

    // Socket settings
    listenFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ip);
    serverAddress.sin_port = htons(port);

    // Ignore pipe signals
    signal(SIGPIPE, SIG_IGN);

    if(setsockopt(listenFileDescriptor, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
        perror("Listening socket configuration error");
    return EXIT_FAILURE;
    }

    // Bind
    if(bind(listenFileDescriptor, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Socket connection error");
        return EXIT_FAILURE;
    }

    // Listen
    if (listen(listenFileDescriptor, 10) < 0) {
        perror("Error setting socket to listening state");
        return EXIT_FAILURE;
    }

    printf("Chat server started\n");

    while(TRUE){
        socklen_t clilen = sizeof(clientAddress);
        socketFileDescriptor = accept(listenFileDescriptor, (struct sockaddr*)&clientAddress, &clilen);

        // Check if max clients is reached
        if((clientCount + 1) == MAX_CLIENT_COUNT){
            printf("Max clients reached. Rejected: ");
            printClientAddress(clientAddress);
            printf(":%d\n", clientAddress.sin_port);
            close(socketFileDescriptor);
            continue;
        }

        // Client settings
        Client *cli = (Client *)malloc(sizeof(Client));
        cli->address = clientAddress;
        cli->socketFileDescriptor = socketFileDescriptor;
        cli->clientId = newClientId++;

        // Add client
        addClient(cli);
        pthread_create(&threadId, NULL, &clientThread, (void*)cli);

        // Reduce CPU usage
        sleep(1);
    }

    return EXIT_SUCCESS;
}
