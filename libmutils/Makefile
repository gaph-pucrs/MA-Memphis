YELLOW	= \033[0;33m
NC		= \033[0m # No Color

SRCDIR = src
INCDIR = src/include

CC = riscv64-elf-gcc
OBJDUMP = riscv64-elf-objdump
OBJCOPY = riscv64-elf-objcopy

CFLAGS	= -march=rv32im -mabi=ilp32 -Os -std=c11 -fdata-sections -ffunction-sections -Wall -I$(INCDIR)

SRCC = $(wildcard $(SRCDIR)/*.c)
SRCS = $(wildcard $(SRCDIR)/*.S)
OBJ = $(patsubst %.c, %.o, $(SRCC)) $(patsubst %.S, %.o, $(SRCS))

all: libmutils.a

libmutils.a: $(OBJ)
	@riscv64-elf-ar rcs $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	@printf "${YELLOW}Compiling lib: %s ...${NC}\n" "$<"
	@$(CC) -c $< -o $@ $(CFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.S
	@printf "${YELLOW}Compiling lib: %s ...${NC}\n" "$<"
	@$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@rm -f $(SRCDIR)/*.o
	@rm -f *.a

install: libmutils.a
	@mkdir -p ../include
	@mkdir -p ../lib
	@cp -r $(INCDIR)/* ../include
	@cp libmutils.a ../lib/libmutils.a

.PHONY: clean
