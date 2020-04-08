#ifndef SRAM_H
#define SRAM_H

#include <fstream>
#include <string>
#include <sstream>
#include "ps_config.h"
#include "general_function.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

DECLARE_EXTENDED_PHASE(internal_sram_ph);
using namespace std;

struct SRAM: sc_module {
    // TLM-2 socket, defaults to 32-bits wide, base protocol
    tlm_utils::simple_target_socket<SRAM> socket;

    SC_CTOR(SRAM)
        : socket("socket")
        , m_peq(this, &SRAM::peq_cb)
    {
        // Register callbacks for incoming interface method calls
        socket.register_nb_transport_fw(this, &SRAM::nb_transport_fw);
        for(int i=0; i<(MEM_SIZE/4); i++){
            mem[i] = 0;
        }
    }

    // TLM-2 non-blocking transport method

    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans,
            tlm::tlm_phase& phase, sc_time& delay );
    void peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase);
    tlm::tlm_sync_enum send_end_req(tlm::tlm_generic_payload& trans);
    void send_response(tlm::tlm_generic_payload& trans);
    tlm_utils::peq_with_cb_and_phase<SRAM> m_peq;
    uint32_t mem[MEM_SIZE/4]; // 256MB(8B * 0x2000000)
};

#endif
