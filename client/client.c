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
        if(IsCommand(message))
          {
            int res = CommandAnalyzer(name, message, socketFileDescriptor);
            if(res == -1)
              {
                printf("CommandAnalyzer returned %d\n", res);
                break;
              }
          }
        else
          {
            if (strcmp(message, "exit") == 0)
              {
                break;
            } else
              {

                send(socketFileDescriptor, message, strlen(message), 0);
            }
          }




        memset(command, 0, sizeof(message));
        bzero(message, LENGTH);
    }
    catch_ctrl_c_and_exit(2);
}

void receivingThread() {
    char message[LENGTH] = {0};
    char command[COMMAND_LEN] = {0};

    while (TRUE) {
        int receive = recv(socketFileDescriptor, message, LENGTH, 0);
        str_overwrite_stdout();
        printf("received: %s;\n", message);
         str_overwrite_stdout();
        if (receive > 0)
          {
            if(IsCommand(message))
              {
                CommandDefAndRegUp(message, command);

                printf("command: %s;\n", command);
                if(!strcmp(command, "GET_PASSWORD"))
                  {
                    printf("Input password: ");
                    fflush(stdout);
                    //break;
                  }
                else
                  if(!strcmp(command, "WELCOME"))
                    {
                      str_overwrite_stdout();
                      char ** splMes = SplitInit(message + sizeof(char));
                      printf("Successfully connected!\n");
//                      printf("Now:\t%s participants:\n", splMes[1]);
//                      for(int i = 2; i < 2 + atoi(splMes[1]); ++i)
//                        {
//                          printf("\t\t%s", splMes[i]);
//                        }
                      printf("Now you can send message in common chat or use #HELP# to get more detailed information.\n");
                      free(splMes);
                    }
                else{
                    printf("Not welcome!");
                    if(!strcmp(command, "WRONG_PASSWORD"))
                      {
                        printf("in WRONG_PASSWORD\n");
                        if(message[strlen(message)-2] == 'D')
                          {
                            printf("Access denied. Try again.\n");
                            catch_ctrl_c_and_exit(2);
                          }
                        else
                          {
                            printf("Wrong password. %c attempts left.\n", message[strlen(message)-2]);
                            fflush(stdout);
                          }
                      }
                else
                      {
                        if(!strcmp(command, "NOT_REGISTERED"))
                          {
                            printf("New user? Create an account? (y/n)");
                          }
                      }}
              }
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
        memset(command, 0, sizeof(command));
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

