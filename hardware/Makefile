COR  =\033[0;32m
NC   =\033[0m # No Color

LIB = work
TARGET = memphis

SRCDIR = src
PERDIR = peripherals
INCDIR = $(SRCDIR)/include
PERINC = $(PERDIR)/include
INCLUDE = -I$(INCDIR) -I$(PERINC) -I.

CXX = sccom
CXXFLAGS = -nologo -Wall -O2 -std=c++11 -g $(INCLUDE) --include definitions.h

VHDC = vcom
VHDFLAGS = -2008

all: vmap link

link: sc vhd
	@$(CXX) -work $(LIB) -link

sc:
	@$(CXX) -nologo -work $(LIB) $(INCLUDE) -std=c++11 -g src/BrLiteBuffer.cpp
	@$(CXX) -nologo -work $(LIB) $(INCLUDE) -std=c++11 -g src/BrLiteControl.cpp
	@$(CXX) -nologo -work $(LIB) $(INCLUDE) -std=c++11 -g src/BrLiteRouter.cpp
	@$(CXX) -nologo -work $(LIB) $(INCLUDE) -std=c++11 -g src/DMNI.cpp
	@$(CXX) -nologo -work $(LIB) $(INCLUDE) -std=c++11 -g src/ram.cpp
	@$(CXX) -nologo -work $(LIB) $(INCLUDE) -std=c++11 -g peripherals/AppInjector.cpp
	@$(CXX) -nologo -work $(LIB) $(INCLUDE) -std=c++11 -g peripherals/MAInjector.cpp
	@$(CXX) -nologo -work $(LIB) $(INCLUDE) -std=c++11 -g src/PE.cpp
	@$(CXX) -nologo -work $(LIB) $(INCLUDE) -std=c++11 -g src/RiscV.cpp
	@$(CXX) -nologo -work $(LIB) $(INCLUDE) -std=c++11 -g src/Memphis.cpp
	@$(CXX) -nologo -work $(LIB) $(INCLUDE) -std=c++11 -g src/test_bench.cpp
	@$(CXX) -nologo -work $(LIB) $(INCLUDE) -std=c++11 -g src/RouterCCwrapped2.cpp

vhd:
	@$(VHDC) -quiet -work $(LIB) $(VHDFLAGS) src/hemps_pkg.vhd
	@$(VHDC) -quiet -work $(LIB) $(VHDFLAGS) vhdl/standards.vhd
	@$(VHDC) -quiet -work $(LIB) $(VHDFLAGS) vhdl/Hermes_buffer.vhd
	@$(VHDC) -quiet -work $(LIB) $(VHDFLAGS) vhdl/Hermes_crossbar.vhd
	@$(VHDC) -quiet -work $(LIB) $(VHDFLAGS) vhdl/Hermes_switchcontrol.vhd
	@$(VHDC) -quiet -work $(LIB) $(VHDFLAGS) vhdl/RouterCC.vhd

vmap: vlib
	@vmap $(LIB) $(LIB)

vlib:
	@vlib $(LIB)

clean:
	@printf "Cleaning up\n"
	@rm -f *.o
	@rm $(TARGET)
