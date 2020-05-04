#include "systemc.h"
#include "PE.h"
#include "ps_config.h"
#ifndef _PE_WRAPPER_H
#define _PE_WRAPPER_H

enum PEsState{
  sWeightAddrClear, 
  sWeightAddrIncFrom0,
  sWeightAddrIncFrom1, 
  sOldoutputdisWrite0,
  sOldoutputdisWrite1,
  sOldoutput0Write1,
  sOldoutput1Write0,
  sHalt,
};

SC_MODULE(PEUs){
  /* Connection to Clock */
  sc_in_clk  piClk;

  /* Connection to Controller */
  sc_in<sc_uint<8>> vpiInputData[2][NUM_MAC];
  sc_in<sc_uint<6>> piNumPEEnable;
  sc_in<sc_uint<4>> piPEsState;

  /* Connection to Output SRAM */
  sc_inout<sc_uint<8>> vpioOutputSRAMData[2][NUM_PE];

  /* Internal Modules and Signals */
  sc_vector<PEU> vPEUArr;
  // sc_signal<bool> sgEnable;

  /* Connection to PEs */
  sc_signal<sc_uint<8>> vsgWeightOlddata[NUM_PE];
  sc_signal<sc_uint<8>> vsgOutputData[NUM_PE];
  sc_signal<sc_uint<8>> vsgInputData[NUM_PE][NUM_MAC];
  sc_signal<bool>       vsgClock[NUM_PE];
  sc_signal<sc_uint<4>> vsgWeightaddrOldenPEen[NUM_PE];

  void select_WeightOlddata_Output();
  void update_InputData();
  void pass_Clock();
  void set_WeightaddrOldenPEen();

  SC_CTOR(PEUs)
  : vPEUArr("PEUArr", NUM_PE){
    for(int index = 0; index < NUM_PE; index ++){
      vPEUArr[index].piClk(vsgClock[index]);
      vPEUArr[index].piWeightOlddata(vsgWeightOlddata[index]);
      vPEUArr[index].piWeightaddrOldenPEen(vsgWeightaddrOldenPEen[index]);
      vPEUArr[index].poOutputData(vsgOutputData[index]);
      for(int index2 = 0; index2 < NUM_MAC; index2 ++){
        vPEUArr[index].vpiInputData[index2](vsgInputData[index][index2]);
      }
    }
    SC_METHOD(select_WeightOlddata_Output);
    for(int index = 0; index < NUM_PE; index ++)
      sensitive<<vpioOutputSRAMData[0][index]
               <<vpioOutputSRAMData[1][index]
               <<vsgOutputData[index];
    sensitive<<piPEsState;
    
    SC_METHOD(update_InputData);
    for(int index = 0; index < NUM_MAC; index ++)
      sensitive<<vpiInputData[0][index]<<vpiInputData[1][index];
    
    SC_METHOD(pass_Clock);
    sensitive<<piClk;
    
    SC_CTHREAD(set_WeightaddrOldenPEen, piClk.pos());
  }
};
#endif