#ifndef _PEWRAPPER_TOP_H
#define _PEWRAPPER_TOP_H
#include "PE_wrapper.h"

SC_MODULE(PEwrapper_top){
  PEUs UUT;
  sc_time PRD;
  sc_signal<bool> clk;
  sc_signal<sc_uint<8> > vsgInputData[2][NUM_MAC];
  sc_signal<sc_uint<6> > sgNumPEEnable;
  sc_signal<sc_uint<4> > sgPEsState;
  sc_signal<sc_uint<8> > vsgOutputSRAMData[2][NUM_PE];
  
  void threadProcess();

  SC_CTOR(PEwrapper_top)
  : UUT("UUT")
  , PRD(20, SC_NS){
    UUT.piClk(clk);
    UUT.piNumPEEnable(sgNumPEEnable);
    UUT.piPEsState(sgPEsState);
    for(int index = 0; index < NUM_MAC; index ++){
      UUT.vpiInputData[0][index](vsgInputData[0][index]);
      UUT.vpiInputData[1][index](vsgInputData[1][index]);
    }
    for(int index = 0; index < NUM_PE; index ++){
      UUT.vpioOutputSRAMData[0][index](vsgOutputSRAMData[0][index]);
      UUT.vpioOutputSRAMData[1][index](vsgOutputSRAMData[1][index]);
    }
    SC_THREAD(threadProcess);
  }
};

#endif