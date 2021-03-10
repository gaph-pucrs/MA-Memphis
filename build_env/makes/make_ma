BLUE  =\033[0;34m
NC   =\033[0m # No Color

SRCDIR = src
INCDIR = src/include
INCLIBDIR = ../../software/lib/include
INCPKGDIR = ../

CFLAGS = -O2 -Wall -Wno-unused-function -fms-extensions -c -s -std=c99 -G 0 -I$(INCDIR) -I$(INCLIBDIR) -I$(INCPKGDIR) --include ./id_tasks.h
GCC_MIPS          = mips-elf-gcc $(CFLAGS)
AS_MIPS           = mips-elf-as
LD_MIPS           = mips-elf-ld
DUMP_MIPS         = mips-elf-objdump
COPY_MIPS         = mips-elf-objcopy -I elf32-bigmips -O binary

BOOT_TASK         = bootloader.o
BOOT_TASK_SRC     = ../../software/bootloader/mipsi/bootloader.s

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

all: $(TARGET).txt $(TARGET).lst

$(TARGET).txt: $(TARGET).bin
	@printf "${BLUE}Dumping task: %s ...${NC}\n" "$(TARGET)"
	@hexdump -v -e '1/1 "%02x" 1/1 "%02x" 1/1 "%02x" 1/1 "%02x" "\n"' $< > $@

$(TARGET).bin: $(TARGET).elf
	@printf "${BLUE}Generating binary for task: %s ...${NC}\n" "$(TARGET)"
	@$(COPY_MIPS) -R .MIPS.abiflags -R .reginfo $< $@

$(TARGET).elf: $(OBJ) $(BOOT_TASK)
	@printf "${BLUE}Linking task: %s ...${NC}\n" "$(TARGET)"
	@$(LD_MIPS) --section-start=".init"=0 -Map $(TARGET).map -s -N -o $@ $^

$(TARGET).lst: $(TARGET)_debug.elf
	@printf "${BLUE}Listing task: %s ...${NC}\n" "$(TARGET)"
	@$(DUMP_MIPS) -S $< > $@

$(TARGET)_debug.elf: $(OBJ) $(BOOT_TASK)
	@printf "${BLUE}Linking debug task: %s ...${NC}\n" "$(TARGET)"
	@$(LD_MIPS)  --section-start=".init"=0 -Map $(TARGET)_debug.map -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c*
	@printf "${BLUE}Compiling task: %s ...${NC}\n" "$*.c"
	@$(GCC_MIPS) -c $< -o $@ $(CFLAGS)

$(BOOT_TASK): $(BOOT_TASK_SRC)
	@printf "${BLUE}Assemblying %s...${NC}\n" "$<"
	@$(AS_MIPS) --defsym sp_addr=$(PAGE_SP_INIT) -o $@ $^

clean:
	@rm $(TARGET).elf
	@rm $(TARGET).txt
	@rm $(TARGET).map
	@rm $(TARGET).lst
	@rm $(TARGET).bin
	@rm $(BOOT_TASK)
	@rm $(TARGET)_debug.map
	@rm $(TARGET)_debug.elf
	@rm $(SRCDIR)/*.o

.PHONY: clean
