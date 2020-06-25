#include "systemc.h"
#include "ps_config.h"
#include "mm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"
#ifndef _CPU_TB_H
#define _CPU_TB_H

using namespace std;
using namespace sc_core;
using namespace tlm;
using namespace tlm_utils;

SC_MODULE(CPU){
  simple_initiator_socket<CPU> skiToController;
  peq_with_cb_and_phase<CPU> m_peq;
  mm m_mm;
  sc_in<bool> piInterruptFromController;

  uint32_t DataForController[13], AddrForController;
  bool bControllerAvailable;

  tlm_sync_enum nb_transport_bw(tlm_generic_payload& trans,
                                tlm_phase& phase,
                                sc_time& delay){
    m_peq.notify(trans, phase,delay);
    return TLM_ACCEPTED;
  }

  void peq_cb(tlm_generic_payload& trans, const tlm_phase& phase){
    sc_time delay = SC_ZERO_TIME;
    switch(phase){
      case BEGIN_REQ:
      case END_RESP:
        assert(false);
      break;
      case END_REQ:
        break;
      case BEGIN_RESP:
        trans.release();
        tlm_phase fw_phase = END_RESP;
        bControllerAvailable = true;
        skiToController->nb_transport_fw(trans, fw_phase, delay);
    }
  }

  void run_Thread(){
    while(1){
      wait(sc_time(10, SC_NS));
      if(AddrForController < 12 && bControllerAvailable){
        tlm_generic_payload* trans = m_mm.allocate();
        tlm_phase phase = BEGIN_REQ;
        tlm_command cmd = TLM_WRITE_COMMAND;
        sc_time delay = SC_ZERO_TIME;
        trans->acquire();
        trans->set_command(cmd);
        trans->set_address(AddrForController * 4 + CONTROLLER_BASE);
        trans->set_data_ptr(reinterpret_cast<unsigned char*>(DataForController + AddrForController));
        trans->set_data_length(4);
        trans->set_streaming_width(4);
        trans->set_response_status(TLM_INCOMPLETE_RESPONSE);

        tlm_sync_enum status = skiToController->nb_transport_fw(*trans, phase, delay);
        bControllerAvailable = false;

        AddrForController ++;
      }
      else if(piInterruptFromController.read()){
        AddrForController = 0;
        sc_stop();
      }
    }
  }

  SC_CTOR(CPU)
  : skiToController("ToController")
  , m_peq(this, &CPU::peq_cb)
  , AddrForController(0)
  , bControllerAvailable(1){
    skiToController.register_nb_transport_bw(this, &CPU::nb_transport_bw);
    SC_THREAD(run_Thread);
  }
};

#endif