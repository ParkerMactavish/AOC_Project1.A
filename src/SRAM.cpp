#include "SRAM.h"

tlm::tlm_sync_enum SRAM::nb_transport_fw( tlm::tlm_generic_payload& trans,
                                          tlm::tlm_phase& phase, 
                                          sc_time& delay ){
    m_peq.notify(trans, phase, delay);
    return tlm::TLM_ACCEPTED;
}

void SRAM::peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase){
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
      if(phase == internalPhase){
        tlm::tlm_command tlmCmd = trans.get_command();
        uint32_t         u32Addr = trans.get_address() - u32BaseAddr;
        uint32_t*        pu32Data = reinterpret_cast<uint32_t*> (trans.get_data_ptr());
        uint32_t         u32Length = trans.get_data_length();
        if(tlmCmd == tlm::TLM_READ_COMMAND){
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

tlm::tlm_sync_enum SRAM::send_end_req(tlm::tlm_generic_payload& trans){
  tlm::tlm_sync_enum status;
  tlm::tlm_phase tlmBackwardPhase;
  sc_time delay = sc_time(0.8333*15, SC_NS);

  tlmBackwardPhase = tlm::END_REQ;
  status = sktFromDMAC->nb_transport_bw(trans, tlmBackwardPhase, delay);
  if(status == tlm::TLM_COMPLETED){
    trans.release();
    return status;
  }

  m_peq.notify(trans, internalPhase, delay);
  n_trans++;

  return status;
}

void SRAM::send_response(tlm::tlm_generic_payload& trans)
{
  tlm::tlm_sync_enum status;
  tlm::tlm_phase bw_phase;
  sc_time delay = SC_ZERO_TIME;

  //response_in_progress = true;
  bw_phase = tlm::BEGIN_RESP;
  status = sktFromDMAC->nb_transport_bw(trans, bw_phase, delay);
  if (status == tlm::TLM_UPDATED) {
      // The timing annotation must be honored
      m_peq.notify( trans, bw_phase, delay);
  } else if (status == tlm::TLM_COMPLETED) {
      // The initiator has terminated the transaction
      trans.release();
      n_trans--;
  }
}