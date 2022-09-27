KPREFIX =
KERNEL_OBJECTS = \
	start.o \
	process.o \
	page.o \
	libc.o \
	syscall.o \
	calls.o \
	buddy.o \
	pipe.o \
	filedesc.o \
	main.o \
	segmentation.o \
	interrupts.o
USER_OBJECTS = crtso.o libc.o calls.o buddy.o

default: \
	testbuddy \
	kernel.bin

boot: default
	./mkbootimage.sh

%.o: %.c *.h
	$(KPREFIX)gcc -m32 -fno-stack-protector -Wall -nostdinc -fno-builtin -c $<

%.o: %.s
	$(KPREFIX)cpp $< | $(KPREFIX)as -o $*.o

kernel.bin: $(KERNEL_OBJECTS)
	$(KPREFIX)ld -T link.ld -o kernel.bin $(KERNEL_OBJECTS)

testbuddy: testbuddy.c buddy.c buddy.h
	gcc -m32 -DUSERLAND -Wall -o testbuddy testbuddy.c buddy.c

clean:
	rm -f *.o kernel.bin
	rm -f testbuddy
	rm -rf *.dSYM
