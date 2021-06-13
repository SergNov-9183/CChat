#ifndef UTILS_H
#define UTILS_H

#include <signal.h>

typedef char Bool;
#define TRUE 1
#define FALSE 0

#define MES_LENGTH 2048
#define DELTA 32
#define NAME_LENGTH 64
#define COMMAND_LEN 32

extern volatile sig_atomic_t run;

void catch_ctrl_c_and_exit(int signal);
void str_overwrite_stdout();
void removeNewLineSymbol (char* string, int length);

#endif // UTILS_H
