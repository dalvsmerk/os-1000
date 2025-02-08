#pragma once

#include "stdio.h"

__attribute__((noreturn)) void exit(void);

int syscall(int sysno, int arg0, int arg1, int arg2);

// syscall numbers
#define SYS_PUTCHAR 1
#define SYS_GETCHAR 2
#define SYS_EXIT 3
