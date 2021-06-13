#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "server.h"

int main(int argc, char** argv){
    if(argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    signal(SIGINT, catchCtrlC);

    if (init(argv[1]) == FALSE) {
        return EXIT_FAILURE;
    }

    printf("Chat server started\n");

    loadClients();
    execute();
    closeAllClients();

    printf("Chat server unloaded\n");

    return EXIT_SUCCESS;
}
