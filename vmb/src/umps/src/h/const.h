/* File: $Id: const.h,v 1.1 2007-08-29 09:19:38 ruckert Exp $ */

/**************************************************************************** 
 *
 * This header file contains the global constant & macro definitions.
 * 
 ****************************************************************************/

#include <config.h>

// general configuration constants
#define MPSFILETYPE	".umps"
#define AOUTFILETYPE	".aout.umps"
#define BIOSFILETYPE	".rom.umps"
#define COREFILETYPE	".core.umps"
#define STABFILETYPE	".stab.umps"
#define MPSFILEPAT	"*.umps"

// available memory parameters (in 4 KB frames): default, min, max and
// Setup window increment values
#define STARTRAM 64
#define MINRAM	8
#define MAXRAM	(256*4*1024*2)
#define STEPRAM 1

// simulated speed parameters (in MHz): min and max values
#define MINSPEED	1
#define MAXSPEED 	99

// run stepping parameters (number of instructions): default, min and
// max values for MainForm window
#define STARTSTEP	10
#define MINSTEP	1
#define MAXSTEP	1000


// maximum area size for trace ranges: a little more than 4KB
// to avoid troubles in browser refresh if area is too large
#define MAXTRACESIZE	(FRAMESIZE + 1)

/***************************************************************************/

// no more user-serviceable parts below this line

// some utility constants
#define	HIDDEN	static

#if !defined(TRUE) && !defined(FALSE)
#define	FALSE	0
#define	TRUE	1
#endif

#define BLANK	' '
#define DOT	'.'
#define	EOS	'\0'
#define EMPTYSTR	""
#define	EXIT_FAILURE	1
#define	EXIT_SUCCESS	0

// host specific constants
#ifdef WORDS_BIGENDIAN
#define BIGENDIANCPU	1
#else
#define BIGENDIANCPU	0
#endif

// simulation update speed types
#define SPEEDNUM	7

#define SLOWEST	0
#define SLOWER	1
#define SLOW	2
#define NORMAL	3
#define FAST	4
#define FASTER	5
#define FASTEST	6

// number of instructions executed between two screen updates per speed
#define SLOWESTSTEP	1
#define SLOWERSTEP	10
#define SLOWSTEP	100
#define NORMALSTEP	1000
#define FASTSTEP	1000
#define FASTERSTEP	10000
#define FASTESTSTEP	20000

// setup boot types and number
#define BOOTTYPES	2
#define NOCOREBOOT	0
#define COREBOOT	1


// Setup ROM files number and index
#define ROMSNUM 	2
#define NUMEXTROMS  2
#define COREINDEX	0
#define STABINDEX	1
#define EXTEXECINDEX 0
#define EXTBOOTINDEX 1


// hardware constants 

// physical memory page frame size (in words)
#define FRAMESIZE	1024
#define BLOCKSIZE FRAMESIZE
// KB per frame
#define FRAMEKB	4

// miscellaneous MIPS alignment and size definitions needed by modules
// other by processor.cc

// number of ASIDs
#define MAXASID 64

// MIPS NOP instruction 
#define NOP	0UL

// word length in bytes, byte length in bits, sign masks, etc.
#define WORDLEN 4
#define BYTELEN	8
#define WORDSHIFT	2
#define MAXWORDVAL	0xFFFFFFFFUL
#define SIGNMASK	0x80000000UL
#define BYTEMASK	0x000000FFUL

// immediate/lower halfword part mask
#define IMMMASK	0x0000FFFFUL

// word alignment mask
#define ALIGNMASK	0x00000003UL

// halfword bit length 
#define HWORDLEN	16

// exception type constants (simulator internal coding)
#define NOEXCEPTION 	0
#define INTEXCEPTION	1
#define MODEXCEPTION	2
#define UTLBLEXCEPTION	3
#define TLBLEXCEPTION 	4
#define UTLBSEXCEPTION	5
#define TLBSEXCEPTION	6
#define ADELEXCEPTION	7
#define ADESEXCEPTION	8
#define DBEXCEPTION	    9
#define IBEXCEPTION	    10
#define SYSEXCEPTION	11
#define BPEXCEPTION		12
#define RIEXCEPTION	    13
#define CPUEXCEPTION	14
#define OVEXCEPTION	    15

// interrupt handling related constants

// timer interrupt line
#define TIMERINT	2

// device starting interrupt line
#define DEVINTBASE	3

// device register length
#define DEVREGLEN 	4

// interrupts available for registers 
#define DEVINTUSED 	5

// devices per interrupt line
#define DEVPERINT	8

// CAUSE register IP field starting bit  
#define IPMASKBASE	8

// segments base addresses
#define KSEG0BASE	0x00000000UL
#define KSEG0TOP	0x20000000UL
#define KUSEG2BASE	0x80000000UL

// bus memory mapping constants (BIOS/device registers/BOOT/RAM)
// #define BIOSBASE	0x04000000UL /*!< Bios Rom goes from DEVBASE + 2 MEG */
// #define DEVBASE		0x00000000UL
// #define BOOTBASE	0x14000000UL /*!< Boot Rom goes from BIOSBASE + 2 MEG */
// #define RAMBASE		0x24000000UL /*!< RAM goes from BOOTBASE + 2 MEG */

#define BIOSBASE        0x00000000UL
#define DEVBASE         0x10000000UL
#define BOOTBASE        0x1FC00000UL
#define RAMBASE         0x20000000UL


// bus registers addresses (word index from DEVBASE address)
#define	RAMBASEADR	0
#define RAMSIZEADDR	1
#define BIOSBASEAD	2
#define BIOSSIZEADR	3
#define BOOTBASEAD	4
#define BOOTSIZEADR	5
#define	TODHIADDR	6
#define TODLOADDR	7
#define TIMERADDR	8
#define TIMESCALEA	9
#define IDEVBASEAD	10
#define IDEVENDADR	(IDEVBASEAD + DEVINTUSED)
#define CDEVBASEAD	IDEVENDADR
#define CDEVENDADR	(CDEVBASEAD + DEVINTUSED)
#define DEVREGBADR	CDEVENDADR

// start of "proper" device register area 
#define DEVSTART	(DEVBASE + (DEVREGBADR * WORDLEN))

// first invalid physical address in device registers area
#define DEVEND	 	(DEVSTART + (DEVINTUSED * DEVPERINT * DEVREGLEN * WORDLEN))

// Processor structure register numbers
#define CPUREGNUM	34
#define CPUGPRNUM	32
#define CP0REGNUM	9

// TLB sizing
#define TLBMINSIZE	4
#define TLBDFLSIZE	16
#define TLBMAXSIZE	64

// nextPC, succPC, ToDLO, ToDHI, Timer, physPrevPC, physPC registers number
#define OTHERREGNUM	7

// Watch structure register numbers
#define WATCHREGNUM	(CPUREGNUM + CP0REGNUM + OTHERREGNUM)


// forms used in X interface: 5 (main, symbol table, mem browser, 
// TLB display, setup) + devices + terminals
#define FORMSNUM	5

// virtual and physical range insert forms
#define RFORMSNUM	2

// interrupt line offset used for terminals 
// (lots of code must be modified if this changes)

#define TERMINT	4

// memory access types for brkpt/susp/trace ranges in watch.cc and appforms.cc
// modules
#define READWRITE 0x6
#define READ	0x4
#define WRITE	0x2
#define EXEC	0x1
#define EMPTY 	0x0

// memory browsers and watch tables types
#define RTABNUM	3
#define BRKPT	0
#define SUSP	1
#define TRACE	2

// X interface stop mask 
#define DFLSTOPMASK	0xF
#define BRKPTBIT	1
#define SUSPECTBIT	2						
#define EXCBIT	3


// some useful macros

// recognizes bad (unaligned) virtual address 
#define BADADDR(w)	((w & ALIGNMASK) != 0UL)

// returns the sign bit of a word
#define SIGNBIT(w)	(w & SIGNMASK)

// returns 1 if the two strings are equal, 0 otherwise
#define SAMESTRING(s,t)	(strcmp(s,t) == 0)

// returns 1 if a is in open-ended interval [b, c[, 0 otherwise
#define INBOUNDS(a,b,c)		(a >= b && a < c)  

// tests debugging conditions
#define ASSERT(s) if (!(s)) exit(1)
