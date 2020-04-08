#include "DMAC.h"

void DMAC::master_access(uint32_t addr,uint32_t & data,bool write, int length, CS select){
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
  trans->set_streaming_width( length ); // = data_length to indicate no streaming
  trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
  trans->set_dmi_allowed( false ); // Mandatory initial value
  trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value
  // cout<<"length"<<length<<endl<<"addr"<<addr<<endl;
  phase = tlm::BEGIN_REQ;

  // Timing annotation models processing time of initiator prior to call
  delay = sc_time(0, SC_PS);

  // Non-blocking transport call on the forward path
  tlm::tlm_sync_enum status;
  if(select == sram_cs)
    status = sram_socket->nb_transport_fw( *trans, phase, delay );
  else if(select == dram_cs)
    status = dram_socket->nb_transport_fw( *trans, phase, delay );
}

void DMAC::thread_process()
{
  while(1)
  {
    switch(currentState){
      case sIdle:
        if(start.read()){
          addr_src = paddr_src.read();
          addr_dst = paddr_dst.read();
          size = psize.read();
          d2s = pd2s.read();
          cout<<addr_src<<' '<<addr_dst<<' '<<size<<endl;
          if(pd2s.read()){
            master_access(paddr_src.read(), data, DRIVER_READ, DATA_LENGTH/8, dram_cs);
          }
          interrupt.write(0);
          currentState = sRead;
        }
        break;
      case sRead:
        if(d2s){
          master_access(addr_src+count, data, DRIVER_READ, DATA_LENGTH/8, dram_cs);
        }
        break;
      case sWrite:
        if(d2s){
          master_access(addr_dst+count, data, DRIVER_WRITE, DATA_LENGTH/8, sram_cs);
        }
        break;
      case sInt:
        interrupt.write(1);
        break;
    }
    wait( sc_time(CLK_CYCLE, SC_NS) );
  }
}

// TLM-2 backward non-blocking transport method

tlm::tlm_sync_enum DMAC::nb_transport_bw( tlm::tlm_generic_payload& trans,
                                            tlm::tlm_phase& phase, sc_time& delay )
{
  // The timing annotation must be honored
  m_peq.notify( trans, phase, delay );
  return tlm::TLM_ACCEPTED;
}

// Payload event queue callback to handle transactions from target
// Transaction could have arrived through return path or backward path

void DMAC::peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase)
{

  if (phase == tlm::END_REQ || (&trans == request_in_progress && phase == tlm::BEGIN_RESP))
  {
    // The end of the BEGIN_REQ phase
  }
  else if (phase == tlm::BEGIN_REQ || phase == tlm::END_RESP)
    SC_REPORT_FATAL("TLM-2", "Illegal transaction phase received by initiator");

  if (phase == tlm::BEGIN_RESP)
  {
    trans.release();
    
    tlm::tlm_phase fw_phase = tlm::END_RESP;
    sc_time delay = sc_time(0, SC_PS);
    if(currentState == sRead){
      currentState = sWrite;
      if(d2s) dram_socket->nb_transport_fw( trans, fw_phase, delay );
    }
    else if(currentState == sWrite){
      if(d2s) sram_socket->nb_transport_fw( trans, fw_phase, delay);
      if(count < size){
        count++;
        currentState = sRead;
      }
      else{
        currentState = sInt;
      }
    }
  }
}
