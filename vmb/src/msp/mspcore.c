/*
	
*/

#include "mspcore.h"


// Version parameters
char version[] = "1.0";
char howto[] = "MSP430 Simulator for VMB\n";
static msp_word breakpoints[MAX_BREAKPOINTS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static int continuosMode = FALSE;
static msp_word currentInstruction = {0};
static INT32 memoryWriteBack = MEMORY_WRITEBACK_NO;
unsigned int compNegative = 0x80;
unsigned int compCarryByte = ~BYTE_MASK;
unsigned int compCarryWord = ~WORD_MASK;
int x = FALSE;		// execution flag

int ADD_executor () {
	int isByteInstruction;
	void *source;
	void *destination;
	
	UINT32 result = 0;
	UINT16 uiSource, uiDestination;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;
	// Execute
	if (isByteInstruction) {
		uiSource = *(UINT8*)source;
		uiDestination = *(UINT8*)destination;
		result = uiSource + uiDestination;
		IntToByteByPtr(result,destination);
	} else {
		uiSource = *(UINT16*)source;
		uiDestination = *(UINT16*)destination;
		result = uiSource + uiDestination;
		IntToWordByPtr(result,destination);
	}
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	nBit = ((result & compNegative) != 0);

	// (Z-bit)
	zBit = (!(result & WORD_MASK));
	
	// C-Bit
	checkCBit(result, isByteInstruction);
	
	// (V-bit)
	vBit = (((!(uiSource & compNegative)) && (!(uiDestination & compNegative)) && (nBit)) 
		|| ((uiSource & compNegative) && (uiDestination & compNegative) && (!nBit)));

	if (!writeBack(memoryWriteBack, destination, (!isByteInstruction)+1))
		return FALSE;

	// PC to next word
	increasePC();
	clocks++;
	return TRUE;
}

int ADDC_executor () {
	int isByteInstruction;
	void *source, *destination;
	UINT32 result;
	UINT16 uiSource, uiDestination;
	
	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = uiSource + uiDestination + cBit;
		IntToByteByPtr(result,destination);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = uiSource + uiDestination + cBit;
		IntToWordByPtr(result,destination);
	}
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	nBit = (result & compNegative);
	// (Z-bit)
	zBit= (!(result & WORD_MASK));
	// (C-bit)
	checkCBit(result, isByteInstruction);

	
	// (V-bit)
	vBit= ((!(uiSource & compNegative) && !(uiDestination & compNegative) && (nBit)) 
	     || ((uiSource & compNegative) && (uiDestination & compNegative) && (!nBit)));

	if (!writeBack(memoryWriteBack, destination, (!isByteInstruction)+1))
		return FALSE;

	// PC to next word
	increasePC();
	clocks++;
	return TRUE;
}

int AND_executor () {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	UINT16 result;
	UINT16 uiSource, uiDestination;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = uiSource & uiDestination;
		IntToByteByPtr(result,destination);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = uiSource & uiDestination;
		IntToWordByPtr(result,destination);
	}
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	nBit = (result & compNegative);

	// (Z-bit)
	zBit = (!(result & WORD_MASK));

	// Carry bit is set, if result != ZERO
	cBit = !zBit;
	
	// V-bit: always reset
	vBit = 0;

	if (!writeBack(memoryWriteBack, destination, (!isByteInstruction)+1))
		return FALSE;

	// PC to next word
	increasePC();
	clocks++;
	return TRUE;
}

int BIC_executor () {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	UINT16 result;
	UINT16 uiSource, uiDestination;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = (~uiSource) & uiDestination;
		IntToByteByPtr(result,destination);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = (~uiSource) & uiDestination;
		IntToWordByPtr(result,destination);
	}
	
	// Status bits are not affected
	if (!writeBack(memoryWriteBack, destination, (!isByteInstruction)+1))
		return FALSE;

	// PC to next word
	increasePC();
	clocks++;
	return TRUE;
}



int BIS_executor () {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	UINT16 result;
	UINT16 uiSource, uiDestination;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = uiSource | uiDestination;
		IntToByteByPtr(result,destination);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = uiSource | uiDestination;
		IntToWordByPtr(result,destination);
	}
	
	// Status bits are not affected

	if (!writeBack(memoryWriteBack, destination, (!isByteInstruction)+1))
		return FALSE;
	
	// PC to next word
	increasePC();
	clocks++;
	return TRUE;
}

int BIT_executor () {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	UINT16 result;
	UINT16 uiSource, uiDestination;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = uiSource & uiDestination;
		IntToByteByPtr(result,destination);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = uiSource & uiDestination;
		IntToWordByPtr(result,destination);
	}
	
	// Only status bits affected
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	nBit = (result & compNegative);

	// (Z-bit)
	zBit = (!(result & WORD_MASK));

	// Carry bit is set, if result != ZERO
	cBit = !zBit;
	
	// V-bit: always reset
	vBit = FALSE;

	if (!writeBack(memoryWriteBack, destination, (!isByteInstruction)+1))
		return FALSE;
	
	// PC to next word
	increasePC();
	clocks++;
	return TRUE;
}

int CALL_executor () {
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
	registers[SP].asWord -= 2;
	pcForStack = registers[PC].asWord + 2;
	if (!vmbWriteWordAt(registers[SP], &pcForStack))
		return FALSE;
	registers[PC].asWord = operandValue;
	clocks++;
	return TRUE;
}

int CMP_executor () {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	unsigned int result;
	UINT16 uiSource, uiDestination;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = uiSource - uiDestination;
		IntToByteByPtr(result,destination);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = uiSource - uiDestination;
		IntToWordByPtr(result,destination);
	}
	
	// Only status bits affected
	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	nBit = (result & compNegative);

	// (Z-bit)
	zBit = (!(result & WORD_MASK));

	// C-bit
	checkCBit(result, isByteInstruction);

	
	// (V-bit)
	vBit = ((!(uiSource & compNegative) && !(uiDestination & compNegative) && (nBit)) 
		|| ((uiSource & compNegative) && (uiDestination & compNegative) && (!nBit)));

	if (!writeBack(memoryWriteBack, destination, (!isByteInstruction)+1))
		return FALSE;
	
	// PC to next word
	increasePC();
	clocks++;
	return TRUE;
}

int DADD_executor () {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	unsigned int result;
	UINT16 uiSource, uiDestination;
	UINT16 uiDigit, uiDigitMask = 0xF;
	int carry, i, maxI;

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

	i = 0;
	result = 0;
	carry = cBit;
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
		IntToByteByPtr(result,destination);
	} else {
		IntToWordByPtr(result,destination);
	}

	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	nBit = (result & compNegative) != 0;

	// (Z-bit)
	zBit = (!(result & WORD_MASK));

	// (C-bit)
	cBit = carry;
	
	// (V-bit undefined)
	vBit = 0;

	if (!writeBack(memoryWriteBack, destination, (!isByteInstruction)+1))
		return FALSE;
	
	// PC to next word
	increasePC();
	clocks++;
	return TRUE;
}

int JUMP_executor () {
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
		if (!zBit)
			jump = TRUE;
		break;
	case JEQ:
		if (zBit)
			jump = TRUE;
		break;
	case JNC:
		if (!cBit)
			jump = TRUE;
		break;
	case JC:
		if (cBit)
			jump = TRUE;
		break;
	case JN:
		if (nBit)
			jump = TRUE;
		break;
	case JGE:
		if (!(nBit ^ vBit))
			jump = TRUE;
		break;
	case JL:
		if (nBit ^ vBit)
			jump = TRUE;
		break;
	case JMP:
		jump = TRUE;
		break;
	default:
		break;
	}

	increasePC();
	if (jump)
		registers[PC].asWord += 2*iOffset;

	clocks++;
	return TRUE;
}

int MOV_executor () {
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

	if (!writeBack(memoryWriteBack, destination, (!isByteInstruction)+1))
		return FALSE;

	// PC to next word
	increasePC();
	clocks++;
	return TRUE;
}

int PUSH_executor () {
	/*
		Decreases stack pointer by 2 and pushes a byte or word to the stack.
	*/
	void *operand = NULL;
	UINT16 operandValue = 0;
	int isByteInstruction = FALSE;

	if (!decodeF2Instruction(currentInstruction, &operand, &isByteInstruction))
		return FALSE;
	
	// Update stack pointer
	registers[SP].asWord -= 2;

	if (isByteInstruction) {
		operandValue = *(UINT8*)operand;
	} else {
		operandValue = *(UINT16*)operand;
	}

	// Status bits are not affected

	if (!vmbWriteWordAt(registers[SP], &operandValue))
		return FALSE;

	increasePC();
	clocks++;
	return TRUE;
}

int RETI_executor () {
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
	registers[SR].asWord = stack;
	registers[SP].asWord -= 2;
	// Restore programm counter
	if (!vmbReadWordAt(registers[SP], &stack))
		return FALSE;
	registers[PC].asWord = stack;
	registers[SP].asWord -= 2;
	clocks++;
	return TRUE;
}

int RRA_executor () {
	/*
		Rotates the operand to the right (arithmetically):
		MSB->MSB, MSB->MSB-1,...,LSB+1->LSB,LSB->C
	*/
	int bC = FALSE;
	int msb = FALSE;
	int isByteInstruction = FALSE;
	void *operand;

	if (!decodeF2Instruction(currentInstruction, &operand, &isByteInstruction))
		return FALSE;
	
	if (isByteInstruction) {
		UINT8 operandValue;
		operandValue = *(UINT8*)operand;
		
		nBit = (operandValue & 0x80);
		msb = nBit;
		zBit = (!operandValue);
		
		cBit = ((operandValue & 0x1) > 0);

		operandValue >>= 1;
		if (msb)
			operandValue |= 0x80;
		*(UINT8*)operand = operandValue;
	} else {
		UINT16 operandValue;
		operandValue = *(UINT16*)operand;
		nBit = (operandValue & 0x8000);
		msb = nBit;
		cBit = (operandValue & 0x1);
		zBit = (!operandValue);

		operandValue >>= 1;
		if (msb)
			operandValue |= 0x8000;
		*(UINT16*)operand = operandValue;
	}

	vBit = 0;

	if (!writeBack(memoryWriteBack, operand, (!isByteInstruction)+1))
		return FALSE;

	increasePC();
	clocks++;
	return TRUE;
}

int RRC_executor () {
	/*
		Rotates the operand right with carry:
		C->MSB, ..., LSB->C
	*/
	int lsb = FALSE;
	int isByteInstruction = FALSE;
	void *operand;

	if (!decodeF2Instruction(currentInstruction, &operand, &isByteInstruction))
		return FALSE;

	if (isByteInstruction) {
		UINT8 operandValue;
		operandValue = *(UINT8*)operand;
		if ((operandValue & 0x1) > 0)
			lsb = TRUE;

		operandValue >>= 1;
		if (cBit)
			operandValue |= 0x80;
		nBit = cBit;
		cBit = lsb;
		zBit = (!operandValue);
		*(UINT8*)operand = operandValue;
	} else {
		UINT16 operandValue;
		operandValue = *(UINT16*)operand;
		lsb = ((operandValue & 0x1) > 0);

		operandValue >>= 1;
		if (cBit)
			operandValue |= 0x8000;
		nBit = cBit;
		cBit = lsb;
		zBit = (!operandValue);
		*(UINT16*)operand = operandValue;
	}

	vBit = 0;

	if (!writeBack(memoryWriteBack, operand, (!isByteInstruction)+1))
		return FALSE;

	increasePC();
	clocks++;
	return TRUE;
}

int SUB_executor () {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	UINT16 result = 0;
	UINT16 uiSource, uiDestination;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result =  uiDestination - uiSource;
		IntToByteByPtr(result,destination);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result =  uiDestination - uiSource;
		IntToWordByPtr(result,destination);
	}

	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	nBit = ((result & compNegative) > 0);

	// (Z-bit)
	zBit = ((result & WORD_MASK) == 0);

	// (C-bit)
	checkCBit(result, isByteInstruction);

	
	// (V-bit)
	vBit = (((uiSource & compNegative) && !(uiDestination & compNegative) && (nBit)) // pos - neg = neg?
		|| (!(uiSource & compNegative) && (uiDestination & compNegative) && (!nBit)));		// neg - pos = pos?

	if (!writeBack(memoryWriteBack, destination, (!isByteInstruction)+1))
		return FALSE;
	
	// PC to next word
	increasePC();
	clocks++;
	return TRUE;
}

int SUBC_executor () {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	UINT16 result;
	UINT16 uiSource, uiDestination;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result =  uiDestination - uiSource - 1 + cBit;
		IntToByteByPtr(result,destination);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result =  uiDestination - uiSource - 1 + cBit;
		*(UINT16*)destination = (UINT16)(result & BYTE_MASK);
	}

	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	nBit = (result & compNegative);

	// (Z-bit)
	zBit = (!(result & WORD_MASK));

	// (C-bit)
	checkCBit(result, isByteInstruction);
	
	// (V-bit)
	vBit = (((uiSource & compNegative) && !(uiDestination & compNegative) && (nBit))	// pos - neg = neg
		|| (!(uiSource & compNegative) && (uiDestination & compNegative) && (!nBit)));	// neg - pos = pos

	if (!writeBack(memoryWriteBack, destination, (!isByteInstruction)+1))
		return FALSE;
	
	// PC to next word
	increasePC();
	clocks++;
	return TRUE;
}

int SWPB_executor () {
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

	if (!writeBack(memoryWriteBack, operand, 2))
		return FALSE;
	clocks++;
	return TRUE;
}

int SXT_executor () {
	/*
		Extends the sign of the lower byte to the higher byte
	*/
	int isByteInstruction = FALSE;
	void *operand;
	UINT16 operandValue;

	if (!decodeF2Instruction(currentInstruction, &operand, &isByteInstruction))
		return FALSE;

	operandValue = *(UINT16*)operand;

	if (operandValue & 0x80) {
		operandValue |= 0xFF00;
		nBit = TRUE;
	}
	else {
		operandValue &= 0x00FF;
		nBit = FALSE;
	}

	zBit = (!operandValue);

	cBit = !zBit;

	vBit = 0;

	*(UINT16*)operand = operandValue;

	if (!writeBack(memoryWriteBack, operand, 2))
		return FALSE;
	clocks++;
	return TRUE;
}

int XOR_executor () {
	int isByteInstruction = FALSE;
	void *source;
	void *destination;
	UINT16 result;
	UINT16 uiSource, uiDestination;

	if (!decodeF1Instruction(currentInstruction, &source, &destination, &isByteInstruction))
		return FALSE;

	// Execute
	if (isByteInstruction) {
		uiSource = (*(UINT8*)source);
		uiDestination = (*(UINT8*)destination);
		result = uiSource ^ uiDestination;
		IntToByteByPtr(result,destination);
	} else {
		uiSource = (*(UINT16*)source);
		uiDestination = (*(UINT16*)destination);
		result = uiSource ^ uiDestination;
		IntToWordByPtr(result,destination);
	}

	// (N-bit)
	if (!isByteInstruction) compNegative <<= 8;
	nBit = (result & compNegative);

	// (Z-bit)
	zBit = (!(result & WORD_MASK));

	// Carry bit is set, if result != ZERO
	cBit = !zBit;
	
	// V-bit: set if both operands are negative
	vBit = ((uiSource & compNegative) && (uiDestination & compNegative));

	if (!writeBack(memoryWriteBack, destination, (!isByteInstruction)+1))
		return FALSE;
	
	// PC to next word
	increasePC();
	clocks++;
	return TRUE;
}


/*
	Decodes the instruction format and finds executor.
	Executor is returned by reference (*executor).
	Returns format value.
*/
int decodeInstructionFormat(msp_word instruction, executorPtr *executor) {
	UINT16 instructionCode;
	int format = 0;

	instructionCode = instruction.asF1Mask.opcode;
	
	// Decode instruction code
	switch (instructionCode) {
	case 1:		// Format 2
		instructionCode = instruction.asF2Mask.opcode;
		format = 2;
		break;
	case 2:	// Format 3
		instructionCode = instruction.asF3Mask.opcode;
		format = 3;
		break;
	case 3: // Format 3
		instructionCode = instruction.asF3Mask.opcode;
		format = 3;
		break;
	default:	// Format 1
		//instructionCode = instruction.asF1Mask.opcode;
		format = 1;
		break;
	}

	*executor = findExecutor(instructionCode);
	if (executor != NULL)
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
int decodeF1Instruction(msp_word instruction, void **source, 
	void **destination, int *isByte) {

	unsigned int As, Ad, sReg, dReg;
	//unsigned int uiInstruction;
	void *constant;
	char cConstant = 0;
	int createConstant = FALSE;
	int isByteInstruction = FALSE;
	msp_word baseAddress = {0};
	msp_word t_address = {0};
	UINT16 offset = 0;

	// Byte or word?
	isByteInstruction = instruction.asF1Mask.bflag;
	*isByte = isByteInstruction;
	memoryWriteBack = MEMORY_WRITEBACK_NO;
	// Decode source operand
	As = instruction.asF1Mask.as;
	sReg = instruction.asF1Mask.sreg;

	switch (As) {	// Addressing mode
	case ADDR_MODE_REGISTER:
		switch (sReg) {
		case CG2:	// const generator
			createConstant = TRUE;
			cConstant = 0;
			break;
		default:
			*source = &registers[sReg];
			break;
		}
		break;
	case ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE:
		switch (sReg) {
		case CG1:		// = SR
			// absolute addressing, offset (to 0) in next word
			increasePC();
			if (!vmbReadWordAt(registers[PC], &offset))
				return FALSE;
			t_address.asWord = offset;
			if (isByteInstruction) {
				*source = malloc(sizeof(UINT8));
				if (!vmbReadByteAt(t_address,(UINT8*)*source))
					return FALSE;
			} else {
				*source = malloc(sizeof(UINT16));
				if (!vmbReadWordAt(t_address,(UINT16*)*source))
					return FALSE;
			}
			break;
		case CG2:		// const generator
			createConstant = TRUE;
			cConstant = 1;
			break;
		default:
			// indexed addressing: reg+offset, offset in next word
			baseAddress = registers[sReg];
			increasePC();
			if (!vmbReadWordAt(registers[PC], &offset))
					return FALSE;
			t_address.asWord = baseAddress.asWord + offset;
			if (isByteInstruction) {
				*source = malloc(sizeof(UINT8));
				if (!vmbReadByteAt(t_address,(UINT8*)*source))
					return FALSE;
			} else {
				*source = malloc(sizeof(UINT16));
				if (!vmbReadWordAt(t_address,(UINT16*)*source))
					return FALSE;
			}
			break;
		}
		break;
	case ADDR_MODE_INDIRECT_REGISTER:
		switch (sReg) {
		case CG1:		// const generator
			createConstant = TRUE;
			cConstant = 4;
			break;
		case CG2:		// const generator
			createConstant = TRUE;
			cConstant = 2;
			break;
		default:
			// indirect register addressing: address in reg
			t_address = registers[sReg];
			if (isByteInstruction) {
				*source = malloc(sizeof(UINT8));
				if (!vmbReadByteAt(t_address,(UINT8*)*source))
					return FALSE;
			} else {
				*source = malloc(sizeof(UINT16));
				if (!vmbReadWordAt(t_address,(UINT16*)*source))
					return FALSE;
			}
			break;
		}
		break;
	case ADDR_MODE_INDIRECT_AUTOINC_IMMEDIATE:
		switch (sReg) {
		case CG1:		// const generator
			createConstant = TRUE;
			cConstant = 8;
			break;
		case CG2:		// const generator
			createConstant = TRUE;
			cConstant = -1;
			break;
		default:
			// indirect register auto-increment or immediate address mode
			if (sReg == PC) {		// immediate
				increasePC();
			}
			t_address = registers[sReg];
			if (isByteInstruction) {
				*source = malloc(sizeof(UINT8));
				if (!vmbReadByteAt(t_address,(UINT8*)*source))
					return FALSE;
			} else {
				*source = malloc(sizeof(UINT16));
				if (!vmbReadWordAt(t_address,(UINT16*)*source))
					return FALSE;
			}
			if (sReg != PC) {
				registers[sReg].asWord += (!isByteInstruction)+1;
			}
			break;
		}
		break;
	default:
		*source = NULL;
	}

	//switch (sReg) {
	//case CG1:			// CG1 = SR!
	//	switch (As) {	
	//	case ADDR_MODE_REGISTER:			// SR-Register (non-cg case)
	//		// Warning: little endian! Otherwise, the higher byte 
	//		// should be selected for byte operations!
	//		*source = &registers[PC];
	//		break;
	//	case ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE:	/* Equivalent to constant 0 
	//												for base address 
	//												(absolute addressing) */
	//		if (isByteInstruction) {
	//			*source = malloc(sizeof(UINT8));
	//		} else {
	//			*source = malloc(sizeof(UINT16));
	//		}
	//		//baseAddress.asWord = 0;
	//		
	//		increasePC();
	//		if (!vmbReadWordAt(registers[PC], &offset))
	//			 return FALSE;
	//		t_address.asWord = offset;
	//		if (isByteInstruction) {
	//			 if (!vmbReadByteAt(t_address, (UINT8*)*source))
	//				 return FALSE;
	//		}
	//		else {
	//			if (!vmbReadWordAt(t_address, (UINT16*)*source))
	//				return FALSE;
	//		}
	//		break;
	//	case 2:
	//		createConstant = TRUE;
	//		cConstant = 4;
	//		break;
	//	case 3:
	//		createConstant = TRUE;
	//		cConstant = 8;
	//		break;
	//	default:
	//		source = NULL;
	//		break;
	//	}
	//	break;


	//case CG2:
	//	switch (As) {
	//	case 0:
	//		createConstant = TRUE;
	//		cConstant = 0;
	//		break;
	//	case 1:
	//		createConstant = TRUE;
	//		cConstant = 1;
	//		break;
	//	case 2:
	//		createConstant = TRUE;
	//		cConstant = 2;
	//		break;
	//	case 3:
	//		createConstant = TRUE;
	//		cConstant = -1;
	//		break;
	//	default:
	//		source = NULL;
	//		break;
	//	}

	//default:
	//	switch (As) {
	//	case ADDR_MODE_REGISTER:		// Register is the operand
	//		*source = &registers[sReg];
	//		break;
	//	case ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE:
	//		// Register contains the base address, next word contains the offset
	//		if (isByteInstruction) {
	//			*source = malloc(sizeof(UINT8));
	//		} else {
	//			*source = malloc(sizeof(UINT16));
	//		}
	//		baseAddress = registers[sReg];
	//		increasePC();
	//		if (!vmbReadWordAt(registers[PC], &offset))
	//			return FALSE;
	//		t_address.asWord = baseAddress.asWord + offset;
	//		if (!vmbReadWordAt(t_address, (UINT16*)*source))
	//			return FALSE;
	//		break;
	//	case ADDR_MODE_INDIRECT_REGISTER:
	//		// Register contains the address of the operand
	//		if (isByteInstruction) {
	//			*source = malloc(sizeof(UINT8));
	//			if (!vmbReadByteAt(registers[sReg], (UINT8*)*source))
	//				 return FALSE;
	//		} else {
	//			*source = malloc(sizeof(UINT16));
	//			if (!vmbReadWordAt(registers[sReg], (UINT16*)*source))
	//				 return FALSE;
	//		}
	//		break;
	//	case ADDR_MODE_INDIRECT_AUTOINC_IMMEDIATE:
	//		// Register contains the address. Register value will be incremented (byte/wyde)
	//		if (sReg == PC)
	//			increasePC();
	//		//if (!vmbReadWordAt(registers[sReg], &baseAddress.asWord))
	//		//	return FALSE;
	//		if (isByteInstruction) {
	//			*source = malloc(sizeof(UINT8));
	//			if (!vmbReadByteAt(registers[sReg], (UINT8*)*source))
	//				return FALSE;
	//		} else {
	//			*source = malloc(sizeof(UINT16));
	//			if (!vmbReadWordAt(registers[sReg], (UINT16*)*source))
	//				return FALSE;
	//		}

	//		// Execute autoincrement
	//		if (sReg != PC) { // PC is already incremented
	//			UINT16 incr = 1;
	//			if (!isByteInstruction)
	//				incr++;
	//			registers[sReg].asWord += incr;
	//		}
	//		break;
	//	default:
	//		source = NULL;
	//	}
	//}

	if (createConstant) {
		if (isByteInstruction)
			constant = malloc(sizeof(UINT8));
		else
			constant = malloc(sizeof(UINT16));
		*(UINT16*)constant = cConstant;
		*source = constant;
	}
	
	// Decode destination operand
	Ad = instruction.asF1Mask.ad;
	dReg = instruction.asF1Mask.dreg;

	switch (Ad) {
	case ADDR_MODE_REGISTER:
		// Register is the operand
		*destination = &registers[dReg];
		break;
	case ADDR_MODE_INDEXED_SYMBOLIC_ABSOLUTE:
		switch (dReg) {
		case CG1:		// = SR
			// absolute addressing, offset (to 0) in next word
			increasePC();
			if (!vmbReadWordAt(registers[PC], &offset))
				return FALSE;
			t_address.asWord = offset;
			memoryWriteBack = t_address.asWord;
			if (isByteInstruction) {
				*destination = malloc(sizeof(UINT8));
				if (!vmbReadByteAt(t_address,(UINT8*)*destination))
					return FALSE;
			} else {
				*destination = malloc(sizeof(UINT16));
				if (!vmbReadWordAt(t_address,(UINT16*)*destination))
					return FALSE;
			}
			break;
		default:
			// indexed addressing: reg+offset, offset in next word
			baseAddress = registers[dReg];
			increasePC();
			if (!vmbReadWordAt(registers[PC], &offset))
				return FALSE;
			t_address.asWord = baseAddress.asWord + offset;
			memoryWriteBack = t_address.asWord;
			if (isByteInstruction) {
				*destination = malloc(sizeof(UINT8));
				if (!vmbReadByteAt(t_address,(UINT8*)*destination))
					return FALSE;
			} else {
				*destination = malloc(sizeof(UINT16));
				if (!vmbReadWordAt(t_address,(UINT16*)*destination))
					return FALSE;
			}
			break;
		}
		

		//baseAddress = registers[dReg];
		//increasePC();
		//if (!vmbReadWordAt(registers[PC], &offset))
		//	return FALSE;
		//t_address.asWord = baseAddress.asWord + offset;
		//memoryWriteBack = t_address.asWord;
		//if (isByteInstruction) {
		//	*destination = malloc(sizeof(UINT8));
		//	if (!vmbReadByteAt(t_address, (UINT8*)*destination))
		//		return FALSE;
		//} else {
		//	*destination = malloc(sizeof(UINT16));
		//	if (!vmbReadWordAt(t_address, (UINT16*)*destination))
		//		return FALSE;
		//}
		break;
	default:
		*destination = NULL;
	}
	
	if (*source == NULL || *destination == NULL)
		return FALSE;
	else
		return TRUE;
}


int decodeF2Instruction(msp_word instruction, void **operand, int *isByte) {
	unsigned int Ad, DSReg;
	int isByteInstruction = FALSE;
	msp_word baseAddress;
	msp_word t_address;
	UINT16 offset;
	memoryWriteBack = MEMORY_WRITEBACK_NO;
	Ad = instruction.asF2Mask.ad;
	DSReg = instruction.asF2Mask.dsreg;
	isByteInstruction = instruction.asF2Mask.bflag;

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
		t_address.asWord = baseAddress.asWord + offset;
		memoryWriteBack = t_address.asWord;
		if (isByteInstruction) {
			*operand = malloc(sizeof(UINT8));
			if (!vmbReadByteAt(t_address, (UINT8*)*operand))
				return FALSE;
		}
		else {
			*operand = malloc(sizeof(UINT16));
			if (!vmbReadWordAt(t_address, (UINT16*)*operand))
				return FALSE;
		}
		break;
	case ADDR_MODE_INDIRECT_REGISTER:
		// Register contains the address of the operand
		t_address = registers[DSReg];
		memoryWriteBack = t_address.asWord;
		if (isByteInstruction) {
			*operand = malloc(sizeof(UINT8));
			if (!vmbReadByteAt(t_address, (UINT8*)*operand))
				return FALSE;
		} else {
			*operand = malloc(sizeof(UINT16));
			if (!vmbReadWordAt(t_address, (UINT16*)*operand))
				return FALSE;
		}
		break;
	case ADDR_MODE_INDIRECT_AUTOINC_IMMEDIATE:
		// Register contains the address. Register value will be incremented (byte/wyde)
		if (DSReg == PC)
			increasePC();
		//if (!vmbReadWordAt(registers[DSReg], &baseAddress.asWord))
		//	return FALSE;
		//t_address = baseAddress;
		if (DSReg != PC)
			memoryWriteBack = registers[DSReg].asWord;
		//else
		//	memoryWriteBack = MEMORY_WRITEBACK_NO;
		if (isByteInstruction) {
			*operand = malloc(sizeof(UINT8));
			if (!vmbReadByteAt(registers[DSReg], (UINT8*)*operand))
				return FALSE;
		} else {
			*operand = malloc(sizeof(UINT16));
			if (!vmbReadWordAt(registers[DSReg], (UINT16*)*operand))
				return FALSE;
		}
		// Execute autoincrement
		if (DSReg != PC) {
			// PC is already incremented
			registers[DSReg].asWord += (!isByteInstruction)+1;
			//int incr;
			//if (isByteInstruction)
			//	incr = 1;
			//else
			//	incr = 2;
			//registers[DSReg].asWord += incr;
		}
		break;
	default:
		*operand = NULL;
	}

	if (*operand != NULL)
		return TRUE;
	else
		return FALSE;
}

int decodeF3Instruction(msp_word instruction, void **condition, void **offset) {
	unsigned int /*uiInstruction, */uiCondition;
	INT16 iOffset;

	uiCondition = instruction.asF3Mask.condition;
	iOffset = instruction.asF3Mask.offset;
	// Check for sign (offset has 10 bit)
	if (iOffset & 512)
		iOffset &= (-1);

	*condition = &uiCondition;
	*offset = &iOffset;

	return TRUE;
}

executorPtr findExecutor(UINT16 instructionCode) {
     return INSTRUCTIONS[instructionCode];
}
	
void increasePC() {
	registers[PC].asWord += 2;
	return;
}


/*
	WD: Initializes core components
	and starts the execution at the address pointed by
	value @ 0xFFFE (interrupt vector table).
*/
void initCore(void) {
	msp_word t_address;
	// Initialize local storage
	initRegisters();

	if (!executionStartAddress) {
		t_address.asWord = DEFAULT_EXECUTION_START_LOCATION;
		vmbReadWordAt(t_address, &registers[PC].asWord);		/* Initialize the programm 
														counter with the start address */
	} else
		registers[PC].asWord = executionStartAddress;
}


/*
	Initializes the registers
*/
void initRegisters() {
	memset(&registers, 0, sizeof(msp_word) * REGISTERS_COUNT);
}

msp_word fetchInstruction(msp_word msp_address) {
	msp_word result;
	
	if (vmbReadWordAt(msp_address, &result.asWord))
		return result;
	else {
		result.asWord = 0;
		return result;
	}
}

__inline int writeBack (UINT32 write_back_address, void *data, unsigned char size) {
	msp_word t_address;
	t_address.asWord = write_back_address & WORD_MASK;
	if (write_back_address >= 0) {
		if (size == 1) {
			if (!vmbWriteByteAt(t_address, (UINT8*)data))
				return FALSE;
		} else if (size == 2) {
			if (!vmbWriteWordAt(t_address, (UINT16*)data))
				return FALSE;
		} else 
			return FALSE;
	}
	return TRUE;
}

__inline void checkCBit(UINT32 check, int asByte) {
	if (asByte)
		cBit = ((check & (compCarryByte)) != 0);
	else
		cBit = ((check & (compCarryWord)) != 0);
}

void UI() {
	char input_buffer[256];
	char t_buffer[10];
	int t_int;
	int inErrFlag = FALSE;
	msp_word t_address;
	int i,j;

	x = FALSE;
	if (!continuosMode) {
		//Debug output
		fprintf(stdout,"Programm stats:\t");
		fprintf(stdout,"PC @ 0x%4X\t",registers[PC].asWord);
		fprintf(stdout,"SP @ 0x%4X\n\n",registers[SP].asWord);
retry_input_onerror:
		fprintf(stdout,"Choose ENTER/x to execute, bHHHH to set or unset a breakpoint @0xHHHH, q to quit, rN to view register N (0-15), c to continue, mHHHH to view memory at address 0xHHHH: ");
retry_input:
		inErrFlag = FALSE;

		memset(&input_buffer,0,256);
		fscanf(stdin,"%s",&input_buffer[0]);
		if (strlen(&input_buffer[0])) {
			//t_int = -1;
			memset(&t_buffer[0],0,10);
			sscanf(&input_buffer[0],"%[c]",t_buffer);	// Test c
			if (t_buffer[0]) {
				continuosMode = !continuosMode;
				x = continuosMode;
				goto skip_i_checks;
			}

			memset(&t_buffer[0],0,10);
			sscanf(&input_buffer[0],"%[q]",&t_buffer[0]);	// Test q
			if (t_buffer[0])
				return;

			memset(&t_buffer[0],0,10);
			sscanf(&input_buffer[0],"%[x]",&t_buffer[0]);	// Test x
			if (t_buffer[0]) {
				x = TRUE;
				goto skip_i_checks;
			}

			t_int = -1;
			//memset(&t_buffer[0],0,10);
			sscanf(&input_buffer[0],"r%2d",&t_int);	// Test r
			//if (t_int < 0)
			//	sscanf(&input_buffer[0],"r%1d",&t_int);	// Test r
			if ((t_int >= 0)) {
				if (t_int < REGISTERS_COUNT) {
					fprintf(stdout,"\nRegister %d: 0x%4X\n",t_int,
						registers[t_int].asWord);
					goto skip_i_checks;
				} else {
					inErrFlag = TRUE;
					goto skip_i_checks;
				}
			}

			t_int = -1;
			//memset(&t_buffer[0],0,10);
			sscanf(&input_buffer[0],"b%4x",&t_int); // Test b
			if (t_int >= 0) {
				t_address.asWord = t_int;
				//fprintf(stderr,"\nBreakpoint address: 0x%x\n",t_address.asWord);
				// (un)set breakpoint
				for (i=0;i<MAX_BREAKPOINTS;i++) {
					if (breakpoints[i].asWord == t_address.asWord) {
						breakpoints[i].asWord = 0;
						for (j=i;j<MAX_BREAKPOINTS-1;j++) {
							breakpoints[i] = breakpoints[i+1];
						}
						breakpoints[MAX_BREAKPOINTS-1].asWord = 0;
						fprintf(stdout,"\nBreakpoint @0x%X%X unset.\n",t_address.asBytes.hi,t_address.asBytes.lo);
						break;
					}
					if (breakpoints[i].asWord == 0) {
						breakpoints[i] = t_address;
						fprintf(stdout,"\nBreakpoint @0x%X%X set.\n",t_address.asBytes.hi,t_address.asBytes.lo);
						break;
					}
				}
				goto skip_i_checks;
			}

			// Test m
			t_int = -1;
			memset(&t_buffer[0],0,10);
			sscanf(&input_buffer[0],"m%4x",&t_int); 
			if (t_int >= 0) {
				t_address.asWord = t_int;
				vmbReadWordAt(t_address, (UINT16*)&t_buffer[0]);
				fprintf(stdout,"\nMemory @0x%4X: 0x%4X\n",t_address.asWord, 
					((msp_word*)&t_buffer[0])->asWord);
				goto skip_i_checks;
			}

				
skip_i_checks:
			fprintf(stdout,"\n");
		} else {
			inErrFlag = TRUE;
		}

		if (inErrFlag) {
			fprintf(stdout,"Invalid input \"%s\".\n",&input_buffer[0]);
			goto retry_input_onerror;
		}
		if (!x) {
			goto retry_input;
		}
	} else {
		// Check breakpoints
		for(i=0;i<MAX_BREAKPOINTS;i++) {
			if (breakpoints[i].asWord == registers[PC].asWord) {
				continuosMode = FALSE;
				goto retry_input_onerror;
			}
		}
		// Continous mode
		x = TRUE;
	}
}

int main(int argc, char *argv[])
{	
	executorPtr executor;
	unsigned int format = 0;
	unsigned int opcode = 0;
	

	// Init core and start execution
	initVMBInterface();
	fprintf(stderr,"VMB connected.\n");
 boot:
	if (!wait_for_power())
		goto end_simulation;
	initCore();
	fprintf(stderr,"MSP430 initialized.\n");

	// main loop
	while (TRUE) {
		if (!vmb.connected) {
			fprintf(stderr,"VMB disconnected during execution.\n");
			break;
		}
		if (!vmb.power || vmb.reset_flag)
		{ 
		  vmb.reset_flag= 0;
		  goto boot;
		}
		// fetch
		currentInstruction = fetchInstruction(registers[PC]);
		// Call UI
		UI();
		if (!x)
			goto end_simulation;
		
		if (!currentInstruction.asWord) {
			fprintf(stderr,"MSP430-Error: Invalid instruction format @0x%4X in instruction 0x%4X\n",
				registers[PC].asWord,currentInstruction.asWord);
			break;
		}
		format = decodeInstructionFormat(currentInstruction, &executor);
		
		// Opcode for debugging output
		switch (format) {
		case 1:
			opcode = currentInstruction.asF1Mask.opcode;
			break;
		case 2:
			opcode = currentInstruction.asF2Mask.opcode;
			break;
		case 3:
			opcode = currentInstruction.asF3Mask.opcode;
			break;
		default:
			opcode = 0;
			break;
		}
		if (!format) {
			fprintf(stderr,"MSP430-Error: Invalid instruction format @0x%X%X in instruction 0x%X%X\n",
				registers[PC].asBytes.hi,registers[PC].asBytes.lo,
				currentInstruction.asBytes.hi,currentInstruction.asBytes.lo);
			break;
		}
		if (executor == NULL) {
			fprintf(stderr,"MSP430-Error: Invalid opcode %d @0x%X%X in instruction 0x%X%X\n",
				opcode,registers[PC].asBytes.hi,registers[PC].asBytes.lo,
				currentInstruction.asBytes.hi,currentInstruction.asBytes.lo);
			break;
		}
		if (!executor()) {
			fprintf(stderr,"MSP430-Error: Could not execute %s @0x%X%X in instruction 0x%X%X\n",
				INSTRUCTION_NAMES[opcode],
				registers[PC].asBytes.hi, registers[PC].asBytes.lo,
				currentInstruction.asBytes.hi,currentInstruction.asBytes.lo);
			break;
		}
	}
end_simulation:
	fprintf(stderr,"Exiting...");
	vmb_disconnect(&vmb);
	vmb_end();
	return 0;
}

