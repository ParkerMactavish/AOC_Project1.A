#include "systemc.h"
#include "MAC.h"
#include "ps_config.h"
#ifndef _PE_H
#define _PE_H

#define PEEnable  0x9

#define ActivationDisable   0x0
#define ActivationReLU      0x1
#define ActivationSigmoid   0x2
#define ActivationTanh      0x3

SC_MODULE(PE){
  /* Connection between PE and PE wrapper */
  sc_in_clk piClk;
  sc_in<sc_int<64> > piWeightOlddata;
  sc_in<sc_uint<4> > piWeightaddrOldenPEen;
  sc_in<sc_uint<2> > piActivationFunction;
  sc_in<sc_int<32> > vpiInputData[NUM_MAC];
  
  sc_out<sc_int<64> > poOutputData;

  /* Connection between PE and MAC */
  sc_signal<sc_int<32> > vsgMACWeight[NUM_MAC];
  sc_signal<sc_int<64> > vsgMACResult[NUM_MAC];
  sc_signal<sc_int<32> > vsgMACInput[NUM_MAC];

  MAC* vMACArr[NUM_MAC];

  void set_Weight();
  void update_Input();
  void update_Output();

  SC_CTOR(PE){
    for(int index = 0; index < NUM_MAC; index++){
      vMACArr[index] = new MAC("MAC");
      vMACArr[index]->piWeight(vsgMACWeight[index]);
      vMACArr[index]->poResult(vsgMACResult[index]);
      vMACArr[index]->piInput(vsgMACInput[index]);
    }
    SC_CTHREAD(set_Weight, piClk.pos());
    SC_METHOD(update_Input);
    for(int index = 0; index < NUM_MAC; index ++) 
      sensitive<<vpiInputData[index];
    SC_METHOD(update_Output);
    for(int index = 0; index < NUM_MAC; index ++)
      sensitive<<vsgMACResult[index];
  }
};
#endif