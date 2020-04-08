#ifndef DMAC_H
#define DMAC_H

#include "top.h"
#include "ps_config.h"
#include "general_function.h"
#include "mm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"

enum state{
  sIdle, sRead, sWrite, sInt,
};

struct DMAC: sc_module
{
  // TLM-2 socket, defaults to 32-bits wide, base protocol
  tlm_utils::simple_initiator_socket<DMAC> dram_socket, sram_socket;

  SC_CTOR(DMAC)
  : dram_socket("dram_socket")  // Construct and name socket
  , sram_socket("sram_socket")
  , request_in_progress(0)
  , m_peq(this, &DMAC::peq_cb)
  , currentState(sIdle)
  {
    // Register callbacks for incoming interface method calls
    dram_socket.register_nb_transport_bw(this, &DMAC::nb_transport_bw);
    sram_socket.register_nb_transport_bw(this, &DMAC::nb_transport_bw);
    SC_THREAD(thread_process);
  }

  void thread_process();
  void master_access(uint32_t addr,uint32_t & data,bool write, int length, CS select);
  // TLM-2 backward non-blocking transport method
  virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase, sc_time& delay );
  void peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase);

  mm   m_mm;
  uint32_t  addr_src, addr_dst, size, data, count;
  bool d2s;
  state currentState;

  tlm::tlm_generic_payload* request_in_progress;
  sc_event end_request_event;
  tlm_utils::peq_with_cb_and_phase<DMAC> m_peq;

  sc_in<sc_uint<ADDR_LENGTH> > paddr_src, paddr_dst, psize;
  sc_in<bool> pd2s;
  sc_in<sc_uint<1> > start;
  sc_out<bool> interrupt;
};
#endif