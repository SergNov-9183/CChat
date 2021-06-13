#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "server.h"

int main(int argc, char **argv){
    if(argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Ignore pipe signals
    signal(SIGPIPE, SIG_IGN);

    if (init(argv[1]) == FALSE) {
        return EXIT_FAILURE;
    }

    printf("Chat server started\n");

    execute();

    return EXIT_SUCCESS;
}
