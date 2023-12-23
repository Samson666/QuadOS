CC = clang
LD = clang
CFLAGS = -g -m32 -std=gnu11 -nostdlib -ffreestanding  -I ../api/include -I ../libc/include -I../kernel/include
LDFLAGS = -g -std=gnu11 -nostdlib -ffreestanding -m32 -no-pie -L../api -l:api.so -L../libc -lc ../api/crt0.o
ASFLAGS = -felf32

export

all:
	mkdir -p bin
	$(MAKE) -C api
	cp api/*.so bin/
	$(MAKE) -C libc
	$(MAKE) -C files
	cp files/*.exe bin/
	$(MAKE) -C terminal
	cp terminal/*.exe bin/
	$(MAKE) -C draw
	cp draw/*.exe bin/
	$(MAKE) -C timer
	cp timer/*.exe bin/
	$(MAKE) -C breakout
	cp breakout/*.exe bin/
	$(MAKE) -C tasks
	cp tasks/*.exe bin/
	$(MAKE) -C testbed
	cp testbed/*.exe bin/

clean:
	$(MAKE) -C api clean
	$(MAKE) -C libc clean
	$(MAKE) -C draw clean
	$(MAKE) -C terminal clean
	$(MAKE) -C files clean
	$(MAKE) -C breakout clean
	$(MAKE) -C timer clean
	
	rm -f bin/*.exe bin/*.so
