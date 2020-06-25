#include "DRAM_timing.h"

void DRAM_timing::access_dram(uint32_t addr,uint32_t& data,bool write,int length){
  tlm::tlm_generic_payload* trans;
  tlm::tlm_phase phase;
  tlm::tlm_command cmd ;
  sc_time delay;
  if(write==1)
  {
    cmd = tlm::TLM_WRITE_COMMAND; 
  }
  else
  {
    cmd = tlm::TLM_READ_COMMAND; 
  }
  // Grab a new transaction from the memory manager
  trans = m_mm.allocate();
  trans->acquire();

  // Set all attributes except byte_enable_length and extensions (unused)
  trans->set_command( cmd );
  trans->set_address( addr );
  trans->set_data_ptr( reinterpret_cast<unsigned char*>(&data) );
  trans->set_data_length( length ); //data length is 
  trans->set_streaming_width( length); // = data_length to indicate no streaming
  trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
  trans->set_dmi_allowed( false ); // Mandatory initial value
  trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value
  // cout<<"length"<<length<<endl<<"addr"<<addr<<endl;
  phase = tlm::BEGIN_REQ;

  // Timing annotation models processing time of initiator prior to call
  delay = sc_time(0, SC_PS);

  // Non-blocking transport call on the forward path
  tlm::tlm_sync_enum status;
  status = skiToDRAM->nb_transport_fw( *trans, phase, delay );
}

tlm::tlm_sync_enum DRAM_timing::nb_transport_bw(tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase, sc_time& delay){
  // The timing annotation must be honored
  m_peq.notify( trans, phase, delay );
  return tlm::TLM_ACCEPTED;
}

void DRAM_timing::peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase){
  if (phase == tlm::END_REQ || (&trans == request_in_progress && phase == tlm::BEGIN_RESP)){
    // The end of the BEGIN_REQ phase
  }
  else if (phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
    SC_REPORT_FATAL("TLM-2", "Illegal transaction phase received by initiator");

  if (phase == tlm::BEGIN_RESP){
    trans.release();
    tlm::tlm_phase fw_phase = tlm::END_RESP;
    sc_time delay = sc_time(0, SC_PS);
    skiToDRAM->nb_transport_fw( trans, fw_phase, delay );
    state = 0;
    // index ++;
  }
}

void DRAM_timing::threadProcess(){
  #ifdef CON
  sc_trace_file* tf = sc_create_vcd_trace_file("./vcd/DRAM_timing_consecutive");
  #elif DIS
  sc_trace_file* tf = sc_create_vcd_trace_file("./vcd/DRAM_timing_discrete");
  #endif
  sc_trace(tf, data, "Data");
  #ifdef CON
  for(; index < 52*52; index ++){
    UUT.mem[index] = index;
  }
  #elif DIS
  for(; index < 52; index ++){
    for(index2 = 0; index2 < 52; index2 ++){
      UUT.mem[index*416+index2] = index*52+index2;
    }
  }
  #endif
  index = 0;
  index2 = 0;
  while(1){
    #ifdef CON
    if(state == 0){
      if(index == 52*52) break;
      access_dram(index*4, data, 0, 4);
      index ++;
      state = 1;
    }
    cout<<index<<endl;
    wait(sc_time(2.5, SC_NS));
  
    #elif DIS
    if(state == 0){
      if(index == 52) break;
      access_dram((index*416+index2)*4, data, 0, 4);
      index2++;
      state = 1;
    }
    if(index2 == 52){
      index2 = 0;
      index ++;
    }
    wait(sc_time(2.5, SC_NS));
    #endif
  }
  
  cout<<index <<endl<<sc_time_stamp();
  sc_close_vcd_trace_file(tf);
  sc_stop();
}