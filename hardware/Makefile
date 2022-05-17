COR  =\033[0;32m
NC   =\033[0m # No Color

TARGET = memphis

SRCDIR = src
PERDIR = peripherals
INCDIR = $(SRCDIR)/include
PERINC = $(PERDIR)/include
INCLUDE = -I$(INCDIR) -I$(PERINC) -I.

CXX = sccom
CXXFLAGS = -Wall -O2 -fpermissive $(INCLUDE) --include definitions.h

SRC = $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(PERDIR)/*.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))

all: $(TARGET)
	
$(SRCDIR)/%.o: $(SRCDIR)/%.cpp vmap
	@printf "${COR}Compiling %s...${NC}\n" "$(dir $<)$*.cpp"
	@$(CXX) $< $(CXXFLAGS) -work work
	
$(PERDIR)/%.o: $(PERDIR)/%.cpp vmap
	@printf "${COR}Compiling %s...${NC}\n" "$(dir $<)$*.cpp"
	$(CXX) $< $(CXXFLAGS) -work work

vmap: vlib
	vmap work work

vlib:
	vlib work

clean:
	@printf "Cleaning up\n"
	@rm -f *.o
	@rm $(TARGET)
