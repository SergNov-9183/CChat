#include "commands.h"
#include "utils.h"
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
  while(str[--index]==' ' || str[--index]=='\n');

  if(str[0] == '#' && str[index] != '#')
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
  printf("-------------------------------------\n");
  while(!feof(help))
    {
      fgets(str, NAME_LENGTH, help);
      printf("%s", str);
    }
  printf("-------------------------------------\n");
  str_overwrite_stdout();
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
  str_overwrite_stdout();
  return;
}

char** SplitInit(char* message)
{
  int gk = GrillCounter(message) + 1;
  //printf("grillcount = %d\nmessage: %s\n", gk, message);
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
  char command[COMMAND_LEN] = {0};
  char buffer[LENGTH] = {0};
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
           {
             printf("You have been to the Central Room. Have a good time!\n");
             send(socketFileDescriptor, message, strlen(message), 0);
             str_overwrite_stdout();
           }
         else
           {
             if(!strcmp(command, "PRIVATE"))
             {
               if(IsCommandCorrect(message, 3))
                 {
                   //printf("Request: %s\n", message);
                   SendRequest(socketFileDescriptor, message, "Request sent (for Private Dialogue)");
                   str_overwrite_stdout();
                 }
               else
                 {
                   printf("\b~ Invalid syntax. Unable to create Private Room.\n");
                   printf(">~ Correct syntax: #private#nickname#\n");
                   str_overwrite_stdout();
                 }
             }
            else
             {
                if(!strcmp(command, "ADD_FRIEND"))
                  {
                    if(IsCommandCorrect(message, 3))
                      {
                        SendRequest(socketFileDescriptor, message, "Request sent (Adding Friend)");

                      }
                    else
                      {
                        printf("\b~ Invalid syntax. Unable to Add Friend.\n");
                        printf(">~ Correct syntax: #add_friend#nickname#\n");
                        str_overwrite_stdout();
                      }
                  }
                else
                  {
                    if(!strcmp(command, "REMOVE_FRIEND"))
                      {
                        if(IsCommandCorrect(message, 3))
                          {
                            SendRequest(socketFileDescriptor, message, "Request sent (Removing Friend)");
                            str_overwrite_stdout();
                          }
                        else
                          {
                            printf("\b~ Invalid syntax. Unable to Remove Friend.\n");
                            printf(">~ Correct syntax: #remove_friend#nickname#\n");
                            str_overwrite_stdout();
                          }
                      }
                    else
                      {
                        if(!strcmp(command, "CREATE_LOCAL_CHAT"))
                          {
                            if(IsCommandCorrect(message, 2))
                              {
                                SendRequest(socketFileDescriptor, message,  "Request sent (Creating Private Room)");
                                str_overwrite_stdout();
                              }
                            else
                              {
                                printf("\b~ Invalid syntax. Unable to Create Local Chat.\n");
                                printf(">~ Correct syntax: #create_local_chat#\n");
                                str_overwrite_stdout();
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
                                        printf("\b~ Input your data according to pattern:\n");
                                        printf(">~ \t#NEW_CLIENT#fullname#password#\n");
                                        str_overwrite_stdout();
                                        //send(socketFileDescriptor, message, strlen(message), 0);

                                      }
                                    if(!strcmp(command, "N"))
                                      {
                                        //printf("\b~ Your session will be closed.\n");
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
                                        SendRequest(socketFileDescriptor, message,  "Request sent (To Create an Account)");
                                        str_overwrite_stdout();
                                      }
                                    else
                                      {
                                        printf("\b~ Invalid syntax. Unable to create New Account.\n");
                                        printf(">~ Correct syntax: #NEW_CLIENT#fullname#password#\n");
                                        str_overwrite_stdout();
                                      }
                                  }
                                else
                                  {
                                    if(!strcmp(command, "CLIENT_LIST"))
                                      {
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
                                        if(!ind)
                                          {
                                            printf("\b~ There is nobody in this (central) room. Waiting for others...\n");
                                            str_overwrite_stdout();
                                          }
                                        else
                                          {
                                            printf("\b~ Only %d ", ind);
                                            if(ind == 1)
                                              printf("participant ");
                                            else
                                              printf("participants ");
                                            printf("now:\n");
                                            for(int i = 0; i < ind; ++i)
                                              {
                                                printf("\t\t%s\n", clients[i]);
                                              }
                                          }
                                        str_overwrite_stdout();
                                        fclose(clientsList);
                                      }
                                    else
                                      {
                                        if(!strcmp(command, "ADD_ROOM"))
                                          {
                                            if(IsCommandCorrect(message, 3))
                                              {
                                                char buf[LENGTH] = {0};
                                                char ** splMes = SplitInit(message + sizeof(char));
                                                sprintf(buf, "---\nYou have just created the Common Room (%s).", splMes[1]);
                                                SendRequest(socketFileDescriptor, message,  buf);
                                                printf(">~ You can use #Help# to get more specific information.\n");
                                                str_overwrite_stdout();
                                              }
                                            else
                                            {
                                                printf("\b~ Invalid syntax. Unable to create a Common Room.\n");
                                                printf(">~ Correct syntax: #ADD_ROOM#roomname#\n");
                                                str_overwrite_stdout();
                                            }
                                          }
                                        else
                                          {
                                            if(!strcmp(command, "GO_TO_ROOM"))
                                              {

                                                if(IsCommandCorrect(message, 3))
                                                  {
                                                    char buf[LENGTH] = {0};
                                                    char ** splMes = SplitInit(message + sizeof(char));
                                                    sprintf(buf, "\b~ You have moved to the Common Room (%s).", splMes[1]);
                                                    SendRequest(socketFileDescriptor, message, buf);
                                                    printf(">~ You can use #Help# to get more relevant information.\n");
                                                    str_overwrite_stdout();
                                                  }
                                                else
                                                {
                                                    printf("\b~ Invalid syntax. Unable to Move to the Common Room.\n");
                                                    printf(">~ Correct syntax: #GO_TO_ROOM#roomname#\n");
                                                    str_overwrite_stdout();
                                                }
                                              }
                                            else
                                              {
                                                if(!strcmp(command, "INVITE_CLIENT"))
                                                  {

                                                    if(IsCommandCorrect(message, 3))
                                                      {
                                                        char ** splMes = SplitInit(message + sizeof(char));
                                                        sprintf(buffer, "\b~ You've just invited %s to this Room.\n", splMes[1]);
                                                        SendRequest(socketFileDescriptor, message,  buffer);
                                                        str_overwrite_stdout();
                                                      }
                                                    else
                                                    {
                                                        printf("\b~ Invalid syntax. Unable to Create New Account.\n");
                                                        printf(">~ Correct syntax: #INVITE_CLIENT#nickname#\n");
                                                        str_overwrite_stdout();
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

    }
  else
    {
      sprintf(buffer, "%s\n", message);
      send(socketFileDescriptor, buffer, strlen(buffer), 0);
      //printf("Message sent.\n");
      //fflush(stdout);
    }
  return 0;
}
