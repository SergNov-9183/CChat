#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "client.h"
#include "server.h"

#ifdef _WIN32
#include <winsock2.h>
    BOOL WINAPI windowsEventHandler(DWORD signal) {
        BOOL result = FALSE;
        if (CTRL_C_EVENT == signal || CTRL_CLOSE_EVENT == signal) {
            catchCtrlC(0);
            result = TRUE;
        }
        return result;
    }
#endif

int main(int argc, char** argv){
    if(argc != 2){
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

#ifdef _WIN32
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;

    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        return EXIT_FAILURE;
    }

    SetConsoleCtrlHandler(windowsEventHandler, TRUE);
#else
    signal(SIGINT, catchCtrlC);
#endif

    if (init(argv[1]) == FALSE) {
        closeSocket(listenFileDescriptor);
#ifdef _WIN32
        WSACleanup();
#endif
        return EXIT_FAILURE;
    }

    printf("Chat server started\n");

    loadRooms();
    loadClients();
    execute();
    closeAllClients();

    printf("Chat server unloaded\n");

#ifdef _WIN32
    WSACleanup();
#endif
    return EXIT_SUCCESS;
}
