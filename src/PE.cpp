#include "PE.h"

void PEU::set_Weight(){
  while(1){
    uint8_t tmpWeightaddrOldenPEen =  piWeightaddrOldenPEen.read();
    switch(tmpWeightaddrOldenPEen){
      case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
        vsgMACWeight[tmpWeightaddrOldenPEen] = piWeightOlddata;
      break;
      case PEOldDataDis:
      case PEOldDataEn:
      case PEDisable:
      default:;    
    }
    wait();
  }
}

void PEU::update_Input(){
  for(int index = 0; index < NUM_MAC; index ++)
    vsgMACInput[index] = vpiInputData[index];
}

void PEU::update_Output(){
  uint8_t u8TmpSum = 0;
  if(piWeightaddrOldenPEen.read() == PEOldDataDis){
    for(int index = 0; index < NUM_MAC; index ++)
      u8TmpSum += vsgMACResult[index].read();
    poOutputData = u8TmpSum;
  }
  else if(piWeightaddrOldenPEen.read() == PEOldDataEn){
    for(int index = 0; index < NUM_MAC; index ++)
      u8TmpSum += vsgMACResult[index].read();
    u8TmpSum += piWeightOlddata.read();
    poOutputData = u8TmpSum;
  }
}