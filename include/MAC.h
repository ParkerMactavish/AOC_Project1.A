#include "systemc.h"
#include "ps_config.h"
#define DEBUG
#ifndef MAC_H
#define MAC_H

SC_MODULE(MAC)
{
    sc_in<sc_uint<8> > piWeight, piInput;
    sc_out<sc_uint<8> > poResult;

    void multi( );
    SC_CTOR(MAC) {
        SC_METHOD(multi);
        sensitive<<piWeight<<piInput;
    }
};

#endif