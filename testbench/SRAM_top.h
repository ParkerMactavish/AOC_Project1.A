#ifndef _SRAM_TOP_H
#define _SRAM_TOP_H

#include "ps_config.h"
#include "general_function.h"
#include "mm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"

SC_MODULE(SRAM_top){
  // TLM-2 socket, defaults to 32-bits wide, base protocol
  tlm_utils::simple_initiator_socket<SRAM_top> socket0, socket1;
  tlm_utils::peq_with_cb_and_phase<SRAM_top> m_peq;

  mm m_mm;
  uint32_t  *pu32Data_q, *pu32Data_d, testIndex, testPhase;
  bool is_SRAMAvailable;
  void thread_process();
  void master_access(uint32_t addr, uint32_t* data, bool write, int length);

  // TLM-2 backward non-blocking transport method
  virtual tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans,
                                             tlm::tlm_phase& phase,
                                             sc_time& delay);

  void peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase);

  SC_CTOR(SRAM_top)
  : socket0("socket")  // Construct and name socket
  , socket1("socket1")
  , m_peq(this, &SRAM_top::peq_cb)
  , testIndex(0)
  , testPhase(0)
  , is_SRAMAvailable(1){
    // Register callbacks for incoming interface method calls
    socket0.register_nb_transport_bw(this, &SRAM_top::nb_transport_bw);
    socket1.register_nb_transport_bw(this, &SRAM_top::nb_transport_bw);
    pu32Data_q = new uint32_t[2];
    pu32Data_d = new uint32_t[2];
    SC_THREAD(thread_process);
  }

};
#endif