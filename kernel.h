#pragma once

#include "stdio.h"

// do {} while (0) allows multi-line macros to be expanded correctly
// https://www.jpcert.or.jp/sc-rules/c-pre10-c.html
#define PANIC(fmt, ...)                                                            \
  do {                                                                             \
    printf("Kernel panic at %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    while (1) {};                                                                  \
  } while (0)
