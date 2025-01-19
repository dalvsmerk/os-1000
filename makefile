SHELL=/bin/bash

QEMU := qemu-system-riscv32

CC := /usr/local/opt/llvm/bin/clang
CFLAGS := -std=c11 -O2 -g3 -Wall -Wextra --target=riscv32 -ffreestanding -nostdlib

BREW_PREFIX := $(shell brew --prefix)
LLVM_PREFIX := $(BREW_PREFIX)/opt/llvm/bin

build:
	$(CC) $(CFLAGS) -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o kernel.elf \
		kernel.c \
		stdio.c \
		string.c

run:
	$(QEMU) -machine virt -bios default -nographic -serial mon:stdio -no-reboot \
	  -kernel kernel.elf

disas: kernel.elf
	$(LLVM_PREFIX)/llvm-objdump -d kernel.elf | less

addr2line: kernel.elf
	$(LLVM_PREFIX)/llvm-addr2line -e kernel.elf $(addr)
