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
	filesystem.o \
	fscalls.o \
	unixproc.o \
	main.o \
	segmentation.o \
	interrupts.o
USER_OBJECTS = crtso.o libc.o calls.o buddy.o
PROGRAMS = sh ls cat find mptest

default: \
	fstool \
	testbuddy \
	$(PROGRAMS) \
	kernel.bin

boot: default
	./mkbootimage.sh

%.o: %.c *.h
	$(KPREFIX)gcc -m32 -fno-stack-protector -Wall -nostdinc -fno-builtin -c $<

%.o: %.s
	$(KPREFIX)cpp $< | $(KPREFIX)as -o $*.o

kernel.bin: $(KERNEL_OBJECTS)
	$(KPREFIX)ld -T link.ld -o kernel.bin $(KERNEL_OBJECTS)

fstool: fstool.c filesystem.c filesystem.h
	gcc -m32 -DUSE_UNIXPROC -DUSERLAND -Wall -o fstool fstool.c filesystem.c

testbuddy: testbuddy.c buddy.c buddy.h
	gcc -m32 -DUSERLAND -Wall -o testbuddy testbuddy.c buddy.c

$(PROGRAMS): %: %.o $(USER_OBJECTS)
	$(KPREFIX)ld -T link-user.ld -o $@ $(USER_OBJECTS) $@.o

clean:
	rm -f *.o kernel.bin
	rm -f fstool
	rm -f testbuddy
	rm -f $(PROGRAMS)
	rm -rf *.dSYM
