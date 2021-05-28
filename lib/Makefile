YELLOW	= \033[0;33m
NC		= \033[0m # No Color

SRCDIR = src
INCDIR = src/include
DEFDIR = ../include

CC		= mips-elf-gcc
DEFINES = -DPRINTF_DISABLE_SUPPORT_FLOAT=1 -DPRINTF_DISABLE_SUPPORT_EXPONENTIAL=1 -DPRINTF_DISABLE_SUPPORT_LONG_LONG=1 -DPRINTF_DISABLE_SUPPORT_PTRDIFF_T=1
CFLAGS	= -O2 -Wall -ffreestanding -fno-builtin -std=c99 -G 0 -I$(INCDIR) -I$(DEFDIR) --include kernel_definitions.h $(DEFINES)

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

all: $(OBJ)

$(SRCDIR)/%.o: $(SRCDIR)/%.c*
	@printf "${YELLOW}Compiling lib: %s ...${NC}\n" "$*.c"
	@$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@rm $(SRCDIR)/*.o

.PHONY: clean
