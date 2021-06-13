#include "commands.h"
#include <sys/socket.h>
#define NAME_LENGTH 128
#define LENGTH 2048
#define COMMAND_LEN 32
int IsCommand(char* str)
{
  //printf("%s", "Command?\n");
  int index = strlen(str);
  while(str[--index]==' ');
  if(str[0] == '#' && str[index] == '#')
    return 1;
  return 0;
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

void SendRequest(int socketFileDescriptor, char* textOfRequest, char *message)
{
  printf("%s\n", message);
  send(socketFileDescriptor, textOfRequest, strlen(textOfRequest), 0);
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
                   sprintf(buffer, "%s: %s\n", name, message);
                   SendRequest(socketFileDescriptor, buffer, "Request sent (for Private Messaging)");
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
                        sprintf(buffer, "%s: %s\n", name, message);
                        SendRequest(socketFileDescriptor, buffer, "Request sent (Adding friend)");
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
                            sprintf(buffer, "%s: %s\n", name, message);
                            SendRequest(socketFileDescriptor, buffer, "Request sent (Removing Friend)");
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
                                sprintf(buffer, "%s: %s\n", name, message);
                                SendRequest(socketFileDescriptor, buffer,  "Request sent (Creating Local Chat)");
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
                                        sprintf(buffer, "%s: %s\n", name, message);
                                        send(socketFileDescriptor, buffer, strlen(buffer), 0);
                                      }
                                    if(!strcmp(command, "N"))
                                      {
                                        printf("Your session will be closed.\n");
                                        close(socketFileDescriptor);
                                      }
                                  }
                              }
                            else
                              {
                                if(!strcmp(command, "NEW_CLIENT"))
                                  {
                                    if(IsCommandCorrect(message, 4))
                                      {
                                        sprintf(buffer, "%s: %s\n", name, message);
                                        SendRequest(socketFileDescriptor, buffer,  "Request sent (To create an account)");
                                      }
                                    else
                                      {
                                        printf("Invalid syntax. Unable to Create New Account.\n");
                                        printf("Correct syntax: ##NEW_CLIENT#fullname#password#\n");
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
      sprintf(buffer, "%s: %s\n", name, message);
      send(socketFileDescriptor, buffer, strlen(buffer), 0);
    }
  return 0;
}
