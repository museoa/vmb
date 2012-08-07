/*
	
*/

#include "mspbus.h"
#include "mspcore.h"


int ADD_executor (char **programmCounter) {
	int isByteInstruction;
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	UINT32 result = 0;
	UINT16 uiSource, uiDestination;
	int bN, bZ, bC, bV;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = *(UINT8*)source;
		uiDestination = *(UINT8*)destination;
		result = uiSource + uiDestination;
		*(UINT8*)destination = (UINT8)(result & 0xFF);
	} else {
		uiSource = *(UINT16*)source;
		uiDestination = *(UINT16*)destination;
		result = uiSource + uiDestination;
		*(UINT16*)destination = (UINT16)(result & 0xFFFF);
	}
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if (result & compNegative)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if (!(result & 0xFFFF))
		bZ = TRUE;
	else
		bZ = FALSE;

	// (C-bit)
	if (!isByteInstruction) 
		compCarry |= 0xFF00;
	if (result & (~compCarry)) {
		bC = TRUE;
	}
	else {
		bC = FALSE;
	}
	
	// (V-bit)
	if (((!(uiSource & compNegative)) && (!(uiDestination & compNegative)) && (bN)) 
		|| ((uiSource & compNegative) && (uiDestination & compNegative) && (!bN))) {
			bV = TRUE;
	}
	else {
		bV = FALSE;
	}

	setStatusBits(&bC, &bZ, &bN, &bV);

	if (memoryWriteBack >= 0) {
		if (isByteInstruction) {
			if (! vmbWriteByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)destination)) {
				return FALSE;
			}
		} else {
			if (! vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)destination)) {
				return FALSE;
			}
		}
	}

	// PC to next word
	increasePC();
	return TRUE;
}

int ADDC_executor (char **programmCounter) {
	int isByteInstruction;
	//char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	void *source, *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	UINT32 result;
	//int iSource, iDestination;
	//unsigned int uiSource, uiDestination;
	UINT16 uiSource, uiDestination;
	int bN, bZ, bC, bV;
	
	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = uiSource + uiDestination + getCarryBit();
		*(UINT8*)destination = (UINT8)(result & 0xFF);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = uiSource + uiDestination + getCarryBit();
		*(UINT16*)destination = (UINT16)(result & 0xFFFF);
	}
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if (result & compNegative)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if (!(result & 0xFFFF))
		bZ = TRUE;
	else
		bZ = FALSE;

	// (C-bit)
	if (isByteInstruction) compCarry |= 0xFF00;
	if (result & (~compCarry)) {
		bC = TRUE;
	}
	else {
		bC = FALSE;
	}
	
	// (V-bit)
	if ((!(uiSource & compNegative) && !(uiDestination & compNegative) && (bN)) 
		|| ((uiSource & compNegative) && (uiDestination & compNegative) && (!bN))) {
			bV = TRUE;
	}
	else {
		bV = FALSE;
	}

	setStatusBits(&bC, &bZ, &bN, &bV);

	if (memoryWriteBack >= 0) {
		if (isByteInstruction) {
			if (!vmbWriteByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)destination))
				return FALSE;
		} else {
			if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)destination))
				return FALSE;
		}
	}

	// PC to next word
	increasePC();
	return TRUE;
}

int AND_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	UINT16 result;
	//int iSource, iDestination;
	UINT16 uiSource, uiDestination;
	int bN, bZ, bC, bV;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = uiSource & uiDestination;
		*(UINT8*)destination = (UINT8)(result & 0xFF);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = uiSource & uiDestination;
		*(UINT16*)destination = (UINT16)(result & 0xFFFF);
	}
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if (result & compNegative)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if (!(result & 0xFFFF))
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

	if (memoryWriteBack >= 0) {
		if (isByteInstruction) {
			if (!vmbWriteByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)destination))
				return FALSE;
		} else {
			if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)destination))
				return FALSE;
		}
	}

	// PC to next word
	increasePC();
	return TRUE;
}

int BIC_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	UINT16 result;
	UINT16 uiSource, uiDestination;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = (~uiSource) & uiDestination;
		*(UINT8*)destination = (UINT8)(result & 0xFF);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = (~uiSource) & uiDestination;
		*(UINT16*)destination = (UINT16)(result & 0xFFFF);
	}
	
	// Status bits are not affected

	if (memoryWriteBack >= 0) {
		if (isByteInstruction) {
			if (!vmbWriteByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)destination))
				return FALSE;
		} else {
			if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)destination))
				return FALSE;
		}
	}
	
	// PC to next word
	increasePC();
	return 1;
}

int BIS_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	UINT16 result;
	UINT16 uiSource, uiDestination;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = uiSource | uiDestination;
		*(UINT8*)destination = (UINT8)(result & 0xFF);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = uiSource | uiDestination;
		*(UINT16*)destination = (UINT16)(result & 0xFFFF);
	}
	
	// Status bits are not affected

	if (memoryWriteBack >= 0) {
		if (isByteInstruction) {
			if (!vmbWriteByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)destination))
				return FALSE;
		} else {
			if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)destination))
				return FALSE;
		}
	}
	
	// PC to next word
	increasePC();
	return 1;
}

int BIT_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	UINT16 result;
	UINT16 uiSource, uiDestination;
	int bN, bZ, bC, bV;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = uiSource & uiDestination;
		*(UINT8*)destination = (UINT8)(result & 0xFF);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = uiSource & uiDestination;
		*(UINT16*)destination = (UINT16)(result & 0xFFFF);
	}
	
	// Only status bits affected
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if (result & compNegative)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if (!(result & 0xFFFF))
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

	if (memoryWriteBack >= 0) {
		if (isByteInstruction) {
			if (!vmbWriteByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)destination))
				return FALSE;
		} else {
			if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)destination))
				return FALSE;
		}
	}
	
	// PC to next word
	increasePC();
	return TRUE;
}

int CALL_executor (char **programmCounter) {
	/*
		Decreases the stack pointer, pushes the PC (or rather the address of the next
		instruction) to the stack and overwrites the programm counter with the
		operand.
	*/
	void *operand;
	int isByteInstruction;
	UINT16 operandValue;
	UINT16 pcForStack;

	if (!decodeF2Instruction(currentInstruction, &operand, &isByteInstruction))
		return FALSE;

	operandValue = *(UINT16*)operand;
	registers[SP] -= 2;
	pcForStack = registers[PC]+2;
	if (!vmbWriteWordAt(registers[SP], &pcForStack))
		return FALSE;
	registers[PC] = operandValue;

	//increasePC();

	return TRUE;
}

int CMP_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	UINT16 uiSource, uiDestination;
	int bN, bZ, bC, bV;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = uiSource - uiDestination;
		*(UINT8*)destination = (UINT8)(result & 0xFF);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = uiSource - uiDestination;
		*(UINT16*)destination = (UINT16)(result & 0xFFFF);
	}
	
	// Only status bits affected
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if (result & compNegative)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if (!(result & 0xFFFF))
		bZ = TRUE;
	else
		bZ = FALSE;

	// C-bit
	if (isByteInstruction) compCarry |= 0xFF00;
	if ((result & (~compCarry)) != 0) {
		bC = TRUE;
	}
	else {
		bC = FALSE;
	}
	
	// (V-bit)
	if ((!(uiSource & compNegative) && !(uiDestination & compNegative) && (bN)) 
		|| ((uiSource & compNegative) && (uiDestination & compNegative) && (!bN))) {
			bV = TRUE;
	}
	else {
		bV = FALSE;
	}


	setStatusBits(&bC, &bZ, &bN, &bV);

	if (memoryWriteBack >= 0) {
		if (isByteInstruction) {
			if (!vmbWriteByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)destination))
				return FALSE;
		} else {
			if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)destination))
				return FALSE;
		}
	}
	
	// PC to next word
	increasePC();
	return TRUE;
}

int DADD_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	UINT16 uiSource, uiDestination;
	UINT16 uiDigit, uiDigitMask = 0xF;
	int carry, i, maxI;
	int bN, bZ, bC, bV;


	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		maxI = 2;
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
	}
	else {
		maxI = 4;
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
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
		*(UINT8*)destination = (UINT8)(result & 0xFF);
	} else {
		*(UINT16*)destination = (UINT16)(result & 0xFFFF);
	}

	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if (result & compNegative)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if (!(result & 0xFFFF))
		bZ = TRUE;
	else
		bZ = FALSE;

	// (C-bit)
	bC = carry;
	
	// (V-bit undefined)
	bV = FALSE;

	setStatusBits(&bC, &bZ, &bN, &bV);

	if (memoryWriteBack >= 0) {
		if (isByteInstruction) {
			if (!vmbWriteByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)destination))
				return FALSE;
		} else {
			if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)destination))
				return FALSE;
		}
	}
	
	// PC to next word
	increasePC();
	return TRUE;
}

int JUMP_executor (char **programmCounter) {
	// Jumps to address PC+2*offset (offset is signed!)
	void *condition;
	unsigned int uiCondition;
	void *offset;
	INT16 iOffset;
	int jump = FALSE;

	if (!decodeF3Instruction(currentInstruction, &condition, &offset))
		return FALSE;

	uiCondition = *(unsigned int*)condition;
	iOffset = *(INT16*)offset;

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
		registers[PC] += 2*iOffset;
	} else {
		increasePC();
	}

	return TRUE;
}

int MOV_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction)
		*(UINT8*)destination = *(UINT8*)source;
	else
		*(UINT16*)destination = *(UINT16*)source;

	// Status bits are not affected

	if (memoryWriteBack >= 0) {
		if (isByteInstruction) {
			if (!vmbWriteByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)destination))
				return FALSE;
		} else {
			if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)destination))
				return FALSE;
		}
	}

	// PC to next word
	increasePC();
	return TRUE;
}

int PUSH_executor (char **programmCounter) {
	/*
		Decreases stack pointer by 2 and pushes a byte or word to the stack.
	*/
	void *operand = NULL;
	UINT16 operandValue = 0;
	int isByteInstruction = FALSE;

	if (!decodeF2Instruction(currentInstruction, &operand, &isByteInstruction))
		return FALSE;
	
	// Update stack pointer
	registers[SP] -= 2;

	if (isByteInstruction) {
		operandValue = *(UINT8*)operand;

	} else {
		operandValue = *(UINT16*)operand;
	}

	// Status bits are not affected

	if (!vmbWriteWordAt(registers[SP], &operandValue))
		return FALSE;

	increasePC();

	return TRUE;
}

int RETI_executor (char **programmCounter) {
	/*
		Return from interrupt:
		Word @TOS is moved to SR, stack pointer decreased by 2, @TOS is moved to PC,
		stack pointer decreased by 2.
	*/
	void *operand;
	int isByteInstruction = FALSE;
	UINT16 stack;

	if (!decodeF2Instruction(currentInstruction, &operand, &isByteInstruction))
		return FALSE;

	// Restore status register
	if (!vmbReadWordAt(registers[SP], &stack))
		return FALSE;
	registers[SR] = stack;
	registers[SP] -= 2;
	// Restore programm counter
	if (!vmbReadWordAt(registers[SP], &stack))
		return FALSE;
	registers[PC] = stack;
	registers[SP] -= 2;

	//increasePC();

	return TRUE;
}

int RRA_executor (char **programmCounter) {
	/*
		Rotates the operand to the right (arithmetically):
		MSB->MSB, MSB->MSB-1,...,LSB+1->LSB,LSB->C
	*/
	int bC = FALSE;
	int bN, bZ, bV;
	int msb = FALSE;
	int isByteInstruction = FALSE;
	void *operand;

	if (!decodeF2Instruction(currentInstruction, &operand, &isByteInstruction))
		return FALSE;
	
	if (isByteInstruction) {
		UINT8 operandValue;
		operandValue = *(UINT8*)operand;
		if (operandValue & 0x80) {
			msb = TRUE;
			bN = TRUE;
		} else {
			bN = FALSE;
		}
		if (!operandValue)
			bZ = TRUE;
		else
			bZ = FALSE;
		if ((operandValue & 0x1) > 0)
			bC = TRUE;

		operandValue >>= 1;
		if (msb)
			operandValue |= 0x80;
		*(UINT8*)operand = operandValue;
	} else {
		UINT16 operandValue;
		operandValue = *(UINT16*)operand;
		if (operandValue & 0x8000) {
			msb = TRUE;
			bN = TRUE;
		} else {
			bN = FALSE;
		}
		if (operandValue & 0x1)
			bC = TRUE;
		if (!operandValue)
			bZ = TRUE;
		else
			bZ = FALSE;

		operandValue >>= 1;
		if (msb)
			operandValue |= 0x8000;
		*(UINT16*)operand = operandValue;
	}

	bV = FALSE;

	setStatusBits(&bC, &bZ, &bN, &bV);

	if (memoryWriteBack >= 0) {
		if (isByteInstruction) {
			if (!vmbWriteByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)operand))
				return FALSE;
		} else {
			if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)operand))
				return FALSE;
		}
	}

	increasePC();

	return TRUE;
}

int RRC_executor (char **programmCounter) {
	/*
		Rotates the operand right with carry:
		C->MSB, ..., LSB->C
	*/
	int bN, bZ, bC, bV;
	int lsb = FALSE;
	int isByteInstruction = FALSE;
	void *operand;

	if (!decodeF2Instruction(currentInstruction, &operand, &isByteInstruction))
		return FALSE;

	bC = getCarryBit();

	if (isByteInstruction) {
		UINT8 operandValue;
		operandValue = *(UINT8*)operand;
		if ((operandValue & 0x1) > 0)
			lsb = TRUE;

		operandValue >>= 1;
		if (bC) {
			operandValue |= 0x80;
			bN = TRUE;
		} else {
			bN = FALSE;
		}
		bC = lsb;
		if (!operandValue)
			bZ = TRUE;
		else
			bZ = FALSE;
		*(UINT8*)operand = operandValue;
	} else {
		UINT16 operandValue;
		//unsigned int uiOperandValue;
		operandValue = *(UINT16*)operand;
		//uiOperandValue = wordToUInt(operandValue);
		if ((operandValue & 0x1) > 0)
			lsb = TRUE;

		operandValue >>= 1;
		if (bC) {
			operandValue |= 0x8000;
			bN = TRUE;
		} else {
			bN = FALSE;
		}
		bC = lsb;
		if (!operandValue)
			bZ = TRUE;
		else
			bZ = FALSE;
		*(UINT16*)operand = operandValue;
	}

	bV = FALSE;

	setStatusBits(&lsb, &bZ, &bN, &bV);

	if (memoryWriteBack >= 0) {
		if (isByteInstruction) {
			if (!vmbWriteByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)operand))
				return FALSE;
		} else {
			if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)operand))
				return FALSE;
		}
	}

	increasePC();

	return TRUE;
}

int SUB_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	UINT16 result = 0;
	UINT16 uiSource, uiDestination;
	int bN, bZ, bC, bV;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = uiSource - uiDestination;
		*(UINT8*)destination = (UINT8)(result & 0xFF);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = uiSource - uiDestination;
		*(UINT16*)destination = (UINT16)(result & 0xFFFF);
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
	if (!isByteInstruction) compCarry |= 0xFF00;
	if (result & (~compCarry)) {
		bC = TRUE;
	}
	else {
		bC = FALSE;
	}
	
	// Wurde ein arithmetischer Fehler ausgelöst? (V-bit)
	if ((!(uiSource & compNegative) && (uiDestination & compNegative) && (bN)) // pos - neg = neg?
		|| ((uiSource & compNegative) && !(uiDestination & compNegative) && (!bN))) {		// neg - pos = pos?
			bV = TRUE;
	}
	else {
		bV = FALSE;
	}

	setStatusBits(&bC, &bZ, &bN, &bV);

	if (memoryWriteBack >= 0) {
		if (isByteInstruction) {
			if (!vmbWriteByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)destination))
				return FALSE;
		} else {
			if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)destination))
				return FALSE;
		}
	}
	
	// PC to next word
	increasePC();
	return TRUE;
}

int SUBC_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	UINT16 result;
	UINT16 uiSource, uiDestination;
	int bN, bZ, bC, bV;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = uiSource - uiDestination + getCarryBit();
		*(UINT8*)destination = (UINT8)(result & 0xFF);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = uiSource - uiDestination + getCarryBit();
		*(UINT16*)destination = (UINT16)(result & 0xFF);
	}

	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if (result & compNegative)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if (!(result & 0xFFFF))
		bZ = TRUE;
	else
		bZ = FALSE;

	// (C-bit)
	if (!isByteInstruction) compCarry |= 0xFF00;
	if (result & (~compCarry)) {
		bC = TRUE;
	}
	else {
		bC = FALSE;
	}
	
	// (V-bit)
	if ((!(uiSource & compNegative) && (uiDestination & compNegative) && (bN)) 
		|| ((uiSource & compNegative) && !(uiDestination & compNegative) && (!bN))) {
			bV = TRUE;
	}
	else {
		bV = FALSE;
	}

	setStatusBits(&bC, &bZ, &bN, &bV);

	if (memoryWriteBack >= 0) {
		if (isByteInstruction) {
			if (!vmbWriteByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)destination))
				return FALSE;
		} else {
			if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)destination))
				return FALSE;
		}
	}
	
	// PC to next word
	increasePC();
	return TRUE;
}

int SWBP_executor (char **programmCounter) {
	/*
		Swaps the both bytes of the operand
	*/
	UINT8 tmp;
	int isByteInstruction = FALSE;
	void *operand;
	UINT16 operandValue;


	if (!decodeF2Instruction(currentInstruction, &operand, &isByteInstruction))
		return FALSE;

	operandValue = *(UINT16*)operand;
	tmp = (operandValue & 0xFF00)>>8;
	operandValue <<= 8;
	operandValue |= tmp;
	*(UINT16*)operand = operandValue;

	if (memoryWriteBack >= 0) {
		if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)operand))
			return FALSE;
	}

	return TRUE;
}

int SXT_executor (char **programmCounter) {
	/*
		Extends the sign of the lower byte to the higher byte
	*/
	int isByteInstruction = FALSE;
	void *operand;
	UINT16 operandValue;
	int bN, bZ, bC, bV;

	if (!decodeF2Instruction(currentInstruction, &operand, &isByteInstruction))
		return FALSE;

	operandValue = *(UINT16*)operand;

	if (operandValue & 0x80) {
		operandValue |= 0xFF00;
		bN = TRUE;
	}
	else {
		operandValue &= 0x00FF;
		bN = FALSE;
	}

	if (!operandValue)
		bZ = TRUE;
	else
		bZ = FALSE;

	bC = !bZ;

	bV = FALSE;

	*(UINT16*)operand = operandValue;

	setStatusBits(&bC, &bZ, &bN, &bV);

	if (memoryWriteBack >= 0) {
		if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)operand))
			return FALSE;
	}

	return TRUE;
}

int XOR_executor (char **programmCounter) {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	UINT16 result;
	UINT16 uiSource, uiDestination;
	int bN, bZ, bC, bV;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = uiSource ^ uiDestination;
		*(UINT8*)destination = (UINT8)(result & 0xFF);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = uiSource ^ uiDestination;
		*(UINT16*)destination = (UINT16)(result & 0xFFFF);
	}

	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	if (result & compNegative)
		bN = TRUE;
	else
		bN = FALSE;

	// (Z-bit)
	if (!(result & 0xFFFF))
		bZ = TRUE;
	else
		bZ = FALSE;

	// Carry bit is set, if result != ZERO
	bC = !bZ;
	
	// V-bit: set if both operands are negative
	bV = ((uiSource & compNegative) && (uiDestination & compNegative));

	setStatusBits(&bC, &bZ, &bN, &bV);

	if (memoryWriteBack >= 0) {
		if (isByteInstruction) {
			if (!vmbWriteByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)destination))
				return FALSE;
		} else {
			if (!vmbWriteWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)destination))
				return FALSE;
		}
	}
	
	// PC to next word
	increasePC();
	return TRUE;
}


/*
	Decodes the instruction format and finds executor.
	Executor is returned by reference (*executor).
	Returns format value.
*/
int decodeInstructionFormat(UINT16 instruction, executorPtr *executor) {
	UINT16 instructionCode;
	int format = 0;

	instructionCode = (instruction & OPCODE_MASKS.FORMAT1.CommandBitMask)>>OPCODE_MASKS.FORMAT1.CommandBitMaskShift;
	
	// Decode instruction code
	switch (instructionCode) {
	case 1:		// Format 2
		instructionCode = (instruction & OPCODE_MASKS.FORMAT2.CommandBitMask)>>OPCODE_MASKS.FORMAT2.CommandBitMaskShift;
		format = 2;
		break;
	case 2:	// Format 3
		instructionCode = (instruction & OPCODE_MASKS.FORMAT3.CommandBitMask)>>OPCODE_MASKS.FORMAT3.CommandBitMaskShift;
		format = 3;
		break;
	case 3: // Format 3
		instructionCode = (instruction & OPCODE_MASKS.FORMAT3.CommandBitMask)>>OPCODE_MASKS.FORMAT3.CommandBitMaskShift;
		format = 3;
		break;
	default:	// Format 1
		format = 1;
		break;
	}

	*executor = findExecutor(instructionCode);
	if (*executor != NULL)
		return format;
	else
		return 0;
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
int decodeF1Instruction(UINT16 instruction, void **source, 
	void **destination, int *isByte) {

	unsigned int As, Ad, sReg, dReg;
	//unsigned int uiInstruction;
	void *constant;
	char cConstant = 0;
	int createConstant = FALSE;
	int isByteInstruction = FALSE;
	UINT16 baseAddress = 0;
	UINT16 offset = 0;

	//uiInstruction = wordToUInt(instruction);

	// Byte or word?
	if ((instruction & OPCODE_MASKS.FORMAT1.BWBitMask) > 0)
		isByteInstruction = TRUE;
	*isByte = isByteInstruction;

	

	// Decode source operand
	As = ((instruction & OPCODE_MASKS.FORMAT1.AsBitMask)>>OPCODE_MASKS.FORMAT1.AsBitMaskShift);
	sReg = ((instruction & OPCODE_MASKS.FORMAT1.SRegBitMask)>>OPCODE_MASKS.FORMAT1.SRegBitMaskShift);

	switch (sReg) {
	case CG1:			// CG1 = SR!
		switch (As) {	
		case ADDR_MODE_REGISTER:			// SR-Register (non-cg case)
			// Warning: little endian! Otherwise, the higher byte 
			// should be selected for byte operations!
			*source = &registers[PC];
			break;
		case ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE:	/* Equivalent to constant 0 
													for base address 
													(absolute addressing) */
			if (isByteInstruction) {
				*source = malloc(sizeof(UINT8));
			} else {
				*source = malloc(sizeof(UINT16));
			}
			baseAddress = 0;
			increasePC();
			 if (!vmbReadWordAt(registers[PC], &offset))
				 return FALSE;
			if (isByteInstruction) {
				 if (!vmbReadByteAt(baseAddress+offset, (UINT8*)*source))
					 return FALSE;
			}
			else {
				if (!vmbReadWordAt(baseAddress+offset, (UINT16*)*source))
					return FALSE;
			}
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
			// Warning: little endian! Otherwise, the higher byte 
			// should be selected for byte operations!
			*source = &registers[sReg];
			break;
		case ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE:
			// Register contains the base address, next word contains the offset
			if (isByteInstruction) {
				*source = malloc(sizeof(UINT8));
			} else {
				*source = malloc(sizeof(UINT16));
			}
			baseAddress = registers[sReg];
			increasePC();
			if (!vmbReadWordAt(registers[PC], &offset))
				return FALSE;
			if (!vmbReadWordAt(baseAddress+offset, (UINT16*)*source))
				return FALSE;
			break;
		case ADDR_MODE_INDIRECT_REGISTER:
			// Register contains the address of the operand
			if (isByteInstruction) {
				*source = malloc(sizeof(UINT8));
				if (!vmbReadByteAt(registers[sReg], (UINT8*)*source))
					 return FALSE;
			} else {
				*source = malloc(sizeof(UINT16));
				if (!vmbReadWordAt(registers[sReg], (UINT16*)*source))
					 return FALSE;
			}
			break;
		case ADDR_MODE_INDIRECT_AUTOINC_IMMEDIATE:
			// Register contains the address. Register value will be incremented (byte/wyde)
			if (sReg == PC)
				increasePC();
			if (!vmbReadWordAt(registers[sReg], &baseAddress))
				return FALSE;
			if (isByteInstruction) {
				*source = malloc(sizeof(UINT8));
				if (!vmbReadByteAt(baseAddress, (UINT8*)*source))
					return FALSE;
			} else {
				*source = malloc(sizeof(UINT16));
				if (!vmbReadWordAt(baseAddress, (UINT16*)*source))
					return FALSE;
			}

			// Execute autoincrement
			if (sReg != PC) { // PC is already incremented
				UINT16 incr = 1;
				if (!isByteInstruction)
					incr++;
				registers[sReg] += incr;
			}
			break;
		default:
			source = NULL;
		}
	}

	if (createConstant) {
		if (isByteInstruction)
			constant = malloc(sizeof(INT8));
		else
			constant = malloc(sizeof(INT16));
		*(UINT16*)constant = cConstant;
		*source = constant;
	}
	
	// Decode destination operand
	Ad = (instruction & OPCODE_MASKS.FORMAT1.AdBitMask)>>OPCODE_MASKS.FORMAT1.AdBitMaskShift;
	dReg = ((instruction & OPCODE_MASKS.FORMAT1.DRegBitMask)>>OPCODE_MASKS.FORMAT1.DRegBitMaskShift);

	
	switch (Ad) {
	case ADDR_MODE_REGISTER:
		// Register is the operand
		*destination = &registers[dReg];
		memoryWriteBack = MEMORY_WRITEBACK_NO;
		break;
	case ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE:
		// Register contains the base address, next word contains the offset
		baseAddress = registers[dReg];
		increasePC();
		if (!vmbReadWordAt(registers[PC], &offset))
			return FALSE;
		memoryWriteBack = baseAddress+offset;
		if (isByteInstruction) {
			*destination = malloc(sizeof(UINT8));
			if (!vmbReadByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)*destination))
				return FALSE;
		} else {
			*destination = malloc(sizeof(UINT16));
			if (!vmbReadWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)*destination))
				return FALSE;
		}
		break;
	default:
		destination = NULL;
	}
	
	if (source == NULL || destination == NULL)
		return FALSE;
	else
		return TRUE;
}


int decodeF2Instruction(UINT16 instruction, void **operand, int *isByte) {
	unsigned int Ad, DSReg;
	int isByteInstruction = FALSE;
	UINT16 baseAddress;
	UINT16 offset;

	Ad = (instruction & OPCODE_MASKS.FORMAT2.AdBitMask)>>OPCODE_MASKS.FORMAT2.AdBitMaskShift;
	DSReg = (instruction & OPCODE_MASKS.FORMAT2.DSRegBitMask)>>OPCODE_MASKS.FORMAT2.DSRegBitMaskShift;
	if (instruction & OPCODE_MASKS.FORMAT2.BWBitMask)
		isByteInstruction = TRUE;

	*isByte = isByteInstruction;

	switch (Ad) {
	case ADDR_MODE_REGISTER:		// Register is the operand
		*operand = &registers[DSReg];
		break;
	case ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE:
		// Register contains the base address, next word contains the offset
		baseAddress = registers[DSReg];
		increasePC();
		if (!vmbReadWordAt(registers[PC], &offset))
			return FALSE;
		//offset = getWordAt(registers[PC]);
		memoryWriteBack = baseAddress+offset;
		if (isByteInstruction) {
			*operand = malloc(sizeof(UINT8));
			if (!vmbReadByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)*operand))
				return FALSE;
		}
		else {
			*operand = malloc(sizeof(UINT16));
			if (!vmbReadWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)*operand))
				return FALSE;
		}
		break;
	case ADDR_MODE_INDIRECT_REGISTER:
		// Register contains the address of the operand
		memoryWriteBack = registers[DSReg];
		if (isByteInstruction) {
			*operand = malloc(sizeof(UINT8));
			if (!vmbReadByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)*operand))
				return FALSE;
		} else {
			*operand = malloc(sizeof(UINT16));
			if (!vmbReadWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)*operand))
				return FALSE;
		}
		break;
	case ADDR_MODE_INDIRECT_AUTOINC_IMMEDIATE:
		// Register contains the address. Register value will be incremented (byte/wyde)
		if (DSReg == PC)
			increasePC();
		if (!vmbReadWordAt(registers[DSReg], &baseAddress))
			return FALSE;
		memoryWriteBack = baseAddress;
		if (isByteInstruction) {
			*operand = malloc(sizeof(UINT8));
			if (!vmbReadByteAt((UINT16)(memoryWriteBack&0xFFFF), (UINT8*)*operand))
				return FALSE;
		} else {
			*operand = malloc(sizeof(UINT16));
			if (!vmbReadWordAt((UINT16)(memoryWriteBack&0xFFFF), (UINT16*)*operand))
				return FALSE;
		}
		//operand = getAddress(baseAddress);
		// Execute autoincrement
		if (DSReg != PC) {
			// PC is already incremented
			int incr;
			if (isByteInstruction)
				incr = 1;
			else
				incr = 2;
			registers[DSReg] += incr;
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

int decodeF3Instruction(UINT16 instruction, void **condition, void **offset) {
	unsigned int /*uiInstruction, */uiCondition;
	INT16 iOffset;

	uiCondition = (instruction & OPCODE_MASKS.FORMAT3.ConditionBitMask)
		>>OPCODE_MASKS.FORMAT3.ConditionBitMaskShift;
	iOffset = (instruction & OPCODE_MASKS.FORMAT3.OffsetBitMask)
		>>OPCODE_MASKS.FORMAT3.OffsetBitMaskShift;
	// Check for sign (offset has 10 bit)
	if (iOffset & 512)
		iOffset &= (-1);

	*condition = &uiCondition;
	*offset = &iOffset;

	return TRUE;
}

executorPtr findExecutor(UINT16 instructionCode) {
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

void increasePC() {
	registers[PC] += 2;
	return;
}

void setStatusBits(int *carry, int *zero, int *negative, int *arithmetic) {
	if (carry != NULL) {
		if (*carry)
			registers[SR] |= 0x1;
		else
			registers[SR] &= 0xFFFE;
	}

	if (zero != NULL) {
		if (*zero)
			registers[SR] |= 0x2;
		else
			registers[SR] &= 0xFFFD;
	}

	if (negative != NULL) {
		if (*negative)
			registers[SR] |= 0x4;
		else
			registers[SR] &= 0xFFFB;
	}

	if (arithmetic != NULL) {
		if (*arithmetic)
			registers[SR] |= 0x100;
		else
			registers[SR] &= 0xFEFF;
	}
}

int getCarryBit() {
	if (registers[SR] & 0x1)
		return TRUE;
	else
		return FALSE;
}
int getZeroBit() {
	if (registers[SR] & 0x2)
		return TRUE;
	else
		return FALSE;
}
int getNegativeBit() {
	if (registers[SR] & 0x4)
		return TRUE;
	else
		return FALSE;
}
int getArithmeticBit() {
	if (registers[SR] & 0x1FF)
		return TRUE;
	else
		return FALSE;
}


/*
	WD: Initializes core components
	and starts the execution at the address pointed by
	value @ 0xFFFE (interrupt vector table).
*/
void initCore(void) {
	// Initialize local storage
	initRegisters();
	// Initialize vmb interface
	initVMBInterface();
	
	vmbReadWordAt(EXECUTION_START_AT, &registers[PC]);		/* Initialize the programm 
														counter with the start address */
	/*
	// Initialize stack pointer to the top of RAM (must be in user programm)
	registers[SP] = RAM_START_AT + vmbGetRamSize() - 2;	
	*/
	executionLoop();
}


/*
	Initializes the registers
*/
void initRegisters() {
	//if (registers == NULL)
	//	registers = (UINT16*)malloc(sizeof(UINT16) * REGISTERS_COUNT);
	memset(&registers, 0, sizeof(UINT16) * REGISTERS_COUNT);
}

UINT16 fetchInstruction(UINT16 msp_address) {
	UINT16 result;
	if (vmbReadWordAt(msp_address, &result))
		return result;
	else
		return 0;
}

void executionLoop() {
	executorPtr executor;
	while (TRUE) {
		currentInstruction = fetchInstruction(registers[PC]);
		if (!currentInstruction)
			break;
		if (!decodeInstructionFormat(currentInstruction, &executor))
			break;
		if (executor == NULL || !executor(NULL))
			break;
	}
}

