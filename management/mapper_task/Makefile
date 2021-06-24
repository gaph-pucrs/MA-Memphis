BLUE  =\033[0;34m
NC   =\033[0m # No Color

TARGET = mapper_task

SRCDIR = src
INCDIR = src/include
INCLIB = ../../lib/src/include
INCDEF = ../../include
INCMAN = ../

CFLAGS	= -Os -Wall -ffreestanding -fno-builtin -G 0 -std=c99  -I$(INCDIR) -I$(INCLIB) -I$(INCDEF) -I$(INCMAN) --include id_tasks.h --include kernel_definitions.h
CC		= mips-elf-gcc
AS		= mips-elf-as
LD		= mips-elf-ld
OBJDUMP	= mips-elf-objdump
OBJCOPY	= mips-elf-objcopy

LIBDIR = ../../lib/src
INCLIB = $(LIBDIR)/include

LIBSRC = $(wildcard $(LIBDIR)/*.c)
LIBOBJ = $(patsubst %.c, %.o, $(LIBSRC))

BOOTDIR = ../../bootloader/mipsi
BOOTSRC = $(wildcard $(BOOTDIR)/*.s)
BOOTOBJ = $(patsubst %.s, %.o, $(BOOTSRC))

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

all: $(TARGET).txt $(TARGET).lst

$(TARGET).txt: $(TARGET).bin
	@printf "${BLUE}Dumping task: %s ...${NC}\n" "$(TARGET)"
	@hexdump -v -e '1/1 "%02x" 1/1 "%02x" 1/1 "%02x" 1/1 "%02x" "\n"' $< > $@

$(TARGET).bin: $(TARGET).elf
	@printf "${BLUE}Generating binary for task: %s ...${NC}\n" "$(TARGET)"
	@$(OBJCOPY) -R .MIPS.abiflags -R .reginfo -O binary $< $@

$(TARGET).elf: $(OBJ) $(BOOTOBJ) $(LIBOBJ)
	@printf "${BLUE}Linking task: %s ...${NC}\n" "$(TARGET)"
	@$(LD) --section-start=".init"=0 -Map $(TARGET).map -s -N -o $@ $^

$(TARGET).lst: $(TARGET)_debug.elf
	@printf "${BLUE}Listing task: %s ...${NC}\n" "$(TARGET)"
	@$(OBJDUMP) -S $< > $@

$(TARGET)_debug.elf: $(OBJ) $(BOOTOBJ) $(LIBOBJ)
	@printf "${BLUE}Linking debug task: %s ...${NC}\n" "$(TARGET)"
	@$(LD) --section-start=".init"=0 -Map $(TARGET)_debug.map -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	@printf "${BLUE}Compiling task: %s ...${NC}\n" "$*.c"
	@$(CC) -c $< -o $@ $(CFLAGS)

$(BOOTDIR)/%.o: $(BOOTDIR)/%.s
	@make -C $(BOOTDIR)

$(LIBDIR)/%.o: $(LIBDIR)/%.c
	@make -C ../../lib

clean:
	@rm $(TARGET).elf
	@rm $(TARGET).txt
	@rm $(TARGET).map
	@rm $(TARGET).lst
	@rm $(TARGET).bin
	@rm $(TARGET)_debug.map
	@rm $(TARGET)_debug.elf
	@rm $(SRCDIR)/*.o

.PHONY: clean
