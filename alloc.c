#include "kernel.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

#define PAGE_SIZE 4096 // 4kb

extern uint32_t __free_ram[], __free_ram_end[];

/**
 * Bump allocator, allocates n pages of PAGE_SIZE
 */
void *balloc_pages(size_t n) {
  static uint32_t *next_free_addr = (uint32_t *)__free_ram;
  uint32_t *ptr = next_free_addr;
  next_free_addr += n * PAGE_SIZE;

  if ((uint32_t)next_free_addr > (uint32_t)__free_ram_end) {
    PANIC("out of memory");
  }

  memset(ptr, 0, PAGE_SIZE);
  return (void *)ptr;
}
