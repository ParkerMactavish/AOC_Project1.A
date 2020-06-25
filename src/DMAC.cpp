#include "DMAC.h"

DMAC::accessStatus DMAC::access_Mem(uint32_t u32Addr, uint32_t& u32Data, bool bWrite, int iLength){
  if(!bMemAvailable[get_PortId(u32Addr)])
    return BUSY;
  tlm::tlm_generic_payload* tlmTrans;
  tlm::tlm_phase tlmPhase;
  tlm::tlm_command tlmCmd;
  sc_time delay = SC_ZERO_TIME;
  if(bWrite==1){
    tlmCmd = tlm::TLM_WRITE_COMMAND; 
  }
  else{
    tlmCmd = tlm::TLM_READ_COMMAND; 
  }
  // Grab a new transaction from the memory manager
  tlmTrans = m_mm.allocate();
  tlmTrans->acquire();
  // Set all attributes except byte_enable_length and extensions (unused)
  tlmTrans->set_command(tlmCmd);
  tlmTrans->set_address(u32Addr);
  tlmTrans->set_data_ptr(reinterpret_cast<unsigned char*>(&u32Data));
  tlmTrans->set_data_length(iLength); //data length is 
  tlmTrans->set_streaming_width(iLength); // = data_length to indicate no streaming
  tlmTrans->set_byte_enable_ptr(0); // 0 indicates unused
  tlmTrans->set_dmi_allowed(false); // Mandatory initial value
  tlmTrans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE); // Mandatory initial value
  tlmPhase = tlm::BEGIN_REQ;

  tlm::tlm_sync_enum status;
  status = vskiToMem[get_PortId(u32Addr)]->nb_transport_fw( *tlmTrans, tlmPhase, delay );
  if(status == tlm::TLM_ACCEPTED){  
    bMemAvailable[get_PortId(u32Addr)] = 0;
    return SUCCESS;
  }
}

void DMAC::run_thread(){
  poInterrupt = 0;
  DMAC_START = 0;
  while(1)
  {
    wait(sc_time(DMAC_CLK_CYCLE, SC_NS));
    accessStatus retStatusRead, retStatusWrite;
    tlm::tlm_phase tlmBackwardPhase = tlm::BEGIN_RESP;
    sc_time tlmDelay = SC_ZERO_TIME;
    switch(currentState){
      case sIdle:
        if(bCommandFromController){
          vu32ControllReg[u32AddressFromController/4] = u32DataFromController;
          sktFromController->nb_transport_bw(*tlmTmpTrans, tlmBackwardPhase, tlmDelay);
          bCommandFromController = 0;
          currentState = sIdle;
          cout<<"DMAC"<<vu32ControllReg[u32AddressFromController/4]<<endl;
        }
        if(DMAC_START){
          bIsDstNHWC = TRANSPOSE_OUTPUT & 1;
          bIsSrcNHWC = TRANSPOSE_OUTPUT & 2;
          if(access_Mem(SRC_ADDR, u32ReadData, DRIVER_READ, DATA_LENGTH/8) == SUCCESS){
            currentState = sTrans;
            u32SrcWidthOffset = 1;
            u32SrcChannelOffset = 0;
            u32SrcHeightOffset = 0;
            u32DstWidthOffset = 0;
            u32DstHeightOffset = 0;
            u32DstChannelOffset = 0;
          }
        }
        break;

      case sTrans:
        if(u32DstHeightOffset == (HEIGHT - 1) &&
           u32DstWidthOffset == (WIDTH - 1) &&
           u32DstChannelOffset == (CHANNEL_NUM - 1) &&
           bMemAvailable[get_PortId(get_Address(FROM_SRC))] &&
           access_Mem(get_Address(FROM_DST), u32ReadData, DRIVER_WRITE, DATA_LENGTH/8) == SUCCESS){
          currentState = sWaitMemWrite;
        }
        else if(bMemAvailable[get_PortId(get_Address(FROM_SRC))] &&
                bMemAvailable[get_PortId(get_Address(FROM_DST))]){
          // cout<<(int32_t)u32ReadData<<' '<<get_Address(FROM_DST)<<' '<<get_Address(FROM_SRC)<<endl;
          u32WriteData = u32ReadData;
          retStatusRead = access_Mem(get_Address(FROM_SRC), u32ReadData, DRIVER_READ, DATA_LENGTH/8);
          retStatusWrite = access_Mem(get_Address(FROM_DST), u32WriteData, DRIVER_WRITE, DATA_LENGTH/8);
          if(retStatusRead == SUCCESS && retStatusWrite == SUCCESS){
            calculate_NextAddress(u32DstWidthOffset, u32DstHeightOffset, u32DstChannelOffset, bIsDstNHWC);
            calculate_NextAddress(u32SrcWidthOffset, u32SrcHeightOffset, u32SrcChannelOffset, bIsSrcNHWC);
          }
        }
        break;
      
      case sWaitMemWrite:
        if(bMemAvailable[get_PortId(get_Address(FROM_DST))] &&
           bMemAvailable[get_PortId(get_Address(FROM_SRC))]){
          poInterrupt = 1;
          currentState = sInt;
        }
          
      case sInt:
        if(bCommandFromController && u32AddressFromController == 0x2c && u32DataFromController == 1){
          sktFromController->nb_transport_bw(*tlmTmpTrans, tlmBackwardPhase, tlmDelay);
          bCommandFromController = 0;
          currentState = sIdle;
          DMAC_START = 0;
          poInterrupt = 0;
        }
        break;
    }
  }
}

tlm::tlm_sync_enum DMAC::nb_transport_bw( int dummy,
                                          tlm::tlm_generic_payload& trans,
                                          tlm::tlm_phase& phase, 
                                          sc_time& delay ){
  // The timing annotation must be honored
  m_peq.notify(trans, phase, delay);
  return tlm::TLM_ACCEPTED;
}

// Payload event queue callback to handle transactions from target
// Transaction could have arrived through return path or backward path
void DMAC::peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase){
  tlm::tlm_sync_enum status;
  sc_time delay = SC_ZERO_TIME;
  int iPortId;
  tlm::tlm_phase tlmForwardPhase;
  tlm::tlm_phase tlmBackwardPhase;
  switch(phase){
    case tlm::BEGIN_REQ:
      tlmBackwardPhase = tlm::END_REQ;
      status = sktFromController->nb_transport_bw(trans, tlmBackwardPhase, delay);
      bCommandFromController = 1;
      u32DataFromController = *(reinterpret_cast<uint32_t*>(trans.get_data_ptr()));
      u32AddressFromController = trans.get_address() - u32BaseAddr;
      tlmTmpTrans = m_mm.allocate();
      tlmTmpTrans->deep_copy_from(trans);
      tlmTmpTrans->acquire();
      tlmTmpTrans->acquire();
      if(status == tlm::TLM_COMPLETED){
        trans.release();
        break;
      }
      break;
    
    case tlm::END_RESP:
      trans.release();
      break;
    
    case tlm::END_REQ:
      break;

    case tlm::BEGIN_RESP:
      iPortId = get_PortId(trans.get_address());
      trans.release();
      tlmForwardPhase = tlm::END_RESP;
      bMemAvailable[iPortId] = 1;
      vskiToMem[iPortId]->nb_transport_fw(trans, tlmForwardPhase, delay);
      break;
  }
}

tlm::tlm_sync_enum DMAC::nb_transport_fw(tlm::tlm_generic_payload& trans,
                                         tlm::tlm_phase& phase,
                                         sc_time& delay){
  m_peq.notify(trans, phase, delay);
  return tlm::TLM_ACCEPTED;
}

int DMAC::get_PortId(uint32_t u32Addr){
  if(u32Addr < DRAM4_BASE + DRAM4_SIZE){
    return 0;
  }
  else if(u32Addr < OUTPUT_SRAM0_BASE){
    return 1;
  }
  else if(u32Addr < DMAC_BASE){
    return 2;
  }
  else{
    assert(false);
  }
}

uint32_t DMAC::get_Address(bool is_Src){
  uint32_t u32TmpAddr;
  if(is_Src){
    if(bIsSrcNHWC){
      u32TmpAddr = SRC_ADDR +
                   4 * (SECONDARY_STRIDE_SRC * u32SrcWidthOffset +
                        PRIMARY_STRIDE_SRC * u32SrcHeightOffset + 
                        u32SrcChannelOffset);
    }
    else{
      u32TmpAddr = SRC_ADDR +
                   4 * (SECONDARY_STRIDE_SRC * u32SrcHeightOffset +
                        PRIMARY_STRIDE_SRC * u32SrcChannelOffset +
                        u32SrcWidthOffset);
    }
  }
  else{
    if(bIsDstNHWC){
      u32TmpAddr = DST_ADDR +
                   4 * (SECONDARY_STRIDE_DST * u32DstWidthOffset +
                        PRIMARY_STRIDE_DST * u32DstHeightOffset +
                        u32DstChannelOffset);
    }
    else{
      u32TmpAddr = DST_ADDR +
                   4 * (SECONDARY_STRIDE_DST * u32DstHeightOffset +
                        PRIMARY_STRIDE_DST * u32DstChannelOffset +
                        u32DstWidthOffset);
    }
  }
  return u32TmpAddr;
}

void DMAC::calculate_NextAddress(uint32_t& u32WidthOffset, uint32_t& u32HeightOffset, uint32_t& u32ChannelOffset, bool is_NHWC){
  if(u32WidthOffset == WIDTH - 1 && u32HeightOffset == HEIGHT - 1){
    u32WidthOffset = 0;
    u32HeightOffset = 0;
    u32ChannelOffset ++;
  }
  else if(u32WidthOffset == WIDTH - 1){
    u32HeightOffset ++;
    u32WidthOffset = 0;
  }
  else{
    u32WidthOffset ++;
  }
}