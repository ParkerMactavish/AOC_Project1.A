#ifndef _DMAC_TOP_H
#define _DMAC_TOP_H
#include "DRAM.h"
#include "DMAC.h"
#include "SRAM.h"
#include "general_function.h"
#include "mm.h"
#include "ps_config.h"
#include <iostream>
#include <fstream>

SC_MODULE(DMAC_top){
  DMAC* UUT;
  DRAM* DRAM0;
  SRAM* SRAMs[4]; 
  tlm_utils::simple_initiator_socket<DMAC_top> skiToDMAC;
  void threadProcess(){
    for(int i = 0; i < 3; i ++){
      for(int j = 0; j < 416; j ++){
        for(int k = 0; k < 416; k++)
        DRAM0->mem[(i*416*416+j*416+k)] = (i<<16)+(j<<8)+k;
      }
    }
    for(int i = 0; i < 13; i ++){
      for(int j = 0; j < 13; j ++){
        for(int k = 0; k < 16; k ++){
          SRAMs[2]->u32Mem[i*13*16+j*16+k] = (i<<16)+(j<<8)+k;
        }
      }
    }
    while(1){
      if(is_DMACAvailable){
        if(testPhase == 0){
          access_DMAC(testIndex * 4 + DMAC_BASE, &testPattern[testIndex], 1, 4);
          testIndex ++;
        }
        is_DMACAvailable = 0;  
      }
      if(testPhase && pINT.read()){
        cout<<"Read Interrupt"<<endl;
        uint32_t tmpData = 1;
        access_DMAC(32, &tmpData, 1, 4);
        cout<<"SRAM:"<<endl;
        for(int i = 0; i < 52; i ++){
          for(int j = 0; j < 52; j ++){
            for(int k = 0; k < 3; k ++){
              cout<<hex<<SRAMs[0]->u32Mem[k+3*j+156*i]<<' ';
            }
            cout<<endl;
          }
          cout<<endl;
        }
        cout<<endl<<endl<<"DRAM:"<<endl;
        for(int i = 0; i < 16; i ++){
          for(int j = 0; j < 13; j ++){
            for(int k = 0; k < 13; k ++){
              cout<<hex<<DRAM0->mem[0x1fb000/4+i*416*416+j*416+k]<<' ';
            }
            cout<<endl;
          }
          cout<<endl;
        }
        sc_stop();
      }
      if(!testPhase && testIndex > 10){
        testPhase ++;
        testIndex = 0;
      }
      wait( sc_time(CLK_CYCLE, SC_NS) );
    }
  }
  void access_DMAC(uint32_t u32Addr,uint32_t* pu32Data,bool write,int length){
    tlm::tlm_generic_payload* trans;
    tlm::tlm_phase phase;
    tlm::tlm_command cmd ;
    sc_time delay = SC_ZERO_TIME;
    if(write==1) {
      cmd = tlm::TLM_WRITE_COMMAND; 
    }
    else{
      cmd = tlm::TLM_READ_COMMAND; 
    }
    // Grab a new transaction from the memory manager
    trans = m_mm.allocate();
    trans->acquire();

    // Set all attributes except byte_enable_length and extensions (unused)
    trans->set_command( cmd );
    trans->set_address( u32Addr );
    trans->set_data_ptr( reinterpret_cast<unsigned char*>(pu32Data) );
    trans->set_data_length( length ); //data length is 
    trans->set_streaming_width( length ); // = data_length to indicate no streaming
    trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
    trans->set_dmi_allowed( false ); // Mandatory initial value
    trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value
    // cout<<"length"<<length<<endl<<"addr"<<addr<<endl;
    phase = tlm::BEGIN_REQ;

    // Non-blocking transport call on the forward path
    tlm::tlm_sync_enum status;
    status = skiToDMAC->nb_transport_fw( *trans, phase, delay );
  }
  virtual tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay){
    m_peq.notify( trans, phase, delay );
    return tlm::TLM_ACCEPTED;
  }

  void peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase){
    if (phase == tlm::END_REQ)
    {
      //cout<<"WRRR"<<phase<<*u32Data_q<<endl;
      // The end of the BEGIN_REQ phase
    }
    else if (phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
      SC_REPORT_FATAL("TLM-2", "Illegal transaction phase received by initiator");

    if (phase == tlm::BEGIN_RESP)
    {
      // trans.release();
      tlm::tlm_phase fw_phase = tlm::END_RESP;
      is_DMACAvailable = 1;
      sc_time delay = sc_time(0, SC_PS);
      skiToDMAC->nb_transport_fw( trans, fw_phase, delay );
    }
  }

  tlm_utils::peq_with_cb_and_phase<DMAC_top> m_peq;
  int state;
  uint32_t testPattern[11];                            
  uint32_t testIndex, testPhase, is_DMACAvailable;
  mm   m_mm;
  sc_signal<bool> pINT;

  typedef DMAC_top SC_CURRENT_USER_MODULE;
  DMAC_top(
    ::sc_core::sc_module_name,
    char* fileName
  )
  : m_peq(this, &DMAC_top::peq_cb)
  , testIndex(0)
  , testPhase(0)
  , is_DMACAvailable(1){
    UUT = new DMAC("UUT", DMAC_BASE);
    DRAM0 = new DRAM("DRAM");
    SRAMs[0] = new SRAM("INPUT_SRAM0", INPUT_SRAM0_BASE, INPUT_SRAM_SIZE);
    SRAMs[1] = new SRAM("INPUT_SRAM1", INPUT_SRAM1_BASE, INPUT_SRAM_SIZE);
    SRAMs[2] = new SRAM("OUTPUT_SRAM0", OUTPUT_SRAM0_BASE, OUTPUT_SRAM_SIZE);
    SRAMs[3] = new SRAM("OUTPUT_SRAM1", OUTPUT_SRAM1_BASE, OUTPUT_SRAM_SIZE);
    skiToDMAC.register_nb_transport_bw(this, &DMAC_top::nb_transport_bw);
    skiToDMAC(UUT->sktFromController);
    UUT->poInterrupt(pINT);
    UUT->vskiToMem[0](DRAM0->socket);
    UUT->vskiToMem[1](SRAMs[0]->sktFromDMAC);
    UUT->vskiToMem[2](SRAMs[1]->sktFromDMAC);
    UUT->vskiToMem[3](SRAMs[2]->sktFromDMAC);
    UUT->vskiToMem[4](SRAMs[3]->sktFromDMAC);
    // fstream fs(fileName, fstream::in);
    // // for(int i = 0; i < 11; i ++){
    // //   fs>>testPattern[i];
    // //   cout<<testPattern[i]<<endl;
    // // }
    // cout<<fileName<<endl;
    // cout<<fs.gcount()<<endl;
    // char tmpChar;
    // while(fs.get(tmpChar))
    //   cout<<(int)tmpChar<<' ';
    for(int i = 0; i < 11; i ++){
      cin>>testPattern[i];
    }
    SC_THREAD(threadProcess);
  }

};
#endif