#pragma once

#include "panic.h"

// todo: replace panic with assert-specific message
#define assert(cond, msg, ...)                                                 \
  if (!(cond))                                                                 \
  panic(msg, ##__VA_ARGS__)
