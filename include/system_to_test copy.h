#include "systemc.h"
#include "ps_config.h"
#include "Controller.h"
#include "DRAM.h"
#include "INPUT_SRAM.h"
#include "OUTPUT_SRAM.h"
#include "DMAC.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm.h"

#ifndef _SYSTEM_H
#define _SYSTEM_H

SC_MODULE(SYSTEM_TO_TEST){
  DMAC* DMAC0;
  DRAM* DRAM0; 
  ISRAM* ISRAM0;
  OSRAM* OSRAM0;
  // Controller* Controller0;
  
  simple_initiator_socket_tagged<SYSTEM_TO_TEST>* vskiDummy;
  tlm_sync_enum nb_transport_bw(int dummy,
                                tlm_generic_payload& trans,
                                tlm_phase& phase,
                                sc_time& delay){
                                  return TLM_ACCEPTED;
                                }
  // simple_target_socket<SYSTEM_TO_TEST> sktDummy;
  // tlm_sync_enum nb_transport_fw(tlm_generic_payload& trans,
  //                               tlm_phase& phase,
  //                               sc_time& delay){
  //                                 return TLM_ACCEPTED;
  //                               }


  SC_CTOR(SYSTEM_TO_TEST){
    DMAC0 = new DMAC("DMAC0", DMAC_BASE);
    DRAM0 = new DRAM("DRAM0");
    ISRAM0 = new ISRAM("ISRAM0", INPUT_SRAM0_BASE, INPUT_SRAM_SIZE << 1);
    OSRAM0 = new OSRAM("OSRAM0", OUTPUT_SRAM0_BASE, (OUTPUT_SRAM_SIZE << 1) + WEIGHT_SRAM_SIZE);
    // Controller0 = new Controller("Controller0", CONTROLLER_BASE);

    sc_signal<bool> sgDMAC_Controller_Interrupt;
    /* DMAC and Mems */
    DMAC0->vskiToMem[0](DRAM0->socket);
    DMAC0->vskiToMem[1](ISRAM0->sktFromDMAC);
    DMAC0->vskiToMem[2](OSRAM0->sktFromDMAC);
    DMAC0->poInterrupt(sgDMAC_Controller_Interrupt);


    /* Controller and Components */
    // Controller0->vskiToComponents[0](ISRAM0->sktFromController);
    // Controller0->vskiToComponents[1](OSRAM0->sktFromController);
    // Controller0->vskiToComponents[2](DMAC0->sktFromController);
    // Controller0->piInterruptFromDMAC(sgDMAC_Controller_Interrupt);

    vskiDummy = new simple_initiator_socket_tagged<SYSTEM_TO_TEST>[3];
    for(int index = 0; index < NUM_SRAM + 1; index ++){
      vskiDummy[index].register_nb_transport_bw(this, &SYSTEM_TO_TEST::nb_transport_bw, index);
    }
    // sktDummy.register_nb_transport_fw(this, &SYSTEM_TO_TEST::nb_transport_fw);
    vskiDummy[0](ISRAM0->sktFromController);
    vskiDummy[1](OSRAM0->sktFromController);
    vskiDummy[2](DMAC0->sktFromController);
  }
};

#endif