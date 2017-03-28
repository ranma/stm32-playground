CROSS_COMPILE	:= arm-none-eabi-
TEXT_START	:= 0x08002000

CC		:= $(CROSS_COMPILE)gcc
LD		:= $(CROSS_COMPILE)ld
OBJCOPY		:= $(CROSS_COMPILE)objcopy
OBJDUMP		:= $(CROSS_COMPILE)objdump

BIN_FLAGS	:= -O binary -R .reginfo -R .note -R .comment -R .mdebug -S

CFLAGS		= -Wall -Wstrict-prototypes -Os -g \
		  -fstrict-aliasing -fomit-frame-pointer \
		  -fno-pic -ffunction-sections -pipe \
		  -mthumb -march=armv7-m -mcpu=cortex-m3 \
		  -Wa,-mthumb,-mcpu=cortex-m3 \
		  -fno-builtin-free -fno-builtin-printf \
		  -fno-builtin-puts

ASFLAGS		:= $(CFLAGS) -D__ASSEMBLY__

LDFLAGS		= -static -g --gc-sections
LDFLAGS		+= -e startup -T loader.lds -Map loader.map -Ttext $(TEXT_START)

OBJECTS		:= board.o loader.o printf.o usb.o startup_ARMCM3.o

all: loader.bin

# Don't build dependencies, this may die if $(CC) isn't gcc
dep:

install:

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.s : %.c
	$(CC) $(CFLAGS) -S -g -fverbose-asm -o $@ $<

%.lst : %.c
	$(CC) $(CFLAGS) -c -g -Wa,-ahl=$@ $< > /dev/null

%.o : %.S
	$(CC) $(ASFLAGS) -c -o $@ $<

loader: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

loader.bin: loader
	$(OBJCOPY) $(BIN_FLAGS) $< $@

mrproper: clean

clean:
	rm -f loader *.elf *.bin *.o *.s *~
