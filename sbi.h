#pragma once

/**
 * From
 * https://github.com/riscv-non-isa/riscv-sbi-doc/blob/master/src/binary-encoding.adoc
 */
struct sbiret {
  long error;
  long value;
};

struct sbiret sbi_call(long n, long a, long b, long c, long d, long e, long fid,
                       long eid) {
  register long a0 __asm__("a0") = n;
  register long a1 __asm__("a1") = a;
  register long a2 __asm__("a2") = b;
  register long a3 __asm__("a3") = c;
  register long a4 __asm__("a4") = d;
  register long a5 __asm__("a5") = e;
  register long a6 __asm__("a6") = fid;
  register long a7 __asm__("a7") = eid;

  __asm__ __volatile__("ecall"
                       : "=r"(a0), "=r"(a1)
                       : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                         "r"(a6), "r"(a7)
                       : "memory");

  return (struct sbiret){.error = a0, .value = a1};
}
