#include "systemc.h"
#include "PE.h"
#ifndef _PE_WRAPPER_H
#define _PE_WRAPPER_H

SC_MODULE(PEwrapper){
  /* Connection to Clock */
  sc_inout_clk  Clk;

  /* Connection to Controller */
  sc_in<sc_uint<8>> piInputData[2][NUM_MAC];
  sc_in<sc_uint<1>> piWeightOutputSel;
  sc_in<sc_uint<4>> piNumPEEnable;
  sc_in<sc_uint<2>> piOutputSRAMWriteEnable;

  /* Connection to Output SRAM */
  sc_inout<sc_uint<8>> pioOutputSRAMData[2][NUM_PE];

  /* Internal Modules and Signals */
  sc_vector<PEU> vPEUArr;

  /* Connection to PEs */
  sc_signal<sc_uint<8>> vsgWeightOlddata[NUM_PE];
  sc_signal<sc_uint<8>> vsgOutputData[NUM_PE];
  sc_signal<sc_uint<8>> vsgInputData[2][NUM_MAC];
  sc_signal<bool>       vsgClock[NUM_PE];
  sc_signal<sc_uint<4>> vsgWeightaddrOldenPEen[NUM_PE];

  void select_WeightOlddata_Output();
  void update_InputData();
  void pass_Clock();
  void set_WeightaddrOldenPEen();

  SC_CTOR(PEwrapper)
  : vPEUArr("PEUArr", NUM_PE){
    for(int index = 0; index < NUM_PE; index ++){
      vPEUArr[index].piClk(vsgClock[index]);
      vPEUArr[index].piWeightOlddata(vsgWeightOlddata[index]);
      vPEUArr[index].piWeightaddrOldenPEen(vsgWeightaddrOldenPEen[index]);
      vPEUArr[index].poOutputData(vsgOutputData[index]);
      for(int index2 = 0; index2 < NUM_MAC; index2 ++){
        if(index < 16){
          vPEUArr[index].vpiInputData[index2](vsgInputData[0][index2]);
        }
        else{
          vPEUArr[index].vpiInputData[index2](vsgInputData[1][index2]);
        }
      }
    }
    SC_METHOD(select_WeightOlddata_Output);
    for(int index = 0; index < NUM_PE; index ++)
      sensitive<<pioOutputSRAMData[0][index]<<pioOutputSRAMData[1][index];
    sensitive<<piOutputSRAMWriteEnable;
      
    SC_METHOD(pass_Clock());
    sensitive<<piClk;
  }
};
#endif