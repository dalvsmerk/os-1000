SHELL=/bin/bash

QEMU := qemu-system-riscv32

CC := /usr/local/opt/llvm/bin/clang
CFLAGS := -std=c11 -O2 -g3 -Wall -Wextra --target=riscv32 -ffreestanding -nostdlib


BREW_PREFIX := $(shell brew --prefix)
LLVM_PREFIX := $(BREW_PREFIX)/opt/llvm/bin

OBJCOPY := $(LLVM_PREFIX)/llvm-objcopy

KERSOURCE := kernel.c stdio.c string.c alloc.c shell.bin.o
USRSOURCE := shell.c user.c stdio.c

user:
	$(CC) $(CFLAGS) -Wl,-Tuser.ld -Wl,-Map=shell.map -o shell.elf $(USRSOURCE)
	$(OBJCOPY) --set-section-flags .bss=alloc,contents -O binary shell.elf shell.bin
	$(OBJCOPY) -Ibinary -Oelf32-littleriscv shell.bin shell.bin.o

kernel: shell.bin.o
	$(CC) $(CFLAGS) -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o kernel.elf $(KERSOURCE)

run:
	$(QEMU) -machine virt -bios default -nographic -serial mon:stdio -no-reboot \
	  -kernel kernel.elf

disas:
	$(LLVM_PREFIX)/llvm-objdump -d $(bin) | less

addr2line: kernel.elf
	$(LLVM_PREFIX)/llvm-addr2line -e kernel.elf $(addr)

lsnames:
	$(LLVM_PREFIX)/llvm-nm $(bin)
