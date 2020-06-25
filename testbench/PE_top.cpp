#include "PE_top.h"

void PE_TOP::threadProcess(){
  int index = 0, phase = 0;
  sc_int<32> inputData[4][NUM_MAC] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8},
    {-8, -7, -6, -5, -4, -3, -2, -1, 0},
    {5, 5, 5, 5, 5, 5, 5, 5, 5},
    {-10, -10, -10, -10, -10, -10, -10, -10, -10}
  };

  sc_uint<32> weight[NUM_MAC] = {
    1, 2, 3, 4, -5, -6, -7, -8, -9
  };
  sc_trace_file* tf = sc_create_vcd_trace_file("./vcd/PE");
  sc_trace(tf, UUT.piClk, "Clock");
  sc_trace(tf, UUT.piWeightOlddata, "WeightOlddata");
  sc_trace(tf, UUT.piWeightaddrOldenPEen, "WeightaddrOldenPEen");
  char tmpStr[7] = "Input0";
  for(int i = 0; i < NUM_MAC; i ++){
    sc_trace(tf, UUT.vMACArr[i]->piInput, tmpStr);
    tmpStr[5]++;
  }
  sc_trace(tf, UUT.poOutputData, "Output");
  sc_trace(tf, index, "INDEX");
  clk = 1;
  while(1){
    wait(PRD/2.0);
    switch(phase){
      case 0:
        sgWeightaddrOldenPEen = index;
        sgWeightOlddata.write((uint64_t)weight[index]);
        cout<<"Current Weight"<<weight[index]<<endl;
            
        index++;
        if(index == 9){
          index = 0;
          phase ++;
        }
        break;
      case 1:
        sgWeightaddrOldenPEen = PEEnable;
        sgWeightOlddata.write(0);
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
        sgWeightOlddata.write(37);
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
    cout<<"I"<<index<<endl;
    // cout<<"Weight: "<<sgWeightOlddata<<endl;
    cout<<"PE Output"<<sgOutputData<<endl<<"Phase"<<phase<<endl;
    for(int i = 0; i < NUM_MAC; i ++)
      cout<<i<<": "<<UUT.vsgMACWeight[i]<<endl;
    cout<<endl;
    clk = 0;
    wait(PRD/2.0);
    clk = 1;
  }
}