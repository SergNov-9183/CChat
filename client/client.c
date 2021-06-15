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
    char buffer[LENGTH] = {0};
    char command[COMMAND_LEN] = {0};

    while(TRUE) {
        fgets(message, LENGTH, stdin);
        str_overwrite_stdout();
        //removeNewLineSymbol(message, LENGTH);
        if(IsCommand(message) == 2)
          {
            int res = CommandAnalyzer(name, message, socketFileDescriptor);
            if(res == -1)
              {
                //printf("CommandAnalyzer returned %d\n", res);
                break;
              }
          }
        else
          {

            if (IsCommand(message) == 1)
              {
                printf("\b~ Is not a Chat command.\n");
                str_overwrite_stdout();
            } else
              {
                sprintf(buffer, "%s\n", message);
                send(socketFileDescriptor, buffer, strlen(message), 0);
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
        if (receive > 0)
          {
            if(IsCommand(message))
              {
                CommandDefAndRegUp(message, command);

                //printf("message: %s;\n", message);
                if(!strcmp(command, "GET_PASSWORD"))
                  {
                    printf("\b~ Input password (%c attempts left): ", message[strlen(message)-3]);
                    if(message[strlen(message)-2] == 'D')
                      {
                        printf("\b~ Access denied. Try again.\n");
                        exit(0);
                        //catch_ctrl_c_and_exit(2);
                      }
                    str_overwrite_stdout();
                    //break;

                  }
                else
                  if(!strcmp(command, "WELCOME"))
                    {
                      FILE* clientsList = fopen("common_clients.txt", "wt");
                      printf("\b\b\n>~ Welcome to chat!\n");
                      str_overwrite_stdout();
                      char ** splMes = SplitInit(message + sizeof(char));
                      if(atoi(splMes[1]) == 0)
                        {
                          printf("\b~ There is nobody in this (central) Room. Waiting for others...\n");
                          str_overwrite_stdout();
                        }
                      else
                        {
                          if(!atoi(splMes[1]))
                            {
                              printf("\b~ There is nobody in this (Central) Room. Waiting for others...\n");
                              str_overwrite_stdout();
                            }
                          else
                            {
                              printf("\b~ Only %d ", atoi(splMes[1]));
                              if(atoi(splMes[1]) == 1)
                                printf("participant ");
                              else
                                printf("participants ");
                              printf("now:\n");
                            }
                          str_overwrite_stdout();
                          for(int i = 2; i < 2 + atoi(splMes[1]); ++i)
                            {
                              fprintf(clientsList, "%s\n", splMes[i]);
                              printf("\t\t%s\n", splMes[i]);
                            }
                          str_overwrite_stdout();
                        }
                      printf("\b~ Now you can send message in Central Room or use #HELP# to get more detailed information.\n\n");
                      str_overwrite_stdout();
                      fclose(clientsList);
                      free(splMes);
                    }
                else{
                    if(!strcmp(command, "WRONG_PASSWORD"))
                      {
                        if(message[strlen(message)-2] == 'D')
                          {
                            printf("\b~ Access denied. Try again.\n");
                            exit(0);
                            //catch_ctrl_c_and_exit(2);
                          }
                        else
                          {
                            printf("\b~ Wrong password. %c attempts left.\n", message[strlen(message)-2]);
                            str_overwrite_stdout();
                          }

                      }
                else
                      {
                        if(!strcmp(command, "NOT_REGISTERED"))
                          {
                            printf("\b~ New user? Create an account? (#Y/N#)");
                            str_overwrite_stdout();
                          }
                        else
                          {
                            if(!strcmp(command, "WELCOME"))
                              {

                              }
                            else
                              {
                                if(!strcmp(command, "CLIENT_JOINED"))
                                  {
                                    FILE* clientsList = fopen("common_clients.txt", "a+");
                                    printf("File clients is opened\n");
                                    char ** splMes = SplitInit(message + sizeof(char));
                                    printf("\b~ Client %s has joined\n", splMes[1]);
                                    str_overwrite_stdout();
                                    fprintf(clientsList, "%s", splMes[1]);
                                    printf("Client %s is added to a clients list.\n", splMes[1]);
                                    str_overwrite_stdout();
                                    fclose(clientsList);
                                    free(splMes);
                                  }
                                else
                                  {
                                    if(!strcmp(command, "CLIENT_LEFT"))
                                      {
                                        char str[NAME_LENGTH] = {0};
                                        FILE* clientsList = fopen("common_clients.txt", "rt");
                                        char ** splMes = SplitInit(message + sizeof(char));
                                        char clients [MAX_CLIENTS][NAME_LENGTH] = {0};
//                                                char** clients = (char**)malloc(MAX_CLIENTS*sizeof (char*));
//                                                for(int i = 0; i< MAX_CLIENTS; ++i)
//                                                  {
//                                                    clients[i] = (char*)malloc(NAME_LENGTH * sizeof (char));
//                                                    clients[i] = NULL;
//                                                  }
                                        int ind = 0;
                                        while(!feof(clientsList))
                                          {
                                            fgets(str, NAME_LENGTH, clientsList);
                                            str[strlen(str)] = '\0';
                                            //printf("str = %s, ind = %d\n", str, ind);
                                            strncpy(clients[ind++], str, NAME_LENGTH);
                                          }
                                        //printf("Final ind = %d\n", ind);
                                        fclose(clientsList);
                                        for(int i = 0; i < ind; ++i)
                                          {
                                            clients[i][strlen(clients[i])-1] = '\0';
                                            if(!strcmp(clients[i], splMes[1]))
                                              bzero(clients[i], strlen(clients[i]));
                                          }
                                        bzero(clients[ind-1], strlen(clients[ind-1]));
                                        clientsList = fopen("common_clients.txt", "wt");
                                        //printf("clients:\n");
                                        for(int i = 0; i < ind; ++i)
                                          {
                                            //printf("%s", clients[i]);
                                            if(strlen(clients[i]))
                                              {
                                                fprintf(clientsList, "%s\n", clients[i]);
                                              }
                                          }
                                        printf("\b~ Client %s has left\n", splMes[1]);
                                        str_overwrite_stdout();
                                        fclose(clientsList);
                                        free(splMes);
                                      }
                                    else
                                      {
                                        if(!strcmp(command, "INVITATION_TO_CLIENT"))
                                          {
                                            char ** splMes = SplitInit(message + sizeof(char));
                                            printf("\b~ You were invited to the Common Room (%s) by %s.\n", splMes[2], splMes[1]);
                                            str_overwrite_stdout();
                                          }

                                      }
                                  }

                              }
                          }
                      }

                    }
              }
            else
              {
                str_overwrite_stdout();
                printf("%s\n", message);
                str_overwrite_stdout();
              }

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

    str_overwrite_stdout();
    printf("\b~ Please enter your name: ");
    fgets(name, 32, stdin);
    removeNewLineSymbol(name, strlen(name));
    str_overwrite_stdout();


    if (strlen(name) > 32 || strlen(name) < 2){
        printf("\b~ Name must be less than 30 and more than 2 characters.\n");
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
        printf("\b~ Can't connect to server\n");
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
    printf("\b~ Can't create a sending thread\n");
    return FALSE;
}

Bool createReceivingThread() {
    if (pthread_create(&recvMessageThread, NULL, (void *) receivingThread, NULL) == 0) {
        return TRUE;
    }
    printf("\b~ Can't create a receiving thread\n");
    return FALSE;
}

void execute() {
    while (run == TRUE);
    printf("\n\b~ Your session will be closed...\n");
    close(socketFileDescriptor);

}

