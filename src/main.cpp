//#include "top_module.h"

#ifdef PE
#include "PE_top.h"
#else
#include "DMAC.h"
#include "SRAM.h"
#include "DRAM.h"
#include "DRAM_wrapper.h"
#include "Controller.h"
#include"PE_wrapper.h"
#include"PE.h"
#include"Testbench.h"
#endif

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

#ifdef PE
int sc_main(int argc, char* argv[]){
  PE_TOP* top = new PE_TOP("TOP");
  sc_start();
  return 0;
}
#else
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
  // dram_wrapper = new DRAM_wrapper("dram_wrapper");
  wait_self = new wait_int("wait");

  // sc_signal<sc_uint<ADDR_LENGTH>> m_addr;
  // sc_signal<sc_uint<DATA_LENGTH>> wr_data, data_out;
  // sc_signal<sc_uint<1>> wr_enable;
  // dram_wrapper->dram_addr_i(m_addr);
  // dram_wrapper->data(wr_data);
  // dram_wrapper->wr_enable(wr_enable);
  // dram_wrapper->dram_dataout_i(data_out);
  // dram_wrapper->socket.bind(dram->socket);

  dmac->paddr_src(addr_src);
  dmac->paddr_dst(addr_dst);
  dmac->psize(size);
  dmac->pd2s(d2s);
  dmac->start(start);
  dmac->interrupt(interrupt);
  dmac->dram_socket(dram->socket);
  dmac->sram_socket(sram->socket);


  wait_self->int_in(interrupt);



//   char* name[20];
//   int num = 25;
//   sc_signal<uint32_t>  sig[num];
//   sc_out<uint32_t> _mem[num];
//   for(int i=0; i<num; i++)
//     _mem[i](sig[i]);
//   for(int i=0; i<num; i++)
//     _mem[i].write(sram->mem[i+0x100000]);

//   int j=0;
//   sc_trace_file* tf = sc_create_vcd_trace_file("wave");
//  for(int i = 0x100000; i < 0x100000+num; i ++){
//    sprintf(name,"%x\0",i);
//     sc_trace(tf,sig[j],name);
//     j++;
//   }
  

// -----------------Controller--------------------------//
sc_signal<bool> enable;
sc_signal<bool> which;
//sc_signal<bool> finish;
//sc_signal<bool>  pe_enable;
//sc_signal<uint32_t>  addr[PESIZE];
Controller* con = new Controller("controller");
PEwrapper * wrapper  = new PEwrapper("pewrapper");
con->enable(enable);
con->which(which);
wrapper->pe_write=&(con->pe_write);
con->pe_enable=&(wrapper->pe_enable);

con->sram_ifirst = sram;
con->sram_weight=sram;
con->pe_mem=PE::pe_mem;
for(int i=0; i<NUM_PE; i++)
  con->pe_local[i]=wrapper->pe_vec[i]->pe_local;
con->input_len = 5; 
PE::input_len = 5;
enable.write(1);
which.write(0);

for(int i = 0x000000; i < 0x000000+SIZE_TILE; i ++){
    sram->mem[i]=i;
  }

  cout<<hex;
  // for(int i = 0; i < 1000; i ++){
  //   dram->mem[i] = i+1000;
  // }
  // addr_src.write(0x000000);
  // addr_dst.write(0x000000);
  // size.write(100);
  // d2s.write(1);
  // sc_start(1, SC_NS);
  // start.write(1);

  // for(int i = 0; i < 10; i ++){
  //   m_addr.write(i*4);
  //   wr_enable.write(0);
  //   sc_start(5, SC_NS);
  //   cout<<data_out.read()<<endl;
  // }
  sc_start(10*CLK_CYCLE, SC_NS);
  // for(int i = 0x000000; i < 0x000000+25; i ++){
  //   cout<<i<<' '<<sram->mem[i]<<endl;
  // }


//   cout<<"---------------Controller-------------------"<<endl;
//   cout<<"ibuffer:"<<endl;
//   for(int i = 0; i <SIZE_TILE; i ++){
//     cout<<i<<' '<<con->ibuffer[i]<<endl;
//   }
// cout<<"wbuffer:"<<endl;
//  for(int i = 0; i <SIZE_MAC; i ++)
//       cout<<i<<' '<<con->wbuffer[0][i]<<endl;

  
  cout << "\n*****    Finish     *****\n";

  return 0;
}
#endif