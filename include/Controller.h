#include "systemc.h"
#include "PE.h"
#include "DMAC.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"
#include "ps_config.h"
#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#define CONTROLLER_REG_SIZE 13
#define INPUT_DATA_OFFSET   vu32ControllReg[0]
#define OUTPUT_DATA_OFFSET  vu32ControllReg[1]
#define WEIGHT_DATA_OFFSET  vu32ControllReg[2]
#define BIAS_DATA_OFFSET    vu32ControllReg[3]
#define INPUT_DATA_WIDTH    vu32ControllReg[4]
#define INPUT_DATA_HEIGHT   vu32ControllReg[5]
#define INPUT_DATA_CHANNEL  vu32ControllReg[6]
#define FILTER_WIDTH        vu32ControllReg[7]
#define FILTER_HEIGHT       vu32ControllReg[8]
#define OUTPUT_DATA_CHANNEL vu32ControllReg[9]
#define ACTIVATION_FUNCTION vu32ControllReg[10]
#define CONTROLLER_START    vu32ControllReg[11]
#define CONTROLLER_CLEAR   vu32ControllReg[12]

/**
 * @Group: DMAC Controler Registers Indexs
 * @{
 */
#define srcAddrIndex            0
#define dstAddrIndex            1
#define widthIndex              2
#define heightIndex             3
#define channelNumIndex         4
#define secondaryStrideSrcIndex 5
#define secondaryStrideDstIndex 6
#define primaryStrideSrcIndex   7
#define primaryStrideDstIndex   8
#define transposeOutputIndex    9
#define dmacStartIndex          10
#define dmacClearIndex          11
/**
 * @}
 */

using namespace tlm_utils;
using namespace tlm;
using namespace sc_core;

SC_MODULE(Controller){
  /* State Machine Declaration */
  enum State{
    sIdle, sMovingBias, sSettingBias, sMovingWeight, sSettingWeight, sPrepareForGenerating, sMovingInput, sGeneratingOutput, sMovingOutput, sInt
  };
  /* Memory Access Status */
  enum accessStatus{
    BUSY = -1,
    SUCCESS = 0,
  };
  /**
   * Group: Internal Registers
   * @{
   */
  /* Input Registers */
  int32_t i32InputReg[9];
  /* Boundary Registers */
  uint32_t u32InputWidthBoundaryL,
           u32InputWidthBoundaryR,
           u32OutputChannelBoundaryF,
           u32OutputChannelBoundaryE,
  /* Indexing Registers */
           u32InputWidthIndex,
           u32InputHeightIndex,
           u32OutputWidthIndex,
           u32OutputHeightIndex,
           u32InputChannelIndex,
          //  u32InputChannelIndexDMAC,
           u32OutputChannelIndex,
          //  u32OutputChannelIndexDMAC,
           u32WeightSettingIndex,
  /* Slice Width Register */
           u32SliceWidth,
  /* DMAC Control Registers */
           u32AddrForDMAC,
           u32DataForDMAC[DMAC_REG_SIZE],
  /* DMAC Clear Signal */
           cu32ClearDMACAddr = 0x2c + DMAC_BASE,
           cu32ClearDMACData = 1,
  /* Input SRAM Control Registers */
           u32InputSRAMAddr[3],
  /* Output SRAM Control Registers */
           *pu32OutputSRAMData,
           *pu32OldSRAMData;
  /* Ping-pong Input/Output SRAM Select Register */
  // bool     bPingpongInputSRAM,
  //          bPingpongOutputSRAM,
  //          bPingpongInputSRAMDMAC,
  //          bPingpongOutputSRAMDMAC;
  /**
   * @}
   */
  // ProcessState currentControllerState;
  // DMACState currentDMACState;
  State currentState;
  /* Connection between Controller and DMAC */
  sc_in<bool> piInterruptFromDMAC;
  
  /* PE instances */
  PE* vPE[NUM_PE];

  /* Connection to PEs */
  sc_signal<sc_int<64>> vsgWeightOlddata[NUM_PE];
  sc_signal<sc_int<64>> vsgOutputData[NUM_PE];
  sc_signal<sc_int<32>> vsgInputData[NUM_PE][NUM_MAC];
  sc_signal<sc_uint<4>> vsgWeightaddrOldenPEen[NUM_PE];
  sc_signal<sc_uint<2>> vsgActivationFunction[NUM_PE];
  sc_signal<bool>       vsgPEClk[NUM_PE];

  /* Connection between Controller and DMAC, SRAM */
  typedef simple_initiator_socket_tagged<Controller> tInitiatorSocket;
  tInitiatorSocket* vskiToComponents;
  tlm_sync_enum nb_transport_bw(int dummy,
                                tlm_generic_payload& trans,
                                tlm_phase& phase,
                                sc_time& delay);
  
  bool bComponentAvailable[NUM_SRAM + 1], bOutputDataReq, bNewOutputSRAM, bOldOutputSRAM;

  /* Connection between Controller and CPU */
  simple_target_socket<Controller> sktFromCPU;
  tlm_sync_enum nb_transport_fw(tlm_generic_payload& trans,
                                tlm_phase& phase,
                                sc_time& delay);
  mm m_mm;
  tlm_generic_payload* tlmTmpTrans;
  peq_with_cb_and_phase<Controller> m_peq;
  void peq_cb(tlm_generic_payload& trans, const tlm_phase& phase);
  sc_out<bool> poInterruptToCPU;
  bool bCommandFromCPU;
  uint32_t u32BaseAddr;
  uint32_t u32AddrFromCPU, u32DataFromCPU;

  /* Addressable Registers for CPU Control */
  uint32_t vu32ControllReg[CONTROLLER_REG_SIZE];

  void run_Thread();
  accessStatus access_DMAC(uint32_t u32Addr, uint32_t& u32Data);
  accessStatus access_InputSRAM(uint32_t u32Addr, uint32_t* pu32Data, int iLength);
  accessStatus access_OutputSRAM(uint32_t u32Addr, uint32_t* pu32Data, int iLength, bool bRW);
  uint32_t generate_DMACData(uint32_t u32DMACOffset);
  void update_Data();
  void set_DMACMoveWeight();
  void set_DMACMoveInput();
  void set_DMACMoveOutput();
  uint32_t get_PortId(uint32_t u32Addr);

  typedef Controller SC_CURRENT_USER_MODULE;
  Controller(
    ::sc_core::sc_module_name,
    uint32_t u32BaseAddr
  )
  : sktFromCPU("FromCPU")
  , m_peq(this, &Controller::peq_cb)
  , currentState(sIdle)
  , poInterruptToCPU(0)
  , bCommandFromCPU(0)
  , u32BaseAddr(u32BaseAddr){
    for(int index0 = 0; index0 < NUM_PE; index0 ++){
      vPE[index0] = new PE("PE");
      vPE[index0]->piWeightOlddata(vsgWeightOlddata[index0]);
      vPE[index0]->piWeightaddrOldenPEen(vsgWeightaddrOldenPEen[index0]);
      vPE[index0]->piActivationFunction(vsgActivationFunction[index0]);
      vPE[index0]->poOutputData(vsgOutputData[index0]);
      vPE[index0]->piClk(vsgPEClk[index0]);
      for(int index1 = 0; index1 < NUM_MAC; index1 ++){
        vPE[index0]->vpiInputData[index1](vsgInputData[index0][index1]);
        vsgInputData[index0][index1] = 0;
      }
      vsgWeightOlddata[index0] = 0;
      vsgWeightaddrOldenPEen[index0] = 0;
      vsgActivationFunction[index0] = 0;
      vsgPEClk[index0] = 0;
    }
    vskiToComponents = new tInitiatorSocket[NUM_SRAM + 1];
    for(int index = 0; index < NUM_SRAM + 1; index ++){
      vskiToComponents[index].register_nb_transport_bw(this, &Controller::nb_transport_bw, index);
      bComponentAvailable[index] = 1;
    }
    tlmTmpTrans = new tlm_generic_payload;

    sktFromCPU.register_nb_transport_fw(this, &Controller::nb_transport_fw);

    pu32OutputSRAMData = new uint32_t[NUM_PE*2];
    pu32OldSRAMData = new uint32_t[NUM_PE*2];
    SC_THREAD(run_Thread);
    
  }
};

#endif