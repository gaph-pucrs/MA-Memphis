YELLOW  = \033[0;33m
NC   	= \033[0m # No Color

SRCDIR = .

AS	= riscv64-elf-as -march=rv32im -mabi=ilp32

SRC = $(wildcard $(SRCDIR)/*.s)
OBJ = $(patsubst %.s, %.o, $(SRC))

all: $(OBJ)

$(SRCDIR)/%.o: $(SRCDIR)/%.s
	@printf "${YELLOW}Assemblying %s...${NC}\n" "$<"
	@$(AS) --defsym sp_addr=$(PAGE_SP_INIT) -o $@ $^

clean:
	@rm $(SRCDIR)/*.o

.PHONY: clean
