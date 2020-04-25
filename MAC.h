#include "systemc.h"
#include "ps_config.h"
#define DEBUG
#ifndef MAC_H
#define MAC_H

SC_MODULE(MAC)
{
    sc_in <bool> enable;
    sc_in<uint32_t> weight,input ;

    uint32_t result_mac;

    void multi( )
    {
        result_mac = weight.read()*input.read();
        #ifdef DEBUG
            cout<<weight.read()<<" x "<<input.read()<<"="<<result_mac<<endl;
        #endif
    }

    SC_CTOR(MAC) {

    }
};

#endif