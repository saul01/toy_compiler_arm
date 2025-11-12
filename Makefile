CC=arm-none-eabi-as
LD=arm-none-eabi-ld
QEMU=qemu-system-arm
OBJDUMP=arm-none-eabi-objdump
OBJCOPY=arm-none-eabi-objcopy

all: test.elf

test.o: test.s
	$(CC) -mcpu=cortex-m3 -mthumb -o test.o test.s

test.elf: test.o linker.ld
	$(LD) -T linker.ld -o test.elf test.o

dump: test.elf
	$(OBJDUMP) -D test.elf > test.dump

run: test.elf
	$(QEMU) -M lm3s6965evb -kernel test.elf -nographic

clean:
	rm -f *.o *.elf *.dump

