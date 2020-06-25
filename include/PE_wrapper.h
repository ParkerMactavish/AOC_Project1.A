#include "systemc.h"
#include "PE.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"
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

SC_MODULE(PEs){
  /* Connection to Controller */
  tlm_utils::simple_target_socket<PEs> sktInputFromController;
  sc_in<sc_uint<6>> piNumPEEnable;
  sc_in<sc_uint<4>> piPEsState;

  /* Connection to Output SRAM */
  tlm_utils::simple_initiator_socket<PEs> skiToOSRAM;

  /* Internal Modules and Signals */
  sc_vector<PE> vPEArr;
  // sc_signal<bool> sgEnable;

  /* Connection to PEs */
  sc_signal<sc_int<64>> vsgWeightOlddata[NUM_PE];
  sc_signal<sc_int<64>> vsgOutputData[NUM_PE];
  sc_signal<sc_int<32>> vsgInputData[NUM_PE][NUM_MAC];
  sc_signal<bool>       vsgClock[NUM_PE];
  sc_signal<sc_uint<4>> vsgWeightaddrOldenPEen[NUM_PE];
  sc_signal<sc_uint<2>> vsgActivationFunction[NUM_PE];

  void select_WeightOlddata_Output();
  void update_InputData();
  void pass_Clock();
  void set_WeightaddrOldenPEen();

  SC_CTOR(PEs)
  : vPEArr("PEArr", NUM_PE){
    for(int index = 0; index < NUM_PE; index ++){
      vPEArr[index].piClk(vsgClock[index]);
      vPEArr[index].piWeightOlddata(vsgWeightOlddata[index]);
      vPEArr[index].piWeightaddrOldenPEen(vsgWeightaddrOldenPEen[index]);
      vPEArr[index].piActivationFunction(vsgActivationFunction[index]);
      vPEArr[index].poOutputData(vsgOutputData[index]);
      for(int index2 = 0; index2 < NUM_MAC; index2 ++){
        vPEArr[index].vpiInputData[index2](vsgInputData[index][index2]);
      }
    }
    SC_METHOD(select_WeightOlddata_Output);
    for(int index = 0; index < NUM_PE; index ++)
      sensitive<<vsgOutputData[index];
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