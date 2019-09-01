KPREFIX =
KERNEL_OBJECTS = \
	start.o \
#ifdef USE_PROCESSES
	process.o \
#endif
#ifdef USE_PAGING
	page.o \
#endif
#ifdef USE_LIBC
	libc.o \
#endif
#ifdef USE_SYSCALLS
	syscall.o \
	calls.o \
#endif
#ifdef USE_MALLOC
	buddy.o \
#endif
#ifdef USE_FILEDESC
	pipe.o \
	filedesc.o \
#endif
#ifdef USE_UNIXPROC
	filesystem.o \
	fscalls.o \
#endif
#ifdef USE_UNIXPROC
	unixproc.o \
#endif
	main.o \
	segmentation.o \
	interrupts.o
USER_OBJECTS = crtso.o libc.o calls.o buddy.o
#ifdef USE_UNIXPROC
PROGRAMS = sh ls cat find mptest
#endif

default: \
#ifdef USE_UNIXPROC
	fstool \
#endif
#ifdef USE_MALLOC
	testbuddy \
#endif
#ifdef USE_UNIXPROC
	$(PROGRAMS) \
#endif
	kernel.bin

boot: default
	./mkbootimage.sh

%.o: %.c *.h
	$(KPREFIX)gcc -m32 -fno-stack-protector -Wall -nostdinc -fno-builtin -c $<

%.o: %.s
	$(KPREFIX)cpp $< | $(KPREFIX)as -o $*.o

kernel.bin: $(KERNEL_OBJECTS)
	$(KPREFIX)ld -T link.ld -o kernel.bin $(KERNEL_OBJECTS)
#ifdef USE_UNIXPROC

fstool: fstool.c filesystem.c filesystem.h
	gcc -m32 -DUSE_UNIXPROC -DUSERLAND -Wall -o fstool fstool.c filesystem.c
#endif
#ifdef USE_MALLOC

testbuddy: testbuddy.c buddy.c buddy.h
	gcc -m32 -DUSERLAND -Wall -o testbuddy testbuddy.c buddy.c
#endif
#ifdef USE_UNIXPROC

$(PROGRAMS): %: %.o $(USER_OBJECTS)
	$(KPREFIX)ld -T link-user.ld -o $@ $(USER_OBJECTS) $@.o
#endif

clean:
	rm -f *.o kernel.bin
#ifdef USE_UNIXPROC
	rm -f fstool
#endif
#ifdef USE_MALLOC
	rm -f testbuddy
#endif
#ifdef USE_UNIXPROC
	rm -f $(PROGRAMS)
#endif
	rm -rf *.dSYM
