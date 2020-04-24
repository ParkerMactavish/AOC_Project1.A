#include "systemc.h"
#include "MAC.h"
#define DEBUG
#ifndef PE_H
#define PE_H

SC_MODULE(PE)
{
    sc_out <uint32_t>sum;
    sc_out<uint32_t> weight_pe[NUM_MAC];
    sc_out<uint32_t> input_pe[NUM_MAC];
    //sc_out<bool> pe_finish;

    uint32_t pe_local[NUM_MAC];//different kernel for different pe
    static uint32_t pe_mem[NUM_MAC];//same input for all pe

    //MAC mac_arr[NUM_MAC];
    sc_vector <MAC> mac_vec;

    void work_pe()
    {
        for(int i=0 ; i<NUM_MAC ; i++)
        {
            weight_pe[i].write(pe_local[i]);
            input_pe[i].write(pe_mem[i]);
        }
        for(int i=0 ; i<NUM_MAC ; i++)
        {
            mac_vec[i].multi();
        }
        acc();
        wait();
    }
    void acc(){
            sum.write(0);
            for(int i=0 ; i<NUM_MAC;i++){
                sum.write(sum.read()+mac_vec[i].result_mac);
            }
    }


    SC_CTOR(PE)
    : mac_vec("mac_vec",NUM_MAC)
    {
        for(int i=0 ; i<NUM_MAC ; i++)
        {
            mac_vec[i].weight(weight_pe[i]);
            mac_vec[i].input(input_pe[i]);
            //mac1.weight(weight_pe[i]);
            //mac1.input(input_pe[i]);
        }
    }
};
#endif