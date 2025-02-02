#include "builtins.h"

void putchar(char c);

void printf(char *fmt, ...) {
  va_list vargs;
  va_start(vargs, fmt);

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;

      switch (*fmt) {
      case '\0':
        putchar('%');
        goto end;
      case '%':
        putchar('%');
        break;
      case 's': {
        const char *s = va_arg(vargs, const char *);
        while (*s != '\0') {
          putchar(*s);
          s++;
        }
        break;
      }
      case 'd': {
        int d = va_arg(vargs, int);

        if (d < 0) {
          putchar('-');
          d = -d;
        }

        int divisor = 1;
        while (d / divisor > 9) {
          divisor *= 10;
        }

        while (divisor > 0) {
          putchar('0' + d / divisor);
          d %= divisor;
          divisor /= 10;
        }

        break;
      }
      case 'x': {
        int x = va_arg(vargs, int);
        // More readable form of hex, not to confuse with dec accidentally
        // when reading output
        putchar('0');
        putchar('x');
        for (int i = 7; i >= 0; i--) {
          int nibble = (x >> (i * 4)) & 0xf;
          putchar("0123456789abcdef"[nibble]);
        }
      }
      }
    } else {
      putchar(*fmt);
    }

    fmt++;
  }

end:
  va_end(vargs);
}
