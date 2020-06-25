#include <iomanip>
#include <iostream>

//====================================================
// Simulation Platform
//====================================================

	#ifndef TIME_SCALE
	#define TIME_SCALE (SC_NS)
	#endif

  #ifndef CLK_CYCLE
  #define CLK_CYCLE (12.5)
  #endif

	#ifndef DMAC_CLK_CYCLE
	#define DMAC_CLK_CYCLE (12.6)
	#endif

	// Note: When you set TIME_SCALE as SC_NS, then
	// CLK_CYCLE = (5.0) 200MHz, (2.5) 400MhHz, (1.66) 600MHz, (1.25) 800Mhz, (1.0) 1000MHz, (0.833) 1200Mhz
	// Make sure that CLC_CYCLE is a float type value.

//====================================================
// Memory Subsystem
//====================================================
// DRAM Configuration

#define ADDR_LENGTH (32) 
#define DATA_LENGTH (32) 
#define DATA_BUS_BANDWIDTH (256)        //Configuable

#define NUM_MAC (9)
#define NUM_PE (8)

//~~~~~~~~
//  DRAM
//~~~~~~~~
#define MEM_SIZE (0x2000000) //MEM_SIZE (0x2000000)bytes,Configuable, uint32_t mem[MEMSIZE] 

#define ROW_BIT (11)
#define COLUMN_BIT (10)
#define BANK_BIT (8)

#ifdef FAST_SIM
	#define tRP (1)
	#define tRCD (1)
	#define tCL (1)
	#define tRAS (1)
	#define magic_delay (1)
#else  // using DDR4-2400
	#define tRP (15)
	#define tRCD (15)
	#define tCL (15)
	#define tRAS (39)
	#define magic_delay (0.8333) // DRAM bus cycle time (ns) (0.8333)
#endif

//****************
#define DRAM4_ID (4)
#define DRAM4_BASE (0X00000000)
#define DRAM4_SIZE (MEM_SIZE)

#define DRIVER_READ (0)
#define DRIVER_WRITE (1)

/**
 * SRAM
 */
#define NUM_SRAM          (2)

#define INPUT_SRAM_SIZE   (0x16000<<2)  //13*13*512
#define INPUT_SRAM0_BASE  (DRAM4_BASE + DRAM4_SIZE)
#define INPUT_SRAM1_BASE  (INPUT_SRAM0_BASE + INPUT_SRAM_SIZE)

#define OUTPUT_SRAM_SIZE  (0x580000)  //INPUT_SRAM_SIZE*NUM_PE*2
#define OUTPUT_SRAM0_BASE (INPUT_SRAM1_BASE + INPUT_SRAM_SIZE)
#define OUTPUT_SRAM1_BASE (OUTPUT_SRAM0_BASE + OUTPUT_SRAM_SIZE)

#define WEIGHT_SRAM_SIZE  (648<<2)  //9*9*NUM_PE
#define WEIGHT_SRAM_BASE  (OUTPUT_SRAM1_BASE + OUTPUT_SRAM_SIZE)
/**
 * END SRAM
 */

/**
 * DMAC CONTROLLER
 */
#define DMAC_RESERVED    0x100
#define DMAC_BASE         (WEIGHT_SRAM_BASE + WEIGHT_SRAM_SIZE)

#define CONTROLLER_CLK_CYCLE  40
#define CONTROLLER_BASE   (DMAC_BASE + DMAC_RESERVED)
