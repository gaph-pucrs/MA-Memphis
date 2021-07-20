COR  =\033[0;32m
NC   =\033[0m # No Color

TARGET = memphis

SRCDIR = src
PERDIR = peripherals
INCDIR = $(SRCDIR)/include
PERINC = $(PERDIR)/include
INCLUDE = -I$(INCDIR) -I$(PERINC) -I.

CXX = g++
CXXFLAGS = -Wall -O2 -fpermissive -std=c++11 $(INCLUDE) --include definitions.h
LDFLAGS = -lsystemc

SRC = $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(PERDIR)/*.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))

all: $(TARGET)

$(TARGET): $(OBJ)
	@printf "${COR}Linking %s ...${NC}\n" "$@"
	@$(CXX) $^ -o $@ $(LDFLAGS)
	
$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	@printf "${COR}Compiling %s...${NC}\n" "$(dir $<)$*.cpp"
	@$(CXX) -c $< -o $@ $(CXXFLAGS)
	
$(PERDIR)/%.o: $(PERDIR)/%.cpp
	@printf "${COR}Compiling %s...${NC}\n" "$(dir $<)$*.cpp"
	@$(CXX) -c $< -o $@ $(CXXFLAGS)

clean:
	@printf "Cleaning up\n"
	@rm -f *.o
	@rm $(TARGET)