#ifndef DMAC_H
#define DMAC_H

#include "top.h"
#include "ps_config.h"
#include "general_function.h"
#include "mm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"

#define DMAC_REG_SIZE       12

#define SRC_ADDR                  vu32ControllReg[0]
#define DST_ADDR                  vu32ControllReg[1]
#define WIDTH                     vu32ControllReg[2]
#define HEIGHT                    vu32ControllReg[3]
#define CHANNEL_NUM               vu32ControllReg[4]
#define SECONDARY_STRIDE_SRC      vu32ControllReg[5]
#define SECONDARY_STRIDE_DST      vu32ControllReg[6]
#define PRIMARY_STRIDE_SRC        vu32ControllReg[7]
#define PRIMARY_STRIDE_DST        vu32ControllReg[8]
#define TRANSPOSE_OUTPUT          vu32ControllReg[9]
#define DMAC_START                vu32ControllReg[10]
#define DMAC_CLEAR                vu32ControllReg[11]

#define NCHW_NCHW 0
#define NCHW_NHWC 1
#define NHWC_NCHW 2

#define FROM_SRC  true
#define FROM_DST  false

struct DMAC: sc_module{
  enum state{
    sIdle, sTrans, sWaitMemWrite, sInt,
  };

  enum accessStatus{
    BUSY = -1,
    SUCCESS = 0,
  };

  void run_thread();
  accessStatus access_Mem(uint32_t u32Addr,uint32_t& u32Data,bool pWrite, int iLength);
  int get_PortId(uint32_t u32Addr);
  uint32_t get_Address(bool is_Src);
  void calculate_NextAddress(uint32_t& u32WidthOffset, uint32_t& u32HeightOffset, uint32_t& u32ChannelOffset, bool is_NHWC);
  // TLM-2 backward non-blocking transport method
  virtual tlm::tlm_sync_enum nb_transport_bw(int dummy,
                                             tlm::tlm_generic_payload& trans,
                                             tlm::tlm_phase& phase, 
                                             sc_time& delay);
  virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans,
                                             tlm::tlm_phase& phase,
                                             sc_time& delay);
  /* Payload Event Queue Callback */
  void peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase);
  
  tlm_utils::peq_with_cb_and_phase<DMAC> m_peq;
  mm   m_mm;
  tlm::tlm_generic_payload* tlmTmpTrans;

  /* Connection between DMAC and DRAM, SRAM */
  typedef tlm_utils::simple_initiator_socket_tagged<DMAC> tInitiatorSocket;
  tInitiatorSocket* vskiToMem;
  /* Connection between Controller and DMAC */
  tlm_utils::simple_target_socket<DMAC> sktFromController;

  /* Addressable Registers */
  uint32_t vu32ControllReg[DMAC_REG_SIZE];
  /* Controller Transaction Base Address, Address and Data */
  uint32_t u32BaseAddr;
  uint32_t u32AddressFromController,
           u32DataFromController;
  bool bCommandFromController;
  /* Internal State Reg */
  state currentState;
  uint32_t u32ReadData, 
           u32WriteData,
           u32DstWidthOffset,
           u32DstHeightOffset,
           u32DstChannelOffset,
           u32SrcWidthOffset,
           u32SrcHeightOffset,
           u32SrcChannelOffset;
  bool bIsSrcNHWC, bIsDstNHWC;
  bool bMemAvailable[NUM_SRAM + 1];
  /* Interrupt Output Port to Controller */
  sc_out<bool> poInterrupt;

  typedef DMAC SC_CURRENT_USER_MODULE;
  DMAC(
    ::sc_core::sc_module_name,
    uint32_t u32BaseAddr
  )
  : sktFromController("FromController")
  , m_peq(this, &DMAC::peq_cb)
  , currentState(sIdle)
  , bCommandFromController(0)
  , u32BaseAddr(u32BaseAddr){
    // Register callbacks for incoming interface method calls
    vskiToMem = new tInitiatorSocket[NUM_SRAM + 1];
    for(int index = 0; index < NUM_SRAM + 1; index ++){
      vskiToMem[index].register_nb_transport_bw(this, &DMAC::nb_transport_bw, index);
      bMemAvailable[index] = true;
    }
    sktFromController.register_nb_transport_fw(this, &DMAC::nb_transport_fw);
    SC_THREAD(run_thread);
  }
};
#endif