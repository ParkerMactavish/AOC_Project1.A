//#include "top_module.h"

#ifdef pe
#include "PE_top.h"
#elif PEwrapper
#include "PEwrapper_top.h"
#elif DRAMt
#include "DRAM_timing.h"
#elif DATA_TEST
#include <fstream>
#include <iostream>
using namespace std;
#elif sram
#include "SRAM_top.h"
#include "DRAM.h"
#include "OUTPUT_SRAM.h"
#include "INPUT_SRAM.h"
#include "tlm_utils/simple_initiator_socket.h"
#elif dmac
#include "DMAC_top.h"
#else
#include <fstream>
#include <iostream>
#include "system_to_test.h"
#include "systemc.h"
#include "CPU_tb.h"
using namespace std;
using namespace tlm;
using namespace tlm_utils;
#endif

#define LAYER1

#ifdef DATA_TEST
SC_MODULE(wait_int){
  sc_in<bool> int_in;
  void trigger();
  SC_CTOR(wait_int){
    SC_THREAD(trigger); //<< SC_THREAD is a macro.
    sensitive << int_in;
  }
};

void wait_int::trigger(){ //<< Did you mean this instead of Thread class
  while(1){
    wait(); //Wait for mySignal
    if(int_in){
      sc_stop();
    }
  }
}
#endif

#ifdef pe
int sc_main(int argc, char* argv[]){
  PE_TOP* top = new PE_TOP("TOP");
  sc_start();
  return 0;
}
#elif PEwrapper
int sc_main(int argc, char* argv[]){
  PEwrapper_top* top = new PEwrapper_top("TOP");
  sc_start();
  return 0;
}
#elif DRAMt
int sc_main(int argc, char* argv[]){
  DRAM_timing* top = new DRAM_timing("TOP");
  sc_start();
  return 0;
}
#elif DATA_TEST
int main(){
  fstream fsinData = fstream("./data/DRAM_INPUT/Standard_NCHW/data.txt", ios::in), 
          fsinWeight = fstream("./data/DRAM_INPUT/Standard_NCHW/weight_3x3.txt", ios::in);
  int index = 0, tmpData;
  while(!fsinData.eof()){
    fsinData>>tmpData;
    index ++;
  }
  cout<<index<<endl;
  index = 0;
  while(fsinWeight){
    fsinWeight>>tmpData;
    index ++;
  }
  cout<<index<<endl;

  return 0;
}
#elif sram
int sc_main(int argc, char* argv[]){
  SRAM_top* top = new SRAM_top("top");
  // DRAM* UUT = new DRAM("UUT");
  ISRAM* OSRAM0 = new ISRAM("OSRAM0", INPUT_SRAM0_BASE, (INPUT_SRAM_SIZE << 1));

  top->socket0(OSRAM0->sktFromDMAC);
  top->socket1(OSRAM0->sktFromController);
  sc_start();
  for(int i = 0; i < INPUT_SRAM_SIZE/4; i ++){
    cout<<OSRAM0->u32Mem[i]<<endl;
  }
  return 0;
}
#elif dmac
int sc_main(int argc, char* argv[]){
  DMAC_top* top = new DMAC_top("top", "../data/DMAC_Test.txt");
  sc_start();
}
#else
int sc_main(int argc, char* argv[])
{
  SYSTEM_TO_TEST* UUT = new SYSTEM_TO_TEST("UUT");
  CPU* dummyCPU = new CPU("dummyCPU");
  sc_signal<bool> sgIntFromCtrlToCPU;
  dummyCPU->skiToController(UUT->CTRL0.sktFromCPU);
  dummyCPU->piInterruptFromController(sgIntFromCtrlToCPU);
  UUT->CTRL0.poInterruptToCPU(sgIntFromCtrlToCPU);


  #ifdef LAYER1
  /**
   * @ First Layer
   * @{
   */
  cout << "*******    First Layer     *******"<<endl;
  ifstream fs_in0("./data/DRAM_INPUT/model1/layer1/layer1_input.txt");
  int tmpInt;
  cout<<"Reading Input Stage"<<endl;
  for(int i = 0; i < 3; i ++){
    for(int j = 0; j < 416; j ++){
      for(int k = 0; k < 416; k ++){
        fs_in0 >> tmpInt;
        UUT->DRAM0.mem[i*416*416+j*416+k] = tmpInt;
      }
    }
  }
  fs_in0.close();
  ifstream fs_w0("./data/DRAM_INPUT/model1/layer1/layer1_weight.txt");
  cout<<"Reading Weight Stage"<<endl;
  for(int i = 0; i < 16; i ++){
    for(int j = 0; j < 3; j ++){
      for(int k = 0; k < 9; k ++){
        fs_w0 >> tmpInt;
        UUT->DRAM0.mem[519168+i*9*3+j*9+k] = tmpInt;
      }
    }
  }
  fs_w0.close();
  dummyCPU->DataForController[0] = 0;
  dummyCPU->DataForController[1] = 519600*4;
  dummyCPU->DataForController[2] = 519168*4;
  dummyCPU->DataForController[3] = 0xffffffff;
  dummyCPU->DataForController[4] = 416;
  dummyCPU->DataForController[5] = 416;
  dummyCPU->DataForController[6] = 3;
  dummyCPU->DataForController[7] = 3;
  dummyCPU->DataForController[8] = 3;
  dummyCPU->DataForController[9] = 16;
  dummyCPU->DataForController[10] = 0;
  dummyCPU->DataForController[11] = 1;
  sc_start();
  for(int i = 0; i < 9; i ++){
    for(int j = 0; j < 8; j ++){
      cout<<(int32_t)UUT->OSRAM0.u32Mem[i*8+j+((WEIGHT_SRAM_BASE-OUTPUT_SRAM0_BASE)>>2)]<<' ';
    }   
    cout<<endl;
  }
  
  bool flag = true;
  if(flag) cout<<"INPUTS1 Correct"<<endl;
  for(int i = 0; i < NUM_PE; i ++){
    for(int j = 0; j < NUM_MAC; j ++){
      cout<<UUT->CTRL0.vPE[i]->vsgMACWeight[j]<<' ';
    }
    cout<<endl;
  }
  flag = true;
  fstream fs_golden("./data/DRAM_INPUT/model1/layer1/layer1_output.txt");
  int32_t i32Tmp;
  for(int i = 0; i < 16; i ++){
    for(int j = 0; j < 414; j ++){
      for(int k = 0; k < 414; k ++){
        fs_golden>>i32Tmp;
        if((int32_t)UUT->DRAM0.mem[519600+i*414*414+j*414+k] != i32Tmp){
          cout<<"ERROR"<<i<<", "<<j<<", "<<k<<", GOLDEN: "<<i32Tmp<<", WRONG:"<<(int32_t)UUT->DRAM0.mem[519312+i*414*414+j*414+k]<<endl;
          flag = false;
        }
        cout<<(int32_t)UUT->DRAM0.mem[519312+i*414*414+j*414+k]<<' ';
      }
      // cout<<endl;
    }
    // cout<<endl;
  }
  fs_golden.close();
  if(flag) cout<<"***   Layer1 all right    ***"<<endl;
  /**
   * @}
   * @ End First Layer
   */
  #endif

  #ifdef LAYER2
  /**
   * @ Second Layer
   * @{
   */
  cout << "*******    Second Layer     *******"<<endl;
  ifstream fs_in0("./data/DRAM_INPUT/model1/layer2/layer2_input.txt");
  cout<<"Reading Input Stage"<<endl;
  int tmpInt;
  for(int i = 0; i < 32; i ++){
    for(int j = 0; j < 104; j ++){
      for(int k = 0; k < 104; k ++){
        fs_in0 >> tmpInt;
        UUT->DRAM0.mem[i*104*104+j*104+k] = tmpInt;
      }
    }
  }
  fs_in0.close();
  ifstream fs_w0("./data/DRAM_INPUT/model1/layer2/layer2_weight.txt");
  cout<<"Reading Weight Stage"<<endl;
  for(int i = 0; i < 64; i ++){
    for(int j = 0; j < 32; j ++){
      for(int k = 0; k < 9; k ++){
        fs_w0 >> tmpInt;
        UUT->DRAM0.mem[346112+i*9*32+j*9+k] = tmpInt;
      }
    }
  }
  fs_w0.close();
  dummyCPU->DataForController[0] = 0;
  dummyCPU->DataForController[1] = 364544*4;
  dummyCPU->DataForController[2] = 346112*4;
  dummyCPU->DataForController[3] = 0xffffffff;
  dummyCPU->DataForController[4] = 104;
  dummyCPU->DataForController[5] = 104;
  dummyCPU->DataForController[6] = 32;
  dummyCPU->DataForController[7] = 3;
  dummyCPU->DataForController[8] = 3;
  dummyCPU->DataForController[9] = 64;
  dummyCPU->DataForController[10] = 0;
  dummyCPU->DataForController[11] = 1;
  sc_start();
  for(int i = 0; i < 9; i ++){
    for(int j = 0; j < 8; j ++){
      cout<<(int32_t)UUT->OSRAM0.u32Mem[i*8+j+((WEIGHT_SRAM_BASE-OUTPUT_SRAM0_BASE)>>2)]<<' ';
    }   
    cout<<endl;
  }

  bool flag = true;
  int32_t i32Tmp;
  ifstream fs_golden("./data/DRAM_INPUT/model1/layer2/layer2_output.txt");
  for(int i = 0; i < 64; i ++){
    for(int j = 0; j < 102; j ++){
      for(int k = 0; k < 102; k ++){
        fs_golden>>i32Tmp;
        if((int32_t)UUT->DRAM0.mem[364544+i*102*102+j*102+k] != i32Tmp){
          cout<<"ERROR"<<i<<", "<<j<<", "<<k<<", GOLDEN: "<<i32Tmp<<", WRONG:"<<(int32_t)UUT->DRAM0.mem[364544+i*102*102+j*102+k]<<endl;
          flag = false;
        }
        // cout<<(int32_t)UUT->DRAM0.mem[519312+i*414*414+j*414+k]<<' ';
      }
      // cout<<endl;
    }
    // cout<<endl;
  }
  if(flag) cout<<"***   Layer2 all right    ***"<<endl;
  /**
   * @}
   * @ End Second Layer
   */
  #endif

  cout << "\n*****    Finish     *****\n";

  return 0;
}
#endif  