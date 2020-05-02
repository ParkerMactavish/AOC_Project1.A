   #include "Controller.h"
   
void Controller::thread_process(){
    *pe_enable=0;
    while (1)
    {
        if(!data_available){
            //cout<<"con get data"<<endl;
            *pe_enable=0;
            if(enable.read()){
                sram_icur = !which.read() ? sram_ifirst : sram_isecond;
                read_from_sram();
                data_available = 1;
            }
        }
        else if(pe_write){
            //cout<<"pe get data"<<endl;
            write_to_pe();
            cal_and_write_addr();               //write addr
            update_status();                           //calculate data_available
            *pe_enable=1;
            pe_write =0;
        }
        else{
            //cout<<"con wait"<<endl;
            //wait for PE
        }
        wait( sc_time(CLK_CYCLE, SC_NS) );
    }
}

int Controller::read_from_sram(){
    for(int i=0; i<SIZE_TILE; i++)
        ibuffer[i] = sram_icur->mem[i];
    cout<<endl;
    for(int j=0; j<NUM_PE; j++)
        for(int i=0; i<SIZE_MAC; i++)
            wbuffer[j][i] = sram_weight->mem[i];
}

int Controller::write_to_pe(){
    for(int i=0; i<input_len; i++){
         pe_mem[i]=ibuffer[i];
         for(int j=0; j<NUM_PE; j++)
            pe_local[j][i]=wbuffer[j][i];
    }
}

int Controller::cal_and_write_addr(){


}

int  Controller::update_status(){



}