#include "kernel.h"
#include "alloc.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

extern char __bss[], __bss_end[], __stack_top[];

void handle_trap(struct trap_frame *frame) {
  uint32_t scause = READ_CSR(scause);
  uint32_t stval = READ_CSR(stval);
  uint32_t sepc = READ_CSR(sepc);

  PANIC("unexpected trap scause=%x, stval=%x, sepc=%x", scause, stval, sepc);
}

__attribute__((naked)) __attribute__((aligned(4))) void trap_entry(void) {
  __asm__ __volatile__("csrw sscratch, sp\n"
                       "addi sp, sp, -4 * 31\n"
                       "sw ra, 4 * 0(sp)\n"
                       "sw gp, 4 * 1(sp)\n"
                       "sw tp, 4 * 2(sp)\n"
                       "sw t0, 4 * 3(sp)\n"
                       "sw t1, 4 * 4(sp)\n"
                       "sw t2, 4 * 5(sp)\n"
                       "sw t3, 4 * 6(sp)\n"
                       "sw t4, 4 * 7(sp)\n"
                       "sw t5, 4 * 8(sp)\n"
                       "sw t6, 4 * 9(sp)\n"
                       "sw a0, 4 * 10(sp)\n"
                       "sw a1, 4 * 11(sp)\n"
                       "sw a2, 4 * 12(sp)\n"
                       "sw a3, 4 * 13(sp)\n"
                       "sw a4, 4 * 14(sp)\n"
                       "sw a5, 4 * 15(sp)\n"
                       "sw a6, 4 * 16(sp)\n"
                       "sw a7, 4 * 17(sp)\n"
                       "sw s0, 4 * 18(sp)\n"
                       "sw s1, 4 * 19(sp)\n"
                       "sw s2, 4 * 20(sp)\n"
                       "sw s3, 4 * 21(sp)\n"
                       "sw s4, 4 * 22(sp)\n"
                       "sw s5, 4 * 23(sp)\n"
                       "sw s6, 4 * 24(sp)\n"
                       "sw s7, 4 * 25(sp)\n"
                       "sw s8, 4 * 26(sp)\n"
                       "sw s9, 4 * 27(sp)\n"
                       "sw s10, 4 * 28(sp)\n"
                       "sw s11, 4 * 29(sp)\n"

                       "csrr a0, sscratch\n"
                       "sw a0, 4 * 30(sp)\n"

                       "mv a0, sp\n"
                       "call handle_trap\n"

                       "lw ra, 4 * 0(sp)\n"
                       "lw gp, 4 * 1(sp)\n"
                       "lw tp, 4 * 2(sp)\n"
                       "lw t0, 4 * 3(sp)\n"
                       "lw t1, 4 * 4(sp)\n"
                       "lw t2, 4 * 5(sp)\n"
                       "lw t3, 4 * 6(sp)\n"
                       "lw t4, 4 * 7(sp)\n"
                       "lw t5, 4 * 8(sp)\n"
                       "lw t6, 4 * 9(sp)\n"
                       "lw a0, 4 * 10(sp)\n"
                       "lw a1, 4 * 11(sp)\n"
                       "lw a2, 4 * 12(sp)\n"
                       "lw a3, 4 * 13(sp)\n"
                       "lw a4, 4 * 14(sp)\n"
                       "lw a5, 4 * 15(sp)\n"
                       "lw a6, 4 * 16(sp)\n"
                       "lw a7, 4 * 17(sp)\n"
                       "lw s0, 4 * 18(sp)\n"
                       "lw s1, 4 * 19(sp)\n"
                       "lw s2, 4 * 20(sp)\n"
                       "lw s3, 4 * 21(sp)\n"
                       "lw s4, 4 * 22(sp)\n"
                       "lw s5, 4 * 23(sp)\n"
                       "lw s6, 4 * 24(sp)\n"
                       "lw s7, 4 * 25(sp)\n"
                       "lw s8, 4 * 26(sp)\n"
                       "lw s9, 4 * 27(sp)\n"
                       "lw s10, 4 * 28(sp)\n"
                       "lw s11, 4 * 29(sp)\n"
                       "lw sp, 4 * 30(sp)\n"
                       "sret");
}

void kernel_main(void) {
  memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

  WRITE_CSR(stvec, (uint32_t)trap_entry);
  printf("registered trap handler\n");

  char *ptr1 = (char *)balloc_pages(2);
  char *ptr2 = (char *)balloc_pages(1);

  printf("ptr1=%x\n", (uint32_t)ptr1);
  printf("ptr2=%x\n", (uint32_t)ptr2);

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
