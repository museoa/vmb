#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif
#include "mspbus.h"

#ifndef MSP_CORE
#define MSP_CORE
// Registers
enum msp_register {
	PC,SP,SR,CG1=2,CG2,
	R0=0,R1,R2,R3,R4,R5,R6,R7,R8,R9,R10,R11,R12,R13,R14,R15
};

// SR-Bits
enum status_bit {
	C=0,Z,N,GIE,CPU_OFF,OSC_OFF,SCG0,SCG1,V
};

// Address modes
enum addr_mode {
	ADDR_MODE_REGISTER,						// Register operands
	ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE,	// Indexed: Register+X points to the operand (X is stored in next word)
											// Symbolic: register is PC
											// Absolute: register is SR/CG1, X is the absolute address
	ADDR_MODE_INDIRECT_REGISTER,			// Register points to the operand
	ADDR_MODE_INDIRECT_AUTOINC_IMMEDIATE	// Register points to the operand, register content is autoincremented
											// by 1 for .B-instructions, by 2 - for .W-instructions
											// Immediate: The register is PC (the following word contains the pointer)
};

// Instruction executor function definition
typedef int (*executorPtr)();

// Opcode-Masken
// Format1: double operand instructions
typedef struct {
	unsigned int CommandBitMask;
	unsigned int SRegBitMask;
	unsigned int AdBitMask;
	unsigned int BWBitMask;
	unsigned int AsBitMask;
	unsigned int DRegBitMask;
	unsigned int CommandBitMaskShift;
	unsigned int SRegBitMaskShift;
	unsigned int AdBitMaskShift;
	unsigned int BWBitMaskShift;
	unsigned int AsBitMaskShift;
	unsigned int DRegBitMaskShift;
} _FORMAT1_MASK;
#define FORMAT1_MASK {0xF000,0x0F00,0x0080,0x0040,0x0030,0x000F,12,8,7,6,4,0}

// Format2: single operand instructions
typedef struct {
	unsigned int CommandBitMask;
	unsigned int BWBitMask;
	unsigned int AdBitMask;
	unsigned int DSRegBitMask;
	unsigned int CommandBitMaskShift;
	unsigned int BWBitMaskShift;
	unsigned int AdBitMaskShift;
	unsigned int DSRegBitMaskShift;
} _FORMAT2_MASK;
#define FORMAT2_MASK {0xFF80,0x0040,0x0030,0x000F,7,6,4,0}

// Format3: jump instructions
typedef struct {
	unsigned int CommandBitMask;
	unsigned int ConditionBitMask;
	unsigned int OffsetBitMask;
	unsigned int CommandBitMaskShift;
	unsigned int ConditionBitMaskShift;
	unsigned int OffsetBitMaskShift;
} _FORMAT3_MASK;
#define FORMAT3_MASK {0xE000,0x1C00,0x03FF,13,10,0}

typedef struct {
	_FORMAT1_MASK FORMAT1;
	_FORMAT2_MASK FORMAT2;
	_FORMAT3_MASK FORMAT3;
}_OPCODE_MASKS;
static _OPCODE_MASKS OPCODE_MASKS = {FORMAT1_MASK,FORMAT2_MASK,FORMAT3_MASK};

extern int ADD_executor();
extern int ADDC_executor();
extern int AND_executor();
extern int BIC_executor();
extern int BIS_executor();
extern int BIT_executor();
extern int CALL_executor();
extern int CMP_executor();
extern int DADD_executor();
extern int JUMP_executor();
extern int MOV_executor();
extern int PUSH_executor();
extern int RETI_executor();
extern int RRA_executor();
extern int RRC_executor();
extern int SUB_executor();
extern int SUBC_executor();
extern int SWPB_executor();
extern int SXT_executor();
extern int XOR_executor();

// Instruction codes
static const executorPtr INSTRUCTIONS[39] = 
{
	0,					// 0
	&JUMP_executor,		// 1
	0,					// 2
	0,					// 3
	&MOV_executor,		// 4
	&ADD_executor,		// 5
	&ADDC_executor,		// 6
	&SUBC_executor,		// 7
	&SUB_executor,		// 8
	&CMP_executor,		// 9
	&DADD_executor,		// 10
	&BIT_executor,		// 11
	&BIC_executor,		// 12
	&BIS_executor,		// 13
	&XOR_executor,		// 14
	&AND_executor,		// 15
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 16-31
	&RRC_executor,		// 32
	&SWPB_executor,		// 33
	&RRA_executor,		// 34
	&SXT_executor,		// 35
	&PUSH_executor,		// 36
	&CALL_executor,		// 37
	&RETI_executor		// 38
};

// Condition constants for the jump instruction
enum jump_conditions {
	JNE = 0,
	JEQ,
	JNC,
	JC,
	JN,
	JGE,
	JL,
	JMP
};

typedef union {
	// MSP is little endian!
	UINT16 asWord;
	struct {
#ifdef BIGENDIAN
		unsigned char V : 1;
		unsigned char RESERVED : 7;
		unsigned char C : 1;
		unsigned char Z : 1;
		unsigned char N : 1;
		unsigned char GIE : 1;
		unsigned char CPUOFF : 1;
		unsigned char OSCOFF : 1;
		unsigned char SCG0 : 1;
		unsigned char SCG1 : 1;
#else
		unsigned char C : 1;
		unsigned char Z : 1;
		unsigned char N : 1;
		unsigned char GIE : 1;
		unsigned char CPUOFF : 1;
		unsigned char OSCOFF : 1;
		unsigned char SCG0 : 1;
		unsigned char SCG1 : 1;
		unsigned char V : 1;
		unsigned char RESERVED : 7;
#endif
	} asBits;
  
  struct {
#ifdef BIGENDIAN
    unsigned char hi : 8;
    unsigned char lo : 8;
#else
    unsigned char lo : 8;
    unsigned char hi : 8;
#endif
  } asBytes;
} msp_register;

#define REGISTERS_COUNT 16
// MSP registers
static msp_register registers[REGISTERS_COUNT];

// Access status bits
#define cBit registers[SR].asBits.C
#define zBit registers[SR].asBits.Z
#define nBit registers[SR].asBits.N
#define vBit registers[SR].asBits.V

static unsigned long clocks = 0;
static UINT16 currentInstruction = 0;
#define MEMORY_WRITEBACK_NO (-1)

static INT32 memoryWriteBack = MEMORY_WRITEBACK_NO;

#define EXECUTION_START_AT 0xFFFE
#define RAM_START_AT 0x200

// Increases the programm counter by 2
extern void increasePC();

extern int decodeInstructionFormat(UINT16 instruction, executorPtr *executor);
extern executorPtr findExecutor(UINT16 instructionCode);
extern int decodeF1Instruction(UINT16 instruction, void **source, void **destination, int *isByte);
extern int decodeF2Instruction(UINT16 instruction, void **operand, int *isByte);
extern int decodeF3Instruction(UINT16 instruction, void **condition, void **offset);
extern void executeInstruction(unsigned int instructionCode);
extern UINT16 fetchInstruction(UINT16 msp_address);
extern void executionLoop();
extern void initCore(void);
extern void initRegisters();


#endif

