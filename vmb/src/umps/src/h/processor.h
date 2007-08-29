/* File: $Id: processor.h,v 1.1 2007-08-29 09:19:38 ruckert Exp $ */

/**************************************************************************** 
 *
 * This header file contains constant & macro definitions for processor.cc
 * and disassemble.cc program modules.
 *
 * All names, bit positions and masks follow MIPS register formats: see
 * documentation.
 *
 * Instruction opcodes too may be retrieved from appropriate documentation:
 * codes are in octal format for easier coding by programmer.
 *
 ****************************************************************************/


// virtual exception handling base address
#define BOOTEXCBASE	(BOOTBASE + 0x100UL)

// exception virtual offsets
#define TLBREFOFFS	0x000UL
#define OTHEREXCOFFS	0x080UL

// CPUEXCEPTION handling constant
#define COPEOFFSET	27

// processor special registers index in Processor structure
#define	HI	32
#define LO	33

// used to decode TLBCLR instruction
#define CONTEXTREG    4

// valid CP0 registers codes
#define	INDEXREG	0
#define RANDOMREG	1
#define	ENTRYLOREG	2
#define BADVADDRREG	8
#define ENTRYHIREG	10
#define STATUSREG	12
#define CAUSEREG	13
#define EPCREG		14
#define PRIDREG	15

// and corresponding register index in Processor structure
#define	INDEX	0
#define RANDOM	1
#define	ENTRYLO	2
#define BADVADDR	3
#define ENTRYHI	4 
#define STATUS	5 
#define CAUSE	6 
#define EPC		7 
#define PRID	8 

// PRID revision value: R2000 CPU, rev. 6.6
#define PRIDREGVAL 0x00000166UL


// STATUS and CAUSE interrupt area mask
#define INTMASK	0x0000FF00UL

// CAUSE reg handling mask and constants
#define CAUSEWMASK	0x0000FF00UL
#define EXCCODEOFFS	2
#define BDBITPOS	31

// TLB EntryHI handling masks and constants 
#define VPNMASK	0xFFFFF000UL
#define ASIDMASK	0x00000FC0UL
#define ASIDOFFS	6
#define OFFSETMASK	(~(VPNMASK))


// TLB EntryLO bit positions and mask
#define GBITPOS	8
#define VBITPOS 9
#define DBITPOS 10
#define ENTRYLOMASK	0xFFFFFF00UL

// RANDOM and INDEX register handling constants
#define RNDIDXOFFS	8
#define RANDOMBASE	(1UL << RNDIDXOFFS)
#define RANDOMSTEP	(1UL << RNDIDXOFFS)


// STATUS register handling constants:

// at reset, are set to 1: cop0 (bit 28), BEV (bit 22)
// all other bits are set to 0 (interrupts disabled, etc.)
#define STATUSRESET	0x10400000UL

// this mask forbids changes to some bits (CU3->1 usable bits, all DS field
// except BEV bit, 0 fixed parts)
#define STATUSMASK	0x1740FF3FUL

// STATUS kernel/user, interrupt enable, VM bit positions
#define KUOBITPOS	5
#define IEOBITPOS	4
#define KUCBITPOS	1
#define IECBITPOS	0
#define VMOBITPOS	26
#define VMCBITPOS	24

// STATUS miscellaneous bit positions 
#define CP0UBITPOS	28
#define BEVBITPOS	22

// load types for Processor completeLoad() method
#define NOLOAD	0
#define LOADREG	1
#define LOADCPREG	2


// instruction decode field masks and constants

#define OPTYPEMASK	0xE0000000UL
#define OPCODEMASK	0xFC000000UL
#define FUNCTMASK	0x0000003FUL
#define RSMASK	0x03E00000UL
#define RTMASK	0x001F0000UL
#define RDMASK	0x0000F800UL
#define SHAMTMASK	0x000007C0UL
#define PCUPMASK	0xF0000000UL
#define COPTYPEMASK	0x03E00000UL
#define COPCODEMASK 0x03FF0000UL
#define COPNUMMASK	0x0C000000UL
#define CALLMASK	0x03FFFFC0UL
#define IMMSIGNPOS	15
#define DWCOPBITPOS	28

#define LINKREG	31

#define OPCODEOFFS	26
#define RSOFFSET	21
#define RTOFFSET	16
#define RDOFFSET	11
#define SHAMTOFFS	6
#define COPTYPEOFFS	21
#define COPCODEOFFS	16


// instruction main opcodes

#define REGTYPE	077
#define IMMTYPE 010
#define COPTYPE	020
#define BRANCHTYPE	000
#define LOADTYPE	040
#define LOADCOPTYPE	060
#define STORECOPTYPE	070
#define STORETYPE	050

// REGTYPE function opcodes

#define ADD	040
#define ADDU	041
#define AND	044
#define BREAK 015
#define DIV	032
#define DIVU	033
#define JALR	011
#define JR	010
#define MFHI	020
#define MFLO	022
#define MTHI	021
#define MTLO	023
#define MULT	030
#define MULTU	031
#define NOR	047
#define OR	045
#define SLL	000
#define SLLV 004
#define SLT	052
#define SLTU	053
#define SRA	003
#define SRAV	007
#define SRL	002
#define SRLV	006
#define SUB	042
#define SUBU 043
#define SYSCALL	014
#define XOR	046


// IMMCOMPTYPE opcodes

#define ADDI	010
#define ADDIU	011
#define ANDI	014
#define LUI	017
#define ORI	015
#define SLTI	012
#define SLTIU	013
#define XORI 	016

// BRANCHTYPE opcodes

#define BEQ	004

#define BGL	001
#define BGEZ	001
#define BGEZAL	021
#define BLTZ	000
#define BLTZAL	020

#define BGTZ	007
#define BLEZ	006
#define BNE	005
#define J	002
#define JAL	003

// COPTYPE opcodes

// detects if instruction refers to CP0
#define COP0SEL	020

#define CO0	020
#define RFE	020
#define TLBP	010
#define TLBR	001
#define TLBWI	002
#define TLBWR	006

#define BC0	010
#define BC0F	0400
#define BC0T	0401

#define MFC0	0
#define MTC0	04

// LOADTYPE opcodes

#define	LB	040
#define LBU	044 
#define LH	041
#define LHU 045
#define LW	043
#define LWL 042
#define LWR 046

//STORETYPE opcodes

#define SB	050
#define SH	051
#define SW	053
#define SWL 052
#define SWR	056

// byte sign extension mask
#define BYTESIGNMASK	0x0000007FUL

// useful macros

// extracts VPN from address
#define VPN(w)		((w & VPNMASK))

// extracts ASID from address
#define ASID(w)	((w & ASIDMASK))

// applies interrupt mask to STATUS/CAUSE register 
#define IM(r)		((r & INTMASK))

// aligns a virtual address (clears lowest bits)
#define ALIGN(va)	(va & ~(ALIGNMASK))

// computes physical address from virtual address and PFN field
#define PHADDR(va, pa)	((va & OFFSETMASK) | (pa & VPNMASK))

// detects which byte is referred by a LB/LBU/LWL/LWR/SB/SWL/SWR instruction
#define BYTEPOS(va)	(va & ALIGNMASK)

// detects which halfword is referred by a LH/LHU/SH instruction
#define HWORDPOS(va)	((va & ALIGNMASK) >> 1)

// instruction fields decoding macros
#define OPCODE(i)	((i & OPCODEMASK) >> OPCODEOFFS)
#define FUNCT(i)	(i & FUNCTMASK)
#define SHAMT(i)	((i & SHAMTMASK) >> SHAMTOFFS)
#define REGSHAMT(r)	(r & (SHAMTMASK >> SHAMTOFFS))
#define RS(i)	((i & RSMASK) >> RSOFFSET)
#define RT(i)	((i & RTMASK) >> RTOFFSET)
#define RD(i)	((i & RDMASK) >> RDOFFSET)

// coprocessor instructions decoding macros
#define COPOPCODE(i) 	((i & COPCODEMASK) >> COPCODEOFFS)
#define COPOPTYPE(i)	((i & COPTYPEMASK) >> COPTYPEOFFS)
#define COPNUM(i)	((i & COPNUMMASK) >> OPCODEOFFS)

// computes jump target from PC and J/JAL instruction itself 
#define JUMPTO(pc, i)	((pc & PCUPMASK) | ((i & ~(OPCODEMASK)) << WORDSHIFT))

// zero-extends a immediate (halfword) quantity
#define ZEXTIMM(i)	(i & IMMMASK)

// computes index value from INDEX register format:
// SIGNMASK is used to clear bit 31, which marks a failed TLB probe
#define RNDIDX(id)	((id & ~(SIGNMASK)) >> RNDIDXOFFS)

// decodes BREAK/SYSCALL argument inside instruction itself
#define CALLVAL(i)	((i & CALLMASK) >> SHAMTOFFS)
