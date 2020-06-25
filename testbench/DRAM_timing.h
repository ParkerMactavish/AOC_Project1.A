#ifndef _DRAM_TIMING_H
#define _DRAM_TIMING_H
#include "DRAM.h"
#include "general_function.h"
#include "mm.h"

SC_MODULE(DRAM_timing){
  DRAM UUT;

  tlm_utils::simple_initiator_socket<DRAM_timing> skiToDRAM;
  void threadProcess();
  void access_dram(uint32_t addr,uint32_t& data,bool write,int length);
  tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay);
  void peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase);

  tlm::tlm_generic_payload* request_in_progress;
  sc_event end_request_event;
  tlm_utils::peq_with_cb_and_phase<DRAM_timing> m_peq;
  int state;
  uint32_t data;
  mm   m_mm;
  int index = 0, index2;

  SC_CTOR(DRAM_timing)
  : UUT("DRAM0")
  , m_peq(this, &DRAM_timing::peq_cb)
  , state(0){
    skiToDRAM.register_nb_transport_bw(this, &DRAM_timing::nb_transport_bw);
    skiToDRAM(UUT.socket);
    SC_THREAD(threadProcess);
  }

};
#endif