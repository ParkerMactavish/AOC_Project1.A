LIB_DIR=-L/usr/local/systemc-2.3.3/lib-linux64 -Wl,-rpath=/usr/local/systemc-2.3.3/lib-linux64

INC_DIR=-I/usr/local/systemc-2.3.3/include -I./include -I./testbench

LIB=-lsystemc -lm

EXE=RESULT

APP_SRC= main.cpp DRAM_wrapper.cpp DRAM.cpp SRAM.cpp mm.cpp DMAC.cpp Testbench.cpp Controller.cpp PE_wrapper.cpp PE.cpp

PE_SRC = main.cpp MAC.cpp PE.cpp

PE_TEST = PE_top.cpp

all:
	g++ -Wall -g -o $(EXE) $(addprefix ./src/, $(APP_SRC)) $(addprefix ./testbench/, $(TEST_SRC)) $(LIB_DIR) $(INC_DIR) $(LIB)
	./$(EXE)

pe:
	g++ -Wall -g -o $(EXE) $(addprefix ./src/, $(PE_SRC)) $(addprefix ./testbench/, $(PE_TEST)) $(LIB_DIR) $(INC_DIR) $(LIB) -D PE
	./$(EXE)

clean:
	rm -rf $(APP_SRC:%.cpp=%.o) $(EXE) *.vcd *.fsdb *Log novas*
