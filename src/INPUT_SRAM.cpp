#include "INPUT_SRAM.h"

tlm::tlm_sync_enum ISRAM::nb_transport_fw(tlm::tlm_generic_payload& trans,
                                          tlm::tlm_phase& phase, 
                                          sc_time& delay ){
    m_peq.notify(trans, phase, delay);
    return tlm::TLM_ACCEPTED;
}

void ISRAM::peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase){
  tlm::tlm_sync_enum status;
  switch (phase) {
    case tlm::BEGIN_REQ:
      trans.acquire();
      status = send_end_req(trans);
      if(status == tlm::TLM_COMPLETED)
        break;
      break;

    case tlm::END_RESP:
      trans.release();
      break;

    case tlm::END_REQ:
    case tlm::BEGIN_RESP:
      SC_REPORT_FATAL("TLM-2", "Illegal transaction phase received by target");
      break;
    default:
      if(phase == ISRAM_InternalPhase){
        tlm::tlm_command tlmCmd = trans.get_command();
        uint32_t         u32Addr = trans.get_address() - u32BaseAddr;
        uint32_t*        pu32Data = reinterpret_cast<uint32_t*> (trans.get_data_ptr());
        uint32_t         u32Length = trans.get_data_length();
        if(trans.get_address() == 0xffffffff && tlmCmd == tlm::TLM_READ_COMMAND){
          pu32Data[0] = u32Mem[(pu32Data[0] - u32BaseAddr)>>2];
          pu32Data[1] = u32Mem[(pu32Data[1] - u32BaseAddr)>>2];
          pu32Data[2] = u32Mem[(pu32Data[2] - u32BaseAddr)>>2];
        }
        else if(trans.get_address() == 0xffffffff && tlmCmd == tlm::TLM_WRITE_COMMAND){
          assert(false);
        }
        else if(tlmCmd == tlm::TLM_READ_COMMAND){
          memcpy(pu32Data, u32Mem + u32Addr/4, u32Length);
        }
        else{
          memcpy(u32Mem + u32Addr/4, pu32Data, u32Length);
        }
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
      }
      send_response(trans);
      break;
  }
}

tlm::tlm_sync_enum ISRAM::send_end_req(tlm::tlm_generic_payload& trans){
  tlm::tlm_sync_enum status;
  tlm::tlm_phase tlmBackwardPhase;
  sc_time delay = sc_time(0.8333*15, SC_NS);

  tlmBackwardPhase = tlm::END_REQ;
  if(trans.get_address() == 0xffffffff){
    status = sktFromController->nb_transport_bw(trans, tlmBackwardPhase, delay);
  }
  else{
    status = sktFromDMAC->nb_transport_bw(trans, tlmBackwardPhase, delay);
  }
  if(status == tlm::TLM_COMPLETED){
    trans.release();
    return status;
  }

  m_peq.notify(trans, ISRAM_InternalPhase, delay);

  return status;
}

void ISRAM::send_response(tlm::tlm_generic_payload& trans)
{
  tlm::tlm_sync_enum status;
  tlm::tlm_phase tlmBackwardPhase;
  sc_time delay = SC_ZERO_TIME;

  //response_in_progress = true;
  tlmBackwardPhase = tlm::BEGIN_RESP;
  if(trans.get_address() == 0xffffffff){
    status = sktFromController->nb_transport_bw(trans, tlmBackwardPhase, delay);
  }
  else{
    status = sktFromDMAC->nb_transport_bw(trans, tlmBackwardPhase, delay);
  }
  if (status == tlm::TLM_UPDATED) {
      // The timing annotation must be honored
      m_peq.notify( trans, tlmBackwardPhase, delay);
  } 
  else if (status == tlm::TLM_COMPLETED) {
      // The initiator has terminated the transaction
      trans.release();
  }
}