CC = i386-elf-gcc
LD = i386-elf-ld
INCLUDES = -Isgfx -Ikernel -Ikernel/include -Ikernel/res
CFLAGS = -g -nostdlib -ffreestanding -m32 -fno-builtin $(INCLUDES) -no-pie
LDFLAGS = -melf_i386 -Tkernel.ld -Lsgfx -lsgfx
ASFLAGS = -felf32
LIBGCC = /usr/bin/../lib/gcc/i386-elf/7.5.0/libgcc.a

SOURCES_C = $(patsubst %.c, %.o, $(wildcard kernel/*.c) $(wildcard kernel/**/*.c))
SOURCES_ASM = $(patsubst %.asm, %.o, $(wildcard kernel/*.asm))

OBJ = $(SOURCES_ASM) $(SOURCES_C)

KERNEL = kernel.bin
IMAGE = os.iso
RAMDISK = ramdisk.fat

all: $(IMAGE)

$(IMAGE): $(KERNEL) $(RAMDISK)
	cp $(KERNEL) image/boot
	cp $(RAMDISK) image/boot
	grub-mkrescue -o $(IMAGE) image

$(KERNEL): $(OBJ) libsgfx
	$(LD) -o $(KERNEL) $(OBJ) $(LDFLAGS) $(LIBGCC)


%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

%.o: %.asm
	nasm $(ASFLAGS) $< -o $@

$(RAMDISK): user
	dd if=/dev/zero of=$(RAMDISK) bs=8M count=1
	mformat -i $(RAMDISK) ::
	mcopy -i $(RAMDISK) userspace/bin/* ::

user: libsgfx
	make -C userspace

libsgfx:
	make -C sgfx

run: $(IMAGE)
	qemu-system-i386 -cdrom $(IMAGE) -machine accel=kvm -m 4G -vga vmware -cpu Haswell-v4 -serial stdio

drun: $(IMAGE)
	qemu-system-i386 -s -S -cdrom $(IMAGE) -machine accel=kvm  -serial stdio

debug:
	gdb --symbols=$(KERNEL) -ex 'target remote localhost:1234'

clean:
	make -C userspace clean
	make -C sgfx clean
	rm -f kernel/*.o kernel/**/*.o $(KERNEL) $(IMAGE) $(RAMDISK)
