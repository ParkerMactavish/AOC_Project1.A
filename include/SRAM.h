#ifndef SRAM_H
#define SRAM_H

#include <fstream>
#include <string>
#include <sstream>
#include "ps_config.h"
#include "general_function.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"

// // Needed for the simple_target_socket
// #define SC_INCLUDE_DYNAMIC_PROCESSES
DECLARE_EXTENDED_PHASE(internalPhase);

using namespace std;
using namespace tlm;
using namespace tlm_utils;

struct SRAM : ::sc_core::sc_module{
  peq_with_cb_and_phase<SRAM> m_peq;
  uint32_t *u32Mem,
           u32BaseAddr,
           u32MemSize; 
  int n_trans;
  /* Socket Conneted to DMAC */
  simple_target_socket<SRAM> sktFromDMAC;

  virtual tlm_sync_enum nb_transport_fw(tlm_generic_payload& trans,
                                        tlm_phase& phase,
                                        sc_time& delay );
  void peq_cb(tlm_generic_payload& trans, const tlm_phase& phase);;
  void send_response(tlm::tlm_generic_payload& trans);
  tlm::tlm_sync_enum send_end_req(tlm::tlm_generic_payload& trans);

  typedef SRAM SC_CURRENT_USER_MODULE;
  SRAM(
    ::sc_core::sc_module_name,
    uint32_t u32BaseAddr,
    uint32_t u32MemSize
  )
  : sktFromDMAC("socket")
  , u32BaseAddr(u32BaseAddr)
  , u32MemSize(u32MemSize)
  , m_peq(this, &SRAM::peq_cb)
  , n_trans(0) {
    // Register callbacks for incoming interface method calls
    sktFromDMAC.register_nb_transport_fw(this, &SRAM::nb_transport_fw);
    u32Mem = new uint32_t[u32MemSize/4];
  }
};
#endif