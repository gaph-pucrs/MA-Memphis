RED  =\033[0;31m
NC   =\033[0m # No Color

TARGET = kernel

CC = riscv64-elf-gcc -march=rv32im -mabi=ilp32
AS = riscv64-elf-as -march=rv32im -mabi=ilp32
LD = riscv64-elf-ld -melf32lriscv
OBJDUMP = riscv64-elf-objdump
OBJCOPY = riscv64-elf-objcopy

SRCDIR = src
INCDIR = $(SRCDIR)/include
HALDIR = hal

LIBDIR = ../lib/src
INCLIB = $(LIBDIR)/include

LIBSRC = $(wildcard $(LIBDIR)/*.c)
LIBOBJ = $(patsubst %.c, %.o, $(LIBSRC))

CFLAGS = -O2 -Wall -fno-builtin -ffreestanding -std=c99 -I$(INCDIR) -I$(HALDIR) -I$(INCLIB) --include kernel_definitions.h

CCSRC = $(wildcard $(SRCDIR)/*.c)
CCOBJ = $(patsubst %.c, %.o, $(CCSRC))

ASSRC = $(wildcard $(HALDIR)/*.s)
ASOBJ = $(patsubst %.s,%.o, $(ASSRC))

all: $(TARGET).txt $(TARGET).lst

$(TARGET).txt: $(TARGET).bin
	@printf "${RED}Generating %s...${NC}\n" "$@"
	@hexdump -v -e '1/4 "%08x" "\n"' $< > $@

$(TARGET).bin: $(TARGET).elf
	@printf "${RED}Generating %s...${NC}\n" "$@"
	@$(OBJCOPY) $< $@ -O binary

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
