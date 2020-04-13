//#include "top_module.h"

#include "DMAC.h"
#include "SRAM.h"
#include "DRAM.h"
#include "DRAM_wrapper.h"
#include"Testbench.h"

SC_MODULE(wait_int){
  sc_in<bool> int_in;
  void trigger();
  SC_CTOR(wait_int){
    SC_THREAD(trigger); //<< SC_THREAD is a macro.
    sensitive << int_in;
  }
};

void wait_int::trigger(){ //<< Did you mean this instead of Thread class
  while(1){
    wait(); //Wait for mySignal
    if(int_in){
      sc_stop();
    }
  }
}

int sc_main(int argc, char* argv[])
{
  // DRAM_wrapper *dram_wrapper; //initiator
  DMAC *dmac;
  DRAM *dram;
  SRAM *sram;   //target
  DRAM_wrapper *dram_wrapper;
  wait_int *wait_self;

//signal
  sc_signal<sc_uint<ADDR_LENGTH>> addr_src, addr_dst, size;
  sc_signal<sc_uint<1>> start;
  sc_signal<bool> d2s;
  sc_signal<bool> interrupt;

  dmac = new DMAC("dmac");
  dram = new DRAM("dram");
  sram = new SRAM("sram");
  wait_self = new wait_int("wait");

  dmac->paddr_src(addr_src);
  dmac->paddr_dst(addr_dst);
  dmac->psize(size);
  dmac->pd2s(d2s);
  dmac->start(start);
  dmac->interrupt(interrupt);
  dmac->dram_socket(dram->socket);
  dmac->sram_socket(sram->socket);


  wait_self->int_in(interrupt);
  
  Testbench tb("data/config.txt",dram,sram);
  tb.begin();

  addr_src.write(0x000000);
  addr_dst.write(0x400000);
  size.write(100);
  d2s.write(1);
  sc_start(1, SC_NS);
  start.write(1);
  sc_start();
tb.end();

  cout << "\n*****    Finish     *****\n";

  return 0;
}
