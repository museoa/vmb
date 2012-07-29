#define MEMORY_MANAGER 1

#ifndef MSP_COMMON
#include "msp-common.h"
#endif

#ifndef NULL
#include <Windows.h>
#endif

#define REGISTERS_COUNT 16
// MSP registers
static MSP_WORD* registers;
// Status bits
//static int bitCarry = FALSE;
//static int bitZero = FALSE;
//static int bitNegative = FALSE;
//static int bitArithmeticException = FALSE;

// Maps an MSP address to the "real" address
void* getAddress(MSP_WORD mspAddress);
// Increases the programm counter
void increasePC();
// Load functions for msp memory
MSP_WORD getWordAt(MSP_WORD msp_address);
char getByteAt(MSP_WORD msp_address);

// Sets the status bits. If arguments are NULL the corresponding status bits won't be affected.
void setStatusBits(int *carry, int *zero, int *negative, int *arithmetic);
// Getters for the status bits
int getCarryBit();
int getZeroBit();
int getNegativeBit();
int getArithmeticBit();
