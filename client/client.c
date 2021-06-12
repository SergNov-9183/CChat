#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "Gen_util.h"
#define MES_LENGTH 2048
#define DELTA 32
#define NAME_LENGTH 64

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

// Global variables
int client_socket = 0;
char name[NAME_LENGTH];
volatile sig_atomic_t flag = 0;



void ExitByCtrlC(int sig)
{
  flag = 1;
  return;
}

void MakeNullTermStr (char* arr, int length) {    //Перебрось ф-ю в Gen_util.c и раскомменнтируй её объявление в Gen_util.h. (у меня сейчас всё работает)
  int i;
  for (i = 0; i < length; i++) { // trim \n
      if (arr[i] == '\n') {
          arr[i] = '\0';
          break;
        }
    }
}


void str_overwrite_stdout() {
  printf("%s", "> ");
  fflush(stdout);
}


void send_msg_handler() {
  char message[MES_LENGTH] = {0};
  char buffer[MES_LENGTH + NAME_LENGTH] = {0};

  while(1)
  {
      str_overwrite_stdout();
      fgets(message, MES_LENGTH, stdin);
      MakeNullTermStr(message, MES_LENGTH);

      if (strcmp(message, "exit") == 0) {
          break;
        } else {
          sprintf(buffer, "%s: %s\n", name, message);
          send(client_socket, buffer, strlen(buffer), 0);
        }

      bzero(message, MES_LENGTH);
      bzero(buffer, MES_LENGTH + NAME_LENGTH);
  }
  ExitByCtrlC(2);
}

void recv_msg_handler() {
  char message[MES_LENGTH] = {};
  while (1) {
      int receive = recv(client_socket, message, MES_LENGTH, 0);
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

int main(int argc, char **argv){
  if(argc != 2){
      printf("Usage: %s <port>\n", argv[0]);
      return EXIT_FAILURE;
    }

  char *IP = "127.0.0.1";
  int port = atoi(argv[1]);

  signal(SIGINT, ExitByCtrlC);

  printf("Please enter your name: ");
  fgets(name, NAME_LENGTH, stdin);
  MakeNullTermStr(name, strlen(name));


  if (strlen(name) > NAME_LENGTH || strlen(name) < 2){
      printf("Name must be less than 30 and more than 2 characters.\n");
      return EXIT_FAILURE;
    }

  struct sockaddr_in server;

  /* Socket settings */
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(client_socket == INVALID_SOCKET)
    {
      printf("Error create socket. Try again later.\n");
      return -1;
    }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(IP);
  server.sin_port = htons(port);


  // Connect to Server
  if (connect(client_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
      printf("ERROR: connect\n");
      close(client_socket);
      return EXIT_FAILURE;
    }

  // Send name
  send(client_socket, name, NAME_LENGTH, 0);

  printf("=== WELCOME TO THE CHATROOM ===\n");

  pthread_t send_msg_thread;
  if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
      printf("ERROR: pthread\n");
      return EXIT_FAILURE;
    }

  pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
      printf("ERROR: pthread\n");
      return EXIT_FAILURE;
    }

  while (1){
      if(flag){
          printf("\nBye\n");
          break;
        }
    }

  close(client_socket);

  return EXIT_SUCCESS;
}
