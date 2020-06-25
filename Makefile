LIB_DIR=-L/usr/local/systemc-2.3.3/lib-linux64 -Wl,-rpath=/usr/local/systemc-2.3.3/lib-linux64

INC_DIR=-I/usr/local/systemc-2.3.3/include -I./include -I./testbench

LIB=-lsystemc -lm

EXE=RESULT

APP_SRC = INPUT_SRAM OUTPUT_SRAM mm DMAC Controller PE DRAM
APP_OBJ = $(addprefix ./obj/,$(APP_SRC:=.o))
APP_CPP = ./src/$(APP_SRC:=.cpp)

DMAC_SRC = main.cpp DRAM.cpp SRAM.cpp mm.cpp DMAC.cpp
DMAC_TEST =

PE_SRC = main.cpp PE.cpp
PE_TEST = PE_top.cpp

PEwrapper_SRC = $(PE_SRC) PE_wrapper.cpp
PEwrapper_TEST = PEwrapper_top.cpp

DRAM_TIMING_SRC = main.cpp DRAM.cpp mm.cpp
DRAM_TIMING_TEST = DRAM_timing.cpp

SRAM_SRC = INPUT_SRAM mm DRAM
SRAM_TEST = SRAM_top
SRAM_OBJ = $(addprefix ./obj/,$(SRAM_SRC:=.o))
SRAM_TEST_OBJ =  $(addprefix ./testbench/,$(SRAM_TEST:=.o))
SRAM_CPP = ./src/$(SRAM_SRC:=.cpp) 
SRAM_TEST_CPP = ./testbench/$(SRAM_TEST:=.cpp)

all: $(EXE)
	./bin/$<

$(EXE): $(APP_OBJ)
	g++ -Wall -g $(APP_OBJ) ./src/main.cpp -o $(addprefix ./bin/, $@) $(LIB_DIR) $(INC_DIR) $(LIB)


obj/%.o: ./src/%.cpp
	g++ -Wall -g -c $< -o $@ $(INC_DIR) $(LIB)

dmac: DMAC_TOP
	./bin/DMAC_TOP
DMAC_TOP:
	g++ -Wall -g -o ./bin/DMAC_TOP $(addprefix ./src/, $(DMAC_SRC)) $(addprefix ./testbench/, $(DMAC_TEST)) $(LIB_DIR) $(INC_DIR) $(LIB) -D dmac

pe:
	g++ -Wall -g -o ./bin/PE_TOP $(addprefix ./src/, $(PE_SRC)) $(addprefix ./testbench/, $(PE_TEST)) $(LIB_DIR) $(INC_DIR) $(LIB) -D pe
	./bin/PE_TOP

pewrapper: 
	g++ -Wall -g -o ./bin/PEwrapper_TOP $(addprefix ./src/, $(PEwrapper_SRC)) $(addprefix ./testbench/, $(PEwrapper_TEST)) $(LIB_DIR) $(INC_DIR) $(LIB) -D PEwrapper
	./bin/PEwrapper_TOP

dramt: bin/DRAMt_TOP
	./bin/DRAMt_TOP
bin/DRAMt_TOP:
	g++ -Wall -g -o ./bin/DRAMt_TOP $(addprefix ./src/, $(DRAM_TIMING_SRC)) $(addprefix ./testbench/, $(DRAM_TIMING_TEST)) $(LIB_DIR) $(INC_DIR) $(LIB) -D DRAMt -D CON

sram: $(SRAM_OBJ) $(SRAM_TEST_OBJ)
	g++ -Wall -g -o ./bin/SRAM_TOP $(SRAM_TEST_OBJ) $(SRAM_OBJ) ./src/main.cpp $(LIB_DIR) $(INC_DIR) $(LIB) -D sram
	./bin/SRAM_TOP

obj/%.o: ./src/%.cpp
	g++ -Wall -g -c $< -o $@ $(INC_DIR) $(LIB)

testbench/%.o: ./testbench/%.cpp
	g++ -Wall -g -c $< -o $@ $(INC_DIR) $(LIB)

clean:
	rm -rf $(APP_SRC:%.cpp=%.o) $(EXE) *.vcd *.fsdb *Log novas* ./bin/*
