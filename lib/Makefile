YELLOW	= \033[0;33m
NC		= \033[0m # No Color

SRCDIR = src
INCDIR = src/include

CC = riscv64-elf-gcc
OBJDUMP = riscv64-elf-objdump
OBJCOPY = riscv64-elf-objcopy

CFLAGS	= -march=rv32im -mabi=ilp32 -Os -fdata-sections -ffunction-sections -Wall -std=c99  -I$(INCDIR) --include kernel_definitions.h
# LDFLAGS = -march=rv32im -mabi=ilp32

SRCC = $(wildcard $(SRCDIR)/*.c)
SRCS = $(wildcard $(SRCDIR)/*.S)
OBJ = $(patsubst %.c, %.o, $(SRCC)) $(patsubst %.S, %.o, $(SRCS))

all: $(OBJ)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	@printf "${YELLOW}Compiling lib: %s ...${NC}\n" "$<"
	@$(CC) -c $< -o $@ $(CFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.S
	@printf "${YELLOW}Compiling lib: %s ...${NC}\n" "$<"
	@$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@rm $(SRCDIR)/*.o

.PHONY: clean
