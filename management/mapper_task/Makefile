BLUE  =\033[0;34m
NC   =\033[0m # No Color

TARGET = mapper_task

SRCDIR = src
INCDIR = src/include
INCLIB = ../../lib/src/include
INCMAN = ../

CC = riscv64-elf-gcc
OBJDUMP = riscv64-elf-objdump
OBJCOPY = riscv64-elf-objcopy

CFLAGS	= -march=rv32im -mabi=ilp32 -Os -fdata-sections -ffunction-sections -Wall -std=c99  -I$(INCDIR) -I$(INCLIB) -I$(INCMAN) --include id_tasks.h -DPKG_N_PE_X=2 -DPKG_MAX_TASKS_APP=10 -DPKG_N_PE=4 -DPKG_MAX_LOCAL_TASKS=1 -DAPP_INJECTOR=0xc0000101 -DMAINJECTOR=0xe0000000
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
	@rm -f $(SRCDIR)/*.o
	@rm -f $(TARGET).bin
	@rm -f $(TARGET).elf
	@rm -f $(TARGET).lst
	@rm -f $(TARGET).map
	@rm -f $(TARGET).txt

.PHONY: clean
