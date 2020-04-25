#include "systemc.h"
#include "MAC.h"
#define DEBUG
#ifndef PE_H
#define PE_H

SC_MODULE(PE)
{
    //sc_out <uint32_t>sum;
    // sc_out<uint32_t> weight_pe[NUM_MAC];
    // sc_out<uint32_t> input_pe[NUM_MAC];
    //sc_out<bool> pe_finish;

    uint32_t pe_local[SIZE_MAC];//different kernel for different pe
    static uint32_t pe_mem[SIZE_MAC];//same input for all pe
    static int input_len;           //how  many input data available

    //MAC mac_arr[NUM_MAC];
    //sc_vector <MAC> mac_vec;
    uint32_t macs[SIZE_MAC];

    void work_pe()
    {
        // for(int i=0 ; i<NUM_MAC ; i++)
        // {
        //     weight_pe[i].write(pe_local[i]);
        //     input_pe[i].write(pe_mem[i]);
        // }
        // for(int i=0 ; i<NUM_MAC ; i++)
        // {
        //     mac_vec[i].multi();
        // }
        acc();
        //wait();
    }
    void acc(){
            // sum.write(0);
            // for(int i=0 ; i<NUM_MAC;i++){
            //     sum.write(sum.read()+mac_vec[i].result_mac);
            // }
            int temp = 0;
            for(int i=0; i<PE::input_len;  i++){
                temp+=PE::pe_mem[i]*pe_local[i];
                cout<<PE::pe_mem[i]<<"*"<<pe_local[i]<<endl;
            }
            cout<<temp;
    }


    SC_CTOR(PE)
    //: mac_vec("mac_vec",NUM_MAC)
    {
        // for(int i=0 ; i<NUM_MAC ; i++)
        // {
        //     mac_vec[i].weight(weight_pe[i]);
        //     mac_vec[i].input(input_pe[i]);
        //     //mac1.weight(weight_pe[i]);
        //     //mac1.input(input_pe[i]);
        // }
    }
};
#endif