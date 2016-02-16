/* File: $Id: utility.e,v 1.2 2008-05-04 15:46:59 mbbh Exp $ */

/****************************************************************************
 *
 * External declarations from the corresponding module .cc
 *
 ****************************************************************************/

// This function is used to signal an abnormal termination of the program
void Panic(const char * message);


// This function acts as interface for fl_show_alert from form.h: this way, 
// XFORMS library is included in graphic interface modules only
void ShowAlert(const char * s1, const char * s2, const char * s3);

// This function acts like ShowAlert but also quits the program
void ShowAlertQuit(const char * s1, const char * s2, const char * s3);


// This function sets to 1 the (bitPos % 32) bit of the word w
Word SetBit (Word w, unsigned int bitPos);


// This function resets to 0 the (bitPos % 32) bit of the word w
Word ResetBit (Word w, unsigned int bitPos);


// This function returns the bitPos bit value in w
Boolean BitVal(Word w, unsigned int bitPos);


// This function adds the _unsigned_ quantities a1 and a2,
// puts result into dest, and returns TRUE if a overflow occurred,
// FALSE otherwise
Boolean UnsAdd(Word *dest, Word a1, Word a2);


// This function subtacts the _unsigned_ quantity s2 from s1,
// puts result into dest, and returns TRUE if a underflow occurred,
// FALSE otherwise
Boolean UnsSub(Word *dest, Word s1, Word s2);


// This function adds the _signed_ quantities a1 and a2, puts result into
// dest (casting it to unsigned), and returns TRUE if a overflow occurred,
// FALSE otherwise
Boolean SignAdd(Word *dest, SWord a1, SWord a2);


// This function subtracts the _signed_ quantity s2 from s1, puts result
// into dest (casting it to unsigned), and returns TRUE if a underflow 
// occurred, FALSE otherwise
Boolean SignSub(Word *dest, SWord s1, SWord s2);


// This function multiplies the _unsigned_ quantities m1 and m2,
// returning back the high and low part of the unsigned 64 bit result via
// hip and lop pointers
void SignMult(SWord m1, SWord m2, SWord * hip, SWord * lop);


// This function multiplies the _signed_ quantities m1 and m2,
// returning back the high and low part of the signed 64 bit result
// via hip and lop pointers
void UnsMult(Word m1, Word m2, Word * hip, Word * lop);


// This function prints a variable list of arguments to the standard
// error, and waits for an input to continue. Used for debugging
void trace(char *format, ...);


// This function converts a string to a Word (typically, an address) value.
// Returns TRUE if conversion was successful, FALSE otherwise
Boolean StrToWord(const char * str, Word * value);

void perfStat(unsigned int uiSlot,Boolean bIsLeaving);
void perfPrint();
void perfReset();

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


