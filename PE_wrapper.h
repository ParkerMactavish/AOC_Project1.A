#include "systemc.h"
#include "PE.h"
#ifndef PE_WRAPPER_H
#define PE_WRAPPER_H

SC_MODULE(PEwrapper){
    sc_in<bool> prp_enable;
    sc_out<bool> prp_finish;
    
    sc_vector <PE> pe_vec;


void work_PEwrapper(){
     for(int i=0 ; i<NUM_PE ; i++){
        pe_vec[i].work_pe();
    }
    prp_finish.write(true);
    wait();
}

/*uint32_t* get_pe_local(int pe_label){
    return &pe_vec[pe_label].pe_local;
}

uint32_t* get_pe_mem(int pe_label){
    return &pe_vec[pe_label].pe_mem;
}*/
   
   SC_CTOR(PEwrapper)
   :pe_vec("pe_vec",NUM_PE)
   {
        SC_THREAD(work_PEwrapper);
        sensitive << prp_enable;
   }
};
#endif