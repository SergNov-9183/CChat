#include "commands.h"
#include <sys/socket.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define NAME_LENGTH 64
#define LENGTH 2048
#define COMMAND_LEN 32
#define MAX_CLIENTS 50

int IsCommand(char* str)
{
  //printf("%s", "Command?\n");
  int index = strlen(str);
  while(str[--index]==' ');

  if(str[0] == '#')
      return 1;
  if(str[0] == '#' && str[index] == '#')
      return 2;
  return 0;
}

int GrillCounter(char* message)
{
  int num = 0;
  for(int i = 0; i < strlen(message); ++i)
    {
      if(message[i] == '#')
        ++num;
    }
  return num;
}

void CommandDefAndRegUp(char* str, char* command)
{
  int i = 0; //первый символ имени команды
  while(str[++i]!='#')
    {
      if(str[i]>='a' && str[i]<='z')
        str[i]-=32;
      command[i-1] = str[i];
    }
  return;
}

void HelpCommand()
{
  FILE* help = fopen("HELP.txt", "rt");
  if(!help)
    printf("error open file\n");
  char str[NAME_LENGTH] = {0};
  printf("-------------------------------------");
  printf("\n");
  while(!feof(help))
    {
      fgets(str, NAME_LENGTH, help);
      printf("%s", str);
    }
  printf("\n-------------------------------------\n");
  fclose(help);
  return;
}

int IsCommandCorrect (char* str, int numberOfGrills)
{
  unsigned grillCounter = 0;
  for(int i = 0; i< strlen(str); ++i)
    {
      if(str[i] == '#')
        ++grillCounter;
      if(i>0 && str[i] == str[i-1] && str[i-1] == '#')
        return 0;
    }
  if(grillCounter == numberOfGrills)
    return 1;
  return 0;
}

char** SplitString(char* str, const char a_delim)
{
    char* a_str = malloc(sizeof(char)*strlen(str)+1);
    strcpy(a_str, str);
    char** result = 0;
    size_t count = 0;
    char* tmp = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    count += last_comma < (a_str + strlen(a_str) - 1);

    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }
    free(a_str);
    return result;
}

void SendRequest(int socketFileDescriptor, char* textOfRequest, char *message)
{
  int res = send(socketFileDescriptor, textOfRequest, strlen(textOfRequest), 0);
  if(res!=-1)
    printf("%s\n", message);
  else
    printf("Warning!!!\n");
}

char** SplitInit(char* message)
{
  int gk = GrillCounter(message) + 1;
  printf("grillcount = %d\nmessage: %s\n", gk, message);
  char** splitted = (char**)malloc(gk * sizeof (char*));
  for (int i = 0; i < gk; ++i)
    {
      splitted[i] = (char*)malloc(NAME_LENGTH * sizeof (char));
    }
  splitted = SplitString(message, '#');
  return splitted;
}

int CommandAnalyzer(char* name, char* message, int socketFileDescriptor)
{
  char buffer[LENGTH + 32] = {0};
  char command[COMMAND_LEN] = {0};
  if(IsCommand(message))
    {
      //printf("%s - message\n", message);
      CommandDefAndRegUp(message, command);
      //printf("%s - command\n", command);
     if(!strcmp(command, "HELP"))
        HelpCommand();
     else
       {
         if(!strcmp(command, "EXIT"))
           return -1;
         else
           {
             if(!strcmp(command, "PRIVATE"))
             {
               if(IsCommandCorrect(message, 3))
                 {
                   SendRequest(socketFileDescriptor, message, "Request sent (for Private Messaging)");

                 }
               else
                 {
                   printf("Invalid syntax. Unable to create Private Chat.\n");
                   printf("Correct syntax: #private#nickname#\n");
                 }
             }
            else
             {
                if(!strcmp(command, "ADD_FRIEND"))
                  {
                    if(IsCommandCorrect(message, 3))
                      {
                        SendRequest(socketFileDescriptor, message, "Request sent (Adding friend)");

                      }
                    else
                      {
                        printf("Invalid syntax. Unable to Add Friend.\n");
                        printf("Correct syntax: #add_friend#nickname#\n");
                      }
                  }
                else
                  {
                    if(!strcmp(command, "REMOVE_FRIEND"))
                      {
                        if(IsCommandCorrect(message, 3))
                          {
                            SendRequest(socketFileDescriptor, message, "Request sent (Removing Friend)");

                          }
                        else
                          {
                            printf("Invalid syntax. Unable to Remove Friend.\n");
                            printf("Correct syntax: #remove_friend#nickname#\n");
                          }
                      }
                    else
                      {
                        if(!strcmp(command, "CREATE_LOCAL_CHAT"))
                          {
                            if(IsCommandCorrect(message, 2))
                              {
                                SendRequest(socketFileDescriptor, message,  "Request sent (Creating Local Chat)");

                              }
                            else
                              {
                                printf("Invalid syntax. Unable to Create Local Chat.\n");
                                printf("Correct syntax: #create_local_chat#\n");
                              }
                          }
                        else
                          {
                            if(!strcmp(command, "Y") || !strcmp(command, "N"))
                              {
                                if(IsCommandCorrect(message, 2))
                                  {
                                    if(!strcmp(command, "Y"))
                                      {
                                        printf("Input your data according to pattern:\n");
                                        printf("\t#NEW_CLIENT#fullname#password#\n");
                                        send(socketFileDescriptor, message, strlen(message), 0);

                                      }
                                    if(!strcmp(command, "N"))
                                      {
                                        printf("Your session will be closed.\n");
                                        return -1;

                                      }
                                  }
                              }
                            else
                              {
                                if(!strcmp(command, "NEW_CLIENT"))
                                  {
                                    printf("#NEW_CLIENT# called\n");
                                    if(IsCommandCorrect(message, 4))
                                      {
                                        printf("CommandCorrect\nSent:%s;\n", message);
                                        SendRequest(socketFileDescriptor, message,  "Request sent (To create an account)");
                                      }
                                    else
                                      {
                                        printf("CommandINcorrect\n");
                                        printf("Invalid syntax. Unable to Create New Account.\n");
                                        printf("Correct syntax: #NEW_CLIENT#fullname#password#\n");
                                      }
                                  }
                                else
                                  {
                                    if(!strcmp(command, "WELCOME"))
                                      {
                                        str_overwrite_stdout();
                                        FILE* clientsList = fopen("common_clients.txt", "wt");

                                        char ** splMes = SplitInit(message + sizeof(char));
                                        printf("Successfully connected!\n");
                                        printf("Now:\t%s participants:\n", splMes[1]);
                                        for(int i = 2; i < 2 + atoi(splMes[1]); ++i)
                                          {
                                            fprintf(clientsList, "%s\n", splMes[i]);
                                            printf("\t\t%s\n", splMes[i]);
                                          }
                                        printf("Now you can send message in common chat or use #HELP# to get more detailed information.\n");
                                        fclose(clientsList);
                                        free(splMes);
                                      }
                                    else
                                      {
                                        if(!strcmp(command, "CLIENT_ENTER"))
                                          {
                                            str_overwrite_stdout();
                                            FILE* clientsList = fopen("common_clients.txt", "a+");
                                            char ** splMes = SplitInit(message + sizeof(char));
                                            printf("%s has joined\n", splMes[1]);
                                            fprintf(clientsList, "%s\n", splMes[1]);
                                            fclose(clientsList);
                                            free(splMes);
                                          }
                                        else
                                          {
                                            if(!strcmp(command, "CLIENT_LEFT"))
                                              {
                                                str_overwrite_stdout();
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
                                                    printf("str = %s, ind = %d\n", str, ind);
                                                    strncpy(clients[ind++], str, NAME_LENGTH);
                                                  }
                                                printf("Final ind = %d\n", ind);
                                                fclose(clientsList);
                                                for(int i = 0; i < ind; ++i)
                                                  {
                                                    clients[i][strlen(clients[i])-1] = '\0';
                                                    if(!strcmp(clients[i], splMes[1]))
                                                      bzero(clients[i], strlen(clients[i]));
                                                  }
                                                bzero(clients[ind-1], strlen(clients[ind-1]));
                                                clientsList = fopen("common_clients.txt", "wt");
                                                printf("clients:\n");
                                                for(int i = 0; i < ind; ++i)
                                                  {
                                                    printf("%s", clients[i]);
                                                    if(strlen(clients[i]))
                                                      {
                                                        fprintf(clientsList, "%s\n", clients[i]);
                                                      }
                                                  }
                                                printf("%s has left\n", splMes[1]);
                                                fclose(clientsList);
                                                free(splMes);
                                              }
                                            else
                                              {
                                                if(!strcmp(command, "LIST_CLIENT"))
                                                  {
                                                    str_overwrite_stdout();
                                                    char clients [MAX_CLIENTS][NAME_LENGTH] = {0};
                                                    FILE* clientsList = fopen("common_clients.txt", "rt");
                                                    int ind = 0;
                                                    char str[NAME_LENGTH] = {0};
                                                    while(!feof(clientsList))
                                                      {
                                                        fgets(str, NAME_LENGTH, clientsList);
                                                        str[strlen(str)] = '\0';
                                                        printf("str = %s, ind = %d\n", str, ind);
                                                        strncpy(clients[ind++], str, NAME_LENGTH);
                                                      }
                                                    --ind;
                                                    printf("Now:\t%d participants:\n", ind);
                                                    for(int i = 0; i < ind; ++i)
                                                      {
                                                        printf("%s", clients[i]);
                                                      }
                                                    fclose(clientsList);
                                                  }
                                              }
                                          }

                                      }
                                  }
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
      //if(!IsCommand(message))
      send(socketFileDescriptor, message, strlen(message), 0);
      //printf("Message sent.\n");
      //fflush(stdout);
    }
  return 0;
}
