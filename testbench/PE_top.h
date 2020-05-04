#ifndef _PE_TOP_H
#define _PE_TOP_H
#include "PE.h"
#include "systemc.h"

SC_MODULE(PE_TOP){
  PEU UUT;
  sc_time PRD;
  sc_signal<bool> clk;
  sc_signal<sc_uint<8> > sgWeightOlddata;
  sc_signal<sc_uint<4> > sgWeightaddrOldenPEen;
  sc_signal<sc_uint<8> > vsgInputData[NUM_MAC];
  sc_signal<sc_uint<8> > sgOutputData;

  void threadProcess();

  SC_CTOR(PE_TOP)
  : UUT("UUT")
  , PRD(20, SC_NS){
    UUT.piClk(clk);
    UUT.piWeightOlddata(sgWeightOlddata);
    UUT.piWeightaddrOldenPEen(sgWeightaddrOldenPEen);
    for(int i = 0; i < NUM_MAC; i ++)
      UUT.vpiInputData[i](vsgInputData[i]);
    UUT.poOutputData(sgOutputData);
    SC_THREAD(threadProcess);
  }
};

#endif