SHELL=/bin/bash

QEMU := qemu-system-riscv32

CC := /usr/local/opt/llvm/bin/clang
CFLAGS := -std=c11 -O2 -g3 -Wall -Wextra --target=riscv32 -ffreestanding -nostdlib


BREW_PREFIX := $(shell brew --prefix)
LLVM_PREFIX := $(BREW_PREFIX)/opt/llvm/bin

OBJCOPY := $(LLVM_PREFIX)/llvm-objcopy

KERSOURCE := kernel.c stdio.c string.c alloc.c virtioblk.c
USRSOURCE := shell.c user.c stdio.c string.c

help:
	@echo "Makefile for OS in 1000 lines"
	@echo
	@echo "Build"
	@echo "    user        build user-mode application"
	@echo "    kernel      build kernel"
	@echo "    clean       remove all compilation artifacts"
	@echo
	@echo "Execute"
	@echo "    run         run QEMU virtual machine based on kernel.elf binary"
	@echo
	@echo "Debugging"
	@echo "    disas       show disassembly of a binary, example:"
	@echo "                make disas bin=kernel.elf"
	@echo "    addr2line   show physical address of kernel line of code, example:"
	@echo "                make addr2line addr=0x800802b9"
	@echo "    lsnames     list names of compiled objects of a binary, example:"
	@echo "                make lsnames bin=shell.bin.o"

user:
	$(CC) $(CFLAGS) -Wl,-Tuser.ld -Wl,-Map=shell.map -o shell.elf $(USRSOURCE)
	$(OBJCOPY) --set-section-flags .bss=alloc,contents -O binary shell.elf shell.bin
	$(OBJCOPY) -Ibinary -Oelf32-littleriscv shell.bin shell.bin.o

kernel: shell.bin.o
	$(CC) $(CFLAGS) -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o kernel.elf $(KERSOURCE) shell.bin.o

clean:
	rm *.map *.bin *.bin.o *.elf

run:
	$(QEMU) -machine virt -bios default -nographic -serial mon:stdio -no-reboot \
		-d unimp,guest_errors,int,cpu_reset,strace -D qemu.log \
		-drive id=drive0,file=lorem.txt,format=raw,if=none \
    -device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.0 \
		-trace "file=qemu-trace.log,enable=virtio_*" \
	  -kernel kernel.elf

disas:
	$(LLVM_PREFIX)/llvm-objdump -d $(bin) | less

addr2line: kernel.elf
	$(LLVM_PREFIX)/llvm-addr2line -e kernel.elf $(addr)

lsnames:
	$(LLVM_PREFIX)/llvm-nm $(bin)
