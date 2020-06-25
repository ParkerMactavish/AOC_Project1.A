#include "PE_wrapper.h"

void PEs::select_WeightOlddata_Output(){
  switch(piPEsState.read()){
    case sWeightAddrIncFrom0:
      for(uint8_t index = 0; index < piNumPEEnable.read(); index ++)
        vsgWeightOlddata[index] = vpioOutputSRAMData[0][index];
      break;
    case sWeightAddrIncFrom1:
      for(uint8_t index = 0; index < piNumPEEnable.read(); index ++)
        vsgWeightOlddata[index] = vpioOutputSRAMData[1][index];
      break;
    case sOldoutputdisWrite0: 
      for(uint8_t index = 0; index < piNumPEEnable.read(); index ++){
        vsgWeightOlddata[index] = 0;
        vpioOutputSRAMData[0][index] = vsgOutputData[index];
      }
      break;
    case sOldoutputdisWrite1:
      for(uint8_t index = 0; index < piNumPEEnable.read(); index ++){
        vsgWeightOlddata[index] = 0;
        vpioOutputSRAMData[1][index] = vsgOutputData[index];
      }
      break;
    case sOldoutput0Write1:
      for(uint8_t index = 0; index < piNumPEEnable.read(); index ++){
        vsgWeightOlddata[index] = vpioOutputSRAMData[0][index];
        vpioOutputSRAMData[1][index] = vsgOutputData[index];
      }
      break;
    case sOldoutput1Write0:
      for(uint8_t index = 0; index < piNumPEEnable.read(); index ++){
        vsgWeightOlddata[index] = vpioOutputSRAMData[1][index];
        vpioOutputSRAMData[0][index] = vsgOutputData[index];
      }
      break;
    case sWeightAddrClear: case sHalt: default:;
  }
}

void PEs::update_InputData(){
  for(uint8_t index = 0; index < NUM_PE; index ++){
    for(uint8_t index2 = 0; index2 < NUM_MAC; index2 ++){
      vsgInputData[index][index2] = (index<16)?vpiInputData[0][index2]
                                              :vpiInputData[1][index2];
    }
  }
}

void PEs::pass_Clock(){
  for(uint8_t index = 0; index < NUM_PE; index ++){
    vsgClock[index] = piClk;
  }
}

void PEs::set_WeightaddrOldenPEen(){
  while(1){
    switch(piPEsState.read()){
      case sWeightAddrClear:
        for(uint8_t index = 0; index < piNumPEEnable.read(); index ++){
          vsgWeightaddrOldenPEen[index].write(0xf);
          cout<<vsgWeightaddrOldenPEen[index].read()<<' ';
        }
        break;

      case sWeightAddrIncFrom0:
        for(uint8_t index = 0; index < piNumPEEnable.read(); index ++){
          vsgWeightaddrOldenPEen[index].write(vsgWeightaddrOldenPEen[index].read() + 1);
        }
        for(uint8_t index = piNumPEEnable.read(); index < NUM_MAC; index ++){
          vsgWeightaddrOldenPEen[index] = PEDisable;
        }
        break;
      
      case sWeightAddrIncFrom1:      
        for(uint8_t index = 0; index < piNumPEEnable.read(); index ++){
          vsgWeightaddrOldenPEen[index].write(vsgWeightaddrOldenPEen[index].read()+1);
        }
        for(uint8_t index = piNumPEEnable.read(); index < NUM_MAC; index ++){
          vsgWeightaddrOldenPEen[index] = PEDisable;
        }     
        break;

      case sOldoutputdisWrite0:
        for(uint8_t index = 0; index < piNumPEEnable.read(); index ++){
          vsgWeightaddrOldenPEen[index] = PEOldDataDis;
        }
        for(uint8_t index = piNumPEEnable.read(); index < NUM_MAC; index ++){
          vsgWeightaddrOldenPEen[index] = PEDisable;
        }
        break;

      case sOldoutputdisWrite1:
        for(uint8_t index = 0; index < piNumPEEnable.read(); index ++){
          vsgWeightaddrOldenPEen[index] = PEOldDataDis;
        }
        for(uint8_t index = piNumPEEnable.read(); index < NUM_MAC; index ++){
          vsgWeightaddrOldenPEen[index] = PEDisable;
        }
        break;

      case sOldoutput0Write1:      
        for(uint8_t index = 0; index < piNumPEEnable.read(); index ++){
          vsgWeightaddrOldenPEen[index] = PEOldDataEn;
        }
        for(uint8_t index = piNumPEEnable.read(); index < NUM_MAC; index ++){
          vsgWeightaddrOldenPEen[index] = PEDisable;
        }
        break;

      case sOldoutput1Write0:
        for(uint8_t index = 0; index < piNumPEEnable.read(); index ++){
          vsgWeightaddrOldenPEen[index] = PEOldDataEn;
        }
        for(uint8_t index = piNumPEEnable.read(); index < NUM_MAC; index ++){
          vsgWeightaddrOldenPEen[index] = PEDisable;
        }
        break;

      case sHalt: default:
        for(uint8_t index = 0; index < NUM_MAC; index ++)
          vsgWeightaddrOldenPEen[index] = PEDisable;
    }
    wait();
  }  
}