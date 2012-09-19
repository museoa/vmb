/*


*/


#ifndef MSP_SHARED
#define MSP_SHARED

#include "vmb.h"		// Datatypes & interfaces

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#define WORD_MASK 0xFFFF
#define BYTE_MASK 0xFF

typedef union {
	// MSP is little endian!
	UINT16 asWord;
	struct {
#ifdef BIGENDIAN
		unsigned char RESERVED : 7;
		unsigned char V : 1;
		unsigned char SCG1 : 1;
		unsigned char SCG0 : 1;
		unsigned char OSCOFF : 1;
		unsigned char CPUOFF : 1;
		unsigned char GIE : 1;
		unsigned char N : 1;
		unsigned char Z : 1;
		unsigned char C : 1;
#else
		unsigned char C : 1;
		unsigned char Z : 1;
		unsigned char N : 1;
		unsigned char GIE : 1;
		unsigned char CPUOFF : 1;
		unsigned char OSCOFF : 1;
		unsigned char SCG0 : 1;
		unsigned char SCG1 : 1;
		unsigned char V : 1;
		unsigned char RESERVED : 7;
#endif
	} asBits;

	// Byte access
	struct {
#ifdef BIGENDIAN
		unsigned char hi : 8;
		unsigned char lo : 8;
#else
		unsigned char lo : 8;
		unsigned char hi : 8;
#endif
	} asBytes;

	// Opcode masks
	struct F1Mask {
#ifdef BIGENDIAN
		unsigned int opcode : 4;
		unsigned int sreg : 4;
		unsigned int ad : 1;
		unsigned int bflag : 1;
		unsigned int as : 2;
		unsigned int dreg : 4;
#else
		unsigned int dreg : 4;
		unsigned int as : 2;
		unsigned int bflag : 1;
		unsigned int ad : 1;
		unsigned int sreg : 4;
		unsigned int opcode : 4;
#endif
	} asF1Mask;

	struct F2Mask {
#ifdef BIGENDIAN
		unsigned int opcode : 9;
		unsigned int bflag : 1;
		unsigned int ad : 2;
		unsigned int dsreg : 4;
#else
		unsigned int dsreg : 4;
		unsigned int ad : 2;
		unsigned int bflag : 1;
		unsigned int opcode : 9;
#endif
	} asF2Mask;

	struct F3Mask {
#ifdef BIGENDIAN
		unsigned int opcode : 3;
		unsigned int condition : 3;
		signed int offset : 10;
#else
		signed int offset : 10;
		unsigned int condition : 3;
		unsigned int opcode : 3;
#endif
	} asF3Mask;
} msp_word;

// Configuration parameters and options
#include "option.h"		// configuration
#include "param.h"
extern char version[];
extern char howto[];
static int executionStartAddress = 0;
static int translate_ram_to_upper = 0;

#endif