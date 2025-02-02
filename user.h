#pragma once
#include "stdio.h"

__attribute__((noreturn)) void exit(void);

void syscall(int sysno, int arg0, int arg1, int arg2);

// syscall numbers
#define SYS_PUTCHAR 1
