#include "kernel.h"
#include "alloc.h"
#include "builtins.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define PROC_POOL_SIZE 8

// process states
#define PROC_UNUSED 0
#define PROC_RUNNABLE 1

#define PROC_STACK_SIZE 8192 // 8kb

extern char __bss[], __bss_end[], __stack_top[];

void handle_trap(struct trap_frame *frame) {
  uint32_t scause = READ_CSR(scause);
  uint32_t stval = READ_CSR(stval);
  uint32_t sepc = READ_CSR(sepc);

  PANIC("unexpected trap scause=%x, stval=%x, sepc=%x", scause, stval, sepc);
}

__attribute__((naked)) __attribute__((aligned(4))) void trap_entry(void) {
  __asm__ __volatile__(
      // user stack pointer -- sp;
      // kernel stack pointer -- sscratch;
      // swap sp and sscratch
      // kernel stack pointer -> sp
      // user stack pointer -> sscratch
      "csrrw sp, sscratch, sp\n"

      // save registers to kernel stack
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

      // save user stack pointer at the time of exception
      "csrr a0, sscratch\n"

      // save user stack pointer to the end of kernel stack
      "sw a0, 4 * 30(sp)\n"

      // reset kernel stack pointer
      "addi a0, sp, 4 * 31\n"
      "csrw sscratch, a0\n"

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

__attribute__((naked)) void switch_context(uint32_t *prev_sp,
                                           uint32_t *next_sp) {
  __asm__ __volatile__("addi sp, sp, -13 * 4\n"
                       "sw ra, 4 * 0(sp)\n"
                       "sw s0, 4 * 1(sp)\n"
                       "sw s1, 4 * 2(sp)\n"
                       "sw s2, 4 * 3(sp)\n"
                       "sw s3, 4 * 4(sp)\n"
                       "sw s4, 4 * 5(sp)\n"
                       "sw s5, 4 * 6(sp)\n"
                       "sw s6, 4 * 7(sp)\n"
                       "sw s7, 4 * 8(sp)\n"
                       "sw s8, 4 * 9(sp)\n"
                       "sw s9, 4 * 10(sp)\n"
                       "sw s10, 4 * 11(sp)\n"
                       "sw s11, 4 * 12(sp)\n"

                       "sw sp, (a0)\n"
                       "lw sp, (a1)\n"

                       "lw ra, 4 * 0(sp)\n"
                       "lw s0, 4 * 1(sp)\n"
                       "lw s1, 4 * 2(sp)\n"
                       "lw s2, 4 * 3(sp)\n"
                       "lw s3, 4 * 4(sp)\n"
                       "lw s4, 4 * 5(sp)\n"
                       "lw s5, 4 * 6(sp)\n"
                       "lw s6, 4 * 7(sp)\n"
                       "lw s7, 4 * 8(sp)\n"
                       "lw s8, 4 * 9(sp)\n"
                       "lw s9, 4 * 10(sp)\n"
                       "lw s10, 4 * 11(sp)\n"
                       "lw s11, 4 * 12(sp)\n"

                       "addi sp, sp, 13 * 4\n"
                       "ret\n");
}

void map_page(uint32_t *table1, uint32_t vaddr, paddr_t paddr, uint32_t flags) {
  if (!is_aligned(vaddr, PAGE_SIZE)) {
    PANIC("vaddr %x is not aligned to 4kb page size", vaddr);
  }

  if (!is_aligned(paddr, PAGE_SIZE)) {
    PANIC("paddr %x is not aligned to 4kb page size", paddr);
  }

  uint32_t vpn1 = (vaddr >> 22) & 0x3FF;

  if ((table1[vpn1] & PAGE_V) == 0) {
    uint32_t pt_paddr = (uint32_t)balloc_pages(1);
    // Shift right by lower 12 bits (by PAGE_SIZE) and align paddr
    // to page table entry format
    // 31..20 | 19..10 | 9.7 | 6...0
    // PPN[1] | PPN[0] | RWS | FLAGS
    table1[vpn1] = (pt_paddr / PAGE_SIZE) << 10 | PAGE_V;
  }

  uint32_t vpn0 = (vaddr >> 12) & 0x3FF;
  // Basically, restore pt_paddr allocated above
  uint32_t *table0 = (uint32_t *)((table1[vpn1] >> 10) * PAGE_SIZE);
  table0[vpn0] = ((paddr / PAGE_SIZE) << 10) | flags | PAGE_V;
}

extern char __kernel_base[], __free_ram_end[];

struct process {
  int pid;
  int state;
  vaddr_t sp;
  uint32_t *page_table;
  uint32_t stack[PROC_STACK_SIZE];
};

struct process proc_pool[PROC_POOL_SIZE];

struct process *create_process(uint32_t pc) {
  struct process *proc = NULL;

  int i;
  for (i = 0; i < PROC_POOL_SIZE; i++) {
    if (proc_pool[i].state == PROC_UNUSED) {
      proc = &proc_pool[i];
      break;
    }
  }

  if (proc == NULL) {
    PANIC("no available slots for new process");
  }

  uint32_t *sp = (uint32_t *)&proc->stack[sizeof(proc->stack)];

  *--sp = 0;  // s11
  *--sp = 0;  // s10
  *--sp = 0;  // s9
  *--sp = 0;  // s8
  *--sp = 0;  // s7
  *--sp = 0;  // s6
  *--sp = 0;  // s5
  *--sp = 0;  // s4
  *--sp = 0;  // s3
  *--sp = 0;  // s2
  *--sp = 0;  // s1
  *--sp = pc; // ra

  uint32_t *page_table = (uint32_t *)balloc_pages(1);
  for (paddr_t paddr = (paddr_t)__kernel_base; paddr < (uint32_t)__free_ram_end;
       paddr += PAGE_SIZE) {
    // vaddr = paddr for kernel memory map
    // each kernel process will have memory map to all memory: .text, .data, etc
    map_page(page_table, paddr, paddr, PAGE_R | PAGE_W | PAGE_X);
  }

  proc->pid = i + 1;
  proc->state = PROC_RUNNABLE;
  proc->page_table = page_table;
  proc->sp = (uint32_t)sp;
  return proc;
}

void delay(void) {
  for (int i = 0; i < 30000000; i++) {
    __asm__ __volatile__("nop");
  }
}

struct process *current_proc;
struct process *idle_proc;

void yield_roundrobin(void) {
  struct process *next = idle_proc;

  for (int i = 0; i < PROC_POOL_SIZE; i++) {
    struct process *proc = &proc_pool[(current_proc->pid + i) % PROC_POOL_SIZE];
    int runnable = proc->state == PROC_RUNNABLE;
    int non_idle = proc->pid > 0;

    // in theory, check for runnable should be enough;
    // our implementation doesn't free up slots in process pool;
    // check for non idle just in case memory is in invalid state
    if (runnable && non_idle) {
      next = proc;
      break;
    }
  }

  if (next == current_proc) {
    // No need to switch context if there is only one runnable process
    return;
  }

  __asm__ __volatile__(
      "sfence.vma\n"
      "csrw satp, %[satp]\n"
      "sfence.vma\n"
      "csrw sscratch, %[sscratch]\n"
      :
      // Enable SV32 virtual address translation mode
      // and setup pointer to process root page table
      : [satp] "r"(SATP_SV32 | ((uint32_t)next->page_table / PAGE_SIZE)),
        [sscratch] "r"((uint32_t)&next->stack[sizeof(next->stack)]));

  struct process *prev = current_proc;
  current_proc = next;
  switch_context(&prev->sp, &next->sp);
}

struct process *proc_a;
struct process *proc_b;

void proc_a_entry(void) {
  printf("starting process A\n");
  // int i = 0;
  // while (i < 100) {
  while (1) {
    // printf("A%d\n", i);
    putchar('A');
    yield_roundrobin();
    // i++;
  }
}

void proc_b_entry(void) {
  printf("starting process B\n");
  // int i = 0;
  // while (i < 100) {
  while (1) {
    // printf("B%d\n", i);
    putchar('B');
    yield_roundrobin();
    // i++;
  }
}

void kernel_main(void) {
  memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

  WRITE_CSR(stvec, (uint32_t)trap_entry);
  printf("registered trap handler\n");

  idle_proc = create_process((uint32_t)NULL);
  idle_proc->pid = -1;
  current_proc = idle_proc;

  proc_a = create_process((uint32_t)proc_a_entry);
  proc_b = create_process((uint32_t)proc_b_entry);
  yield_roundrobin();
  PANIC("switched to idle process");

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
