#include "PE.h"
#include "cmath"

void PE::set_Weight(){
  while(1){
    wait();
    // cout<<piWeightaddrOldenPEen.read()<<endl;
    uint8_t tmpWeightaddrOldenPEen =  piWeightaddrOldenPEen.read();
    switch(tmpWeightaddrOldenPEen){
      case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
        vsgMACWeight[tmpWeightaddrOldenPEen] = piWeightOlddata.read()(31, 0);
      break;
      case PEEnable:
      default:;    
    }
  }
}

void PE::update_Input(){
  for(int index = 0; index < NUM_MAC; index ++)
    vsgMACInput[index] = vpiInputData[index];
}

void PE::update_Output(){
  int64_t i64TmpSum = piWeightOlddata.read();
  for(int index = 0; index < NUM_MAC; index ++)
    i64TmpSum += vsgMACResult[index].read();
  if(piWeightaddrOldenPEen.read() == PEEnable){
    switch(piActivationFunction.read()){
      case ActivationDisable:
        poOutputData = i64TmpSum;
        break;
      case ActivationReLU:
        poOutputData = (i64TmpSum > 0) ? i64TmpSum : 0;
        break;
      case ActivationSigmoid:
        poOutputData = (int64_t)(1/(1+exp(-i64TmpSum)));
        break;
      case ActivationTanh:
        poOutputData = (int64_t)(exp(i64TmpSum)-exp(i64TmpSum))/(exp(i64TmpSum)+exp(i64TmpSum));
    }
  }
  else{
    poOutputData = 0;
  }
}