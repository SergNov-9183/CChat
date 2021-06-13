#include "client.h"
#include "commands.h"

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
    char message[LENGTH] = {0};
    char buffer[LENGTH + 32] = {0};
    char command[COMMAND_LEN] = {0};

    while(TRUE) {
        str_overwrite_stdout();
        fgets(message, MES_LENGTH, stdin);
        removeNewLineSymbol(message, MES_LENGTH);
        if(CommandAnalyzer(name, message, socketFileDescriptor))


        memset(command, 0, sizeof(message));
        bzero(message, LENGTH);
        bzero(buffer, LENGTH + 32);
    }
    catch_ctrl_c_and_exit(2);
}

void receivingThread() {
    char message[LENGTH] = {0};
    char command[COMMAND_LEN] = {0};

    while (TRUE) {
        int receive = recv(socketFileDescriptor, message, LENGTH, 0);
        if (receive > 0)
          {
            if(IsCommand(message))
              {
                CommandDefAndRegUp(message, command);
                if(!strcmp(command, "GET_PASSWORD"))
                  {
                    printf("Input password: ");
                    break;
                  }
                else
                  if(!strcmp(command, "WELCOME"))
                    {
                      printf("Successfully connected!\n");
                      printf("Now you can send message in common chat or use #HELP# to get more useful information.\n");
                    }
                else
                    if(!strcmp(command, "WRONG_PASSWORD"))
                      {
                        if(message[strlen(message)-2] == '0')
                          printf("Access denied. Try again.\n");
                        else
                          printf("Wrong password. %c attempts left.\n", message[strlen(message)-2]);
                      }
                else
                      {
                        if(!strcmp(command, "NOT_REGISTERED"))
                          {
                            printf("New user? Create an account? (y/n)");
                          }
                      }
              }
            printf("%s", message);
            str_overwrite_stdout();
          }
        else if (receive == 0)
          {
            break;
          }
        else
          {
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
}
