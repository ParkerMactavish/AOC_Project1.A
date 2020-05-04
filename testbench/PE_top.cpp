#include "PE_top.h"

void PE_TOP::threadProcess(){
  int index = 0, phase = 0;
  sc_uint<8> inputData[4][NUM_MAC] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8},
    {8, 7, 6, 5, 4, 3, 2, 1, 0},
    {5, 5, 5, 5, 5, 5, 5, 5, 5},
    {10, 10, 10, 10, 10, 10, 10, 10, 10}
  };

  sc_uint<8> weight[NUM_MAC] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9
  };
  sc_trace_file* tf = sc_create_vcd_trace_file("./vcd/PE");
  sc_trace(tf, UUT.piClk, "Clock");
  sc_trace(tf, UUT.piWeightOlddata, "WeightOlddata");
  sc_trace(tf, UUT.piWeightaddrOldenPEen, "WeightaddrOldenPEen");
  char tmpStr[7] = "Input0";
  for(int i = 0; i < NUM_MAC; i ++){
    sc_trace(tf, UUT.vMACArr[i].piInput, tmpStr);
    tmpStr[5]++;
  }
  sc_trace(tf, UUT.poOutputData, "Output");
  sc_trace(tf, index, "INDEX");
  clk = 0;
  while(1){
    clk = 0;
    sgWeightaddrOldenPEen = 0;
    switch(phase){
      case 0:
        sgWeightaddrOldenPEen = index;
        sgWeightOlddata = weight[index];
        index++;
        if(index == 9){
          index = 0;
          phase ++;
        }
        break;
      case 1:
        sgWeightaddrOldenPEen = PEOldDataDis;
        sgWeightOlddata = 37;
        for(int j = 0; j < NUM_MAC; j ++){
          vsgInputData[j] = inputData[index][j];
        }
        index ++;
        if(index == 4){
          index = 0;
          phase ++;
        }
        break;
      case 2:
        sgWeightaddrOldenPEen = PEOldDataEn;
        sgWeightOlddata = 37;
        for(int j = 0; j < NUM_MAC; j ++){
          vsgInputData[j] = inputData[index][j];
        }
        index ++;
        if(index == 4){
          index = 0;
          phase ++;
        }
        break;
      case 3:
        sc_stop();
        sc_close_vcd_trace_file(tf);
    }
    // cout<<"Weight: "<<sgWeightOlddata<<endl;
    cout<<"PE Output"<<sgOutputData<<endl<<"Phase"<<phase<<endl;
    for(int i = 0; i < NUM_MAC; i ++)
      cout<<i<<": "<<UUT.vsgMACWeight[i]<<endl;
    cout<<endl;
    wait(PRD/2.0);
    clk = 1;
    wait(PRD/2.0);
  }
}