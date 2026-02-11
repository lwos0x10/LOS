# ===== Config =====
TARGET      = kernel.elf
BUILD_DIR   = build
ISO_DIR     = $(BUILD_DIR)/iso
GRUB_DIR    = $(ISO_DIR)/boot/grub

CC          = i686-elf-gcc
LD          = i686-elf-ld
AS          = nasm

CFLAGS      = -ffreestanding -m32 -O2 -Wall -Wextra
LDFLAGS     = -m elf_i386 -T linker.ld

SRC_C       := $(shell find . -name "*.c")
SRC_ASM     := $(shell find . -name "*.asm")

OBJ_C       := $(patsubst ./%.c,$(BUILD_DIR)/%.o,$(SRC_C))
OBJ_ASM     := $(patsubst ./%.asm,$(BUILD_DIR)/%.o,$(SRC_ASM))

OBJS        := $(OBJ_C) $(OBJ_ASM)

# ===== Default target =====
all: $(BUILD_DIR)/$(TARGET)

# ===== Compile C =====
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ===== Compile ASM =====
$(BUILD_DIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) -f elf32 $< -o $@

# ===== Link =====
$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# ===== Run (build ISO + QEMU) =====
run: all
	@mkdir -p $(GRUB_DIR)
	cp $(BUILD_DIR)/$(TARGET) $(ISO_DIR)/boot/
	echo 'set timeout=0' >  $(GRUB_DIR)/grub.cfg
	echo 'set default=0' >> $(GRUB_DIR)/grub.cfg
	echo 'menuentry "My OS" {' >> $(GRUB_DIR)/grub.cfg
	echo '  multiboot2 /boot/$(TARGET)' >> $(GRUB_DIR)/grub.cfg
	echo '  boot' >> $(GRUB_DIR)/grub.cfg
	echo '}' >> $(GRUB_DIR)/grub.cfg
	grub-mkrescue -o $(BUILD_DIR)/myos.iso $(ISO_DIR)
	qemu-system-i386 -cdrom $(BUILD_DIR)/myos.iso

# ===== Clean =====
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean
