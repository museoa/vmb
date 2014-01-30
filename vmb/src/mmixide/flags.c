#include <windows.h>
#include "option.h"
#include "param.h"
#include "winopt.h"
#include "winmain.h"
#include "edit.h"
#include "assembler.h"
#include "debug.h"
#include "mmix-internals.h"
#include "symtab.h"
#define STATIC_BUILD
#include "../scintilla/include/scilexer.h"

regtable regtab= {
	{"xpos",&xpos,TYPE_DWORD},
	{"ypos",&ypos,TYPE_DWORD},
	/* editor options */
	{"tabwidth",&tabwidth,TYPE_DWORD},
	{KEY_FLAGS ,&autosave,0},
	{KEY_FLAGS ,&show_line_no,1},
	{KEY_FLAGS ,&show_profile,2},
	{KEY_FLAGS ,&syntax_highlighting,3},
	{KEY_FLAGS ,&show_whitespace,4},
	{"fontsize",&fontsize,TYPE_DWORD},
	{"codepage",&codepage,TYPE_DWORD},

	{"opcolor",&syntax_color[SCE_MMIXAL_OPCODE_VALID],TYPE_DWORD},
	{"errcolor",&syntax_color[SCE_MMIXAL_OPCODE_UNKNOWN],TYPE_DWORD},
	{"regcolor",&syntax_color[SCE_MMIXAL_REGISTER],TYPE_DWORD},
	{"symcolor",&syntax_color[SCE_MMIXAL_SYMBOL],TYPE_DWORD},
	{"commentcolor",&syntax_color[SCE_MMIXAL_COMMENT],TYPE_DWORD},

    /* assembler options */
	{"boption",&b_option,TYPE_DWORD},
	{KEY_FLAGS ,&x_option,5},
	{KEY_FLAGS ,&l_option,6},
	{KEY_FLAGS ,&auto_assemble,7},

	/*debugger options */
	{KEY_FLAGS,&break_at_Main,8},
	{KEY_FLAGS,&break_after,9},
	{KEY_FLAGS,&show_debug_local,10},
	{KEY_FLAGS,&show_debug_global,11},
	{KEY_FLAGS,&show_debug_special,12},
	{KEY_FLAGS,&show_debug_regstack,13},
	{KEY_FLAGS,&show_debug_text,14},
	{KEY_FLAGS,&show_debug_data,15},
	{KEY_FLAGS,&show_debug_pool,16},
	{KEY_FLAGS,&show_debug_neg,17},
	{KEY_FLAGS,&show_trace,18},
	{KEY_FLAGS,&show_operating_system,19},
	{KEY_FLAGS,&tracing_exceptions,20},
	{KEY_FLAGS,&auto_connect,21},

    /* symbol table */
	{KEY_FLAGS,&symtab_locals,22},
	{KEY_FLAGS,&symtab_registers,23},
	{KEY_FLAGS,&symtab_small,24},

	{NULL,NULL,0}};
