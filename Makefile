# File names
ISO = os.iso
KERNEL = kernel

# Object files
START = start.o
KERNEL_OBJ = kernel.o idt.o idt_load.o isr_stubs.o userprog.o syscall_entry.o isr_common_stub.o isr.o basics.o

# Compiler and tools
CC = gcc
CFLAGS = -I./helpers -I./structs -I./filesystem -I./posix -I. -m32 -ffreestanding -fno-stack-protector
NASM = nasm
NASMFLAGS = -f elf32

# Default target
all: $(ISO)

# Assembly compilation
start.o: start.asm
	$(NASM) $(NASMFLAGS) $< -o $@

syscall_entry.o: syscall_entry.asm
	$(NASM) $(NASMFLAGS) $< -o $@

idt_load.o: helpers/idt_load.asm
	$(NASM) $(NASMFLAGS) $< -o $@

isr_common_stub.o: isr_common_stub.asm
	nasm -f elf32 isr_common_stub.asm -o isr_common_stub.o

isr_stubs.o: isr_stubs.asm
	$(NASM) $(NASMFLAGS) $< -o $@

# C compilation
basics.o: helpers/basics.c helpers/basics.h
	$(CC) $(CFLAGS) -c helpers/basics.c -o basics.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

isr.o: isr.c
	gcc -I./structs -m32 -ffreestanding -fno-stack-protector -c isr.c -o isr.o

idt.o: helpers/idt.c helpers/idt.h
	$(CC) $(CFLAGS) -c helpers/idt.c -o $@

# Linking kernel
$(KERNEL): $(START) $(KERNEL_OBJ)
	ld -m elf_i386 -T linker.ld -o $@ $(START) $(KERNEL_OBJ)

# ISO structure
iso/boot/grub:
	mkdir -p iso/boot/grub
	cp grub/grub.cfg iso/boot/grub/

iso/boot/kernel: $(KERNEL)
	mkdir -p iso/boot
	cp $< iso/boot/kernel

# ISO build
$(ISO): iso/boot/grub iso/boot/kernel
	grub-mkrescue -o $@ iso

# Run QEMU
run: $(ISO)
	qemu-system-x86_64 -d int,cpu_reset -drive file=disk.img,format=raw,if=ide -cdrom $(ISO) -serial file:output.log

# Clean build artifacts
clean:
	rm -f *.o $(KERNEL) $(ISO)
	rm -rf iso/
	rm -f  helpers/*.o

