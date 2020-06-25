#include "SRAM_top.h"

void SRAM_top::master_access(uint32_t u32Addr,uint32_t* pu32Data,bool write,int length)
{
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
  status = socket0->nb_transport_fw( *trans, phase, delay );
}


void SRAM_top::thread_process()
{
  while(1)
  {
    cout<<is_SRAMAvailable<<' ';
    if(is_SRAMAvailable){
      if(!testPhase){
        pu32Data_d[0] = testIndex;
        pu32Data_d[1] = testIndex + 2;
        master_access(INPUT_SRAM0_BASE+testIndex,pu32Data_d, 1, 8);//write  length=1
      }
      else{
        master_access(INPUT_SRAM0_BASE+testIndex,pu32Data_q, 0, 4);//read  length=1
      }
      testIndex += 8;
      is_SRAMAvailable = 0;  
    }
    if(!testPhase && testIndex > INPUT_SRAM_SIZE * 2){
      testPhase ++;
      testIndex = 0;
    }
    else if(testPhase && testIndex > INPUT_SRAM_SIZE * 2){
      sc_stop();
    }
    cout<<"CLK "<<sc_time_stamp()<<' '<<testIndex<<' ';
    if(testPhase){
      cout<<"R"<<pu32Data_q[0]<<' '<<pu32Data_q[1];
    }
    else{
      cout<<"W"<<pu32Data_d[0]<<' '<<pu32Data_d[1];
    }
    cout<<endl;
    wait( sc_time(CLK_CYCLE, SC_NS) );
  }
}

// TLM-2 backward non-blocking transport method

tlm::tlm_sync_enum SRAM_top::nb_transport_bw( tlm::tlm_generic_payload& trans,
                                            tlm::tlm_phase& phase, sc_time& delay )
{
  // The timing annotation must be honored
  m_peq.notify( trans, phase, delay );
  return tlm::TLM_ACCEPTED;
}

// Payload event queue callback to handle transactions from target
// Transaction could have arrived through return path or backward path

void SRAM_top::peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase)
{
  if (phase == tlm::END_REQ)
  {
    //cout<<"WRRR"<<phase<<*u32Data_q<<endl;
    // The end of the BEGIN_REQ phase
  }
  else if (phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
    SC_REPORT_FATAL("TLM-2", "Illegal transaction phase received by initiator");

  if (phase == tlm::BEGIN_RESP)
  {
    trans.release();
    tlm::tlm_phase fw_phase = tlm::END_RESP;
    is_SRAMAvailable = 1;
    sc_time delay = sc_time(0, SC_PS);
    socket0->nb_transport_fw( trans, fw_phase, delay );
  }
}

