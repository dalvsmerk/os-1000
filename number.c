#include "number.h"
#include "assert.h"
#include "stdlib.h"

int oct2dec(char *oct, size_t size) {
  assert(oct != NULL, "fs: attempting to access NULL pointer");

  // oct 0123 to dec
  // 0 * 8^3 + 1 * 8^2 + 2 * 8^1 + 3 * 8^0
  int dec = 0;
  for (int i = 0; i < size; i++) {
    if (oct[i] < '0' || oct[i] > '7')
      break;

    dec = dec * 8 + (oct[i] - '0');
  }
  return dec;
}
