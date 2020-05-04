#include "PE.h"

void PEU::threadProcess(){
  while(1){
    uint8_t tmpWeightaddrOldenPEen =  piWeightaddrOldenPEen.read();
    switch(tmpWeightaddrOldenPEen){
      case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
        vsgMACWeight[tmpWeightaddrOldenPEen] = piWeightOlddata;
      break;
      case PEOldDataDis:{
        uint8_t u8TmpSum = 0;
        for(int index = 0; index < NUM_MAC; index ++)
          u8TmpSum += vsgMACResult[index].read();
        poOutputData = u8TmpSum;
      }
      break;
      case PEOldDataEn:{
        uint8_t u8TmpSum = 0;
        for(int index = 0; index < NUM_MAC; index ++)
          u8TmpSum += vsgMACResult[index].read();
        u8TmpSum += piWeightOlddata.read();
        poOutputData = u8TmpSum;
      }
      break;
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