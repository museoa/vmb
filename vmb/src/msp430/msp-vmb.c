#include "msp-vmb.h"

/*
	Argument opcode: pointer to the instruction word
*/
int ADD_executor (char **programmCounter) {
	int byte = FALSE;
	char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	unsigned int instruction = wordToUInt(instructionWordPtr);
	char *source;
	char *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;

	// Byte or Word?
	if ((instruction & OPCODE_MASKS.FORMAT1.BWBitMask) > 0) {
		byte = TRUE;
	}

	if (!getF1Operands(instructionWordPtr, byte, source, destination))
		return 0;

	// Execute
	if (byte) {
		iSource = byteToInt(source);
		uiSource = byteToUInt(source);
		iDestination = byteToInt(destination);
		uiDestination = byteToUInt(destination);
		result = uiSource + uiDestination;
		destination[0] = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(source);
		iDestination = wordToInt(destination);
		uiSource = wordToUInt(source);
		uiDestination = wordToUInt(destination);
		result = uiSource + uiDestination;
		destination[0] = (char)(result & 0xFF);
		destination[1] = (char)((result & 0xFF00)>>8);
	}
	// (N-bit)
	if (!byte) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bitNegative = TRUE;
	else
		bitNegative = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bitZero = TRUE;
	else
		bitZero = FALSE;

	// (C-bit)
	if (byte) compCarry |= (compCarry <<= 8);
	if ((result & (~compCarry)) != 0) {
		bitCarry = TRUE;
	}
	else {
		bitCarry = FALSE;
	}
	
	// (V-bit)
	if (((iSource > 0) && (iDestination > 0) && (bitNegative)) 
		|| ((iSource < 0) && (iDestination < 0) && (!bitNegative))) {
			bitArithmeticException = TRUE;
	}
	else {
		bitArithmeticException = FALSE;
	}

	// PC to next word
	registers[PC*2] += 2;
	return 1;
}

int ADDC_executor (char **programmCounter) {
	int byte = FALSE;
	char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	unsigned int instruction = wordToUInt(instructionWordPtr);
	char *source;
	char *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;

	// Byte or word?
	if ((instruction & OPCODE_MASKS.FORMAT1.BWBitMask) > 0) {
		byte = TRUE;
	}

	if (!getF1Operands(instructionWordPtr, byte, source, destination))
		return 0;

	// Execute
	if (byte) {
		iSource = byteToInt(source);
		uiSource = byteToUInt(source);
		iDestination = byteToInt(destination);
		uiDestination = byteToUInt(destination);
		result = uiSource + uiDestination + bitCarry;
		destination[0] = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(source);
		iDestination = wordToInt(destination);
		uiSource = wordToUInt(source);
		uiDestination = wordToUInt(destination);
		result = uiSource + uiDestination + bitCarry;
		destination[0] = (char)(result & 0xFF);
		destination[1] = (char)((result & 0xFF00)>>8);
	}
	// (N-bit)
	if (!byte) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bitNegative = TRUE;
	else
		bitNegative = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bitZero = TRUE;
	else
		bitZero = FALSE;

	// (C-bit)
	if (byte) compCarry |= (compCarry <<= 8);
	if ((result & (~compCarry)) != 0) {
		bitCarry = TRUE;
	}
	else {
		bitCarry = FALSE;
	}
	
	// (V-bit)
	if (((iSource > 0) && (iDestination > 0) && (bitNegative)) 
		|| ((iSource < 0) && (iDestination < 0) && (!bitNegative))) {
			bitArithmeticException = TRUE;
	}
	else {
		bitArithmeticException = FALSE;
	}
	// PC to next word
	registers[PC*2] += 2;
	return 1;
}

int AND_executor (char **programmCounter) {
	int byte = FALSE;
	char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	unsigned int instruction = wordToUInt(instructionWordPtr);
	char *source;
	char *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;

	// Byte or word?
	if ((instruction & OPCODE_MASKS.FORMAT1.BWBitMask) > 0) {
		byte = TRUE;
	}

	if (!getF1Operands(instructionWordPtr, byte, source, destination))
		return 0;

	// Execute
	if (byte) {
		iSource = byteToInt(source);
		uiSource = byteToUInt(source);
		iDestination = byteToInt(destination);
		uiDestination = byteToUInt(destination);
		result = uiSource & uiDestination;
		destination[0] = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(source);
		iDestination = wordToInt(destination);
		uiSource = wordToUInt(source);
		uiDestination = wordToUInt(destination);
		result = uiSource & uiDestination;
		destination[0] = (char)(result & 0xFF);
		destination[1] = (char)((result & 0xFF00)>>8);
	}
	// (N-bit)
	if (!byte) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bitNegative = TRUE;
	else
		bitNegative = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bitZero = TRUE;
	else
		bitZero = FALSE;

	// Carry bit is set, if result != ZERO
	if (bitZero)
		bitCarry = FALSE;
	else
		bitCarry = TRUE;
	
	// V-bit: always reset
	bitArithmeticException = FALSE;

	// PC to next word
	registers[PC*2] += 2;
	return 1;
}

int BIC_executor (char **programmCounter) {
	int byte = FALSE;
	char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	unsigned int instruction = wordToUInt(instructionWordPtr);
	char *source;
	char *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;

	// Byte or Word?
	if ((instruction & OPCODE_MASKS.FORMAT1.BWBitMask) > 0) {
		byte = TRUE;
	}

	if (!getF1Operands(instructionWordPtr, byte, source, destination))
		return 0;

	// Execute
	if (byte) {
		iSource = byteToInt(source);
		uiSource = byteToUInt(source);
		iDestination = byteToInt(destination);
		uiDestination = byteToUInt(destination);
		result = (~uiSource) & uiDestination;
		destination[0] = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(source);
		iDestination = wordToInt(destination);
		uiSource = wordToUInt(source);
		uiDestination = wordToUInt(destination);
		result = (~uiSource) & uiDestination;
		destination[0] = (char)(result & 0xFF);
		destination[1] = (char)((result & 0xFF00)>>8);
	}
	
	// Status bits are not affected
	
	// PC to next word
	registers[PC*2] += 2;
	return 1;
}

int BIS_executor (char **programmCounter) {
	int byte = FALSE;
	char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	unsigned int instruction = wordToUInt(instructionWordPtr);
	char *source;
	char *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;

	// Byte or Word?
	if ((instruction & OPCODE_MASKS.FORMAT1.BWBitMask) > 0) {
		byte = TRUE;
	}

	if (!getF1Operands(instructionWordPtr, byte, source, destination))
		return 0;

	// Execute
	if (byte) {
		iSource = byteToInt(source);
		uiSource = byteToUInt(source);
		iDestination = byteToInt(destination);
		uiDestination = byteToUInt(destination);
		result = uiSource | uiDestination;
		destination[0] = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(source);
		iDestination = wordToInt(destination);
		uiSource = wordToUInt(source);
		uiDestination = wordToUInt(destination);
		result = uiSource | uiDestination;
		destination[0] = (char)(result & 0xFF);
		destination[1] = (char)((result & 0xFF00)>>8);
	}
	
	// Status bits are not affected
	
	// PC to next word
	registers[PC*2] += 2;
	return 1;
}

int BIT_executor (char **programmCounter) {
	int byte = FALSE;
	char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	unsigned int instruction = wordToUInt(instructionWordPtr);
	char *source;
	char *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;

	// Byte or Word?
	if ((instruction & OPCODE_MASKS.FORMAT1.BWBitMask) > 0) {
		byte = TRUE;
	}

	if (!getF1Operands(instructionWordPtr, byte, source, destination))
		return 0;

	// Execute
	if (byte) {
		iSource = byteToInt(source);
		uiSource = byteToUInt(source);
		iDestination = byteToInt(destination);
		uiDestination = byteToUInt(destination);
		result = uiSource & uiDestination;
	} else {
		iSource = wordToInt(source);
		iDestination = wordToInt(destination);
		uiSource = wordToUInt(source);
		uiDestination = wordToUInt(destination);
		result = uiSource & uiDestination;
	}
	
	// Only status bits affected
	// (N-bit)
	if (!byte) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bitNegative = TRUE;
	else
		bitNegative = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bitZero = TRUE;
	else
		bitZero = FALSE;

	// Carry bit is set, if result != ZERO
	if (bitZero)
		bitCarry = FALSE;
	else
		bitCarry = TRUE;
	
	// V-bit: always reset
	bitArithmeticException = FALSE;
	
	// PC to next word
	registers[PC*2] += 2;
	return 1;
}

int CALL_executor (char **programmCounter) {
	return 0;
}

int CMP_executor (char **programmCounter) {
	int byte = FALSE;
	char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	unsigned int instruction = wordToUInt(instructionWordPtr);
	char *source;
	char *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;

	// Byte or Word?
	if ((instruction & OPCODE_MASKS.FORMAT1.BWBitMask) > 0) {
		byte = TRUE;
	}

	if (!getF1Operands(instructionWordPtr, byte, source, destination))
		return 0;

	// Execute
	if (byte) {
		iSource = byteToInt(source);
		uiSource = byteToUInt(source);
		iDestination = byteToInt(destination);
		uiDestination = byteToUInt(destination);
		result = uiSource - uiDestination;
	} else {
		iSource = wordToInt(source);
		iDestination = wordToInt(destination);
		uiSource = wordToUInt(source);
		uiDestination = wordToUInt(destination);
		result = uiSource - uiDestination;
	}
	
	// Only status bits affected
	// (N-bit)
	if (!byte) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bitNegative = TRUE;
	else
		bitNegative = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bitZero = TRUE;
	else
		bitZero = FALSE;

	// C-bit
	if (byte) compCarry |= (compCarry <<= 8);
	if ((result & (~compCarry)) != 0) {
		bitCarry = TRUE;
	}
	else {
		bitCarry = FALSE;
	}
	
	// (V-bit)
	if (((iSource > 0) && (iDestination < 0) && (bitNegative)) 
		|| ((iSource < 0) && (iDestination > 0) && (!bitNegative))) {
			bitArithmeticException = TRUE;
	}
	else {
		bitArithmeticException = FALSE;
	}
	
	// PC to next word
	registers[PC*2] += 2;
	return 1;
}

int DADD_executor (char **programmCounter) {
	int byte = FALSE;
	char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	unsigned int instruction = wordToUInt(instructionWordPtr);
	char *source;
	char *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;
	unsigned int uiDigit, uiDigitMask = 0xF;
	int carry, i, maxI;

	// Byte or Word?
	if ((instruction & OPCODE_MASKS.FORMAT1.BWBitMask) > 0) {
		byte = TRUE;
	}

	if (!getF1Operands(instructionWordPtr, byte, source, destination))
		return 0;

	// Execute
	if (byte) {
		maxI = 2;
		uiSource = byteToUInt(source);
		uiDestination = byteToUInt(destination);
	}
	else {
		maxI = 4;
		uiSource = wordToUInt(source);
		uiDestination = wordToUInt(destination);
	}

	carry = FALSE;
	i = 0;
	result = 0;
	carry = bitCarry;
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
		
	destination[0] = (char)(result & 0xFF);
	if (!byte)
		destination[1] = (char)((result & 0xFF00)>>8);

	// (N-bit)
	if (!byte) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bitNegative = TRUE;
	else
		bitNegative = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bitZero = TRUE;
	else
		bitZero = FALSE;

	// (C-bit)
	bitCarry = carry;
	
	// (V-bit undefined)
	bitArithmeticException = FALSE;

	// PC to next word
	registers[PC*2] += 2;
	return 1;
}

int JUMP_executor (char **programmCounter) {
	return 0;
}

int MOV_executor (char **programmCounter) {
	int byte = FALSE;
	char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	unsigned int instruction = wordToUInt(instructionWordPtr);
	char *source;
	char *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;

	// Byte or Word?
	if ((instruction & OPCODE_MASKS.FORMAT1.BWBitMask) > 0) {
		byte = TRUE;
	}

	if (!getF1Operands(instructionWordPtr, byte, source, destination))
		return 0;

	// Execute
	destination[0] = source[0];
	if (!byte)
		destination[1] = source[1];

	// Status bits are not affected

	// PC to next word
	registers[PC*2] += 2;
	return 1;
}

int PUSH_executor (char **programmCounter) {
	return 0;
}

int RETI_executor (char **programmCounter) {
	return 0;
}

int RRA_executor (char **programmCounter) {
	return 0;
}

int RRC_executor (char **programmCounter) {
	return 0;
}

int SUB_executor (char **programmCounter) {
	int byte = FALSE;
	char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	unsigned int instruction = wordToUInt(instructionWordPtr);
	char *source;
	char *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;

	// Byte oder Word?
	if ((instruction & OPCODE_MASKS.FORMAT1.BWBitMask) > 0) {
		byte = TRUE;
	}

	if (!getF1Operands(instructionWordPtr, byte, source, destination))
		return 0;

	// Execute
	if (byte) {
		iSource = byteToInt(source);
		iDestination = byteToInt(destination);
		uiSource = byteToUInt(source);
		uiDestination = byteToUInt(destination);
		result = uiSource - uiDestination;
		destination[0] = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(source);
		iDestination = wordToInt(destination);
		uiSource = wordToUInt(source);
		uiDestination = wordToUInt(destination);
		result = uiSource - uiDestination;
		destination[0] = (char)(result & 0xFF);
		destination[1] = (char)((result & 0xFF00)>>8);
	}

	// Ist Ergebnis negativ? (N-bit)
	if (!byte) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bitNegative = TRUE;
	else
		bitNegative = FALSE;

	// Ist Ergebnis 0? (Z-bit)
	if ((result & 0xFFFF) == 0)
		bitZero = TRUE;
	else
		bitZero = FALSE;

	// Wurde ein Übertrag erzeugt (C-bit)
	if (byte) compCarry |= (compCarry <<= 8);
	if ((result & (~compCarry)) != 0) {
		bitCarry = TRUE;
	}
	else {
		bitCarry = FALSE;
	}
	
	// Wurde ein arithmetischer Fehler ausgelöst? (V-bit)
	if (((iSource > 0) && (iDestination < 0) && (bitNegative)) 
		|| ((iSource < 0) && (iDestination > 0) && (!bitNegative))) {
			bitArithmeticException = TRUE;
	}
	else {
		bitArithmeticException = FALSE;
	}

	// PC zum nächsten Wort
	registers[PC*2] += 2;
	return 1;
}

int SUBC_executor (char **programmCounter) {
	int byte = FALSE;
	char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	unsigned int instruction = wordToUInt(instructionWordPtr);
	char *source;
	char *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;

	// Byte or Word?
	if ((instruction & OPCODE_MASKS.FORMAT1.BWBitMask) > 0) {
		byte = TRUE;
	}

	if (!getF1Operands(instructionWordPtr, byte, source, destination))
		return 0;

	// Execute
	if (byte) {
		iSource = byteToInt(source);
		iDestination = byteToInt(destination);
		uiSource = byteToUInt(source);
		uiDestination = byteToUInt(destination);
		result = uiSource - uiDestination + bitCarry;
		destination[0] = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(source);
		iDestination = wordToInt(destination);
		uiSource = wordToUInt(source);
		uiDestination = wordToUInt(destination);
		result = uiSource - uiDestination + bitCarry;
		destination[0] = (char)(result & 0xFF);
		destination[1] = (char)((result & 0xFF00)>>8);
	}

	// (N-bit)
	if (!byte) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bitNegative = TRUE;
	else
		bitNegative = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bitZero = TRUE;
	else
		bitZero = FALSE;

	// (C-bit)
	if (byte) compCarry |= (compCarry <<= 8);
	if ((result & (~compCarry)) != 0) {
		bitCarry = TRUE;
	}
	else {
		bitCarry = FALSE;
	}
	
	// (V-bit)
	if (((iSource > 0) && (iDestination < 0) && (bitNegative)) 
		|| ((iSource < 0) && (iDestination > 0) && (!bitNegative))) {
			bitArithmeticException = TRUE;
	}
	else {
		bitArithmeticException = FALSE;
	}

	// PC to next word
	registers[PC*2] += 2;
	return 1;
}

int SWBP_executor (char **programmCounter) {
	return 0;
}

int SXT_executor (char **programmCounter) {
	return 0;
}

int XOR_executor (char **programmCounter) {
	int byte = FALSE;
	char *instructionWordPtr = getAddress(wordToUInt(registers + PC*2));
	unsigned int instruction = wordToUInt(instructionWordPtr);
	char *source;
	char *destination;
	unsigned int compNegative = 0x80;
	unsigned int compCarry = 0xFF;
	unsigned int result;
	int iSource, iDestination;
	unsigned int uiSource, uiDestination;

	// Byte or Word?
	if ((instruction & OPCODE_MASKS.FORMAT1.BWBitMask) > 0) {
		byte = TRUE;
	}

	if (!getF1Operands(instructionWordPtr, byte, source, destination))
		return 0;

	// Execute
	if (byte) {
		iSource = byteToInt(source);
		uiSource = byteToUInt(source);
		iDestination = byteToInt(destination);
		uiDestination = byteToUInt(destination);
		result = uiSource ^ uiDestination;
		destination[0] = (char)(result & 0xFF);
	} else {
		iSource = wordToInt(source);
		iDestination = wordToInt(destination);
		uiSource = wordToUInt(source);
		uiDestination = wordToUInt(destination);
		result = uiSource ^ uiDestination;
		destination[0] = (char)(result & 0xFF);
		destination[1] = (char)((result & 0xFF00)>>8);
	}
	// (N-bit)
	if (!byte) compNegative <<= 8;
	if ((result & compNegative) > 0)
		bitNegative = TRUE;
	else
		bitNegative = FALSE;

	// (Z-bit)
	if ((result & 0xFFFF) == 0)
		bitZero = TRUE;
	else
		bitZero = FALSE;

	// Carry bit is set, if result != ZERO
	bitCarry = !bitZero;
	
	// V-bit: set if both operands are negative
	bitArithmeticException = (iSource < 0 && iDestination < 0);

	// PC to next word
	registers[PC*2] += 2;
	return 1;
}

unsigned int wordToUInt(char *word) {
	return word[0]|(word[1]<<8);
}

int wordToInt(char *word) {
	int result = word[0]|(word[1]<<8);
	// Wenn negativ - mit bits auffüllen
	if ((result & 0x8000) != 0) {
		result |= (~0xFFFF);
	}
	return result;
}

int byteToInt(char *byte) {
	int result = byte[0];
	// Wenn negativ - mit bits auffüllen
	if ((result & 0x80) != 0) {
		result |= (~0xFF);
	}
	return result;
}

unsigned int byteToUInt(char *byte) {
	return byte[0];
}

unsigned int getInstructionCode(char *opcode) {
	unsigned int i_opcode;
	int instructionCode;

	i_opcode = opcode[0]|(opcode[1]<<8);

	instructionCode = (i_opcode & OPCODE_MASKS.FORMAT1.CommandBitMask)>>OPCODE_MASKS.FORMAT1.CommandBitMaskShift;
	
	// Decode instruction code
	switch (instructionCode) {
	case 1:		// Format 2
		instructionCode = (i_opcode & OPCODE_MASKS.FORMAT2.CommandBitMask)>>OPCODE_MASKS.FORMAT2.CommandBitMaskShift;
		break;
	case 2:	// Format 3
		instructionCode = (i_opcode & OPCODE_MASKS.FORMAT3.CommandBitMask)>>OPCODE_MASKS.FORMAT3.CommandBitMaskShift;
		break;
	case 3: // Format 3
		instructionCode = (i_opcode & OPCODE_MASKS.FORMAT3.CommandBitMask)>>OPCODE_MASKS.FORMAT3.CommandBitMaskShift;
		break;
	default:	// Format 1
		break;
	}

	return instructionCode;
}

int getF1Operands(unsigned int opcode, int byte, char *source, char *destination) {
	unsigned int As, Ad, sReg, dReg;
	char * constant = (char*)malloc(2);
	memset(constant,0,2);
	
	// Operanden holen: Source
	As = ((opcode & OPCODE_MASKS.FORMAT1.AsBitMask)>>OPCODE_MASKS.FORMAT1.AsBitMaskShift);
	sReg = ((opcode & OPCODE_MASKS.FORMAT1.SRegBitMask)>>OPCODE_MASKS.FORMAT1.SRegBitMaskShift);
	if (sReg == CG1) {
		if (As == 0) {			// PC-Register
			source = registers + PC*2;
		}
		else {
			if (As == ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE) {			// Kostante 0 (for absolute addressing)
				unsigned int offset = wordToUInt(increasePC());
				source = getAddress(offset);
			} else if (As == 2) {	// Kostante 4
				*constant = 4;
			} else if (As == 3) {	// Kostante 8
				*constant = 8;
			}
			source = constant;
		}
	} else if (sReg == CG2) {
		if (As == 0) {			// Kostante 0 (word processing)
			source = constant;
		} else if (As == 1) {	// Kostante 1
			*constant = 1;
		} else if (As == 2) {	// Kostante 2
			*constant = 2;
		} else if (As == 3) {	// Kostante -1
			*constant = -1;
		}
		source = constant;
	} else {

		if (As == ADDR_MODE_REGISTER) {
			source = 
				&registers[sReg * 2];
		} else if (As == ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE) {
			// Das nächste Wort enthält den Offset
			// PC erhöhen und Offset lesen
			unsigned int baseAddress;
			unsigned int offset;

			baseAddress = wordToUInt(registers + sReg*2);
			offset = wordToUInt(increasePC());
			source = getAddress(baseAddress + offset);

		} else if (As == ADDR_MODE_INDIRECT_REGISTER) {
			source = getAddress(wordToUInt(registers + sReg*2));
		} else if (As == ADDR_MODE_INDIRECT_AUTOINC_IMMEDIATE) {
			unsigned int uintToIncrement = wordToUInt(registers + sReg*2);
			if (sReg == PC)
				source = getAddress(uintToIncrement+2);
			else
				source = getAddress(uintToIncrement);
		
			// Execute Autoincrement
			if (byte && (sReg != PC))
				uintToIncrement += 1;
			else
				uintToIncrement += 2;
			uintToIncrement++;
			registers[sReg*2] = uintToIncrement & 0xFF;
			registers[sReg*2+1] = (uintToIncrement & 0xFF00)>>8;
		} else {
			source = NULL;
		}
	}
	
	// Operanden holen: Destination
	Ad = (opcode & OPCODE_MASKS.FORMAT1.AdBitMask)>>OPCODE_MASKS.FORMAT1.AdBitMaskShift;
	dReg = ((opcode & OPCODE_MASKS.FORMAT1.DRegBitMask)>>OPCODE_MASKS.FORMAT1.DRegBitMaskShift);
	if (Ad == ADDR_MODE_REGISTER) {
		destination = registers + dReg * 2;
	} else if (Ad == ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE) {
		// PC erhöhen
		unsigned int baseAddress;
		unsigned int offset;

		baseAddress = wordToUInt(registers + dReg*2);
		offset = wordToUInt(increasePC());
		destination = getAddress(baseAddress + offset);
	} else {
		destination = NULL;
	}
	
	if (source == NULL || destination == NULL)
		return 0;
	else
		return 1;
}

char* getAddress(unsigned int mspAdress) {
	// Hier findet das Mapping statt

	return NULL;
}

char* increasePC() {
	registers[PC] = registers[PC]+2;
	return getAddress(registers[PC]);
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



int main() {
	//printf("0x%X",OPCODE_MASKS.FORMAT1.CommandBitMask);
	char *opcode = (char*)malloc(2);
	char **pc = &opcode;
	int (*executor)(char**);
	char c1 = 127;
	char c2 = 127;
	char c3 = c1+c2;
	registers = (char*)malloc(2*REGISTERS_COUNT);

	////opcode = (char*)malloc(2);
	////0x5142 in little-endian (ADD.B R1,R2)
	//opcode[0] = 0x42;
	//opcode[1] = 0x51;
	//executor = findExecutor(getInstructionCode(opcode));
	//if (executor == NULL || !executor(pc)) {
	//	printf("Error on executing @ %d",opcode);
	//	return 1;
	//}
	
	printf("c1+c2 = %d",c3);
	
	return 0;
}