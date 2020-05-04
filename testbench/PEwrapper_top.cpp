#include "PEwrapper_top.h"

void PEwrapper_top::threadProcess(){
  int testIndex = 0, testPhase = 0;
  sc_uint<8> inputData[4][NUM_MAC] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8},
    {8, 7, 6, 5, 4, 3, 2, 1, 0},
    {5, 5, 5, 5, 5, 5, 5, 5, 5},
    {10, 10, 10, 10, 10, 10, 10, 10, 10}
  };

  sc_uint<8> weight[NUM_MAC] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9
  };
  sc_trace_file* tf = sc_create_vcd_trace_file("./vcd/PEwrapper");
  sc_trace(tf, UUT.piClk, "Clock");
  char tmpStr[20] = "Input0_0";
  for(int i = 0; i < NUM_MAC; i ++){
    tmpStr[5] = '0';
    sc_trace(tf, UUT.vpiInputData[0][i], tmpStr);
    tmpStr[5] = '1';
    sc_trace(tf, UUT.vpiInputData[1][i], tmpStr);
    tmpStr[7]++;
  }
  sc_trace(tf, UUT.piNumPEEnable, "NumPEEnable");
  sc_trace(tf, UUT.piPEsState, "PEsState");
  strcpy(tmpStr, "Output0_0");  
  for(int i = 0; i < NUM_MAC; i ++){
    tmpStr[6] = '0';
    sc_trace(tf, UUT.vpioOutputSRAMData[0][i], tmpStr);
    tmpStr[6] = '1';
    sc_trace(tf, UUT.vpioOutputSRAMData[1][i], tmpStr);
    tmpStr[8]++;
  }
  sc_trace(tf, testIndex, "testIndex");
  strcpy(tmpStr, "MACResult00.0");
  for(int index = 0; index < NUM_PE; index++){
    tmpStr[12] = '0';
    for(int index2 = 0; index2 < NUM_MAC; index2 ++){
      sc_trace(tf, UUT.vPEUArr[index].vsgMACResult[index2], tmpStr);
      tmpStr[12]++;
    }
    tmpStr[10]++;
    if(tmpStr[10] > '9'){
      tmpStr[9] ++;
      tmpStr[10] = '0';
    }
  }
  strcpy(tmpStr, "MACWeight00.0");
  for(int index = 0; index < NUM_PE; index++){
    tmpStr[12] = '0';
    for(int index2 = 0; index2 < NUM_MAC; index2 ++){
      sc_trace(tf, UUT.vPEUArr[index].vsgMACWeight[index2], tmpStr);
      tmpStr[12]++;
    }
    tmpStr[10]++;
    if(tmpStr[10] > '9'){
      tmpStr[9] ++;
      tmpStr[10] = '0';
    }
  }
  strcpy(tmpStr, "MAC_Input00.0");
  for(int index = 0; index < NUM_PE; index++){
    tmpStr[12] = '0';
    for(int index2 = 0; index2 < NUM_MAC; index2 ++){
      sc_trace(tf, UUT.vPEUArr[index].vsgMACInput[index2], tmpStr);
      tmpStr[12]++;
    }
    tmpStr[10]++;
    if(tmpStr[10] > '9'){
      tmpStr[9] ++;
      tmpStr[10] = '0';
    }
  }
  strcpy(tmpStr, "WOP00");
  for(int index = 0; index < NUM_PE; index++){
    sc_trace(tf, UUT.vsgWeightaddrOldenPEen[index], tmpStr);
    tmpStr[4]++;
    if(tmpStr[4] > '9'){
      tmpStr[3] ++;
      tmpStr[4] = '0';
    }
  }
  strcpy(tmpStr, "poOutput00");
  for(int index = 0; index < NUM_PE; index++){
    sc_trace(tf, UUT.vPEUArr[index].poOutputData, tmpStr);
    tmpStr[9]++;
    if(tmpStr[9] > '9'){
      tmpStr[8] ++;
      tmpStr[9] = '0';
    }
  }
  strcpy(tmpStr, "poOutput00");
  for(int index = 0; index < NUM_PE; index++){
    sc_trace(tf, UUT.vPEUArr[index].poOutputData, tmpStr);
    tmpStr[9]++;
    if(tmpStr[9] > '9'){
      tmpStr[8] ++;
      tmpStr[9] = '0';
    }
  }
  clk = 0;
  while(1){
    clk = 0;
    switch(testPhase){
      /* Clear weight addr for all PEs */
      case -1: case 0:
        sgNumPEEnable = NUM_PE;
        sgPEsState = sWeightAddrClear;
        cout<<"Ard"<<endl;
        testPhase ++;
        break;
      /* Weight from 0 */
      case 1:
        if(testIndex == 10){
          testIndex = 0;
          testPhase ++;
        }
        else if(testIndex == 9){
          sgPEsState = sOldoutputdisWrite1;
          testIndex++;
        }
        else{
          sgNumPEEnable = NUM_PE;
          sgPEsState = sWeightAddrIncFrom0;
          for(int index = 0; index < NUM_PE; index ++){
            vsgOutputSRAMData[0][index] = weight[(testIndex+index)%NUM_MAC];
          }
          testIndex++;
        }
        // for(int index = 0; index < NUM_PE; index ++){
        //   for(int index2 = 0; index2 < NUM_MAC; index ++){
        //     cout<<UUT.vPEUArr[index].vsgMACWeight[index2].read()<<' ';
        //   }
        //   cout<<endl;
        // } 
        break;
      /* Output to 1 */
      case 2:
        sgNumPEEnable = NUM_PE;
        for(int j = 0; j < NUM_MAC; j ++){
          vsgInputData[0][j] = inputData[testIndex][j];
          vsgInputData[1][j] = inputData[testIndex][NUM_MAC-j-1];
        }
        testIndex++;
        if(testIndex == 4){
          testIndex = 0;
          testPhase ++;
        }
        break;
      /* OldOutput from 0 output to 1 */
      case 3:
        sgNumPEEnable = NUM_PE;
        sgPEsState = sOldoutput0Write1;
        for(int j = 0; j < NUM_MAC; j ++){
          vsgInputData[0][j] = inputData[testIndex][j];
          vsgInputData[1][j] = inputData[testIndex][NUM_MAC-j-1];
          vsgOutputSRAMData[0][j] = 1;
        }
        testIndex++;
        if(testIndex == 4){
          testIndex = 0;
          testPhase ++;
        }
        break;
      case 4:
        sc_stop();
        sc_close_vcd_trace_file(tf);
    }
    cout<<sc_time_stamp()<<endl;
    wait(PRD/2.0);
    clk = 1;
    wait(PRD/2.0);
  }
}