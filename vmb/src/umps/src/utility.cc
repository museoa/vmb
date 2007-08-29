/* File: $Id: utility.cc,v 1.1 2007-08-29 09:19:37 ruckert Exp $ */

/****************************************************************************
 *
 * This module contains some utility functions (bit manipulation,
 * etc) used in other modules.
 *
 * It also contains the TimeStamp class and methods; it is used to schedule
 * I/O operations inside the simulator and to handle the system clock.
 * TimeStamp class is very easy to understand: it has been done so to provide
 * some example to be given to C++ beginners. 
 *
 ****************************************************************************/


/****************************************************************************/
/* Inclusion of header files.                                               */
/****************************************************************************/
#include <h/const.h>
#include <h/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <forms.h>


/****************************************************************************/
/* Inclusion of imported declarations.                                      */
/****************************************************************************/


/****************************************************************************/
/* Declarations strictly local to the module.                               */
/****************************************************************************/


/****************************************************************************/
/* Definitions to be exported.                                              */
/****************************************************************************/

// This function is used to signal an abnormal termination of the program
void Panic(const char * message)
{
	fl_ringbell(100);
	fl_show_alert("Abnormal program termination", message, EMPTYSTR, TRUE);
	fl_finish();
	exit(EXIT_FAILURE);
}	


// This function acts as interface for fl_show_alert from form.h: this way, 
// XFORMS library is included in graphic interface modules only
void ShowAlert(const char * s1, const char * s2, const char * s3)
{
	fl_ringbell(100);
	fl_show_alert(s1, s2, s3, TRUE);
}


// This function acts as ShowAlert but also quits the program
void ShowAlertQuit(const char * s1, const char * s2, const char * s3)
{
	fl_ringbell(100);
	fl_show_alert(s1, s2, s3, TRUE);
	fl_finish();
	exit(EXIT_FAILURE);
}

// This function sets to 1 the (bitPos % 32) bit of the word w
Word SetBit (Word w, unsigned bitPos)
{
	return (w | (1 << bitPos));
}


// This function resets to 0 the (bitPos % 32) bit of the word w
Word ResetBit (Word w, unsigned bitPos)
{
	return(w & ((~0) ^ SetBit(0, bitPos)));
}


// This function returns the bitPos bit value in w
Boolean BitVal(Word w, unsigned int bitPos)
{
	return((w >> bitPos) & 1UL);
}


// This function adds the _unsigned_ quantities a1 and a2,
// puts result into dest, and returns TRUE if a overflow occurred,
// FALSE otherwise
Boolean UnsAdd(Word *dest, Word a1, Word a2)
{
	*dest = a1 + a2;

	if (~(a1) < a2)
		// overflow has occurred
		return(TRUE);
	else
		return(FALSE);
}


// This function subtacts the _unsigned_ quantity s2 from s1,
// puts result into dest, and returns TRUE if a underflow occurred,
// FALSE otherwise
Boolean UnsSub(Word *dest, Word s1, Word s2)
{
	*dest = s1 - s2;

	if ( s1 < s2)
		// underflow has occurred
		return(TRUE);
	else
		return(FALSE);
}


// This function adds the _signed_ quantities a1 and a2, puts result into
// dest (casting it to unsigned), and returns TRUE if a overflow occurred,
// FALSE otherwise
Boolean SignAdd(Word *dest, SWord a1, SWord a2) 
{
	*dest = (Word) (a1 + a2);

	if (SIGNBIT(a1) == SIGNBIT(a2) && SIGNBIT(*dest) != SIGNBIT(a1))
		// overflow has occurred: result sign is different from operands'
		return(TRUE); 
	else 
		return(FALSE);
}


// This function subtracts the _signed_ quantity s2 from s1, puts result
// into dest (casting it to unsigned), and returns TRUE if a underflow 
// occurred, FALSE otherwise
Boolean SignSub(Word *dest, SWord s1, SWord s2) 
{
	*dest = (Word) (s1 - s2);

	if (SIGNBIT(s1) != SIGNBIT(s2) && SIGNBIT(*dest) != SIGNBIT(s1)) 
		//underflow has occurred
		return(TRUE); 
	else 
		return(FALSE);
}


// This function multiplies the _unsigned_ quantities m1 and m2,
// returning back the high and low part of the unsigned 64 bit result via
// hip and lop pointers
// Algorithm used is "classical":
// given the 32 bit quantities AB and CD, divided into high and low 16 bit 
// parts A, B, C and D
//
//              AB x
//              CD =
//             ------
//             AD.BD +
//          AC.BC.0
//         ----------
//
// where AD, BD etc. are the 32 bit results of the multiplication of A by D,
// etc., and X.Y means (X << 16) + Y to allow the addition of the intermediate
// results. 
// This chunk of code (C) J. Larus (SPIM, 1990) (with minor modifications)
void UnsMult(Word m1, Word m2, Word * hip, Word * lop)
{
	Word a, b, c, d, x, y;
	
	a = (m1 & ~(IMMMASK)) >> HWORDLEN;
	b = (m1 & IMMMASK);
	c = (m2 & ~(IMMMASK)) >> HWORDLEN;
	d = (m2 & IMMMASK);
	
	*lop = b * d;
	x = (a * d) + (b * c);
	y = (((*lop) >> HWORDLEN) & IMMMASK) + x;
	
	*lop = ((*lop) & IMMMASK) | ((y & IMMMASK) << HWORDLEN);
	*hip = ((y >> HWORDLEN) & IMMMASK) + (a * c);
}


// This function multiplies the _signed_ quantities m1 and m2,
// returning back the high and low part of the signed 64 bit result
// via hip and lop pointers
// This too (C) J. Larus (SPIM, 1990)
void SignMult(SWord m1, SWord m2, SWord * hip, SWord * lop)
{
	Boolean negResult = FALSE;

	// convert negative numbers to positive for unsigned multipl.
	// and keep track of result sign
	if (m1 < 0)
	{
		negResult = !negResult;
		m1 = - m1;
	}
	if (m2 < 0)
	{
		negResult = !negResult;
		m2 = - m2;
	}
	
	UnsMult((Word) m1, (Word) m2, (Word *) hip, (Word *) lop);
	
	if (negResult)
	{
		// must 2-complement result (and keep count of *lop -> *hip carry)
		
		// 1-complement
		*hip = ~(*hip);
		*lop = ~(*lop);
		
		// add 1 to lower word to get 2-complement and check for carry
		if (UnsAdd((Word *) lop, (Word) (*lop), 1UL))
			// overflow occurred: carry out to hip
			(*hip)++;
	}
} 


// This function prints a variable list of arguments to the standard
// error, and waits for an input to continue. Used for debugging
void trace(char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	
	getchar();
}

// This function converts a string to a Word (typically, an address) value.
// Returns TRUE if conversion was successful, FALSE otherwise
Boolean StrToWord(const char * str, Word * value)
{
	char * endp;
	Boolean valid = TRUE;

	// tries to convert the string into a unsigned long
	*value = strtoul(str, &endp, 0); 
	
	if (endp != NULL)
	{
		// there may be some garbage
		while (*endp != EOS && valid)
		{
			if (!isspace(*endp))
				valid = FALSE;
			endp++;
		}
	}
	return(valid);
} 

	
/****************************************************************************/

// This class implements the TimeStamps used for the system clock and
// in the Event objects to schedule device operations and interrupts.
// Every object is a 64 bit counter, split into two 32 bit parts (high & low)
// This class has been built so simple to have some example to be shown to
// C++ beginners

class TimeStamp
{
	public:
		// This method creates a new TS and initializes it (by default to 0, 0)
		TimeStamp(Word hi = 0, Word lo = 0);
		
		// This method creates a new TS and sets it to the value of another,
		// plus an (optional) increment
		TimeStamp(TimeStamp * ts, Word inc = 0);

		// Object deletion uses default handler
		
		// This method increases the TS by 1
		void Increase(void);
		
		// This method returns the current value of hiTS part
		Word getHiTS(void);
		
		// This method returns the current value of loTS part
		Word getLoTS(void);
		
		// This method sets hiTS value
		void setHiTS(Word hi);
		
		// This method sets loTS value
		void setLoTS(Word lo);
		
		// This method compares 2 TSs and returns TRUE if the first is
		// less than or equal to the second, FALSE otherwise
		Boolean LessEq(TimeStamp * ts2);
		
	private:
		// high part of the TimeStamp
		Word hiTS;		
		// low part
		Word loTS;
};


// This method creates a new TS and initializes it (by default to 0, 0)
TimeStamp::TimeStamp(Word hi, Word lo)
{
	loTS = lo;
	hiTS = hi;
}


// This method creates a new TS and sets it to the value of another, plus 
// an (optional) increment
TimeStamp::TimeStamp(TimeStamp * ts, Word inc)
{
	loTS = ts->getLoTS();
	hiTS = ts->getHiTS();
	if (UnsAdd(&loTS, loTS, inc))
		// Overflow occurred: hiTS need to be increased
		hiTS++;
}


// This method increases the TS by 1 
void TimeStamp::Increase(void)
{
	if (UnsAdd(&loTS, loTS, 1UL))
		// unsigned overflow occurred: need to increase HiTS
		hiTS++;
}


// This method returns the current value of hiTS part
Word TimeStamp::getHiTS(void)
{
	return(hiTS);
} 


// This method returns the current value of loTS part
Word TimeStamp::getLoTS(void)
{
	return(loTS);
}


// This method sets hiTS value
void TimeStamp::setHiTS(Word hi)
{
	hiTS = hi;
} 


// This method sets loTS value
void TimeStamp::setLoTS(Word lo)
{
	loTS = lo;
}


// This method compares 2 TSs and returns TRUE if the first is 
// less than or equal to the second, FALSE otherwise
Boolean TimeStamp::LessEq(TimeStamp * ts2)
{
	if (hiTS < ts2->hiTS || (hiTS == ts2->hiTS && loTS <= ts2->loTS))
		return(TRUE);
	else 
		return(FALSE);
} 


/****************************************************************************/
/* Definitions strictly local to the module.                                */
/****************************************************************************/
