SHELL=/bin/bash

QEMU := qemu-system-riscv32

CC := /usr/local/opt/llvm/bin/clang
CFLAGS := -std=c11 -O2 -g3 -Wall -Wextra --target=riscv32 -ffreestanding -nostdlib

build:
	$(CC) $(CFLAGS) -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o kernel.elf \
		kernel.c \
		stdio.c

run:
	$(QEMU) -machine virt -bios default -nographic -serial mon:stdio -no-reboot \
	  -kernel kernel.elf
