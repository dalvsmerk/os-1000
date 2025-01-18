#include "stdint.h"

void *memset(void *buf, char c, size_t n) {
  uint8_t *mem = (uint8_t *)buf;
  while (n--) {
    *mem++ = c;
  }
  return buf;
}

void *memcpy(void *dst, const void *src, size_t n) {
  uint8_t *d = (uint8_t *)dst;
  const uint8_t *s = (const uint8_t *)src;
  while (n--) {
    *d++ = *s++;
  }
  return dst;
}

/**
 * NOTE: Unsafe: no array boundaries checks
 */
char *strcpy(char *dst, const char *src) {
  while (*src) {
    *dst++ = *src++;
  }
  *dst = '\0';
  return dst;
}

int strcmp(const char *a, const char *b) {
  while (*a && *b) {
    // find non-equal bytes
    if (*a != *b)
      break;

    a++;
    b++;
  }

  // POSIX requires comparison of unsigned chars
  // https://www.man7.org/linux/man-pages/man3/strcmp.3.html#:~:text=both%20interpreted%20as%20type%20unsigned%20char
  return *(unsigned char *)a - *(unsigned char *)b;
}
