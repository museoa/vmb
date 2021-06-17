// Scintilla source code edit control
/** @file LexMMIXAL.cxx
 ** Lexer for MMIX Assembler Language.
 ** Written by Christoph Hösler <christoph.hoesler@student.uni-tuebingen.de>
 ** For information about MMIX visit http://www-cs-faculty.stanford.edu/~knuth/mmix.html
 **/
// Copyright 1998-2003 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif


static inline bool IsAWordChar(const int ch) {
	return (ch >=80) || (ch < 0x80 && (isalnum(ch)) || ch == ':' || ch == '_');
}

inline bool isMMIXALOperator(char ch) {
	if (isascii(ch) && isalnum(ch))
		return false;
	if (ch == '+' || ch == '-' || ch == '|' || ch == '^' ||
		ch == '*' || ch == '/' ||
		ch == '%' || ch == '<' || ch == '>' || ch == '&' ||
		ch == '~' || ch == '$' ||
		ch == ',' || ch == '(' || ch == ')' ||
		ch == '[' || ch == ']')
		return true;
	return false;
}

static void ColouriseMMIXALDoc(unsigned int startPos, int length, int initStyle, WordList *keywordlists[],
                            Accessor &styler) {

	WordList &opcodes = *keywordlists[0];
	WordList &special_register = *keywordlists[1];
	WordList &predef_symbols = *keywordlists[2];

	StyleContext sc(startPos, length, initStyle, styler);

	for (; sc.More(); sc.Forward())
	{
       // Multiple instructions on one line are allowed separated by semicolons
        // anything besides a semicolon begins a comment
		if (sc.state == SCE_MMIXAL_TRAILWS) {
			if (isspace(sc.ch)) continue;
			else if (sc.Match(';')) {
			  sc.Forward();
     		  sc.atLineStart=true;
			}
			else  
			  sc.SetState(SCE_MMIXAL_COMMENT);
		}
	
		// No EOL continuation
		if (sc.atLineStart) {
			if (isspace(sc.ch))
				sc.SetState(SCE_MMIXAL_LEADWS);
			else if (IsAWordChar(sc.ch))
					sc.SetState(SCE_MMIXAL_LABEL);
			else if (sc.ch ==  '@' && sc.chNext == 'i') 
				sc.SetState(SCE_MMIXAL_INCLUDE);
		    else 
				sc.SetState(SCE_MMIXAL_COMMENT);
		}
		

 	// Check if first non whitespace character in line is alphanumeric
		if (sc.state == SCE_MMIXAL_LEADWS){
			if (isspace(sc.ch)) continue;	// LEADWS
			else if(IsAWordChar(sc.ch))
				sc.SetState(SCE_MMIXAL_OPCODE_PRE);
			else 
				sc.SetState(SCE_MMIXAL_COMMENT);
		} 

		if (sc.state == SCE_MMIXAL_COMMENT) continue;

		// Determine if the current state should terminate.
		if (sc.state == SCE_MMIXAL_OPERATOR) {			// OPERATOR
			sc.SetState(SCE_MMIXAL_OPERANDS);
		} else if (sc.state == SCE_MMIXAL_NUMBER) {		// NUMBER
			if (!isdigit(sc.ch)) {
				if (IsAWordChar(sc.ch)) {
					char s[100];
					sc.GetCurrent(s, sizeof(s));
					sc.ChangeState(SCE_MMIXAL_REF);
					sc.SetState(SCE_MMIXAL_REF);
				} else {
					sc.SetState(SCE_MMIXAL_OPERANDS);
				}
			}
		} else if (sc.state == SCE_MMIXAL_LABEL) {			// LABEL
			if (!IsAWordChar(sc.ch) ) {
				sc.SetState(SCE_MMIXAL_OPCODE_PRE);
			}
		} else if (sc.state == SCE_MMIXAL_REF) {			// REF
			if (!IsAWordChar(sc.ch) ) {
				char s[100];
				sc.GetCurrent(s, sizeof(s));
				if (*s == ':') {	// ignore base prefix for match
					for (size_t i = 0; i != sizeof(s); ++i) {
						*(s+i) = *(s+i+1);
					}
				}
				if (special_register.InList(s)) {
					sc.ChangeState(SCE_MMIXAL_REGISTER);
				} else if (predef_symbols.InList(s)) {
					sc.ChangeState(SCE_MMIXAL_SYMBOL);
				}
				sc.SetState(SCE_MMIXAL_OPERANDS);
			}
		} else if (sc.state == SCE_MMIXAL_OPCODE_PRE) {	// OPCODE_PRE
				if (!isspace(sc.ch)) {
					sc.SetState(SCE_MMIXAL_OPCODE);
				}
		} else if (sc.state == SCE_MMIXAL_OPCODE) {		// OPCODE
			if (!IsAWordChar(sc.ch) ) {
				char s[100];
				sc.GetCurrent(s, sizeof(s));
				if (opcodes.InList(s)) {
					sc.ChangeState(SCE_MMIXAL_OPCODE_VALID);
				} else {
					sc.ChangeState(SCE_MMIXAL_OPCODE_UNKNOWN);
				}
				if (strcmp(s,"SWYM")==0)
				  sc.SetState(SCE_MMIXAL_COMMENT);
				else
				  sc.SetState(SCE_MMIXAL_OPCODE_POST);
			}
		} else if (sc.state == SCE_MMIXAL_STRING) {		// STRING
			if (sc.ch == '\"') {
				sc.ForwardSetState(SCE_MMIXAL_OPERANDS);
			} else if (sc.atLineEnd) {
				sc.ForwardSetState(SCE_MMIXAL_OPERANDS);
			}
		} else if (sc.state == SCE_MMIXAL_CHAR) {			// CHAR
			if (sc.ch == '\'') {
				sc.ForwardSetState(SCE_MMIXAL_OPERANDS);
			} else if (sc.atLineEnd) {
				sc.ForwardSetState(SCE_MMIXAL_OPERANDS);
			}
		} else if (sc.state == SCE_MMIXAL_REGISTER) {		// REGISTER
			if (!isdigit(sc.ch)) {
				sc.SetState(SCE_MMIXAL_OPERANDS);
			}
		} else if (sc.state == SCE_MMIXAL_HEX) {			// HEX
			if (!isxdigit(sc.ch)) {
				sc.SetState(SCE_MMIXAL_OPERANDS);
			}
		}

		// Determine if a new state should be entered.
		if (sc.state == SCE_MMIXAL_OPCODE_POST ||		// OPCODE_POST
			sc.state == SCE_MMIXAL_OPERANDS) {			// OPERANDS
			if (sc.state == SCE_MMIXAL_OPERANDS && isspace(sc.ch)) {
                  sc.SetState(SCE_MMIXAL_TRAILWS);
            } else if (sc.state == SCE_MMIXAL_OPERANDS && sc.Match(';')) {
				if (IsAWordChar(sc.chNext))
					sc.SetState(SCE_MMIXAL_LABEL);
				else if (isspace(sc.chNext))
			        sc.SetState(SCE_MMIXAL_LEADWS);
				else 
			        sc.SetState(SCE_MMIXAL_COMMENT);
 			} else if (isdigit(sc.ch)) {
				sc.SetState(SCE_MMIXAL_NUMBER);
			} else if (IsAWordChar(sc.ch) || sc.Match('@')) {
				sc.SetState(SCE_MMIXAL_REF);
			} else if (sc.Match('\"')) {
				sc.SetState(SCE_MMIXAL_STRING);
			} else if (sc.Match('\'')) {
				sc.SetState(SCE_MMIXAL_CHAR);
			} else if (sc.Match('$')) {
				sc.SetState(SCE_MMIXAL_REGISTER);
			} else if (sc.Match('#')) {
				sc.SetState(SCE_MMIXAL_HEX);
			} else if (isMMIXALOperator(static_cast<char>(sc.ch))) {
				sc.SetState(SCE_MMIXAL_OPERATOR);
			}
		}
	}
	sc.Complete();
}

static const char * const MMIXALWordListDesc[] = {
	"Operation Codes",
	"Special Register",
	"Predefined Symbols",
	0
};

LexerModule lmMMIXAL(SCLEX_MMIXAL, ColouriseMMIXALDoc, "mmixal", 0, MMIXALWordListDesc);

