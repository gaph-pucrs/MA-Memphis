COR  =\033[0;32m
NC   =\033[0m # No Color

LIB = work
TARGET = memphis

SRCDIR = src
PERDIR = peripherals
INCDIR = $(SRCDIR)/include
PERINC = $(PERDIR)/include
INCLUDE = -I$(INCDIR) -I$(PERINC) -I.

VHDC = vcom
VHDFLAGS = -2008

CXX = sccom
CXXFLAGS = -nologo -Wall -O2 -std=c++11 -g $(INCLUDE)

SRCSC = $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(PERDIR)/*.cpp)
SRCVHD = $(wildcard $(SRCDIR)/*.vhd)
OBJSC = $(patsubst %.cpp, %.o, $(SRCSC))
OBJVHD = 

all: $(SC)

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp vmap
	@printf "${COR}Compiling %s...${NC}\n" "$(dir $<)$*.cpp"
	@$(CXX) $< $(CXXFLAGS) -work $(LIB)
	
$(PERDIR)/%.o: $(PERDIR)/%.cpp vmap
	@printf "${COR}Compiling %s...${NC}\n" "$(dir $<)$*.cpp"
	$(CXX) $< $(CXXFLAGS) -work $(LIB)

vmap: vlib
	vmap $(LIB) $(LIB)

vlib:
	vlib $(LIB)

clean:
	@printf "Cleaning up\n"
	@rm -f *.o
	@rm $(TARGET)
