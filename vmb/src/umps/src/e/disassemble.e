/* File: $Id: disassemble.e,v 1.1 2007-08-29 09:19:37 ruckert Exp $ */

/****************************************************************************
 *
 * External declarations from the corresponding module .cc
 *
 ****************************************************************************/


// This function decodes and returns the opcode type for an instruction
unsigned int OpType(Word instr);

// This function sign-extends an halfword quantity to a full word for
// 2-complement arithmetics
SWord SignExtImm(Word instr);

// This function detects invalid formats for register type instructions,
// checking if fields are set appropriately to zero where needed.
// Returns FALSE if instruction is valid, and a non-zero value otherwise
Boolean InvalidRegInstr(Word instr);

// This function maps the MIPS R2/3000 CP0 register codes to simulator internal
// codes and returns TRUE if the register is implemented, FALSE otherwise
Boolean ValidCP0Reg(unsigned int regnum, unsigned int * cpnum);

// This function returns main processor's register name indexed by position
const char * RegName(unsigned int index);

// This function returns CP0 register name indexed by position
const char * CP0RegName(unsigned int index);

// this function returns the pointer to a static buffer which contains
// the instruction translation into readable form

const char * StrInstr(Word instr);
