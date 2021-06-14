#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdio.h>
#include <string.h>
int IsCommand(char *str);
void CommandDefAndRegUp(char* str, char* command);
void HelpCommand();
int IsCommandCorrect(char* str, int numberOfGrills);
void SendRequest(int socketFileDescriptor, char* textOfRequest, char *message);
int CommandAnalyzer(char* name, char* message,int socketFileDescriptor);
char** SplitInit(char* message);
#endif // COMMANDS_H
