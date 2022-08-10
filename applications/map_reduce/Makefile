GREEN	= \033[0;32m
NC		= \033[0m # No Color

SRCDIR = .

SRC = $(wildcard $(SRCDIR)/*.c)
TARGET = $(patsubst %.c, %.txt, $(SRC))
ELFS = $(patsubst %.c, %.elf, $(SRC))

INCLIB = ../../lib/src/include

CC = riscv64-elf-gcc
OBJDUMP = riscv64-elf-objdump
OBJCOPY = riscv64-elf-objcopy

CFLAGS	= -march=rv32im -mabi=ilp32 -Os -fdata-sections -ffunction-sections -Wall -std=c99 -I$(INCLIB) --include id_tasks.h
LDFLAGS = --specs=nano.specs -march=rv32im -mabi=ilp32 -Wl,-Ttext=0,--gc-sections

LIBDIR = ../../lib/src
LIBSRCC = $(wildcard $(LIBDIR)/*.c)
LIBSRCS = $(wildcard $(LIBDIR)/*.S)
LIBOBJ = $(patsubst %.c, %.o, $(LIBSRCC)) $(patsubst %.S, %.o, $(LIBSRCS))

all: $(TARGET) $(ELFS)

$(SRCDIR)/%.txt: $(SRCDIR)/%.bin
	@printf "${GREEN}Dumping task: %s ...${NC}\n" "$<"
	@hexdump -v -e '1/4 "%08x" "\n"' $< > $@

$(SRCDIR)/%.bin: $(SRCDIR)/%.elf
	@printf "${GREEN}Generating binary for task: %s ...${NC}\n" "$<"
	@$(OBJCOPY) -O binary $< $@

$(SRCDIR)/%.elf: $(SRCDIR)/%.o $(LIBOBJ)
	@printf "${GREEN}Linking task: %s ...${NC}\n" "$<"
	@$(CC) $(LDFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	@printf "${GREEN}Compiling task: %s ...${NC}\n" "$<"
	@$(CC) -c $< -o $@ $(CFLAGS)


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

.PHONY: clean