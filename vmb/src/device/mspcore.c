#include "mspcore.h"

#ifndef MEMORY_MANAGER
#include "mspmemorymanager.h"
#endif

int ADD_executor (char **programmCounter) {
	int isByteInstruction;
	// char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;
	int bN, bZ, bC, bV;

	if (!decodeF1Instruction(getWordAt(registers[PC]),source,destination,&isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		iSource = byteToInt(*(char*)source);
		uiSource = byteToUInt(*(char*)source);
		iDestination = byteToInt(*(char*)destination);
		uiDestination = byteToUInt(*(char*)destination);
		result = uiSource + uiDestination;
		*(char*)destination = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(*(MSP_WORD*)source);
		uiSource = wordToUInt(*(MSP_WORD*)source);
		iDestination = wordToInt(*(MSP_WORD*)destination);
		uiDestination = wordToUInt(*(MSP_WORD*)destination);
		result = uiSource + uiDestination;
		*(MSP_WORD*)destination = intToMSPWord(result & 0xFF);
	}
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bZ = TRUE;
	else
		bZ = FALSE;

	// (C-bit)
	if (isByteInstruction) compCarry |= (compCarry <<= 8);
	if ((result & (~compCarry)) != 0) {
		bC = TRUE;
	}
	else {
		bC = FALSE;
	}
	
	// (V-bit)
	if (((iSource > 0) && (iDestination > 0) && (bN)) 
		|| ((iSource < 0) && (iDestination < 0) && (!bN))) {
			bV = TRUE;
	}
	else {
		bV = FALSE;
	}

	setStatusBits(&bC, &bZ, &bN, &bV);

	// PC to next word
	increasePC();
	return 1;
}

int ADDC_executor (char **programmCounter) {
	int isByteInstruction;
	//char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *source, *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;
	int bN, bZ, bC, bV;
	
	if (!decodeF1Instruction(getWordAt(registers[PC]),source,destination,&isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		iSource = byteToInt(*(char*)source);
		uiSource = byteToUInt(*(char*)source);
		iDestination = byteToInt(*(char*)destination);
		uiDestination = byteToUInt(*(char*)destination);
		result = uiSource + uiDestination + getCarryBit();
		*(char*)destination = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(*(MSP_WORD*)source);
		uiSource = wordToUInt(*(MSP_WORD*)source);
		iDestination = wordToInt(*(MSP_WORD*)destination);
		uiDestination = wordToUInt(*(MSP_WORD*)destination);
		result = uiSource + uiDestination + getCarryBit();
		*(MSP_WORD*)destination = uintToMSPWord(result);
	}
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bZ = TRUE;
	else
		bZ = FALSE;

	// (C-bit)
	if (isByteInstruction) compCarry |= (compCarry <<= 8);
	if ((result & (~compCarry)) != 0) {
		bC = TRUE;
	}
	else {
		bC = FALSE;
	}
	
	// (V-bit)
	if (((iSource > 0) && (iDestination > 0) && (bN)) 
		|| ((iSource < 0) && (iDestination < 0) && (!bN))) {
			bV = TRUE;
	}
	else {
		bV = FALSE;
	}

	setStatusBits(&bC, &bZ, &bN, &bV);

	// PC to next word
	increasePC();
	return 1;
}

int AND_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	//char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;
	int bN, bZ, bC, bV;

	if (!decodeF1Instruction(getWordAt(registers[PC]),source,destination,&isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		iSource = byteToInt(*(char*)source);
		uiSource = byteToUInt(*(char*)source);
		iDestination = byteToInt(*(char*)destination);
		uiDestination = byteToUInt(*(char*)destination);
		result = uiSource & uiDestination;
		*(char*)destination = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(*(MSP_WORD*)source);
		iDestination = wordToInt(*(MSP_WORD*)destination);
		uiSource = wordToUInt(*(MSP_WORD*)source);
		uiDestination = wordToUInt(*(MSP_WORD*)destination);
		result = uiSource & uiDestination;
		*(MSP_WORD*)destination = uintToMSPWord(result);
	}
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bZ = TRUE;
	else
		bZ = FALSE;

	// Carry bit is set, if result != ZERO
	if (bZ)
		bC = FALSE;
	else
		bC = TRUE;
	
	// V-bit: always reset
	bV = FALSE;

	setStatusBits(&bC, &bZ, &bN, &bV);

	// PC to next word
	increasePC();
	return 1;
}

int BIC_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	//char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;

	if (!decodeF1Instruction(getWordAt(registers[PC]),source,destination,&isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		iSource = byteToInt(*(char*)source);
		uiSource = byteToUInt(*(char*)source);
		iDestination = byteToInt(*(char*)destination);
		uiDestination = byteToUInt(*(char*)destination);
		result = (~uiSource) & uiDestination;
		*(char*)destination = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(*(MSP_WORD*)source);
		uiSource = wordToUInt(*(MSP_WORD*)source);
		iDestination = wordToInt(*(MSP_WORD*)destination);
		uiDestination = wordToUInt(*(MSP_WORD*)destination);
		result = (~uiSource) & uiDestination;
		*(MSP_WORD*)destination = uintToMSPWord(result);
	}
	
	// Status bits are not affected
	
	// PC to next word
	increasePC();
	return 1;
}

int BIS_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	//char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;

	if (!decodeF1Instruction(getWordAt(registers[PC]),source,destination,&isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		iSource = byteToInt(*(char*)source);
		uiSource = byteToUInt(*(char*)source);
		iDestination = byteToInt(*(char*)destination);
		uiDestination = byteToUInt(*(char*)destination);
		result = uiSource | uiDestination;
		*(char*)destination = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(*(MSP_WORD*)source);
		uiSource = wordToUInt(*(MSP_WORD*)source);
		iDestination = wordToInt(*(MSP_WORD*)destination);
		uiDestination = wordToUInt(*(MSP_WORD*)destination);
		result = uiSource | uiDestination;
		*(MSP_WORD*)destination = uintToMSPWord(result);
	}
	
	// Status bits are not affected
	
	// PC to next word
	increasePC();
	return 1;
}

int BIT_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	//char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;
	int bN, bZ, bC, bV;

	if (!decodeF1Instruction(getWordAt(registers[PC]),source,destination,&isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		iSource = byteToInt(*(char*)source);
		uiSource = byteToUInt(*(char*)source);
		iDestination = byteToInt(*(char*)destination);
		uiDestination = byteToUInt(*(char*)destination);
		result = uiSource & uiDestination;
		*(char*)destination = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(*(MSP_WORD*)source);
		uiSource = wordToUInt(*(MSP_WORD*)source);
		iDestination = wordToInt(*(MSP_WORD*)destination);
		uiDestination = wordToUInt(*(MSP_WORD*)destination);
		result = uiSource & uiDestination;
		*(MSP_WORD*)destination = uintToMSPWord(result);
	}
	
	// Only status bits affected
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bZ = TRUE;
	else
		bZ = FALSE;

	// Carry bit is set, if result != ZERO
	if (bZ)
		bC = FALSE;
	else
		bC = TRUE;
	
	// V-bit: always reset
	bV = FALSE;

	setStatusBits(&bC, &bZ, &bN, &bV);
	
	// PC to next word
	increasePC();
	return 1;
}

int CALL_executor (char **programmCounter) {
	/*
		Decreases the stack pointer, pushes the PC (or rather the address of the next
		instruction) to the stack and overwrites the programm counter with the
		operand.
	*/
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *operand;
	int isByteInstruction;
	MSP_WORD operandValue;

	if (!decodeF2Instruction(instruction, operand, &isByteInstruction))
		return FALSE;

	operandValue = *(MSP_WORD*)operand;
	registers[SP] = word_add_int(registers[SP],-2);
	*(MSP_WORD*)getAddress(registers[SP]) = word_add_int(registers[PC],2);
	registers[PC] = operandValue;

	increasePC();

	return TRUE;
}

int CMP_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	//char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;
	int bN, bZ, bC, bV;

	if (!decodeF1Instruction(getWordAt(registers[PC]),source,destination,&isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		iSource = byteToInt(*(char*)source);
		uiSource = byteToUInt(*(char*)source);
		iDestination = byteToInt(*(char*)destination);
		uiDestination = byteToUInt(*(char*)destination);
		result = uiSource - uiDestination;
		*(char*)destination = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(*(MSP_WORD*)source);
		iDestination = wordToInt(*(MSP_WORD*)destination);
		uiSource = wordToUInt(*(MSP_WORD*)source);
		uiDestination = wordToUInt(*(MSP_WORD*)destination);
		result = uiSource - uiDestination;
		*(MSP_WORD*)destination = uintToMSPWord(result);
	}
	
	// Only status bits affected
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bZ = TRUE;
	else
		bZ = FALSE;

	// C-bit
	if (isByteInstruction) compCarry |= (compCarry <<= 8);
	if ((result & (~compCarry)) != 0) {
		bC = TRUE;
	}
	else {
		bC = FALSE;
	}
	
	// (V-bit)
	if (((iSource > 0) && (iDestination < 0) && (bN)) 
		|| ((iSource < 0) && (iDestination > 0) && (!bN))) {
			bV = TRUE;
	}
	else {
		bV = FALSE;
	}


	setStatusBits(&bC, &bZ, &bN, &bV);
	
	// PC to next word
	increasePC();
	return 1;
}

int DADD_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	//char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;
	unsigned int uiDigit, uiDigitMask = 0xF;
	int carry, i, maxI;
	int bN, bZ, bC, bV;


	if (!decodeF1Instruction(getWordAt(registers[PC]),source,destination,&isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		maxI = 2;
		uiSource = byteToUInt(*(char*)source);
		uiDestination = byteToUInt(*(char*)destination);
	}
	else {
		maxI = 4;
		uiSource = wordToUInt(*(MSP_WORD*)source);
		uiDestination = wordToUInt(*(MSP_WORD*)destination);
	}

	carry = FALSE;
	i = 0;
	result = 0;
	carry = getCarryBit();
	while (i < maxI) {
		uiDigit = uiSource & uiDigitMask + uiDestination & uiDigitMask;
		if (carry) {
			uiDigit += 1;
			carry = FALSE;
		}
		if (uiDigit > 9) {
			carry = TRUE;
			uiDigit = uiDigit % 10;
		}
		else
			carry = FALSE;
		result |= (uiDigit << (i*4));
		uiDigitMask <<= 4;
		i++;
	}
		
	if (isByteInstruction) {
		*(char*)destination = (char)(result & 0xFF);
	} else {
		*(MSP_WORD*)destination = uintToMSPWord(result);
	}

	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bZ = TRUE;
	else
		bZ = FALSE;

	// (C-bit)
	bC = carry;
	
	// (V-bit undefined)
	bV = FALSE;

	setStatusBits(&bC, &bZ, &bN, &bV);
	
	// PC to next word
	increasePC();
	return 1;
}

int JUMP_executor (char **programmCounter) {
	// Jumps to address PC+2*offset (offset is signed!)
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *condition;
	unsigned int uiCondition;
	void *offset;
	int iOffset;
	int jump = FALSE;
	MSP_WORD destinationAddress;

	if (!decodeF3Instruction(instruction, condition, offset))
		return FALSE;

	uiCondition = *(unsigned int*)condition;
	iOffset = *(int*)offset;

	switch (uiCondition) {
	case JNE:
		if (!getZeroBit())
			jump = TRUE;
		break;
	case JEQ:
		if (getZeroBit())
			jump = TRUE;
		break;
	case JNC:
		if (!getCarryBit())
			jump = TRUE;
		break;
	case JC:
		if (getCarryBit())
			jump = TRUE;
		break;
	case JN:
		if (getNegativeBit())
			jump = TRUE;
		break;
	case JGE:
		if (!(getNegativeBit() ^ getArithmeticBit()))
			jump = TRUE;
		break;
	case JL:
		if (getNegativeBit() ^ getArithmeticBit())
			jump = TRUE;
		break;
	case JMP:
		jump = TRUE;
		break;
	default:
		break;
	}

	if (jump) {
		// The programm counter was not moved to the next instruction yet
		registers[PC] = word_add_int(registers[PC],2+2*iOffset);
	} else {
		increasePC();
	}

	return TRUE;
}

int MOV_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	//char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;

	if (!decodeF1Instruction(getWordAt(registers[PC]),source,destination,&isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction)
		*(char*)destination = *(char*)source;
	else
		*(MSP_WORD*)destination = *(MSP_WORD*)source;

	// Status bits are not affected

	// PC to next word
	increasePC();
	return 1;
}

int PUSH_executor (char **programmCounter) {
	/*
		Decreases stack pointer by 2 and pushes a byte or word to the stack.
	*/
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *operand = NULL;
	
	int isByteInstruction = FALSE;

	if (!decodeF2Instruction(instruction, operand, &isByteInstruction))
		return FALSE;

	
	registers[SP] = word_add_int(registers[SP],-2);

	if (isByteInstruction) {
		char operandValue = *(char*)operand;
		(*(MSP_WORD*)getAddress(registers[SP])).HIGH = 0;
		(*(MSP_WORD*)getAddress(registers[SP])).LOW = operandValue;

	} else {
		MSP_WORD operandValue = *(MSP_WORD*)operand;
		*(MSP_WORD*)getAddress(registers[SP]) = operandValue;
	}

	increasePC();

	return TRUE;
}

int RETI_executor (char **programmCounter) {
	/*
		Return from interrupt:
		@TOS is moved to SR, stack pointer decreased by 2, @TOS is moved to PC,
		stack pointer decreased by 2.
	*/
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *operand;
	int isByteInstruction = FALSE;
	MSP_WORD stack;

	if (!decodeF2Instruction(instruction, operand, &isByteInstruction))
		return FALSE;

	// Restore status register
	stack = *(MSP_WORD*)getAddress(registers[SP]);
	registers[SR] = stack;
	registers[SP] = word_add_int(registers[SP],-2);
	// Restore programm counter
	stack = *(MSP_WORD*)getAddress(registers[SP]);
	registers[PC] = stack;
	registers[SP] = word_add_int(registers[SP],-2);

	increasePC();

	return TRUE;
}

int RRA_executor (char **programmCounter) {
	/*
		Rotates the operand to the right (arithmetically):
		MSB->MSB, MSB->MSB-1,...,LSB+1->LSB,LSB->C
	*/
	MSP_WORD instruction = getWordAt(registers[PC]);
	int bC = FALSE;
	int bN, bZ, bV;
	int msb = FALSE;
	int isByteInstruction = FALSE;
	void *operand;

	if (!decodeF2Instruction(instruction, operand, &isByteInstruction))
		return FALSE;
	
	if (isByteInstruction) {
		char operandValue;
		operandValue = *(char*)operand;
		if ((operandValue & 0x80) > 0) {
			msb = TRUE;
			bN = TRUE;
		} else {
			bN = FALSE;
		}
		if (operandValue == 0)
			bZ = TRUE;
		else
			bZ = FALSE;
		if ((operandValue & 0x1) > 0)
			bC = TRUE;

		operandValue >>= 1;
		if (msb)
			operandValue |= 0x80;
		*(char*)operand = operandValue;
	} else {
		MSP_WORD operandValue;
		unsigned int uiOperandValue;
		operandValue = *(MSP_WORD*)operand;
		uiOperandValue = wordToUInt(operandValue);
		if ((uiOperandValue & 0x8000) > 0) {
			msb = TRUE;
			bN = TRUE;
		} else {
			bN = FALSE;
		}
		if ((uiOperandValue & 0x1) > 0)
			bC = TRUE;
		if (uiOperandValue == 0)
			bZ = TRUE;
		else
			bZ = FALSE;

		uiOperandValue >>= 1;
		if (msb)
			uiOperandValue |= 0x8000;
		operandValue = uintToMSPWord(uiOperandValue);
		*(MSP_WORD*)operand = operandValue;
	}

	bV = FALSE;

	setStatusBits(&bC, &bZ, &bN, &bV);

	increasePC();

	return TRUE;
}

int RRC_executor (char **programmCounter) {
	/*
		Rotates the operand right with carry:
		C->MSB, ..., LSB->C
	*/
	MSP_WORD instruction = getWordAt(registers[PC]);
	int bN, bZ, bC, bV;
	int lsb = FALSE;
	int isByteInstruction = FALSE;
	void *operand;

	if (!decodeF2Instruction(instruction, operand, &isByteInstruction))
		return FALSE;

	bC = getCarryBit();

	if (isByteInstruction) {
		char operandValue;
		operandValue = *(char*)operand;
		if ((operandValue & 0x1) > 0)
			lsb = TRUE;

		operandValue >>= 1;
		if (bC) {
			operandValue |= 0x80;
			bN = TRUE;
		} else {
			bN = FALSE;
		}
		if (operandValue == 0)
			bZ = TRUE;
		else
			bZ = FALSE;
		*(char*)operand = operandValue;
	} else {
		MSP_WORD operandValue;
		unsigned int uiOperandValue;
		operandValue = *(MSP_WORD*)operand;
		uiOperandValue = wordToUInt(operandValue);
		if ((uiOperandValue & 0x1) > 0)
			lsb = TRUE;

		uiOperandValue >>= 1;
		if (bC) {
			uiOperandValue |= 0x8000;
			bN = TRUE;
		} else {
			bN = FALSE;
		}
		if (uiOperandValue == 0)
			bZ = TRUE;
		else
			bZ = FALSE;
		operandValue = uintToMSPWord(uiOperandValue);
		*(MSP_WORD*)operand = operandValue;
	}

	bV = FALSE;

	setStatusBits(&lsb, &bZ, &bN, &bV);

	increasePC();

	return TRUE;
}

int SUB_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	//char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;
	int bN, bZ, bC, bV;

	if (!decodeF1Instruction(instruction,source,destination,&isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		iSource = byteToInt(*(char*)source);
		uiSource = byteToUInt(*(char*)source);
		iDestination = byteToInt(*(char*)destination);
		uiDestination = byteToUInt(*(char*)destination);
		result = uiSource - uiDestination;
		*(char*)destination = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(*(MSP_WORD*)source);
		uiSource = wordToUInt(*(MSP_WORD*)source);
		iDestination = wordToInt(*(MSP_WORD*)destination);
		uiDestination = wordToUInt(*(MSP_WORD*)destination);
		result = uiSource - uiDestination;
		*(MSP_WORD*)destination = uintToMSPWord(result);
	}

	// Ist Ergebnis negativ? (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bN = TRUE;
	else
		bN = FALSE;

	// Ist Ergebnis 0? (Z-bit)
	if ((result & 0xFFFF) == 0)
		bZ = TRUE;
	else
		bZ = FALSE;

	// Wurde ein Übertrag erzeugt (C-bit)
	if (isByteInstruction) compCarry |= (compCarry <<= 8);
	if ((result & (~compCarry)) != 0) {
		bC = TRUE;
	}
	else {
		bC = FALSE;
	}
	
	// Wurde ein arithmetischer Fehler ausgelöst? (V-bit)
	if (((iSource > 0) && (iDestination < 0) && (bN)) 
		|| ((iSource < 0) && (iDestination > 0) && (!bN))) {
			bV = TRUE;
	}
	else {
		bV = FALSE;
	}

	setStatusBits(&bC, &bZ, &bN, &bV);
	
	// PC to next word
	increasePC();
	return 1;
}

int SUBC_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	//char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;
	int bN, bZ, bC, bV;

	if (!decodeF1Instruction(instruction,source,destination,&isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		iSource = byteToInt(*(char*)source);
		uiSource = byteToUInt(*(char*)source);
		iDestination = byteToInt(*(char*)destination);
		uiDestination = byteToUInt(*(char*)destination);
		result = uiSource - uiDestination + getCarryBit();
		*(char*)destination = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(*(MSP_WORD*)source);
		uiSource = wordToUInt(*(MSP_WORD*)source);
		iDestination = wordToInt(*(MSP_WORD*)destination);
		uiDestination = wordToUInt(*(MSP_WORD*)destination);
		result = uiSource - uiDestination + getCarryBit();
		*(MSP_WORD*)destination = uintToMSPWord(result);
	}

	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bZ = TRUE;
	else
		bZ = FALSE;

	// (C-bit)
	if (isByteInstruction) compCarry |= (compCarry <<= 8);
	if ((result & (~compCarry)) != 0) {
		bC = TRUE;
	}
	else {
		bC = FALSE;
	}
	
	// (V-bit)
	if (((iSource > 0) && (iDestination < 0) && (bN)) 
		|| ((iSource < 0) && (iDestination > 0) && (!bN))) {
			bV = TRUE;
	}
	else {
		bV = FALSE;
	}

	setStatusBits(&bC, &bZ, &bN, &bV);
	
	// PC to next word
	increasePC();
	return 1;
}

int SWBP_executor (char **programmCounter) {
	/*
		Swaps the both bytes of the operand
	*/
	MSP_WORD instruction = getWordAt(registers[PC]);
	char tmp;
	int isByteInstruction = FALSE;
	void *operand;
	MSP_WORD operandValue;


	if (!decodeF2Instruction(instruction, operand, &isByteInstruction))
		return FALSE;

	operandValue = *(MSP_WORD*)operand;
	tmp = operandValue.HIGH;
	operandValue.HIGH = operandValue.LOW;
	operandValue.LOW = tmp;
	*(MSP_WORD*)operand = operandValue;

	return TRUE;
}

int SXT_executor (char **programmCounter) {
	/*
		Extends the sign of the lower byte to the higher byte
	*/
	MSP_WORD instruction = getWordAt(registers[PC]);
	int isByteInstruction = FALSE;
	void *operand;
	MSP_WORD operandValue;
	int bN, bZ, bC, bV;

	if (!decodeF2Instruction(instruction, operand, &isByteInstruction))
		return FALSE;

	operandValue = *(MSP_WORD*)operand;

	if ((operandValue.LOW & 0x80) > 0) {
		operandValue.HIGH = 0xFF;
		bN = TRUE;
	}
	else {
		operandValue.HIGH = 0x0;
		bN = FALSE;
	}

	if (operandValue.HIGH == 0 && operandValue.LOW == 0)
		bZ = TRUE;
	else
		bZ = FALSE;

	bC = !bZ;

	bV = FALSE;

	*(MSP_WORD*)operand = operandValue;

	setStatusBits(&bC, &bZ, &bN, &bV);

	return TRUE;
}

int XOR_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	//char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	MSP_WORD instruction = getWordAt(registers[PC]);
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;
	int bN, bZ, bC, bV;

	if (!decodeF1Instruction(instruction,source,destination,&isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		iSource = byteToInt(*(char*)source);
		uiSource = byteToUInt(*(char*)source);
		iDestination = byteToInt(*(char*)destination);
		uiDestination = byteToUInt(*(char*)destination);
		result = uiSource ^ uiDestination;
		*(char*)destination = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(*(MSP_WORD*)source);
		uiSource = wordToUInt(*(MSP_WORD*)source);
		iDestination = wordToInt(*(MSP_WORD*)destination);
		uiDestination = wordToUInt(*(MSP_WORD*)destination);
		result = uiSource ^ uiDestination;
		*(MSP_WORD*)destination = uintToMSPWord(result);
	}
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bZ = TRUE;
	else
		bZ = FALSE;

	// Carry bit is set, if result != ZERO
	bC = !bZ;
	
	// V-bit: set if both operands are negative
	bV = (iSource < 0 && iDestination < 0);

	setStatusBits(&bC, &bZ, &bN, &bV);
	
	// PC to next word
	increasePC();
	return 1;
}


/*
	Decodes the instruction format and finds executor.
	Executor is returned by reference (*executor).
	Returns format value.
*/
int decodeInstructionFormat(MSP_WORD instruction, executorPtr *executor) {
	unsigned int i_opcode;
	int instructionCode;
	int format = 0;

	i_opcode = wordToUInt(instruction);

	instructionCode = (i_opcode & OPCODE_MASKS.FORMAT1.CommandBitMask)>>OPCODE_MASKS.FORMAT1.CommandBitMaskShift;
	
	// Decode instruction code
	switch (instructionCode) {
	case 1:		// Format 2
		instructionCode = (i_opcode & OPCODE_MASKS.FORMAT2.CommandBitMask)>>OPCODE_MASKS.FORMAT2.CommandBitMaskShift;
		format = 2;
		break;
	case 2:	// Format 3
		instructionCode = (i_opcode & OPCODE_MASKS.FORMAT3.CommandBitMask)>>OPCODE_MASKS.FORMAT3.CommandBitMaskShift;
		format = 3;
		break;
	case 3: // Format 3
		instructionCode = (i_opcode & OPCODE_MASKS.FORMAT3.CommandBitMask)>>OPCODE_MASKS.FORMAT3.CommandBitMaskShift;
		format = 3;
		break;
	default:	// Format 1
		format = 1;
		break;
	}

	*executor = findExecutor(instructionCode);
	return format;
}

/*
	Decodes format 1 instructions.
	The decoded results will be stored at the locations indicated by 
	- instruction_code-
	- source-
	- destination-
	and isByte- pointers.

	The function requires a little endian system, so 
	byte instructions can use (only) the lower byte of 
	source/destination addresses, otherwise the appropriate
	cases (e.g. if isByteInstruction -> address of lower register byte 
	is returned for source/destination) should be implemented. The same measures
	should be applied to the constant generator.
*/
int decodeF1Instruction(MSP_WORD instruction, void *source, 
	void *destination, int *isByte) {

	unsigned int As, Ad, sReg, dReg;
	unsigned int uiInstruction;
	MSP_WORD *constant;
	char cConstant = 0;
	int createConstant = FALSE;
	int isByteInstruction = FALSE;
	MSP_WORD baseAddress;
	MSP_WORD offset;

	uiInstruction = wordToUInt(instruction);

	// Byte or word?
	if ((uiInstruction & OPCODE_MASKS.FORMAT1.BWBitMask) > 0)
		isByteInstruction = TRUE;
	*isByte = isByteInstruction;

	// Decode source operand
	As = ((uiInstruction & OPCODE_MASKS.FORMAT1.AsBitMask)>>OPCODE_MASKS.FORMAT1.AsBitMaskShift);
	sReg = ((uiInstruction & OPCODE_MASKS.FORMAT1.SRegBitMask)>>OPCODE_MASKS.FORMAT1.SRegBitMaskShift);

	switch (sReg) {
	case CG1:			// CG1 = SR!
		switch (As) {	
		case ADDR_MODE_REGISTER:			// SR-Register (non-cg case)
			source = &registers[PC];
			break;
		case ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE:	// Equivalent to constant 0
			increasePC();
			offset = getWordAt(registers[PC]);
			source = getAddress(offset);
			break;
		case 2:
			createConstant = TRUE;
			cConstant = 4;
			break;
		case 3:
			createConstant = TRUE;
			cConstant = 8;
			break;
		default:
			source = NULL;
			break;
		}
		break;


	case CG2:
		switch (As) {
		case 0:
			createConstant = TRUE;
			cConstant = 0;
			break;
		case 1:
			createConstant = TRUE;
			cConstant = 1;
			break;
		case 2:
			createConstant = TRUE;
			cConstant = 2;
			break;
		case 3:
			createConstant = TRUE;
			cConstant = -1;
			break;
		default:
			source = NULL;
			break;
		}

	default:
		switch (As) {
		case ADDR_MODE_REGISTER:		// Register is the operand
			source = &registers[sReg];
			break;
		case ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE:
			// Register contains the base address, next word contains the offset
			baseAddress = registers[sReg];
			increasePC();
			offset = getWordAt(registers[PC]);
			source = getAddress(word_add_word(baseAddress,offset));
			break;
		case ADDR_MODE_INDIRECT_REGISTER:
			// Register contains the address of the operand
			source = getAddress(registers[sReg]);
			break;
		case ADDR_MODE_INDIRECT_AUTOINC_IMMEDIATE:
			// Register contains the address. Register value will be incremented (byte/wyde)
			if (sReg == PC)
				increasePC();
			baseAddress = getWordAt(registers[sReg]);
			source = getAddress(baseAddress);
			// Execute autoincrement
			if (sReg != PC) {
				// PC is already incremented
				int incr;
				if (isByteInstruction)
					incr = 1;
				else
					incr = 2;
				registers[sReg] = word_add_int(registers[sReg],incr);
			}
			break;
		default:
			source = NULL;
		}
	}

	if (createConstant) {
		// Endianess!
		//int mallocSize;
		//if (isByteInstruction)
		//	mallocSize = sizeof(char);
		//else
		//	mallocSize = sizeof(MSP_WORD);

		constant = (MSP_WORD*)malloc(sizeof(MSP_WORD));
		*constant = intToMSPWord(cConstant);
		source = constant;
	}
	
	// Decode destination operand
	Ad = (uiInstruction & OPCODE_MASKS.FORMAT1.AdBitMask)>>OPCODE_MASKS.FORMAT1.AdBitMaskShift;
	dReg = ((uiInstruction & OPCODE_MASKS.FORMAT1.DRegBitMask)>>OPCODE_MASKS.FORMAT1.DRegBitMaskShift);

	
	switch (Ad) {
	case ADDR_MODE_REGISTER:
		// Register is the operand
		destination = &registers[dReg];
		break;
	case ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE:
		// Register contains the base address, next word contains the offset
		baseAddress = registers[sReg];
		increasePC();
		offset = getWordAt(registers[PC]);
		source = getAddress(word_add_word(baseAddress,offset));
		break;
	default:
		destination = NULL;
	}
	
	if (source == NULL || destination == NULL)
		return FALSE;
	else
		return TRUE;
}


int decodeF2Instruction(MSP_WORD instruction, void *operand, int *isByte) {
	unsigned int uiInstruction, Ad, DSReg;
	int isByteInstruction = FALSE;
	MSP_WORD baseAddress;
	MSP_WORD offset;

	uiInstruction = wordToUInt(instruction);

	Ad = (uiInstruction & OPCODE_MASKS.FORMAT2.AdBitMask)>>OPCODE_MASKS.FORMAT2.AdBitMaskShift;
	DSReg = (uiInstruction & OPCODE_MASKS.FORMAT2.DSRegBitMask)>>OPCODE_MASKS.FORMAT2.DSRegBitMaskShift;
	if ((uiInstruction & OPCODE_MASKS.FORMAT2.BWBitMask) != 0)
		isByteInstruction = TRUE;

	isByte = &isByteInstruction;

	switch (Ad) {
	case ADDR_MODE_REGISTER:		// Register is the operand
		operand = &registers[DSReg];
		break;
	case ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE:
		// Register contains the base address, next word contains the offset
		baseAddress = registers[DSReg];
		increasePC();
		offset = getWordAt(registers[PC]);
		operand = getAddress(word_add_word(baseAddress,offset));
		break;
	case ADDR_MODE_INDIRECT_REGISTER:
		// Register contains the address of the operand
		operand = getAddress(registers[DSReg]);
		break;
	case ADDR_MODE_INDIRECT_AUTOINC_IMMEDIATE:
		// Register contains the address. Register value will be incremented (byte/wyde)
		if (DSReg == PC)
			increasePC();
		baseAddress = getWordAt(registers[DSReg]);
		operand = getAddress(baseAddress);
		// Execute autoincrement
		if (DSReg != PC) {
			// PC is already incremented
			int incr;
			if (isByteInstruction)
				incr = 1;
			else
				incr = 2;
			registers[DSReg] = word_add_int(registers[DSReg],incr);
		}
		break;
	default:
		operand = NULL;
	}

	if (operand != NULL)
		return TRUE;
	else
		return FALSE;
}

int decodeF3Instruction(MSP_WORD instruction, void *condition, void *offset) {
	unsigned int uiInstruction, uiCondition;
	int iOffset;

	uiInstruction = wordToUInt(instruction);
	uiCondition = (uiInstruction & OPCODE_MASKS.FORMAT3.ConditionBitMask)
		>>OPCODE_MASKS.FORMAT3.ConditionBitMaskShift;
	iOffset = (uiInstruction & OPCODE_MASKS.FORMAT3.OffsetBitMask)
		>>OPCODE_MASKS.FORMAT3.OffsetBitMaskShift;
	// Check for sign (offset has 10 bit)
	if ((iOffset & 512) != 0)
		iOffset &= (-1);

	condition = &uiCondition;
	offset = &iOffset;

	return TRUE;
}

executorPtr findExecutor(unsigned int instructionCode) {
	if (instructionCode == INSTRUCTIONS._ADD.code) {
		return INSTRUCTIONS._ADD.executor;
	} else if (instructionCode == INSTRUCTIONS._ADDC.code) {
		return INSTRUCTIONS._ADDC.executor;
	} else if (instructionCode == INSTRUCTIONS._AND.code) {
		return INSTRUCTIONS._AND.executor;
	} else if (instructionCode == INSTRUCTIONS._BIC.code) {
		return INSTRUCTIONS._BIC.executor;
	} else if (instructionCode == INSTRUCTIONS._BIS.code) {
		return INSTRUCTIONS._BIS.executor;
	} else if (instructionCode == INSTRUCTIONS._BIT.code) {
		return INSTRUCTIONS._BIT.executor;
	} else if (instructionCode == INSTRUCTIONS._CALL.code) {
		return INSTRUCTIONS._CALL.executor;
	} else if (instructionCode == INSTRUCTIONS._CMP.code) {
		return INSTRUCTIONS._CMP.executor;
	} else if (instructionCode == INSTRUCTIONS._DADD.code) {
		return INSTRUCTIONS._DADD.executor;
	} else if (instructionCode == INSTRUCTIONS._JUMP.code) {
		return INSTRUCTIONS._JUMP.executor;
	} else if (instructionCode == INSTRUCTIONS._MOV.code) {
		return INSTRUCTIONS._MOV.executor;
	} else if (instructionCode == INSTRUCTIONS._PUSH.code) {
		return INSTRUCTIONS._PUSH.executor;
	} else if (instructionCode == INSTRUCTIONS._RETI.code) {
		return INSTRUCTIONS._RETI.executor;
	} else if (instructionCode == INSTRUCTIONS._RRA.code) {
		return INSTRUCTIONS._RRA.executor;
	} else if (instructionCode == INSTRUCTIONS._RRC.code) {
		return INSTRUCTIONS._RRC.executor;
	} else if (instructionCode == INSTRUCTIONS._SUB.code) {
		return INSTRUCTIONS._SUB.executor;
	} else if (instructionCode == INSTRUCTIONS._SUBC.code) {
		return INSTRUCTIONS._SUBC.executor;
	} else if (instructionCode == INSTRUCTIONS._SWBP.code) {
		return INSTRUCTIONS._SWBP.executor;
	} else if (instructionCode == INSTRUCTIONS._SXT.code) {
		return INSTRUCTIONS._SXT.executor;
	} else if (instructionCode == INSTRUCTIONS._XOR.code) {
		return INSTRUCTIONS._XOR.executor;
	} else {
		return NULL;
	}
}



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
	if ((registers[SR].LOW & 0x1) > 0)
		return TRUE;
	else
		return FALSE;
}
int getZeroBit() {
	if ((registers[SR].LOW & 0x2) > 0)
		return TRUE;
	else
		return FALSE;
}
int getNegativeBit() {
	if ((registers[SR].LOW & 0x4) > 0)
		return TRUE;
	else
		return FALSE;
}
int getArithmeticBit() {
	if ((registers[SR].HIGH & 0x1) > 0)
		return TRUE;
	else
		return FALSE;
}

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
	return result;
}

MSP_WORD word_add_word(MSP_WORD op1, MSP_WORD op2) {
	MSP_WORD result;
	int iResult;

	iResult = (op1.HIGH<<8)|op1.LOW;
	iResult += (op2.HIGH<<8)|op2.LOW;
	
	result.HIGH = (iResult>>8)&0xFF;
	result.LOW = iResult & 0xFF;
	return result;
}


int main() {
	// Init
	registers = (MSP_WORD*)malloc(sizeof(MSP_WORD) * REGISTERS_COUNT);
	//printf("0x%X",OPCODE_MASKS.FORMAT1.CommandBitMask);
	//char *opcode = (char*)malloc(sizeof(char));
	//char **pc = &opcode;
	//int (*executor)(char**);
	//char c1 = 127;
	//char c2 = 127;
	//char c3 = c1+c2;
	//registers = (MSP_WORD*)malloc(sizeof(MSP_WORD)*REGISTERS_COUNT);

	////opcode = (char*)malloc(2);
	////0x5142 in little-endian (ADD.B R1,R2)
	//opcode[0] = 0x42;
	//opcode[1] = 0x51;
	//executor = findExecutor(getInstructionCode(opcode));
	//if (executor == NULL || !executor(pc)) {
	//	printf("Error on executing @ %d",opcode);
	//	return 1;
	//}
	
	//printf("c1+c2 = %d",c3);
	
	return 0;
}