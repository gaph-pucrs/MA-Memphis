RED  =\033[0;31m
NC   =\033[0m # No Color

TARGET = kernel

ifeq ($(ARCH),)
	ARCH = mipsi
endif

CC = mips-elf-gcc
AS = mips-elf-as
LD = mips-elf-ld
OBJDUMP = mips-elf-objdump
OBJCOPY = mips-elf-objcopy

SRCDIR = src
INCDIR = $(SRCDIR)/include
DEFDIR = ../include
HALDIR = hal/$(ARCH)

LIBDIR = ../lib/src
INCLIB = $(LIBDIR)/include

LIBSRC = $(wildcard $(LIBDIR)/*.c)
LIBOBJ = $(patsubst %.c, %.o, $(LIBSRC))

CFLAGS = -Os -Wall -fno-builtin -ffreestanding -G 0 -std=c99 -I$(INCDIR) -I$(DEFDIR) -I$(HALDIR) -I$(INCLIB) --include kernel_definitions.h

CCSRC = $(wildcard $(SRCDIR)/*.c)
CCOBJ = $(patsubst %.c, %.o, $(CCSRC))

ASSRC = $(wildcard $(HALDIR)/*.s)
ASOBJ = $(patsubst %.s,%.o, $(ASSRC))

all: $(TARGET).txt $(TARGET).lst

$(TARGET).txt: $(TARGET).bin
	@printf "${RED}Generating %s...${NC}\n" "$@"
	@hexdump -v -e '1/1 "%02x" 1/1 "%02x" 1/1 "%02x" 1/1 "%02x" "\n"' $< > $@

$(TARGET).bin: $(TARGET).elf
	@printf "${RED}Generating %s...${NC}\n" "$@"
	@$(OBJCOPY) -R .MIPS.abiflags -R .reginfo $< $@ -I elf32-bigmips -O binary

$(TARGET).elf: $(CCOBJ) $(ASOBJ) $(LIBOBJ)
	@printf "${RED}Linking %s...${NC}\n" "$@"
	@$(LD) $^ --section-start=".init"=0 -Map $(TARGET).map -s -N -o $@

$(TARGET).lst: $(TARGET)_debug.elf
	@printf "${RED}Generating %s...${NC}\n" "$@"
	@$(OBJDUMP) -S $< > $@

$(TARGET)_debug.elf: $(CCOBJ) $(ASOBJ) $(LIBOBJ)
	@printf "${RED}Linking %s...${NC}\n" "$@"
	@$(LD) $^ --section-start=".init"=0 -Map $(TARGET)_debug.map -s -N -o $@

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	@printf "${RED}Compiling %s...${NC}\n" "$<"
	@$(CC) -c $< -o $@ $(CFLAGS)

$(HALDIR)/%.o: $(HALDIR)/%.s
	@printf "${RED}Assemblying %s...${NC}\n" "$<"
	@$(AS) $< -o $@ --defsym sp_addr=$(PAGE_SP_INIT)

$(LIBDIR)/%.o: $(LIBDIR)/%.c
	@make -C ../lib

clean:
	@printf "Cleaning up\n"
	@rm -rf *.o
	@rm -rf *.bin
	@rm -rf *.map
	@rm -rf *.lst
	@rm -rf *.txt
	@rm -rf *.elf

.PHONY: clean
