CFLAGS?=-O2 -g
CFLAGS := $(CFLAGS) -Wall -Wextra -ffreestanding -fno-builtin -fno-stack-protector -mno-red-zone

ARCH := x86_64

LIMINE_DIR := limine

INCLUDE_DIR := include
KLIBC_DIR := klibc
KERNEL_DIR := kernel

CC := ~/x86_64-elf-7.5.0-Linux-x86_64/bin/x86_64-elf-gcc
CXX := ~/x86_64-elf-7.5.0-Linux-x86_64/bin/x86_64-elf-g++
AS := ~/x86_64-elf-7.5.0-Linux-x86_64/bin/x86_64-elf-as
LD := ~/x86_64-elf-7.5.0-Linux-x86_64/bin/x86_64-elf-ld

CFLAGS += -I$(INCLUDE_DIR) -I$(KLIBC_DIR)/include -I$(LIMINE_DIR) -mcmodel=kernel
CXXFLAGS := $(CFLAGS) -fno-exceptions -fno-rtti -std=c++11
CFLAGS += -std=c99

KLIBC_C_SOURCES := $(wildcard $(KLIBC_DIR)/*.c)
KLIBC_OBJS := $(KLIBC_C_SOURCES:.c=.o)

KERNEL_CPP_SOURCES := $(wildcard $(KERNEL_DIR)/*.cpp)
KERNEL_OBJS := $(KERNEL_CPP_SOURCES:.cpp=.o)

KERNEL_ASM_SOURCES := $(wildcard $(KERNEL_DIR)/*.S)
KERNEL_ASM_OBJS := $(KERNEL_ASM_SOURCES:.S=.o)

KERNEL := kernel.bin

LIMINE_CONF := limine.conf

.PHONY: all build iso clean 

all: build iso 

build: $(KERNEL)

$(KERNEL): $(KLIBC_OBJS) $(KERNEL_OBJS) $(KERNEL_ASM_OBJS)
	$(LD) -T $(KERNEL_DIR)/linker.ld -o $@ $^

$(KLIBC_DIR)/%.o: $(KLIBC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_DIR)/%.o: $(KERNEL_DIR)/%.S
	$(AS) $< -o $@

$(KERNEL_DIR)/%.o: $(KERNEL_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

iso: $(KERNEL)
	mkdir -p iso_root/boot
	cp $(KERNEL) iso_root/boot/
	cp $(LIMINE_DIR)/limine-bios.sys iso_root/
	cp $(LIMINE_DIR)/limine-bios-cd.bin iso_root/
	cp $(LIMINE_DIR)/limine-uefi-cd.bin iso_root/
	cp $(LIMINE_CONF) iso_root/
	
	xorriso -as mkisofs -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image \
		--protective-msdos-label \
		iso_root -o WhiteOS.iso
	
	./$(LIMINE_DIR)/limine bios-install WhiteOS.iso

clean:
	rm -f $(KERNEL) $(KLIBC_OBJS) $(KERNEL_OBJS) $(KERNEL_ASM_OBJS)
	rm -rf iso_root/
	rm -f WhiteOS.iso