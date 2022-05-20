GREEN	= \033[0;32m
NC		= \033[0m # No Color

SRCDIR = .

SRC = $(wildcard $(SRCDIR)/*.c)
TARGET = $(patsubst %.c, %.txt, $(SRC))
ELFS = $(patsubst %.c, %.elf, $(SRC))

INCLIB = ../../lib/src/include

CFLAGS	= -O2 -Wall -ffreestanding -fno-builtin -std=c99 -I$(INCLIB) --include id_tasks.h
CC		= riscv64-elf-gcc -march=rv32im -mabi=ilp32
AS		= riscv64-elf-as -march=rv32im -mabi=ilp32
LD		= riscv64-elf-ld -melf32lriscv
OBJCOPY	= riscv64-elf-objcopy

LIBDIR = ../../lib/src
LIBSRC = $(wildcard $(LIBDIR)/*.c)
LIBOBJ = $(patsubst %.c, %.o, $(LIBSRC))

BOOTDIR = ../../bootloader
BOOTSRC = $(wildcard $(BOOTDIR)/*.s)
BOOTOBJ = $(patsubst %.s, %.o, $(BOOTSRC))

all: $(TARGET) $(ELFS)

$(SRCDIR)/%.txt: $(SRCDIR)/%.bin
	@printf "${GREEN}Dumping task: %s ...${NC}\n" "$<"
	@hexdump -v -e '1/4 "%08x" "\n"' $< > $@

$(SRCDIR)/%.bin: $(SRCDIR)/%.elf
	@printf "${GREEN}Generating binary for task: %s ...${NC}\n" "$<"
	@$(OBJCOPY) -O binary $< $@

$(SRCDIR)/%.elf: $(SRCDIR)/%.o $(BOOTOBJ) $(LIBOBJ)
	@printf "${GREEN}Linking task: %s ...${NC}\n" "$<"
	@$(LD) $^ --section-start=".init"=0 -s -N -o $@

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	@printf "${GREEN}Compiling task: %s ...${NC}\n" "$<"
	@$(CC) -c $< -o $@ $(CFLAGS)

$(BOOTDIR)/%.o: $(BOOTDIR)/%.s
	@make -C $(BOOTDIR)

$(LIBDIR)/%.o: $(LIBDIR)/%.c
	@make -C ../../lib

clean:
	@printf "Cleaning up\n"
	@rm -rf *.o
	@rm -rf *.bin
	@rm -rf *.map
	@rm -rf *.lst
	@rm -rf *.txt
	@rm -rf *.elf
