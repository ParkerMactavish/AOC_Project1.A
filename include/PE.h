#include "systemc.h"
#include "MAC.h"
#ifndef _PE_H
#define _PE_H

#define PEDisable     0xb
#define PEOldDataEn   0xa
#define PEOldDataDis  0x9

SC_MODULE(PEU){
  /* Connection between PE and PE wrapper */
  sc_in_clk piClk;
  sc_in<sc_uint<8> > piWeightOlddata;
  sc_in<sc_uint<4> > piWeightaddrOldenPEen;
  sc_in<sc_uint<8> > vpiInputData[NUM_MAC];
  
  sc_out<sc_uint<8> > poOutputData;

  /* Connection between PE and MAC */
  sc_signal<sc_uint<8> > vsgMACWeight[NUM_MAC];
  sc_signal<sc_uint<8> > vsgMACResult[NUM_MAC];
  sc_signal<sc_uint<8> > vsgMACInput[NUM_MAC];

  sc_vector<MAC> vMACArr;

  void threadProcess();
  void update_Input();

  SC_CTOR(PEU)
  : vMACArr("MAC_arr", NUM_MAC){
    for(int index = 0; index < NUM_MAC; index++){
      vMACArr[index].piWeight(vsgMACWeight[index]);
      vMACArr[index].poResult(vsgMACResult[index]);
      vMACArr[index].piInput(vsgMACInput[index]);
    }
    SC_CTHREAD(threadProcess, piClk.pos());
    SC_METHOD(update_Input);
    for(int index = 0; index < NUM_MAC; index ++)
      sensitive<<vpiInputData[index];
  }
};
#endif