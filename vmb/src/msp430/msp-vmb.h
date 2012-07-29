#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>

#ifndef MSP_COMMON
#include "msp-common.h"
#endif

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
#define JNE 0
#define JEQ 1
#define JNC 2
#define JC 3
#define JN 4
#define JGE 5
#define JL 6
#define JMP 7


int decodeInstructionFormat(MSP_WORD instruction, executorPtr *executor);
executorPtr findExecutor(unsigned int instructionCode);
int decodeF1Instruction(MSP_WORD instruction, void *source, void *destination, int *isByte);
int decodeF2Instruction(MSP_WORD instruction, void *operand, int *isByte);
int decodeF3Instruction(MSP_WORD instruction, void *condition, void *offset);
void executeInstruction(unsigned int instructionCode);