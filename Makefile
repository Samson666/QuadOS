CC = i386-elf-gcc
LD = i386-elf-ld
INCLUDES = -Isgfx -Ikernel -Ikernel/include -Ikernel/res
CFLAGS = -g -nostdlib -ffreestanding -m32 -fno-builtin $(INCLUDES) -no-pie -Wimplicit-function-declaration
LDFLAGS = -melf_i386 -Tkernel.ld -Lsgfx -lsgfx
ASFLAGS = -felf32
LIBGCC = /usr/local/i386elfgcc/lib/gcc/i386-elf/12.2.0/libgcc.a

SOURCES_C = $(patsubst %.c, %.o, $(wildcard kernel/*.c) $(wildcard kernel/**/*.c))
SOURCES_ASM = $(patsubst %.asm, %.o, $(wildcard kernel/*.asm))

OBJ = $(SOURCES_ASM) $(SOURCES_C)

KERNEL = kernel.bin
IMAGE = os.iso
RAMDISK = ramdisk.fat
RAMDISK2 = ramdisk2.fat

all: $(IMAGE)

$(IMAGE): $(KERNEL) $(RAMDISK)
	cp $(KERNEL) image/boot
	cp $(RAMDISK) image/boot
	cp $(RAMDISK2) image/boot
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
	dd if=/dev/zero of=$(RAMDISK2) bs=8M count=1
	mformat -i $(RAMDISK2) ::



user: libsgfx
	make -C userspace

libsgfx:
	make -C sgfx

run: $(IMAGE)
	qemu-system-i386 -boot order=d -machine accel=kvm -m 4G -vga vmware -cpu Haswell-v4 -serial stdio -rtc base=localtime \
	-fda /home/samson/Dokumente/Programmierung/QuadOS/floppy1.img -cdrom $(IMAGE)
#IMPORTANT: place the cdrom image at the end of the commandline, else qemu wants to start from the floppy!

drun: $(IMAGE)
	qemu-system-i386 -s -S -cdrom $(IMAGE) -machine accel=kvm  -serial stdio

debug:
	gdb --symbols=$(KERNEL) -ex 'target remote localhost:1234'

clean:
	make -C userspace clean
	make -C sgfx clean
	rm -f kernel/*.o kernel/**/*.o $(KERNEL) $(IMAGE) $(RAMDISK)
