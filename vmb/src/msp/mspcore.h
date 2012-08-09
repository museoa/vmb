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
typedef int (*executorPtr) (char**);

// Instruction struct
typedef struct _INSTRUCTION {
	char name[5];
	unsigned short code;
	unsigned short format;
	executorPtr executor;
} INSTRUCTION;


// MSP430 instructions
typedef struct {
	INSTRUCTION _ADD;
	INSTRUCTION _ADDC;
	INSTRUCTION _AND;
	INSTRUCTION _BIC;
	INSTRUCTION _BIS;
	INSTRUCTION _BIT;
	INSTRUCTION _CALL;
	INSTRUCTION _CMP;
	INSTRUCTION _DADD;
	INSTRUCTION _JUMP;
	INSTRUCTION _MOV;
	INSTRUCTION _PUSH;
	INSTRUCTION _RETI;
	INSTRUCTION _RRA;
	INSTRUCTION _RRC;
	INSTRUCTION _SUB;
	INSTRUCTION _SUBC;
	INSTRUCTION _SWBP;
	INSTRUCTION _SXT;
	INSTRUCTION _XOR;
} _INSTRUCTIONS;

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

// Format2: jump instructions
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

extern int ADD_executor (char **programmCounter);
extern int ADDC_executor (char **programmCounter);
extern int AND_executor (char **programmCounter);
extern int BIC_executor (char **programmCounter);
extern int BIS_executor (char **programmCounter);
extern int BIT_executor (char **programmCounter);
extern int CALL_executor (char **programmCounter);
extern int CMP_executor (char **programmCounter);
extern int DADD_executor (char **programmCounter);
extern int JUMP_executor (char **programmCounter);
extern int MOV_executor (char **programmCounter);
extern int PUSH_executor (char **programmCounter);
extern int RETI_executor (char **programmCounter);
extern int RRA_executor (char **programmCounter);
extern int RRC_executor (char **programmCounter);
extern int SUB_executor (char **programmCounter);
extern int SUBC_executor (char **programmCounter);
extern int SWBP_executor (char **programmCounter);
extern int SXT_executor (char **programmCounter);
extern int XOR_executor (char **programmCounter);

// Instruction codes
#define I_ADD {"ADD",5,1,&ADD_executor}
#define I_ADDC {"ADDC",6,1,&ADDC_executor}
#define I_AND {"AND",15,1,&AND_executor}
#define I_BIC {"BIC",12,1,&BIC_executor}
#define I_BIS {"BIS",13,1,&BIS_executor}
#define I_BIT {"BIT",11,1,&BIT_executor}
#define I_CALL {"CALL",37,2,&CALL_executor}
#define I_CMP {"CMP",9,1,&CMP_executor}
#define I_DADD {"DADD",10,1,&DADD_executor}
#define I_JUMP {"JUMP",1,3,&JUMP_executor}
#define I_MOV {"MOV",4,1,&MOV_executor}
#define I_PUSH {"PUSH",36,2,&PUSH_executor}
#define I_RETI {"RETI",38,2,&RETI_executor}
#define I_RRA {"RRA",34,2,&RRA_executor}
#define I_RRC {"RRC",32,2,&RRC_executor}
#define I_SUB {"SUB",8,1,&SUB_executor}
#define I_SUBC {"SUBC",7,1,&SUBC_executor}
#define I_SWBP {"SWBP",33,2,&SWBP_executor}
#define I_SXT {"SXT",35,2,&SXT_executor}
#define I_XOR {"XOR",14,1,&XOR_executor}

static const _INSTRUCTIONS INSTRUCTIONS = 
	{I_ADD,
	I_ADDC,
	I_AND,
	I_BIC,
	I_BIS,
	I_BIT,
	I_CALL,
	I_CMP,
	I_DADD,
	I_JUMP,
	I_MOV,
	I_PUSH,
	I_RETI,
	I_RRA,
	I_RRC,
	I_SUB,
	I_SUBC,
	I_SWBP,
	I_SXT,
	I_XOR};

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

#define REGISTERS_COUNT 16
// MSP registers
static register registers[REGISTERS_COUNT];

typedef union {
  UINT16 u;
  struct {
    unsigned char Z : 1;
    unsigned char C : 1;
    unsigned int rest: 14;
  } b;
  struct {
#ifdef BIGENDIAN
    unsigned char hi : 8;
    unsigned char lo : 8;
#else
    unsigned char lo : 8;
    unsigned char hi : 8;
#fi
  } bytes;
} register;

#define bC registers[SR].b.C

/* use: registers[SP].u for 16 bit int registers[SR].bits.bZ */

static unsigned long clocks = 0;
static UINT16 currentInstruction = 0;
#define MEMORY_WRITEBACK_NO (-1)

static INT32 memoryWriteBack = MEMORY_WRITEBACK_NO;

#define EXECUTION_START_AT 0xFFFE
#define RAM_START_AT 0x200

// Increases the programm counter
extern void increasePC();

// Sets the status bits. If arguments are NULL the corresponding status bits won't be affected.
extern void setStatusBits(int *carry, int *zero, int *negative, int *arithmetic);
// Getters for the status bits
extern int getCarryBit();
extern int getZeroBit();
extern int getNegativeBit();
extern int getArithmeticBit();

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
