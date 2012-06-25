#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>

//typedef int boolean;
#define TRUE 1
#define FALSE 0

// (Spezial)register
enum msp_register{
	PC,SP,SR,CG1=2,CG2,
	R0=0,R1,R2,R3,R4,R5,R6,R7,R8,R9,R10,R11,R12,R13,R14,R15
};

// Statusregister-Bits
enum status_bit {
	C=0,Z,N,GIE,CPU_OFF,OSC_OFF,SCG0,SCG1,V
};

// Addressierungsmodi
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

typedef int (*executorPtr) (char**);

// Befehle
typedef struct _INSTRUCTION {
	char name[5];
	unsigned short code;
	unsigned short format;
	executorPtr executor;
} INSTRUCTION;

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

int ADD_executor (char **programmCounter);
int ADDC_executor (char **programmCounter);
int AND_executor (char **programmCounter);
int BIC_executor (char **programmCounter);
int BIS_executor (char **programmCounter);
int BIT_executor (char **programmCounter);
int CALL_executor (char **programmCounter);
int CMP_executor (char **programmCounter);
int DADD_executor (char **programmCounter);
int JUMP_executor (char **programmCounter);
int MOV_executor (char **programmCounter);
int PUSH_executor (char **programmCounter);
int RETI_executor (char **programmCounter);
int RRA_executor (char **programmCounter);
int RRC_executor (char **programmCounter);
int SUB_executor (char **programmCounter);
int SUBC_executor (char **programmCounter);
int SWBP_executor (char **programmCounter);
int SXT_executor (char **programmCounter);
int XOR_executor (char **programmCounter);

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

// Opcode-Masken
// Format1: double operand commands
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

// Format2: single operand commands
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

// Format2: jump commands
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

#define REGISTERS_COUNT 16

static char *registers;
static int bitCarry = FALSE;
static int bitZero = FALSE;
static int bitNegative = FALSE;
static int bitArithmeticException = FALSE;

unsigned int wordToUInt(char *word);
unsigned int byteToUInt(char *byte);
int wordToInt(char *word);
int byteToInt(char *byte);
int getF1Operands(char *opcode, int byte, char *source, char *destination);
unsigned int decodeInstruction(char *opcode, int bigEndian);
void executeInstruction(unsigned int instructionCode);
char* getAddress(unsigned int mspAddress);
char* increasePC();