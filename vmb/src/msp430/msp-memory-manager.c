#include "msp-memory-manager.h"

void* getAddress(MSP_WORD mspAdress) {
	// Hier findet das Mapping statt

	return NULL;
}

void increasePC() {
	registers[PC] = word_add_int(registers[PC],2);
	return;
}

MSP_WORD getWordAt(MSP_WORD msp_address) {
	MSP_WORD result;
	void* pcPointer;
	pcPointer = getAddress(msp_address);
	result.LOW = *(char*)pcPointer;
	result.HIGH = *(char*)pcPointer+sizeof(char);
	return result;
}

char getByteAt(MSP_WORD msp_address) {
	char result;
	void* pcPointer;
	pcPointer = getAddress(msp_address);
	result = *(char*)pcPointer;
	return result;
}

void setStatusBits(int *carry, int *zero, int *negative, int *arithmetic) {
	if (carry != NULL) {
		if (*carry)
			registers[SR].LOW = registers[SR].LOW | 0x1;
		else
			registers[SR].LOW = registers[SR].LOW & 0xFE;
	}

	if (zero != NULL) {
		if (*zero)
			registers[SR].LOW = registers[SR].LOW | 0x2;
		else
			registers[SR].LOW = registers[SR].LOW & 0xFD;
	}

	if (negative != NULL) {
		if (*negative)
			registers[SR].LOW = registers[SR].LOW | 0x4;
		else
			registers[SR].LOW = registers[SR].LOW | 0xFB;
	}

	if (arithmetic != NULL) {
		if (*arithmetic)
			registers[SR].HIGH = registers[SR].HIGH | 0x1;
		else
			registers[SR].HIGH = registers[SR].HIGH & 0xFE;
	}
}

int getCarryBit() {
	if (registers[SR].LOW & 0x1 > 0)
		return TRUE;
	else
		return FALSE;
}
int getZeroBit() {
	if (registers[SR].LOW & 0x2 > 0)
		return TRUE;
	else
		return FALSE;
}
int getNegativeBit() {
	if (registers[SR].LOW & 0x4 > 0)
		return TRUE;
	else
		return FALSE;
}
int getArithmeticBit() {
	if (registers[SR].HIGH & 0x1 > 0)
		return TRUE;
	else
		return FALSE;
}