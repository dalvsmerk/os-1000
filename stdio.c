#include "sbi.h"

void putchar(char ch)
{
  sbi_call(ch, 0, 0, 0, 0, 0, 0, 1);

  // NOTE: May need to handle SBI error
  // https://github.com/riscv-non-isa/riscv-sbi-doc/blob/master/src/binary-encoding.adoc#table_standard_sbi_errors

  // From https://www.scs.stanford.edu/~zyedidia/docs/riscv/riscv-sbi.pdf
  // > This SBI call returns 0 upon success or an implementation specific negative error code.
  // So skipping error handling for now
}
