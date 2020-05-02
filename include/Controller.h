#ifndef _CONTROLLER_H
#define _CONTROLLER_H

SC_MODULE(Controller){
  /* Internal Registers */
  uint8_t u8PipelineReg[12];
  uint8_t u8PipelineRegHead;
  uint8_t u8

  /* Global Clock Input */
  sc_in_clk piClk;

  /* Connection between Controller and DMAC */
  sc_in<bool> piInterrupt;
  sc_out<sc_uint<8>> poClear;
  tlm_utils::simple_initiator_socket<Controller> skiToDMAC;
  
  /* Connection between Controller and PE */
  sc_out<sc_uint<8>> poPEInput[9][2];
  sc_out<sc_uint<2>> poPEWeightOutputSel[32];

  /* Connection between Controller and InputSRAM1/InputSRAM2 */
  tlm_utils::simple_initiator_socket<Controller> skiToSRAM[2];
  
  /* Connection between Controller and OutputSRAM */
  sc_out<sc_uint<32>> poOutputSRAMAddr;

  tlm_utils::simple_targe_socket<Controller> sktFromCPU;
  uint32_t u32InputStartAddr, u32OutputStartAddr, u32InputHeight, u32InputWidth, u32InputChannel,
           u32KernelNum, u32KernelWidth, u32KernelHight, u32IsZeroPadding;

  mm m_mm;
  
  tlm::tlm_generic_payload* request_in_progress;
  tlm_utils::peq_with_cb_and_phase<Controller> m_peq;

  void thread_Process();
  void access_DMAC(uint32_t src, uint32_t dst, uint32_t size);
  void access_InputSRAM();
  void update_InputData();

  SC_CTOR(Controller)
  : skiToDMAC("ToDMAC")
  , skiToSRAM[0]("ToInputSRAM1")
  , skiToSRAM[1]("ToInputSRAM2")
  , sktFromCPU("FromCPU")
  , m_peq(this, &Controller::peq_cb)
  , currentState(sIdle){
    poClear=0;
    for(int index = 0; index < 9; index ++){
      poPEInput[index][0] = 0;
      poPEInput[index][1] = 0;
    }
    for(int index = 0; index < 32; index ++){
      poPEWeightOutputSel[index] = 0;
    }
    poOutputSRAMAddr = 0;
    poOutputSRAMClk = 0;
    skInputSRAMControll[0].register_nb_trasport_bw(this, &Controller::nb_transport_bw);
    skInputSRAMControll[1].register_nb_trasport_bw(this, &Controller::nb_transport_bw);
    skDMACCommand.register_nb_transport_bw(this, &Controller::nb_transport_bw);

    SC_THREAD(thread_process);
  }

};

#endif