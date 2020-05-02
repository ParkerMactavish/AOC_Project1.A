#include "ps_config.h"
#include "top.h"
#include "SRAM.h"
#include "DRAM.h"

struct Status
{

};


struct Controller: sc_module {
    sc_in<bool> enable;		                        //from DMAC
    sc_in<bool> which;                              //from DMAC: 0 for sram1, 1 for sram2
    //sc_out<uint32_t> addr[PESIZE] ;   //output address for each PEs
    //sc_out<bool> finish; 		                    //to MAC?CPU? (when whole layer done)

    SC_CTOR(Controller)
    {
        // for(int i=0; i<NUM_PE; i++)
        //      wbuffer [i]= new uint32_t[SIZE_MAC];
        data_available=0;
        //pe_enable.write(0);
        //finish.write(0);
        SC_THREAD(thread_process);
    }

    void thread_process();
    int read_from_sram(); 	            //get input tile and weight from sram
    int write_to_pe();                        //move input data and weight to PE
    int cal_and_write_addr();		//calculate and write the output address for each PE
    int  update_status();

    uint32_t ibuffer[SIZE_TILE];      //store input tile
    uint32_t wbuffer[NUM_PE][SIZE_MAC];  	       //store weight of every PE
    uint32_t* pe_mem;		                 //point to PE static memory
    uint32_t* pe_local[NUM_PE];	      //point to each PE's local memory
    Status status;		                               //self-defined struct for status record
    SRAM* sram_ifirst;
    SRAM* sram_isecond;
    SRAM* sram_weight;
    SRAM* sram_icur;
    int kernel_size;
    int input_len;                               //how many input data available
    bool data_available;
    bool pe_write;                             //is PE available
    bool* pe_enable;   
};