#pragma once

#include "stdio.h"

// syscall numbers
#define SYS_PUTCHAR 1
#define SYS_GETCHAR 2
#define SYS_EXIT 3
#define SYS_READFILE 4
#define SYS_WRITEFILE 5

int syscall(int sysno, int arg0, int arg1, int arg2);

__attribute__((noreturn)) void exit(void);

int readfile(const char *filename, char *buf, int len);

int writefile(const char *filename, const char *buf, int len);
