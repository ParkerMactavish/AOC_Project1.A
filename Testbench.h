#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<stdlib.h>
#include"DRAM.h"
#include"SRAM.h"
#include"ps_config.h"
using namespace std;

#ifndef TESRBENCH_H
#define TESRBENCH_H

struct instruction{
    string op;
    long begin;
    long end;
    string target;
};

class  Testbench{
    public:
        Testbench(string config, DRAM* dram, SRAM* sram);

    //private:
        DRAM* dram;
        SRAM* sram;
        vector<instruction> presim_queue;
        vector<instruction> postsim_queue;
        
        void begin();
        void end();
        void execute(instruction inst);
        friend int parse_instruction(string line, Testbench* tb, int flag);
        void dram_get_input(uint32_t* iter, long begin, long end, string filename);
        void sram_print_mem(uint32_t* iter, long begin, long end,  string filename);
        string _16byte_to_4word(string str);
};

long hex_to_dec(char c);

#endif