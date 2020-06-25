#include "Controller.h"

void Controller::peq_cb(tlm_generic_payload& trans, const tlm_phase& phase){
  tlm_sync_enum status;
  sc_time delay = SC_ZERO_TIME;
  int iPortId;
  tlm_phase tlmForwardPhase;
  tlm_phase tlmBackwardPhase;
  switch(phase){
    case BEGIN_REQ:
      tlmBackwardPhase = END_REQ;
      status = sktFromCPU->nb_transport_bw(trans, tlmBackwardPhase, delay);
      bCommandFromCPU = 1;
      u32DataFromCPU = *(reinterpret_cast<uint32_t*>(trans.get_data_ptr()));
      u32AddrFromCPU = trans.get_address() - u32BaseAddr;
      tlmTmpTrans = m_mm.allocate();
      tlmTmpTrans->deep_copy_from(trans);
      tlmTmpTrans->acquire();
      tlmTmpTrans->acquire();
      if(status == TLM_COMPLETED){
        trans.release();
        break;
      }
      break;
    
    case END_RESP:
      trans.release();
      break;

    case END_REQ:
      break;
    
    case BEGIN_RESP:
      iPortId = get_PortId(trans.get_address());
      trans.release();
      tlmForwardPhase = END_RESP;
      bComponentAvailable[iPortId] = 1;
      vskiToComponents[iPortId]->nb_transport_fw(trans, tlmForwardPhase, delay);
  }
}

tlm_sync_enum Controller::nb_transport_fw(tlm_generic_payload& trans,
                                               tlm_phase& phase,
                                               sc_time& delay){
  m_peq.notify(trans, phase, delay);
  return TLM_ACCEPTED;
}

tlm_sync_enum Controller::nb_transport_bw(int dummy,
                                          tlm_generic_payload& trans,
                                          tlm_phase& phase,
                                          sc_time& delay){
  m_peq.notify(trans, phase, delay);
  return TLM_ACCEPTED;
}

Controller::accessStatus Controller::access_DMAC(uint32_t u32Addr, uint32_t& u32Data){
  if(!bComponentAvailable[2]){
    return BUSY;
  }
  tlm_generic_payload* tlmTrans;
  tlm_phase tlmPhase;
  tlm_command tlmCmd = TLM_WRITE_COMMAND;
  sc_time delay = SC_ZERO_TIME;

  tlmTrans = m_mm.allocate();
  tlmTrans->acquire();

  tlmTrans->set_command(tlmCmd);
  tlmTrans->set_address(u32Addr);
  tlmTrans->set_data_ptr(reinterpret_cast<unsigned char*>(&u32Data));
  tlmTrans->set_data_length(4);
  tlmTrans->set_streaming_width(4); // = data_length to indicate no streaming
  tlmTrans->set_byte_enable_ptr(0); // 0 indicates unused
  tlmTrans->set_dmi_allowed(false); // Mandatory initial value
  tlmTrans->set_response_status(TLM_INCOMPLETE_RESPONSE); // Mandatory initial value
  tlmPhase = BEGIN_REQ;

  tlm_sync_enum status;
  status = vskiToComponents[2]->nb_transport_fw( *tlmTrans, tlmPhase, delay );
  if(status == TLM_ACCEPTED){  
    bComponentAvailable[2] = 0;
    return SUCCESS;
  }
}

Controller::accessStatus Controller::access_InputSRAM(uint32_t u32Addr, uint32_t* pu32Data, int iLength){
  if(!bComponentAvailable[0]){
    return BUSY;
  }
  tlm_generic_payload* tlmTrans;
  tlm_phase tlmPhase = BEGIN_REQ;
  tlm_command tlmCmd = TLM_READ_COMMAND;
  sc_time delay = SC_ZERO_TIME;

  tlmTrans = m_mm.allocate();
  tlmTrans->acquire();

  tlmTrans->set_command(tlmCmd);
  tlmTrans->set_address(u32Addr);
  tlmTrans->set_data_ptr(reinterpret_cast<unsigned char*>(pu32Data));
  tlmTrans->set_data_length(iLength);
  tlmTrans->set_streaming_width(iLength); // = data_length to indicate no streaming
  tlmTrans->set_byte_enable_ptr(0); // 0 indicates unused
  tlmTrans->set_dmi_allowed(false); // Mandatory initial value
  tlmTrans->set_response_status(TLM_INCOMPLETE_RESPONSE); // Mandatory initial value

  tlm_sync_enum status;
  status = vskiToComponents[0]->nb_transport_fw( *tlmTrans, tlmPhase, delay );
  if(status == TLM_ACCEPTED){  
    bComponentAvailable[0] = 0;
    return SUCCESS;
  }
}

Controller::accessStatus Controller::access_OutputSRAM(uint32_t u32Addr, uint32_t* pu32Data, int iLength, bool bRW){
  if(!bComponentAvailable[1]){
    return BUSY;
  }
  tlm_generic_payload* tlmTrans;
  tlm_phase tlmPhase = BEGIN_REQ;
  tlm_command tlmCmd = (bRW == DRIVER_READ) ? TLM_READ_COMMAND:
                                              TLM_WRITE_COMMAND;
  sc_time delay = SC_ZERO_TIME;
  tlmTrans = m_mm.allocate();
  tlmTrans->acquire();
  tlmTrans->set_command(tlmCmd);
  tlmTrans->set_address(u32Addr);
  tlmTrans->set_data_ptr(reinterpret_cast<unsigned char*>(pu32Data));
  tlmTrans->set_data_length(iLength);
  tlmTrans->set_streaming_width(iLength); // = data_length to indicate no streaming
  tlmTrans->set_byte_enable_ptr(0); // 0 indicates unused
  tlmTrans->set_dmi_allowed(false); // Mandatory initial value
  tlmTrans->set_response_status(TLM_INCOMPLETE_RESPONSE); // Mandatory initial value

  tlm_sync_enum status;
  status = vskiToComponents[1]->nb_transport_fw( *tlmTrans, tlmPhase, delay );
  if(status == TLM_ACCEPTED){
    bComponentAvailable[1] = 0;
    return SUCCESS;
  }
}

uint32_t Controller::get_PortId(uint32_t u32Addr){
  if(u32Addr < OUTPUT_SRAM0_BASE){
    return 0;
  }
  else if(u32Addr < DMAC_BASE){
    return 1;
  }
  else if(u32Addr < CONTROLLER_BASE){
    return 2;
  }
  else if(u32Addr == 0xffffffff){
    return 0;
  }
  else{
    assert(false);
  }
}

// void Controller::set_DMACMoveBias(){

// }

void Controller::set_DMACMoveWeight(){
  currentDMACState = sMovingWeight;
  u32AddrForDMAC = 0;
  u32DataForDMAC[srcAddrIndex] = WEIGHT_DATA_OFFSET + 
                                 (u32InputChannelIndexDMAC * FILTER_HEIGHT * FILTER_WIDTH + 
                                 u32OutputChannelIndexDMAC * INPUT_DATA_CHANNEL * FILTER_HEIGHT * FILTER_WIDTH) * 4;
  u32DataForDMAC[dstAddrIndex] = WEIGHT_SRAM_BASE;
  u32DataForDMAC[widthIndex] = FILTER_WIDTH;
  u32DataForDMAC[heightIndex] = FILTER_HEIGHT;
  u32DataForDMAC[channelNumIndex] = u32OutputChannelBoundary;
  u32DataForDMAC[secondaryStrideSrcIndex] = FILTER_WIDTH;
  u32DataForDMAC[secondaryStrideDstIndex] = NUM_PE;
  u32DataForDMAC[primaryStrideSrcIndex] = FILTER_WIDTH * FILTER_HEIGHT * INPUT_DATA_CHANNEL;
  u32DataForDMAC[primaryStrideDstIndex] = FILTER_WIDTH * NUM_PE;
  u32DataForDMAC[transposeOutputIndex] = NCHW_NHWC;
  u32DataForDMAC[dmacStartIndex] = 1;
}

void Controller::set_DMACMoveInput(){
  currentDMACState = sMovingInput;
  u32AddrForDMAC = 0;
  u32DataForDMAC[srcAddrIndex] = INPUT_DATA_OFFSET;
  u32DataForDMAC[dstAddrIndex] = (bPingpongInputSRAMDMAC)?INPUT_SRAM1_BASE:INPUT_SRAM0_BASE;
  u32DataForDMAC[widthIndex] = u32InputWidthBoundary;
  u32DataForDMAC[heightIndex] = INPUT_DATA_HEIGHT ;
  u32DataForDMAC[channelNumIndex] = 1;
  u32DataForDMAC[secondaryStrideSrcIndex] = INPUT_DATA_WIDTH;
  u32DataForDMAC[secondaryStrideDstIndex] = u32InputWidthBoundary;
  u32DataForDMAC[primaryStrideSrcIndex] = INPUT_DATA_WIDTH * INPUT_DATA_HEIGHT;
  u32DataForDMAC[primaryStrideDstIndex] = u32InputWidthBoundary * INPUT_DATA_HEIGHT;
  u32DataForDMAC[transposeOutputIndex] = NCHW_NCHW;
  u32DataForDMAC[dmacStartIndex] = 1;
}

void Controller::set_DMACMoveOutput(){

}

void Controller::update_Data(){

}

void Controller::run_Thread(){
  poInterruptToCPU = 0;
  CONTROLLER_START = 0;
  while(1){
    wait(sc_time(CONTROLLER_CLK_CYCLE/2, SC_NS));
    tlm_phase tlmBackwardPhase = BEGIN_RESP;
    sc_time tlmDelay = SC_ZERO_TIME;
    /* Controller State Machine */
    switch(currentControllerState){
      case sPEIdle:
        if(bCommandFromCPU){
          vu32ControllReg[u32AddrFromCPU/4] = u32DataFromCPU;
          sktFromCPU->nb_transport_bw(*tlmTmpTrans, tlmBackwardPhase, tlmDelay);
          cout<<"CTRL"<<u32DataFromCPU<<endl;
          bCommandFromCPU = 0;
          currentDMACState = sDMACIdle;
        }
        if(CONTROLLER_START && currentDMACState == sDMACIdle){
          currentControllerState = sPEIdle;
          /* Setting u32SliceWidth */
          u32SliceWidth = INPUT_SRAM_SIZE / INPUT_DATA_CHANNEL / INPUT_DATA_HEIGHT;
          if(u32SliceWidth > INPUT_DATA_WIDTH)
            u32SliceWidth = INPUT_DATA_WIDTH;
          /* Setting Boundary */
          u32InputWidthBoundary = u32SliceWidth;
          u32OutputChannelBoundary = (OUTPUT_DATA_CHANNEL > NUM_PE) ? NUM_PE : OUTPUT_DATA_CHANNEL;
          /* Initing Index */
          u32InputWidthIndex = 0;
          u32InputHeightIndex = 0;
          u32InputChannelIndex = 0;
          u32InputChannelIndexDMAC = 0;
          u32OutputChannelIndex = 0;
          set_DMACMoveWeight();
        }
        if(piInterruptFromDMAC && 
           currentDMACState == sMovingWeight &&
           access_DMAC(cu32ClearDMACAddr, cu32ClearDMACData) == SUCCESS){
          currentControllerState = sSettingWeight;
          
          /* Init Weight Set Index */
          u32WeightSettingIndex = 0;

          bPingpongInputSRAM = false;
          bPingpongOutputSRAM = false;
          bPingpongInputSRAMDMAC = false;
          bPingpongOutputSRAMDMAC = false;
          bOutputDataReq = false;
          cout<<"SLICE"<<u32SliceWidth<<endl;
          set_DMACMoveInput();
        }
        break;
      
      case sSettingBias:

      case sSettingWeight:
        if(u32WeightSettingIndex < NUM_MAC && bOutputDataReq == false){
          access_OutputSRAM(WEIGHT_SRAM_BASE + u32WeightSettingIndex * NUM_PE * 4,
                            pu32OutputSRAMData,
                            NUM_PE * 4,
                            DRIVER_READ);
          bOutputDataReq = true;
        }
        if(bOutputDataReq && bComponentAvailable[1]){
          for(int i = 0; i < NUM_PE; i ++){
            vsgWeightOlddata[i].write((int32_t)pu32OutputSRAMData[i]);
            vsgWeightaddrOldenPEen[i] = u32WeightSettingIndex;
          }
          u32WeightSettingIndex++;
          bOutputDataReq = false;
        }
        if(u32WeightSettingIndex == NUM_MAC){
          u32WeightSettingIndex ++;
        }//Avoid Activate the PEs
        if(u32WeightSettingIndex > NUM_MAC && 
           piInterruptFromDMAC &&
           access_DMAC(cu32ClearDMACAddr, cu32ClearDMACData) == SUCCESS){
          u32InputChannelIndexDMAC++;
          set_DMACMoveWeight();
          currentControllerState = sGeneratingOutput;
          for(int i = 0; i < NUM_PE; i ++){
            vsgWeightaddrOldenPEen[i] = PEEnable;
            vsgWeightOlddata[i] = 0;
          }
          u32InputSRAMAddr[0] = INPUT_SRAM0_BASE + 4 * u32InputWidthBoundary * INPUT_DATA_HEIGHT * u32InputChannelIndex;
          u32InputSRAMAddr[1] = INPUT_SRAM0_BASE + 4 * u32InputWidthBoundary * (1 + INPUT_DATA_HEIGHT * u32InputChannelIndex);
          u32InputSRAMAddr[2] = INPUT_SRAM0_BASE + 4 * u32InputWidthBoundary * (2 + INPUT_DATA_HEIGHT * u32InputChannelIndex);
          u32InputWidthIndex = 0;
          u32InputHeightIndex = 2;
          u32OutputWidthIndex = 0;
          u32OutputHeightIndex = 0;          
          access_InputSRAM(0xffffffff, u32InputSRAMAddr, 12);
          bWaitingForDMAC = true;
          bWaitingForController = true;
          cout<<"Start Generating Output"<<endl;
        }
        break;
      
      case sGeneratingOutput:
        if(piInterruptFromDMAC.read()){
          if(bWaitingForDMAC && 
              access_DMAC(cu32ClearDMACAddr, cu32ClearDMACData) == SUCCESS){
            cout<<"MOVE next Input"<<endl;
            bWaitingForDMAC = false;
            bPingpongInputSRAMDMAC = !bPingpongInputSRAMDMAC;
            set_DMACMoveInput();
          }
          else{
            cout<<"Move Input Done"<<endl;
          }
        }
        if(bComponentAvailable[0] && bWaitingForController){
          switch(u32InputWidthIndex){
            case 0xffffffff:
              break;
            case 0:
              i32InputReg[0] = u32InputSRAMAddr[0];
              i32InputReg[3] = u32InputSRAMAddr[1];
              i32InputReg[6] = u32InputSRAMAddr[2];
              break;
            case 1:
              i32InputReg[1] = u32InputSRAMAddr[0];
              i32InputReg[4] = u32InputSRAMAddr[1];
              i32InputReg[7] = u32InputSRAMAddr[2];
              break;
            case 2:
              i32InputReg[2] = u32InputSRAMAddr[0];
              i32InputReg[5] = u32InputSRAMAddr[1];
              i32InputReg[8] = u32InputSRAMAddr[2];
              break;
            default:
            
              /* Shift Input Data */
              for(int i = 0; i < NUM_MAC - 1; i ++)
                i32InputReg[i] = i32InputReg[i + 1];
              
              /* Assign New Data on Edge */
              i32InputReg[2] = u32InputSRAMAddr[0];
              i32InputReg[5] = u32InputSRAMAddr[1];
              i32InputReg[8] = u32InputSRAMAddr[2];
          }
          if(u32InputWidthIndex > 2){
            
            for(int i = 0; i < NUM_PE; i ++){
              // for(int j = 0; j < NUM_MAC; j ++){
              //   cout<<(int32_t)vPE[i]->vsgMACWeight[j].read()<<' '<<(int32_t)vPE[i]->vsgMACInput[j].read()<<"  ";
              // }
              pu32OutputSRAMData[2*i] = ((vsgOutputData[i].read()&0xffffffff00000000)>>32);
              pu32OutputSRAMData[2*i + 1] = (vsgOutputData[i].read()&0xffffffff);
              //cout<<"RE"<<(int32_t)pu32OutputSRAMData[2*i+1]<<endl;
            }
            uint32_t tmpOutputAddr = OUTPUT_SRAM0_BASE + 
                                (u32OutputWidthIndex +
                                u32OutputHeightIndex * (u32InputWidthBoundary - FILTER_WIDTH + 1)) * NUM_PE * 8;
            cout<<' '<<hex<<(tmpOutputAddr-OUTPUT_SRAM0_BASE)<<endl<<endl;
            access_OutputSRAM(tmpOutputAddr,
                              pu32OutputSRAMData,
                              NUM_PE * 8,
                              DRIVER_WRITE);
            /* Assign new Output Indexs */
            if(u32OutputHeightIndex == INPUT_DATA_HEIGHT - FILTER_HEIGHT &&
                u32OutputWidthIndex == u32InputWidthBoundary - FILTER_WIDTH){
              cout<<"Here?"<<endl;
              bWaitingForController = false;
            }
            else if(u32OutputWidthIndex == (u32InputWidthBoundary - FILTER_WIDTH)){
              u32OutputHeightIndex ++;
              u32OutputWidthIndex = 0;
              cout<<u32OutputHeightIndex<<' '<<u32InputHeightIndex<<endl;
            }
            else{
              u32OutputWidthIndex ++;
            }
          }
          /* Calculate Next Input Addr */
          if(u32InputWidthIndex == u32InputWidthBoundary - 1 &&
              u32InputHeightIndex == INPUT_DATA_HEIGHT - 1){
            u32InputChannelIndex ++;
            u32InputWidthIndex = 0xffffffff;
          }
          else if(u32InputWidthIndex == u32InputWidthBoundary - 1){
            u32InputHeightIndex ++;
            u32InputWidthIndex = 0xffffffff;
          }
          else{
            u32InputWidthIndex ++;
          }
          cout<<u32InputWidthIndex<<' '<<u32InputChannelIndex<<endl;
          cout<<"WTF";
          /* Assign Read Input Values */
          for(int i = 0; i < NUM_MAC; i ++){
            for(int j = 0; j < NUM_PE; j ++){
              vsgInputData[j][i] = i32InputReg[i];
            }
          }
          /* Calculate New Address for Input Data */
          u32InputSRAMAddr[0] = INPUT_SRAM0_BASE + 
                              4 * (u32InputChannelIndex * u32InputWidthBoundary * INPUT_DATA_HEIGHT +
                              (u32InputHeightIndex - 2) * u32InputWidthBoundary +
                              u32InputWidthIndex);
          u32InputSRAMAddr[1] = INPUT_SRAM0_BASE + 
                                4 * (u32InputChannelIndex * u32InputWidthBoundary * INPUT_DATA_HEIGHT +
                                (u32InputHeightIndex - 1) * u32InputWidthBoundary +
                                u32InputWidthIndex);
          u32InputSRAMAddr[2] = INPUT_SRAM0_BASE + 
                                4 * (u32InputChannelIndex * u32InputWidthBoundary * INPUT_DATA_HEIGHT +
                                (u32InputHeightIndex) * u32InputWidthBoundary +
                                u32InputWidthIndex);
          if(bWaitingForController)
            access_InputSRAM(0xffffffff, u32InputSRAMAddr, 12);
          cout<<"Is it fine";
        }
        if((bWaitingForController == false) && (bWaitingForDMAC == false)){
          cout<<"REACH"<<endl;
          u32OutputHeightIndex = 0;
          u32OutputWidthIndex = 0;
          u32InputWidthIndex = 0;
          u32InputChannelIndex ++;
          currentControllerState = sSettingWeight;
          bPingpongOutputSRAM = !bPingpongOutputSRAM;
          cout<<"PPO"<<bPingpongOutputSRAM<<endl;
          if(u32InputChannelIndex == 3)
            poInterruptToCPU = 1;
        }
        break;
    }
    

    /* DMAC State Machine */
    switch(currentDMACState){
      case sMovingBias:
        if(u32AddrForDMAC < dmacClearIndex &&
           access_DMAC(u32AddrForDMAC * 4 + DMAC_BASE, u32DataForDMAC[u32AddrForDMAC]) == SUCCESS){
             u32AddrForDMAC ++;
        }
        break;
      case sMovingWeight:
        if(u32AddrForDMAC < dmacClearIndex &&
           access_DMAC(u32AddrForDMAC * 4 + DMAC_BASE, u32DataForDMAC[u32AddrForDMAC]) == SUCCESS){
          u32AddrForDMAC ++;
        }
        break;
      case sMovingInput:
        if(u32AddrForDMAC < dmacClearIndex &&
           access_DMAC(u32AddrForDMAC * 4 + DMAC_BASE, u32DataForDMAC[u32AddrForDMAC]) == SUCCESS){
          u32AddrForDMAC ++; 
        }
        break;
      case sMovingOutput:
        if(u32AddrForDMAC < dmacClearIndex &&
           access_DMAC(u32AddrForDMAC * 4 + DMAC_BASE, u32DataForDMAC[u32AddrForDMAC]) == SUCCESS){
          u32AddrForDMAC ++; 
        }
        break;
      case sDMACIdle:
      default:
        break; 
    }

    for(int index = 0; index < NUM_PE; index ++)
      vsgPEClk[index] = !vsgPEClk[index];

    wait(sc_time(CONTROLLER_CLK_CYCLE/2, SC_NS));
    for(int index = 0; index < NUM_PE; index ++)
      vsgPEClk[index] = !vsgPEClk[index];
  }  
}