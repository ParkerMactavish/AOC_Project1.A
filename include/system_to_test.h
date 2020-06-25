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

#ifndef _SYSTEM_TO_TEST_H
#define _SYSTEM_TO_TEST_H

SC_MODULE(SYSTEM_TO_TEST){
  ISRAM ISRAM0;
  OSRAM OSRAM0;
  DRAM DRAM0;
  DMAC DMAC0;
  Controller CTRL0;

  sc_signal<bool> sgInterruptFromDMACToController;
  
  SC_CTOR(SYSTEM_TO_TEST)
  : ISRAM0("ISRAM0", INPUT_SRAM0_BASE, (INPUT_SRAM_SIZE << 1))
  , OSRAM0("OSRAM0", OUTPUT_SRAM0_BASE, (OUTPUT_SRAM_SIZE << 1) + WEIGHT_SRAM_SIZE)
  , DRAM0("DRAM")
  , DMAC0("DMAC", DMAC_BASE)
  , CTRL0("CTRL0", CONTROLLER_BASE){
    /* DMAC Connections */
    DMAC0.vskiToMem[0](DRAM0.socket);
    DMAC0.vskiToMem[1](ISRAM0.sktFromDMAC);
    DMAC0.vskiToMem[2](OSRAM0.sktFromDMAC);
    DMAC0.poInterrupt(sgInterruptFromDMACToController);

    /* Controller Connections */
    CTRL0.vskiToComponents[0](ISRAM0.sktFromController);
    CTRL0.vskiToComponents[1](OSRAM0.sktFromController);
    CTRL0.vskiToComponents[2](DMAC0.sktFromController);
    CTRL0.piInterruptFromDMAC(sgInterruptFromDMACToController);
  }
};

#endif