# Required packages for building and testing
# sudo apt-get install g++ binutils libc6-dev-i386
# sudo apt-get install VirtualBox grub-legacy xorriso

# Compiler and linker parameters
GCCPARAMS = -m32 -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
GCCPARAMS_PART_A = $(GCCPARAMS) -D BUILD_PART_A
GCCPARAMS_PART_B_FIRST_STRATEGY = $(GCCPARAMS) -D BUILD_PART_B_FIRST_STRATEGY
GCCPARAMS_PART_B_SECOND_STRATEGY = $(GCCPARAMS) -D BUILD_PART_B_SECOND_STRATEGY
GCCPARAMS_PART_B_THIRD_STRATEGY = $(GCCPARAMS) -D BUILD_PART_B_THIRD_STRATEGY
GCCPARAMS_PART_B_DYNAMIC_PRIORITY = $(GCCPARAMS) -D BUILD_PART_B_DYNAMIC_PRIORITY
GCCPARAMS_PART_C_RANDOM_PROCESS_SPAWN = $(GCCPARAMS) -D BUILD_PART_C_RANDOM_PROCESS_SPAWN
GCCPARAMS_PART_C_BUILD_PART_C_INPUT_PRIORITY = $(GCCPARAMS) -D BUILD_PART_C_INPUT_PRIORITY
ASPARAMS = --32
LDPARAMS = -melf_i386


# All objects including assembly
objects = obj/loader.o \
          obj/gdt.o \
          obj/memorymanagement.o \
          obj/drivers/driver.o \
          obj/hardwarecommunication/port.o \
          obj/hardwarecommunication/interruptstubs.o \
          obj/hardwarecommunication/interrupts.o \
          obj/syscalls.o \
          obj/multitasking.o \
          obj/drivers/amd_am79c973.o \
          obj/hardwarecommunication/pci.o \
          obj/drivers/keyboard.o \
          obj/drivers/mouse.o \
          obj/drivers/vga.o \
          obj/drivers/ata.o \
          obj/gui/widget.o \
          obj/gui/window.o \
          obj/gui/desktop.o \
		  obj/program.o \
		  obj/microkernel.o \
          obj/kernel.o \


# Main target
run: kernel.iso
	(killall VirtualBox && sleep 1) || true
	VirtualBox --startvm 'My Operating System' &

# Compile C++ source files to object files
obj/gdt.o: src/gdt.cpp
	mkdir -p $(@D)
	gcc $(GCCPARAMS) -c -o $@ $<
	
obj/%.o: src/%.cpp
	mkdir -p $(@D)
	gcc $(GCCPARAMS) -c -o $@ $<

# Assemble assembly source files to object files
obj/%.o: src/%.s
	mkdir -p $(@D)
	as $(ASPARAMS) -o $@ $<

# Link object files into a binary
kernel.bin: linker.ld $(objects)
	ld $(LDPARAMS) -T $< -o $@ $(objects)

# Build specific parts
build_part_a:
	$(MAKE) clean
	$(MAKE) GCCPARAMS="$(GCCPARAMS_PART_A)" kernel.iso

build_part_b_first_strategy:
	$(MAKE) clean
	$(MAKE) GCCPARAMS="$(GCCPARAMS_PART_B_FIRST_STRATEGY)" kernel.iso

build_part_b_second_strategy:
	$(MAKE) clean
	$(MAKE) GCCPARAMS="$(GCCPARAMS_PART_B_SECOND_STRATEGY)" kernel.iso

build_part_b_third_strategy:
	$(MAKE) clean
	$(MAKE) GCCPARAMS="$(GCCPARAMS_PART_B_THIRD_STRATEGY)" kernel.iso

build_part_b_dynamic_priority:
	$(MAKE) clean
	$(MAKE) GCCPARAMS="$(GCCPARAMS_PART_B_DYNAMIC_PRIORITY)" kernel.iso

build_part_c_random_process_spawn:
	$(MAKE) clean
	$(MAKE) GCCPARAMS="$(GCCPARAMS_PART_C_RANDOM_PROCESS_SPAWN)" kernel.iso

build_part_c_input_priority:
	$(MAKE) clean
	$(MAKE) GCCPARAMS="$(GCCPARAMS_PART_C_BUILD_PART_C_INPUT_PRIORITY)" kernel.iso

# Create ISO image
kernel.iso: kernel.bin
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/kernel.bin
	echo 'set timeout=0'                      > iso/boot/grub/grub.cfg
	echo 'set default=0'                     >> iso/boot/grub/grub.cfg
	echo ''                                  >> iso/boot/grub/grub.cfg
	echo 'menuentry "My Operating System" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin'    >> iso/boot/grub/grub.cfg
	echo '  boot'                            >> iso/boot/grub/grub.cfg
	echo '}'                                 >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=kernel.iso iso
	rm -rf iso

# Install the kernel binary to the boot directory
install: kernel.bin
	sudo cp $< /boot/kernel.bin

# Clean the build directory
.PHONY: clean
clean:
	rm -rf obj kernel.bin kernel.iso