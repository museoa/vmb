#ifndef MSPMUL
#define MSPMUL

#include "mspmulshared.h"

#ifdef WIN32
#include <windows.h>
HWND hMainWnd = NULL; /* there is no Window */
#endif

enum register_names {
	MPY, 
	MPYS, 
	MAC, 
	MACS, 
	OP2, 
	RESLO, 
	RESHI, 
	SUMEXT
};

extern unsigned int mul_mode;
#define REGISTERS_COUNT 8
#define MEMSIZE REGISTERS_COUNT * sizeof(msp_word)
#endif