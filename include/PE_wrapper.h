#include "systemc.h"
#include<vector>
#include "PE.h"
#ifndef PE_WRAPPER_H
#define PE_WRAPPER_H

SC_MODULE(PEwrapper){
    
    PE* pe_vec[NUM_PE];
    bool* pe_write;                             //is PE available
    bool pe_enable;                    


void work_PEwrapper(){
    *pe_write =1;
    while(1){
        if(pe_enable){
            pe_enable=0;
            *pe_write =0;
            cout<<"pe work:"<<endl;
            for(int i=0 ; i<NUM_PE ; i++){
                pe_vec[i]->work_pe();
            }
            cout<<endl;
            *pe_write =1;
        }
        wait(sc_time(CLK_CYCLE, SC_NS));
    }
}

/*uint32_t* get_pe_local(int pe_label){
    return &pe_vec[pe_label].pe_local;
}

uint32_t* get_pe_mem(int pe_label){
    return &pe_vec[pe_label].pe_mem;
}*/
   
   SC_CTOR(PEwrapper)
   {
       for(int i=0; i<NUM_PE; i++)
            pe_vec[i] = new PE("pe");
       //prp_ready.write(1);
        SC_THREAD(work_PEwrapper);
   }
};
#endif