BLUE  =\033[0;34m
NC   =\033[0m # No Color

TARGET = migration_task

SRCDIR = src
INCDIR = src/include
INCLIB = ../../lib/src/include
INCMAN = ../

CC = riscv64-elf-gcc
OBJDUMP = riscv64-elf-objdump
OBJCOPY = riscv64-elf-objcopy

CFLAGS	= -march=rv32im -mabi=ilp32 -Os -fdata-sections -ffunction-sections -Wall -std=c99  -I$(INCDIR) -I$(INCLIB) -I$(INCMAN) --include id_tasks.h --include kernel_definitions.h
LDFLAGS = --specs=nano.specs -march=rv32im -mabi=ilp32 -Wl,-Ttext=0,--gc-sections

LIBDIR = ../../lib/src
INCLIB = $(LIBDIR)/include

LIBSRCC = $(wildcard $(LIBDIR)/*.c)
LIBSRCS = $(wildcard $(LIBDIR)/*.S)
LIBOBJ = $(patsubst %.c, %.o, $(LIBSRCC)) $(patsubst %.S, %.o, $(LIBSRCS))

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

all: $(TARGET).txt $(TARGET).lst

$(TARGET).txt: $(TARGET).bin
	@printf "${BLUE}Dumping task: %s ...${NC}\n" "$(TARGET)"
	@hexdump -v -e '1/4 "%08x" "\n"' $< > $@

$(TARGET).bin: $(TARGET).elf
	@printf "${BLUE}Generating binary for task: %s ...${NC}\n" "$(TARGET)"
	@$(OBJCOPY) -O binary $< $@

$(TARGET).elf: $(OBJ) $(LIBOBJ)
	@printf "${BLUE}Linking task: %s ...${NC}\n" "$(TARGET)"
	@$(CC) $(LDFLAGS) -Wl,-Map=$(TARGET).map -o $@ $^

$(TARGET).lst: $(TARGET).elf
	@printf "${BLUE}Listing task: %s ...${NC}\n" "$(TARGET)"
	@$(OBJDUMP) -S $< > $@

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	@printf "${BLUE}Compiling task: %s ...${NC}\n" "$*.c"
	@$(CC) -c $< -o $@ $(CFLAGS)

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
