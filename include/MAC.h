#include "systemc.h"
#include "ps_config.h"
#ifndef MAC_H
#define MAC_H

SC_MODULE(MAC)
{
    sc_in<sc_int<32>> piWeight, piInput;
    sc_out<sc_int<64>> poResult;

    void multi(){
      this->poResult = this->piWeight.read() * this->piInput.read();
    }
    SC_CTOR(MAC) {
        SC_METHOD(multi);
        sensitive<<piWeight<<piInput;
    }
};

#endif