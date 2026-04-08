# WhiteOS Makefile

KERNEL := WhiteOS
TARGET_SPEC := x86_64-WhiteOS.json
TARGET := x86_64-WhiteOS
CARGO := cargo
QEMU := qemu-system-x86_64
XORRISO := xorriso

# Build mode: debug or release
MODE ?= debug
ifeq ($(MODE),release)
	CARGO_FLAGS := --release
	OUT_DIR := target/$(TARGET)/release
else
	OUT_DIR := target/$(TARGET)/debug
endif

ROOT_DIR := .

KERNEL_ELF := $(OUT_DIR)/$(KERNEL)
ISO := $(OUT_DIR)/$(KERNEL).iso

LIMINE_DIR := limine
LIMINE_REPO := https://codeberg.org/Limine/Limine.git
LIMINE_BRANCH := v11.x-binary

ISO_ROOT := $(OUT_DIR)/iso_root
LIMINE_CONF := $(ISO_ROOT)/limine.conf

QEMU_MEM := 256M
QEMU_FLAGS := -cdrom $(ISO) -m $(QEMU_MEM) -serial stdio

.PHONY: all build clean run iso limine qemu qemu-debug

all: iso

build: $(KERNEL_ELF)

$(KERNEL_ELF): src/* build.rs linker-x86_64.ld $(TARGET_SPEC) Cargo.toml
	$(CARGO) build $(CARGO_FLAGS)

limine: $(LIMINE_DIR)/limine-bios.sys

$(LIMINE_DIR)/limine-bios.sys:
	@echo "Cloning Limine binary release..."
	git clone --depth=1 --branch=$(LIMINE_BRANCH) $(LIMINE_REPO) $(LIMINE_DIR)

iso: $(ISO)

$(ISO): $(KERNEL_ELF) $(LIMINE_DIR)/limine-bios.sys
	@echo "Creating bootable ISO..."
	@mkdir -p $(ISO_ROOT)/boot
	cp $(ROOT_DIR)/limine.conf $(ISO_ROOT)/
	cp $(KERNEL_ELF) $(ISO_ROOT)/
	cp $(LIMINE_DIR)/limine-bios.sys $(ISO_ROOT)/boot/
	cp $(LIMINE_DIR)/limine-bios-cd.bin $(ISO_ROOT)/boot/
	@mkdir -p $(ISO_ROOT)/EFI/BOOT
	cp $(LIMINE_DIR)/BOOTX64.EFI $(ISO_ROOT)/EFI/BOOT/
	cp $(LIMINE_DIR)/limine-uefi-cd.bin $(ISO_ROOT)/boot/
	$(XORRISO) -as mkisofs \
		-b boot/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image \
		--protective-msdos-label \
		$(ISO_ROOT) -o $(ISO)

run: $(ISO)
	$(QEMU) $(QEMU_FLAGS)

qemu-debug: $(ISO)
	$(QEMU) $(QEMU_FLAGS) -s -S

qemu: run

clean:
	$(CARGO) clean
	rm -rf $(OUT_DIR)/iso_root $(OUT_DIR)/$(KERNEL).iso
	rm -rf $(LIMINE_DIR)
