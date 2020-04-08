#include "SRAM.h"

// TLM-2 non-blocking transport method

tlm::tlm_sync_enum SRAM::nb_transport_fw( tlm::tlm_generic_payload& trans,
        tlm::tlm_phase& phase, sc_time& delay )
{
    m_peq.notify( trans, phase, delay);
    return tlm::TLM_ACCEPTED;
}

void SRAM::peq_cb(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase)
{
    tlm::tlm_sync_enum status;
    sc_time delay;
    delay = sc_time(0, SC_PS); // Accept delay
    switch (phase) {
        case tlm::BEGIN_REQ:
            trans.acquire();
            status = send_end_req(trans);
            if (status == tlm::TLM_COMPLETED) {// It is questionable whether this is valid
              break;
            }

            break;

        case tlm::END_RESP:
            // On receiving END_RESP, the target can release the transaction
            // and allow other pending transactions to proceed
            trans.release();
            // {unsigned char*   pptr = trans.get_data_ptr();
            // cout<<(int)*pptr<<(int)*(pptr+1)<<endl;}
            // Target itself is now clear to issue the next BEGIN_RESP
            // response_in_progress = false;
            // if (next_response_pending)
            // {
            //   send_response( *next_response_pending );
            //   next_response_pending = 0;
            // }

            // // ... and to unblock the initiator by issuing END_REQ
            // if (end_req_pending)
            // {
            //   status = send_end_req( *end_req_pending );
            //   end_req_pending = 0;
            // }
            
            break;

        case tlm::END_REQ:
        case tlm::BEGIN_RESP:
            SC_REPORT_FATAL("TLM-2", "Illegal transaction phase received by target");
            break;

        default:
            if (phase == internal_ph) {
                // Execute the read or write commands

                tlm::tlm_command cmd = trans.get_command();
                uint32_t    adr = trans.get_address();

                unsigned char*   ptr = trans.get_data_ptr();
                unsigned int     len = trans.get_data_length();
                unsigned char*   byt = trans.get_byte_enable_ptr();

                if ( cmd == tlm::TLM_READ_COMMAND ) {
                    memcpy(ptr, &mem[adr/4], len);
                } else if ( cmd == tlm::TLM_WRITE_COMMAND ) {
                    memcpy(&mem[adr/4], ptr, len);
                }
                cout<<"in sram"<<mem[adr/4]<<' '<<adr/4<<endl;
                trans.set_response_status( tlm::TLM_OK_RESPONSE );
                send_response(trans);
                break;
            }
    }
}

tlm::tlm_sync_enum SRAM::send_end_req(tlm::tlm_generic_payload& trans)
{
    tlm::tlm_sync_enum status;
    tlm::tlm_phase bw_phase;
    tlm::tlm_phase int_phase = internal_ph;
    tlm::tlm_command cmd = trans.get_command();
    int addr = trans.get_address();
    sc_time delay;

    // Queue the acceptance and the response with the appropriate latency
    bw_phase = tlm::END_REQ;
    delay = sc_time(0, SC_PS); // Accept delay

    status = socket->nb_transport_bw( trans, bw_phase, delay );
    if (status == tlm::TLM_COMPLETED) {
        // Transaction aborted by the initiator
        // (TLM_UPDATED cannot occur at this point in the base protocol, so need not be checked)
        trans.release();
        return status;
    }

    // // Queue internal event to mark beginning of response
    m_peq.notify( trans, int_phase, delay );
    return status;
}

void SRAM::send_response(tlm::tlm_generic_payload& trans)
{
    tlm::tlm_sync_enum status;
    tlm::tlm_phase bw_phase;
    sc_time delay;

    //response_in_progress = true;
    bw_phase = tlm::BEGIN_RESP;
    delay = SC_ZERO_TIME;
    status = socket->nb_transport_bw( trans, bw_phase, delay );
    if (status == tlm::TLM_UPDATED) {
        // The timing annotation must be honored
        m_peq.notify( trans, bw_phase, delay);
    } else if (status == tlm::TLM_COMPLETED) {
        // The initiator has terminated the transaction
        trans.release();
    }
}