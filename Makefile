LIB_DIR=-L/usr/local/systemc-2.3.3/lib-linux64 -Wl,-rpath=/usr/local/systemc-2.3.3/lib-linux64

INC_DIR=-I/usr/local/systemc-2.3.3/include -I./include -I./testbench

LIB=-lsystemc -lm

EXE=RESULT

APP_SRC= main.cpp DRAM_wrapper.cpp DRAM.cpp SRAM.cpp mm.cpp DMAC.cpp Testbench.cpp Controller.cpp PE_wrapper.cpp PE.cpp

PE_SRC = main.cpp MAC.cpp PE.cpp

PE_TEST = PE_top.cpp

PEwrapper_SRC = $(PE_SRC) PE_wrapper.cpp

PEwrapper_TEST = PEwrapper_top.cpp

all:
	g++ -Wall -g -o $(EXE) $(addprefix ./src/, $(APP_SRC)) $(addprefix ./testbench/, $(TEST_SRC)) $(LIB_DIR) $(INC_DIR) $(LIB)
	./$(EXE)

pe:
	g++ -Wall -g -o PE_TOP $(addprefix ./src/, $(PE_SRC)) $(addprefix ./testbench/, $(PE_TEST)) $(LIB_DIR) $(INC_DIR) $(LIB) -D PE
	./PE_TOP

pewrapper:
	g++ -Wall -g -o PEwrapper_TOP $(addprefix ./src/, $(PEwrapper_SRC)) $(addprefix ./testbench/, $(PEwrapper_TEST)) $(LIB_DIR) $(INC_DIR) $(LIB) -D PEwrapper
	./PEwrapper_TOP

clean:
	rm -rf $(APP_SRC:%.cpp=%.o) $(EXE) *.vcd *.fsdb *Log novas*
