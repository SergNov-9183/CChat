#include <stdio.h>
#include <stdlib.h>

#include "client.h"
#include "utils.h"

int main(int argc, char **argv){
    if(argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    signal(SIGINT, catch_ctrl_c_and_exit);

    if (init(argv[1]) == FALSE ||
        createSendingThread() == FALSE ||
        createReceivingThread() == FALSE) {
        return EXIT_FAILURE;
    }

    execute();

    return EXIT_SUCCESS;
}
