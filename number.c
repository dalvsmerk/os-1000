#include "number.h"
#include "ascii.h"
#include "assert.h"
#include "stdlib.h"

int oct2dec(char *oct, size_t len) {
  assert(oct != NULL, "attempting to access NULL pointer");

  // oct 0123 to dec
  // 0 * 8^3 + 1 * 8^2 + 2 * 8^1 + 3 * 8^0
  int dec = 0;
  for (size_t i = 0; i < len; i++) {
    if (oct[i] < '0' || oct[i] > '7')
      break;

    dec = dec * 8 + from_ascii(oct[i]);
  }
  return dec;
}

/**
 * @param dec src
 * @param oct dst
 * @param len dst len
 */
void dec2oct(int dec, char *oct, size_t len) {
  assert(oct != NULL, "attempting to access NULL pointer");

  for (size_t i = len; i > 0; i--) {
    oct[i - 1] = to_ascii(dec % 8);
    dec /= 8;
  }
}
