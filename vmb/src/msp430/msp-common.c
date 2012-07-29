#include "msp-common.h"

unsigned int wordToUInt(MSP_WORD word) {
	//return word.LOW|(word.[1]<<8);
	return word.LOW|(word.HIGH<<8);
}

int wordToInt(MSP_WORD word) {
	int result = word.LOW|(word.HIGH<<8);
	// Wenn negativ - mit bits auffüllen
	if ((result & 0x8000) != 0) {
		result |= (~0xFFFF);
	}
	return result;
}

int byteToInt(char byte) {
	int result = byte;
	// Wenn negativ - mit bits auffüllen
	if ((result & 0x80) != 0) {
		result |= (~0xFF);
	}
	return result;
}

unsigned int byteToUInt(char byte) {
	return byte;
}

MSP_WORD intToMSPWord(int i) {
	MSP_WORD result;
	
	result.HIGH = (i & 0xFF00)>>8;
	result.LOW = (i & 0xFF);
	return result;
}

MSP_WORD uintToMSPWord(unsigned int ui) {
	MSP_WORD result;
	
	result.HIGH = (ui & 0xFF00)>>8;
	result.LOW = (ui & 0xFF);
	return result;
}

MSP_WORD word_add_int(MSP_WORD op1, int op2) {
	MSP_WORD result;
	int iResult;

	iResult = (op1.HIGH<<8)|op1.LOW;
	iResult += op2;
	
	result.HIGH = (iResult>>8)&0xFF;
	result.LOW = iResult & 0xFF;
}

MSP_WORD word_add_word(MSP_WORD op1, MSP_WORD op2) {
	MSP_WORD result;
	int iResult;

	iResult = (op1.HIGH<<8)|op1.LOW;
	iResult += (op2.HIGH<<8)|op2.LOW;
	
	result.HIGH = (iResult>>8)&0xFF;
	result.LOW = iResult & 0xFF;
}