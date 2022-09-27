KPREFIX =
KERNEL_OBJECTS = \
	start.o \
	process.o \
	libc.o \
	main.o \
	segmentation.o \
	interrupts.o
USER_OBJECTS = crtso.o libc.o calls.o buddy.o

default: \
	kernel.bin

boot: default
	./mkbootimage.sh

%.o: %.c *.h
	$(KPREFIX)gcc -m32 -fno-stack-protector -Wall -nostdinc -fno-builtin -c $<

%.o: %.s
	$(KPREFIX)cpp $< | $(KPREFIX)as -o $*.o

kernel.bin: $(KERNEL_OBJECTS)
	$(KPREFIX)ld -T link.ld -o kernel.bin $(KERNEL_OBJECTS)

clean:
	rm -f *.o kernel.bin
	rm -rf *.dSYM
