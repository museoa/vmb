#define MSP_COMMON 1

//typedef int boolean;
#define TRUE 1
#define FALSE 0

typedef struct {
	char LOW;
	char HIGH;
} MSP_WORD;

// Registers
enum msp_register{
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


unsigned int wordToUInt(MSP_WORD word);
unsigned int byteToUInt(char byte);
int wordToInt(MSP_WORD word);
int byteToInt(char byte);
MSP_WORD intToMSPWord(int i);
MSP_WORD uintToMSPWord(unsigned int ui);

MSP_WORD word_add_int(MSP_WORD op1, int op2);
MSP_WORD word_add_word(MSP_WORD op1, MSP_WORD op2);