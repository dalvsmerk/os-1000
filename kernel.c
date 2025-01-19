#include "kernel.h"
#include "stdio.h"
#include "string.h"

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

extern char __bss[], __bss_end[], __stack_top[];

void kernel_main(void) {
  memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

  PANIC("unexpected state");

  const char *s = "Hello World!";
  printf("\n\nMessage %d from %x kernel: %s\n", 42, 123456, s);

  for (;;) {
    __asm__ __volatile__("wfi");
  }
}

__attribute__((section(".text.boot"))) __attribute__((naked)) void boot(void) {
  __asm__ __volatile__("mv sp, %[stack_top]\n"
                       "j kernel_main\n"
                       :
                       : [stack_top] "r"(__stack_top));
}
