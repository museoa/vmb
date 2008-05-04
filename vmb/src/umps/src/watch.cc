/* File: $Id: watch.cc,v 1.2 2008-05-04 15:46:59 mbbh Exp $ */

/****************************************************************************
 *
 * This module defines Watch class and support classes. 
 *
 * A Watch object is used to interface simulator system with user interface,
 * but Watch object is carefully built to be consistent to _interface_, not
 * system status: Watch update its internal status (to reflect changes in
 * system status) only when interface status itself needs to be changed;
 * this primarily for performance reasons.
 *
 * Watch closely monitors simulation, and stops it when breakpoints or
 * suspects are reached, or exceptions happen. To do this, it uses a number
 * of specialized classes for tracking ranges, symbol table contents, etc.
 *
 ****************************************************************************/

/****************************************************************************/
/* Inclusion of header files.                                               */
/****************************************************************************/
#include <h/const.h>
#include <h/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <forms.h>

#include <h/blockdev.h>
#include <h/aout.h>


/****************************************************************************/
/* Inclusion of imported declarations.                                      */
/****************************************************************************/

#include <e/utility.e>
#include <e/disassemble.e>
#include <e/setup.e>
#include <e/xinterface.e>
#include <e/systembus.e>
#include <e/processor.e>
// #include <e/device.e>


/****************************************************************************/
/* Declarations strictly local to the module.                               */
/****************************************************************************/


// CP0 and miscellaneous register index used inside Watch
#define WCP0REGOFFS	34
#define NEXTPCNUM	43
#define SUCCPCNUM	44
#define TODHINUM	45
#define TODLONUM	46
#define TIMERNUM	47
#define PREVPPCNUM	48
#define CURRPPCNUM	49
#define STATUS		5

// KUc bit in STATUS
#define KUCBITPOS 1

// static buffer size and definition
#define STRBUFSIZE	64
HIDDEN char strbuf[STRBUFSIZE];

// names for specific memory ranges
#define BOOTAREANAME	"BOOTROM"
#define BIOSAREANAME	"EXECROM"

// search failure flag
#define NOTFOUND	-1

// misellaneous register names for interface
HIDDEN char * othRegName[] = { 	"nextPC",
								"succPC",
								"ToDHI",
							    "ToDLO",
								"Timer",
								"prevPhysPC",
								"currPhysPC",
						};

// simulation status strings: each one matches a Watch status value
HIDDEN char * statStr[] = {	"Running",
							"Stop:USER",
							"Stop:BRKPT",
							"Stop:USER+BRKPT",
							"Stop:SUSP",
							"Stop:USER+SUSP",
							"Stop:BRKPT+SUSP",
							"Stop:USER+BRKPT+SUSP",
							"Stop:EXC(",
							"Stop:USER+EXC(",
							"Stop:BRKPT+EXC(", 
							"Stop:USER+BRKPT+EXC(", 
							"Stop:SUSP+EXC(",
							"Stop:USER+SUSP+EXC(",
							"Stop:BRKPT+SUSP+EXC(",
							"Stop:USER+BRKPT+SUSP+EXC("
							};

// simulation display update speed strings
HIDDEN char * speedNames[SPEEDNUM] = {	"Slowest",
										"Slower", 
										"Slow",
										"Normal",
										"Fast",
										"Faster",
										"Fastest"
										};


// Watch status bit masks, values and bit positions

#define USERSTOP	1
#define BRKPTSTOP	2
#define SUSPECTSTOP	4
#define EXCSTOP	8
#define RUNNING	0

/****************************************************************************/

// Range class defines breakpoint/suspect/trace ranges (inserted by user and
// monitored by Watch) each as a single object. Each Range object has an
// access mask, telling if it is for a breakpoint (EXEC access), a suspect
// (READ, WRITE or both accesses), or trace (only WRITE access).
// ASID for physical addresses is tagged with MAXASID value.
class Range
{
	public:
	
		// This method creates a new Range object
		Range(Word asid, Word start, Word end, Word access);

		// Object deletion is done by default handler

		// This method returns TRUE if "this" object is less than rLine
		// object, FALSE otherwise.  Lexicographic order on ranges is: first
		// ASID in ascending order, then start address, always ascending:
		// end does not matter since tables will not contain overlapping
		// ranges
		Boolean Less(Range * rLine);
		
		// Given an (ASID, virtual address) pair, this method returns 0 if
		// "this" object _contains_ the address, -1 if it lexicographically
		// precedes the address, and 1 otherwise.
		int Contains(Word asid, Word addr);

		// This method checks if the access given as parameter is compatible
		// with "this" object's access returning TRUE or FALSE as a result
		Boolean Match(Word access);

		// This method returns the internal contents of a Range object
		void getRange(Word * asidp, Word * startp, Word * endp, Word * accessp);
		
	private:
		// Range ASID, start and end virtual addresses, and access type 
		Word rASID, rStart, rEnd, rAccess;
};


// This method creates a new Range object
Range::Range(Word asid, Word start, Word end, Word access)
{
	rASID = asid;
	rStart = start;
	rEnd = end;
	rAccess = access;
}

// This method returns TRUE if "this" object is less than rLine object,
// FALSE otherwise.  Lexicographic order on ranges is: first ASID in
// ascending order, then start address, always ascending: end does not
// matter since tables will not contain overlapping ranges
Boolean Range::Less(Range * rLine)
{
	if (rASID < rLine->rASID || (rASID == rLine->rASID && rStart < rLine->rStart))
		return(TRUE);
	else
		return(FALSE);
}

// Given an (ASID, virtual address) pair, this method returns 0 if "this"
// object _contains_ the address, -1 if it lexicographically precedes the 
// address, and 1 otherwise.
// Note as access is not considered into range matching: this make it
// quicker and, together with no overlapping ranges into same table,
// efficient
int Range::Contains(Word asid, Word addr)
{
	if (rASID > asid || (rASID == asid && rStart > addr))
		return(-1);
	else
		if (rASID < asid || (rASID == asid && addr > rEnd))
			return(1);
		else
			// hit found
			return(0);
}


// This method checks if the access given as parameter is compatible with
// "this" object's access returning TRUE or FALSE as a result
Boolean Range::Match(Word access) 
{
	return((rAccess & access) != EMPTY);
}

// This method returns the internal contents of a Range object
void Range::getRange(Word * asidp, Word * startp, Word * endp, Word * accessp)
{
	*asidp = rASID;
	*startp = rStart;
	*endp = rEnd;
	*accessp = rAccess;
}


/****************************************************************************/

// This class defines objects containing a Range table.A RangeTable objects
// contains all kind of Range objects: they are lexicographically ordered.
// Two ranges which clash for both address range _and_ access cannot be
// inserted, to allow fast table probe.

class RangeTable
{
	public:

		// This method creates an empty range table
		RangeTable(void);
		
		// This method deletes a range table object and all contained ranges
		~RangeTable(void);

		// This method inserts a range in the table, creating a new Range
		// object and preserving order: range insertion is not allowed for a
		// range clashing with those already inside the table
		// It returns the index position for a correct insertion, plus one
		// (so clashes may be detected)
		Boolean Insert(Word asid, Word start, Word end, Word access);

		// This method deletes the range at line index, shuffling others
		void Delete(unsigned int line);

		// This method probes the range table, given a complete virtual
		// address with access type, searching for a range containing it: it
		// returns the Range index plus one if a match is found, and 0
		// otherwise.
		unsigned int Probe(Word asid, Word addr, Word access);

		// This method returns the number of ranges contained in the table
		unsigned int getRangeLNum(void);

		// This method returns the contents of Range at line index requested
		void getRangeL(unsigned int line, Word * asidp, Word * startp, Word * endp, Word * accessp);
			
	private:
		// number of range lines
		unsigned int lineNum;
		
		// pointer to Range table
		Range ** rTab;
};


// This method creates an empty range table
RangeTable::RangeTable(void)
{
	lineNum = 0;
	rTab = NULL;
}


// This method deletes a range table object and all contained ranges
RangeTable::~RangeTable(void)
{
	unsigned int i;
	
	if (rTab != NULL)
	{
		// table not empty
		for (i = 0; i < lineNum; i++)
			delete (rTab[i]);
	
		delete rTab;
	}
}	


// This method inserts a range in the table, creating a new Range object and preserving order: range
// insertion is not allowed for a range clashing with those already inside
// the table. It returns the index position for a correct insertion, plus one
// (so clashes may be detected)
Boolean RangeTable::Insert(Word asid, Word start, Word end, Word access)
{
	unsigned int i, pos;
	Boolean found, isLess;
	Word accTest;
	Range * newR;
	Range ** newT;
	
	if (rTab == NULL) 
	{
		// no ranges inserted
		lineNum = 1;
		rTab = new Range * [1];
		rTab[0] = new Range(asid, start, end, access);
		return(TRUE);
	}
	else
	{
		// a new range cannot collide with others in the same table, even if
		// access is different (eg. READ vs. WRITE)
		if (access & READWRITE)
			accTest = access | READWRITE;
		else 
			// this for EXEC type ranges
			accTest = access;
			
		// probes table
		for (i = start, found = FALSE; i <= end && !found; i += WORDLEN)
			found = (Probe(asid, i, accTest) != 0);
		
		if (!found)
		{
			// insertion is possible: search for position and copy old table into new
			newR = new Range(asid, start, end, access);
			newT = new Range * [lineNum + 1];
	
			// searches for position: after last entry "less" than new one 
			pos = 0; 
			isLess = TRUE; 
			while (pos < lineNum && isLess)
				if ((isLess = (rTab[pos])->Less(newR)))
				{
					// copies old table entry into new table
					newT[pos] = rTab[pos];
					pos++;
				}
			
			// position found (worst case: pos == lineNum)
			newT[pos] = newR;
			lineNum++;
			
			// copies remaining old table entries into new one, if any
			for (i = pos + 1; i < lineNum; i++)
				newT[i] = rTab[i - 1];
			
			// almost done: delete old pointer table and keep new
			delete rTab;
			rTab = newT;
		}	
		// return value: 0 if no insertion, else line position 
		// (one more than real position)
		if (found)
			return(FALSE);
		else
			return(TRUE);
	}
}	


// This method deletes the range at line index, shuffling others
void RangeTable::Delete(unsigned int line)
{
	Range ** newT = NULL;
	unsigned int i;
	
	if (rTab == NULL || line == 0 || line > lineNum)
		Panic("Invalid line number in RangeTable::Delete()");
	
	// else line is correct: constructs new table with one line less
	if (lineNum > 1)
	{
		newT = new Range * [lineNum - 1];

		for (i = 0; i < line - 1; i++)
			newT[i] = rTab[i];
	
		for (i = line; i < lineNum; i++)
			newT[i - 1] = rTab[i]; 
	
		delete (rTab[line - 1]);
		delete rTab;
		rTab = newT;
		lineNum--;	
	}
	else
	{ 
		// table will be emptied 
		delete (rTab[0]);
		delete rTab;
		rTab = NULL; 
		lineNum = 0;
	}
}

// This method probes the range table, given a complete virtual address with
// access type, searching for a range containing it: it returns the Range
// index plus one if a match is found, and 0 otherwise.
// An almost "classical" binary search
unsigned int RangeTable::Probe(Word asid, Word addr, Word access)
{
	int a, b, c, res;
	Boolean found;
	
	if (rTab == NULL)
		// empty table
		return(0);
	else
	{
		// probe table
		a = 0;
		c = lineNum - 1;
		found = FALSE;
		do
		{
			b = (a + c) / 2;
			if ((res = (rTab[b])->Contains(asid, addr)) == 0)
				found = TRUE;
			else
				// res is > 0 if addr is (possibly) in an area with
				// asid or start address greater than current, < 0 otherwise
				if (res > 0)
					a = b + 1;
				else
					c = b - 1;
		}
		while (!found && a <= c);
		
		if (found && (rTab[b])->Match(access))		
			// line number that matches (one more, really)
			return(b + 1);
		else
			// no valid range found or no access match
			return(0);
	}
}	


// This method returns the number of ranges contained in the table
unsigned int RangeTable::getRangeLNum(void)
{
	return(lineNum);
}


// This method returns the contents of Range at line index requested
void RangeTable::getRangeL(unsigned int line, Word * asidp, Word * startp, Word * endp, Word * accessp)
{
	if (rTab == NULL || line == 0 || line > lineNum)
		Panic("Invalid line number in RangeTable::getRangeL()");
	else
		(rTab[line - 1])->getRange(asidp, startp, endp, accessp);
}


/****************************************************************************/


// Symbol class objects represents a symbol table entry each.  They are
// similar to Range objects: they just do not need access type and are
// associated to just one ASID (contained in SetupInfo object)

class Symbol
{
	public:
		// This method builds a Symbol object, given its description as produced by
		// elf2mps utility; no format check, since it's fixed
		Symbol(const char * sName, const char * sInfo);
		
		// This method deletes a Symbol object and contained items
		~Symbol(void);
		
		// This method defines a lexicographic order on Symbols, based on
		// start address; returns TRUE if "this" object is less than Symb,
		// and FALSE otherwise
		Boolean Less(Symbol * symb);
		
		// Given pos virtual address, this method returns -1 if pos is
		// out lower bound, 0 if in, 1 if out upper bound
		int Contains(Word pos);
		
		// This method returns Symbol name
		const char * SymName(void);
		
		//
		// These methods allow to inspect Symbol contents
		//
		
		Word SymStart(void);
		Word SymEnd(void);
		SWord Offset(Word pos);
	
		// this method allows to set symbol size (backpatch for gcc bug)
		void setSize(Word size);
		
	private:
		
		// Symbol name string
		char * sNamep;
		
		// Symbol starting address
		Word sStart;
		
		// Symbol size
		Word sSize;
};


// This method builds a Symbol object, given its description as produced by
// elf2mps utility; no format check, since it's fixed
Symbol::Symbol(const char * sName, const char * sInfo)
{
	const char * nump;
	 
	sNamep = new char [strlen(sName) + 1];
	strcpy(sNamep, sName);
	
	// get start and size
	nump = strchr(sInfo, ':') + 1;
	StrToWord(nump, &sStart);
	nump = strchr(nump, ':') + 1;
	StrToWord(nump, &sSize);

	// backpatch for gcc 3.3.3 "size == 0" bug
	// and for very small objects
	if (sSize < WORDLEN)
		sSize = WORDLEN;
}


// This method deletes a Symbol object and contained items
Symbol::~Symbol(void)
{
	delete sNamep;
}


// This method defines a lexicographic order on Symbols, based on start
// address; returns TRUE if "this" object is less than Symb, and FALSE
// otherwise
Boolean Symbol::Less(Symbol * symb)
{
	if (this->sStart < symb->sStart)
		return(TRUE);
	else
		return(FALSE);
}


// Given pos virtual address, this method returns -1 if pos is
// out lower bound, 0 if in, 1 if out upper bound
int Symbol::Contains(Word pos)
{
	if (sStart > pos)
		return(-1);
	else
		if (pos >= (sStart + sSize))
			return(1);
		else
			return(0);
}


//
// These methods allow to inspect Symbol contents
//

const char * Symbol::SymName(void)
{
	return(sNamep);
}


Word Symbol::SymStart(void)
{
	return(sStart);
}


Word Symbol::SymEnd(void)
{
		return(sStart + sSize - WORDLEN);
}


SWord Symbol::Offset(Word pos)
{
	return((SWord) (pos - sStart));
}


void Symbol::setSize(Word size)
{
	sSize = size;
}

/****************************************************************************/


// SymTable class defines an object containing a complete symbol table. It
// is loaded from file, and is associated to a peculiar ASID, which comes
// from SetupInfo object.  
// For performance reasons, symbol table is split into two parts: one for
// functions and one for memory objects, since functions table wiil be
// accessed more often. Both SymTable object tables are lexicographically
// ordered for fast probing, function table is accessed for any instruction
// reference; both are inspected for mapping breakpoint/suspect/trace ranges
// to mnemonic names.

class SymTable
{
	public:
	
		// This method builds a symbol table from .stab file fName produced by
		// elf2mps utility
		SymTable(Word asid, const char * fName);

		// This method deletes the table and its contents
		~SymTable(void);
		
		// This method probes the table, given a complete address (asid +
		// pos): it probes both tables if fullSearch is TRUE, and only
		// function table otherwise.  It returns symbol name and offset
		// inside it, or NULL if no match is found
		const char * Probe(Word asid, Word pos, Boolean fullSearch, SWord * offsetp);
		
		// This method returns the total number of symbols
		unsigned int getSymNum(void);
		
		// Given the symbol number in [0.. getSymNum() -1] range, this
		// method returns symbol contents (and if it is a function or not,
		// as told by isFunp flag)
		const char * getSymData(unsigned int symNum, Boolean * isFunp, Word * startp, Word * endp);

	private:
		
		// Symbol table ASID from SetupInfo object
		Word symASID;
		
		// number of function symbols
		unsigned int fNum;
		
		// number of memory object symbols
		unsigned int oNum;
		
		// Symbol tables: one for functions, other for memory object symbols
		Symbol ** fTable;
		Symbol ** oTable;
		
		// This method sorts a loaded table with quicksort algorithm		
		void sortSymTab(Symbol ** table, int start, int end);
		
		// This method scans the specified table looking for a Symbol range
		// containing it; returns NOTFOUND if not found, 0..size -1 if found
		// (index into table)
		int symProbe(Symbol ** table, unsigned int size, Word pos);
};


// This method builds a symbol table from .stab file fName produced by
// elf2mps utility
SymTable::SymTable(Word asid, const char * fName)
{
	FILE * sFile = NULL;
	Word tag;
	unsigned int numF, numO, i;
	char sName[STRBUFSIZE];
	char sType[STRBUFSIZE];
	Boolean error;

	symASID = asid;
	fTable = NULL;
	fNum = 0;
	oTable = NULL;
	oNum = 0;

	// tries to access file
	if (fName == NULL || SAMESTRING(fName, EMPTYSTR) || \
		(sFile = fopen(fName, "r")) == NULL || \
		fread((void *) &tag, WORDLEN, 1, sFile) != 1 || \
		tag != STABFILEID || fscanf(sFile, "%X %X ", &numF, &numO) != 2)
	{
		// some error occurred
		if (fName == NULL)
			fName = EMPTYSTR;
		ShowAlert("Unable to load symbol table file:", fName, "Wrong header");
	}		
	else
	{
		// inits symbol table structures
		fNum = numF;
		oNum = numO;
		if (fNum > 0)
		{
			fTable = new Symbol * [fNum];
			for (i = 0; i < fNum; i++)
				fTable[i] = NULL;
		}
		if (oNum > 0) 
		{
			oTable = new Symbol * [oNum];
			for (i = 0; i < oNum; i++)
				oTable[i] = NULL; 
		}				
		
		// scans symbol table file and builds objects
		error = FALSE;
		for (i = 0, numF = 0, numO = 0; i < (fNum + oNum) && !error; i++)
			if (fscanf(sFile, "%s :%s ", sName, sType) != 2)
				error = TRUE;
			else
				if (sType[0] == 'F')
				{
					fTable[numF] = new Symbol(sName, sType);
					numF++;
				}
				else
				{
					// sType[0] == 'O'
					oTable[numO] = new Symbol (sName, sType);
					numO++;
				}
		
		if (error)
		{
			ShowAlert("Unable to load symbol table file:", fName, "Wrong format");
			
			// clean up
			if (fNum > 0)
			{
				for (i = 0; i < fNum; i++)
					if (fTable[i] != NULL)
						delete (fTable[i]);
				delete fTable;
				fTable = NULL;
			}
			fNum = 0;
			
			if (oNum > 0)
			{
				for (i = 0; i < oNum; i++)
					if (oTable[i] != NULL)
						delete (oTable[i]);
				delete oTable;
				oTable = NULL;
			}
			oNum = 0;
		}
		else
		{
			// sort tables
			if (fNum > 1)
			{
				sortSymTab(fTable, 0, fNum - 1);

				// backpatch FUN part of symbol table for gcc bug
				for (i = 0; i < (fNum - 1); i++)
					if (fTable[i]->SymEnd() == fTable[i]->SymStart())
						//backpatch needed
						fTable[i]->setSize(fTable[i+1]->SymStart() - fTable[i]->SymStart());
			}
			
			if (oNum > 1)
				sortSymTab(oTable, 0, oNum - 1);
		}
	}
	if (sFile != NULL)
		fclose(sFile);
}


// This method deletes the table and its contents
SymTable::~SymTable(void)
{
	unsigned int i;
	
	if (fTable != NULL)
	{
		for (i = 0; i < fNum; i++)
			if (fTable[i] != NULL)
				delete(fTable[i]);
		delete fTable;
	}
	if (oTable != NULL)
	{
		for (i = 0; i < oNum; i++)
			if (oTable[i] != NULL)
				delete(oTable[i]);
		delete oTable;
	}
}


// This method probes the table, given a complete address (asid +
// pos): it probes both tables if fullSearch is TRUE, and only
// function table otherwise.  It returns symbol name and offset of pos
// inside it, or NULL if no match is found
const char * SymTable::Probe(Word asid, Word pos, Boolean fullSearch, SWord * offsetp)
{
	int idx;
	
	if (asid != symASID)
		return(NULL);
	else
		// scans function table
		if ((idx = symProbe(fTable, fNum, pos)) != NOTFOUND)
		{
			*offsetp = (fTable[idx])->Offset(pos);
			return((fTable[idx])->SymName());
		}
		else
			// tries in memory object table
			if (fullSearch && (idx = symProbe(oTable, oNum, pos)) != NOTFOUND)
			{
				*offsetp = (oTable[idx])->Offset(pos);
				return((oTable[idx])->SymName());
			}
			else
				return(NULL);
}


// This method returns the total number of symbols
unsigned int SymTable::getSymNum(void)
{
	return(oNum + fNum);
}


// Given the symbol number in [0.. getSymNum() -1] range, this
// method returns symbol contents (and if it is a function or not,
// as told by isFunp flag)
const char * SymTable::getSymData(unsigned int symNum, Boolean * isFunp, Word * startp, Word * endp)
{
	Symbol ** tab;
	
	if (symNum == 0 || symNum > (oNum + fNum))
	{
		Panic("Invalid line number in SymTable::getSymData()");
		// dummy return
		return(NULL);
	}
	else
	{
		symNum--;
		if (symNum < fNum)
		{
			tab = fTable;
			*isFunp = TRUE;
		}
		else
		{
			tab = oTable;
			symNum -= fNum;
			*isFunp = FALSE;
		}
		*startp = (tab[symNum])->SymStart();
		*endp = (tab[symNum])->SymEnd();
		return((tab[symNum])->SymName());
	}
}


// This method scans the specified table looking for a Symbol range
// containing it; returns NOTFOUND if not found, 0..size -1 if found (index
// into table)
// Almost classical binary search
int SymTable::symProbe(Symbol ** table, unsigned int size, Word pos)
{
	int a, b, c, res;
	Boolean found;
	
	if (table == NULL)
		return(NOTFOUND);
	else
	{
		a = 0;
		c = size - 1;
		found = FALSE;
		do
		{
			b = (a + c) / 2;
			if ((res = (table[b])->Contains(pos)) == 0)
				found = TRUE;
			else
				// res is > 0 if pos is (possibly) in an area with
				// start address greater than current, < 0 otherwise
				if (res > 0)
					a = b + 1;
				else
					c = b - 1;
		}
		while (!found && a <= c);
	
		if (found)
			return(b);
		else
			return(NOTFOUND);
	}
}
		

// This method sorts a loaded table with quicksort algorithm
void SymTable::sortSymTab(Symbol ** table, int start, int end)
{
	int i, j;
	Symbol * x, * w;
	
	i = start;
	j = end;
	x = table[(start + end) / 2];
	do
	{
		while ((table[i])->Less(x))
			i++;
		while (x->Less(table[j]))
			j--;
			
		if (i <= j)
		{
			w = table[i];
			table[i] = table[j];
			table[j] = w;
			i++;
			j--;
		}
	}
	while (i <= j);	

	if (start < j)
		sortSymTab(table, start, j);
		
	if (i < end)
		sortSymTab(table, i, end);
}		

					
/****************************************************************************/


// WatchTable class object acts as interface between RangeTable and and
// Watch objects. It encapsulates those table, and provides a unified
// interface to Watch, allowing to change table internal structure without
// affecting it. WatchTable survives simulation reset too: this allows to
// avoid re-entering all the ranges between resets.

// Method listing is almost self-explaining: refer to previous RangeTable
// descriptions for details 

class WatchTable 
{
	public:

		WatchTable(void);
		~WatchTable(void);
		Boolean RangeIns(unsigned int type, Word asid, Word start, Word end, Word access);
		void RangeDel(unsigned int type, unsigned int line);
		unsigned int getRangeLineNum(unsigned int type);
		void getRangeLine(unsigned int type, unsigned int line, Word * asidp, Word * startp, Word * endp, Word * accessp);

		// look how there are specialized Probe() for different ranges, but
		// only one Probe() method for RangeTable

		unsigned int BrkptProbe(Word asid, Word addr, Word access);
		unsigned int SuspProbe(Word asid, Word addr, Word access);
		Boolean TraceProbe(Word addr);
		
	private:
		
		RangeTable * rTable[RTABNUM];
};


WatchTable::WatchTable(void)
{
	unsigned int i;
	
	for (i = 0; i < RTABNUM; i++)
		rTable[i] = new RangeTable();
}


WatchTable::~WatchTable(void)
{
	unsigned int i;
	
	for (i = 0; i < RTABNUM; i++)
		if (rTable[i] != NULL)
			delete (rTable[i]);
}	


Boolean WatchTable::RangeIns(unsigned int type, Word asid, Word start, Word end, Word access)
{
	return((rTable[type])->Insert(asid, start, end, access));
}


void WatchTable::RangeDel(unsigned int type, unsigned int line)
{
	(rTable[type])->Delete(line);
}


unsigned int WatchTable::getRangeLineNum(unsigned int type)
{
	return((rTable[type])->getRangeLNum());	
}


void WatchTable::getRangeLine(unsigned int type, unsigned int line, Word * asidp, Word * startp, Word * endp, Word * accessp)
{
	(rTable[type])->getRangeL(line, asidp, startp, endp, accessp);
}	

//
// look how there are specialized Probe() for different ranges, but
// only one Probe() method for RangeTable
//

unsigned int WatchTable::BrkptProbe(Word asid, Word addr, Word access)
{
	return((rTable[BRKPT])->Probe(asid, addr, access));
}


unsigned int WatchTable::SuspProbe(Word asid, Word addr, Word access)
{
	return((rTable[SUSP])->Probe(asid, addr, access));
}
	

Boolean WatchTable::TraceProbe(Word addr)
{
	return((rTable[TRACE])->Probe(MAXASID, addr, WRITE) != 0);
}


// WatchTable should survive to simulation reset
HIDDEN 	WatchTable * wTable = NULL;


/****************************************************************************/
/* Definitions to be exported.                                              */
/****************************************************************************/

// This class interfaces GUI with simulated machine (Processor+SystemBus).
// Many of its methods simply match those found in Processor and SystemBus
// classes. 
// It controls simulated machine advancements, and allows to change its
// internal state variables (like processor registers and device operation).
// To achieve a superior performance, Watch implements double-buffering:
// that is, it backups internally many of the information it collects about
// simulated machine state, and warn interface to update displayed contents
// only when internal state changes. By careful use of double buffering,
// graphical interface update times shortens considerably.
// Another task performed by Watch class is monitoring address generation by
// Processor and SystemBus, to be able to stop simulation whenever a
// breakpoint or suspected address shows up. It also employs a similar
// technique to update memory traced contents (shown by user interface),
// only when memory contents change.

class Watch
{
	public:
		// This method creates a new Watch object and all simulated machine
		// structures: the newHTable flag controls WatchTable structures
		// creation
		Watch(SetupInfo * stp, XInterface * xi, Boolean newHTable);
		
		// This method deletes a Watch object and all simulated machine
		// structures
		~Watch(void);
		
		// This method allows to delete WatchTable item only: this way, it
		// may survive resets 
		void DeleteHTable(void);
		
		// This method runs the simulated machine for a number of steps; it
		// stop running whenever some conditions are met (breakpoint
		// reached, suspect referenced, exception detected) or stepNum steps
		// are performed
		void Step(unsigned int stepNum);
		
		// This method runs the simulated machine; it stop running whenever
		// some conditions are met (breakpoint reached, suspect referenced,
		// exception detected)
		void Run(void);

		// These methods allow SystemBus, Processor and user interface to
		// signal internal events and user stop
		//
		void SignalBusAccess(Word physAddr, Word access);
		void SignalProcVAccess(Word asid, Word vAddr, Word access);
		void SignalProcExc(unsigned int excCause);
		void SignalUserStop(void);
		
		// These methods allow user interface to set and reset stop
		// conditions and simulation update speed for Watch object, and to
		// set current simulation status
		//
		const char * getStatusStr(void);
		void setSuspStop(Boolean value);
		void setBrkptStop(Boolean value);
		void setExcStop(Boolean value);
		void setUTLBStopK0(Boolean Value);
		void setUTLBStopK1(Boolean Value);
		void setUpdSpeed(unsigned int value);
		Boolean getSuspStop(void);
		Boolean getExcStop(void);
		Boolean getUTLBStopK0(void);
		Boolean getUTLBStopK1(void);
		Boolean getBrkptStop(void);
		unsigned int getUpdSpeed(void);
		const char * getUpdSpeedStr(void);

		// These methods allow to load from disk and access symbol table
		// data, and to insert and delete address ranges for breakpoint,
		// suspects, and traced memory areas, and to know which one stopped
		// the simulation
		//
		void LoadSymbolTable(void);
		unsigned int getSymbolNum(void);
		void getSymbolData(unsigned int symNum, Word * startp,  Word * endp, Boolean * isFunp);
		const char * getSymbolStr(unsigned int symNum);
		Boolean RangeInsert(unsigned int type, Word asid, Word start, Word end, Word access);
		void RangeDelete(unsigned int type, unsigned int line);
		unsigned int getRangeNum(unsigned int type);
		const char * getRangeStr(unsigned int type, unsigned int line);
		void getRange(unsigned int type, unsigned int line, Word * asidp, Word * startp, Word * endp, Word * accessp);
		unsigned int getBrkptLine(void);
		unsigned int getSuspLine(void);
		
		// These methods allow to know whether memory traced areas display
		// need to be updated, and if there is a need for it
		//
		Boolean testResetDirtyMem(void);
		void setDirtyMemReq(Boolean cond);
		
		
		
		// The following methods access and modify Processor registers and
		// return internal status in a form compatible with user interface
		//
		// This method forces the update of the watch register indexed by
		// num and returns TRUE if the register has changed value.
		// num index uses an internal format 
		Boolean UpdateReg(unsigned int num);
		const char * getRegName(unsigned int num);
		Word getReg(unsigned int num);
		Boolean setReg(unsigned int num, Word val);
		const char * getCPUStatusStr(Boolean * isLD, Boolean * isBD, Boolean * isVM);
		const char * getPrevCPUStatusStr(void);
		Boolean UpdateTLB(unsigned int eNum);
		Word getTLBHI(unsigned int eNum);
		Word getTLBLO(unsigned int eNum);
		void setTLB(unsigned int eNum, Word hi, Word lo);
		Word getTLBSize(void);
		
		// These methods allow to read and set words in memory directly
		// and inspect and change device status thru SystemBus interface
		//
		Boolean MemRead(Word physAddr, Word * datap); Boolean
		MemWrite(Word physAddr, Word data);
		
	private:

		// links to other objects
		SetupInfo * setup;
		SystemBus * sysBus;
		Processor * mpsCPU;
		XInterface * xIntf;

		// Symbol table object
		SymTable * symTab;	
		
		// double-buffered system status variables 
		
		// processor registers and TLB
		Word watchReg[WATCHREGNUM];
		Word * tlbHI;
		Word * tlbLO;
		
		// simulation status word
		Word status;
		
		// stop causes status mask
		Word stopMask;
		
		 // stop flag for UTLBs exceptions only
		Boolean stopUTLBK0;
		Boolean stopUTLBK1;
		 
		// update speed setting and number of steps
		unsigned int updSpeed;
		unsigned int updStepNum;
		
		// used to highlight browser lines with brkpt/susp stop cause
		unsigned int brkptLine;
		unsigned int suspLine;
				
		// dirtyMem is used to know whether the memory has been altered:
		// this allows to speed up the interface refresh, especially
		// during single stepping
		// dirtyMemReq tells if dirtyMem monitoring is requested (e.g. the
		// memory display window is open)
		// dirtyMemWatch tells if simulation display update speed set by
		// user forbids monitoring 
		Boolean dirtyMem; 
		Boolean dirtyMemWatch;
		Boolean dirtyMemReq;

		// This method converts an input range into a string		
		const char * rangeToStr(unsigned int type, Word asid, Word start, Word end, Word access);
		
		// This method probes symbol table looking for a match for virtual
		// address given
		const char * symTProbe(Word asid, Word pos, Boolean fullSearch, SWord * offsetp);	
};


// This method creates a new Watch object and all simulated machine
// structures: the newHTable flag controls WatchTable structures creation
Watch::Watch(SetupInfo * stp, XInterface * xi, Boolean newHTable)
{
	setup = stp;
	xIntf = xi;
	
	if (newHTable)
	{
		// this part is executed only one time, at general startup
		if (wTable != NULL)
			delete wTable;
		wTable = new WatchTable();
	}
	
	// builds system
	tlbHI = new Word[setup->getTLBSize()];
	tlbLO = new Word[setup->getTLBSize()];
	
	sysBus = new SystemBus(setup, this);
	mpsCPU = new Processor(this, sysBus);
	sysBus->LinkToProc(mpsCPU);
	
	// sets initial state and loads symbol table
	status = USERSTOP;
	stopMask = setup->getStopMask();
	stopUTLBK0 = setup->getTLBStop(0);
	stopUTLBK1 = setup->getTLBStop(1);
	symTab = NULL;
	LoadSymbolTable();
	suspLine = 0;
	brkptLine = 0;
	dirtyMem = FALSE;
	dirtyMemReq = FALSE;
	setUpdSpeed(setup->getUpdSpeed());
}


// This method deletes a Watch object and all simulated machine
// structures
Watch::~Watch(void)
{
	delete tlbHI;
	delete tlbLO;
	delete sysBus;
	delete mpsCPU;
	if (symTab != NULL)
		delete symTab;
}


// This method allows to delete WatchTable item only: this way, it
// may survive resets
void Watch::DeleteHTable(void)
{
	if (wTable != NULL)
		delete wTable;
}


// This method runs the simulated machine; it stop running whenever some
// conditions are met (breakpoint reached, suspect referenced, exception
// detected), as set in Watch stopMask; it will refresh graphic interface
// after updStepsNum are performed, or sooner if memory display update is
// active (dirtyMemWatch flag on), and memory display window is open
// (dirtyMemReq flag on).
void Watch::Run(void) 
{
	unsigned int steps = updStepNum;
	
	status = RUNNING;
	
	suspLine = 0;
	brkptLine = 0;
	
	// syncs watch status and interface
	xIntf->RefreshAllVForms(FALSE);
	
	while (!(status & stopMask))
	{
		// events triggered by these procedures are signaled and modify
		// Watch status as a side effect
		sysBus->ClockTick();
		mpsCPU->Cycle();
		steps--;
		if (steps == 0 || (dirtyMemReq && dirtyMem && dirtyMemWatch))
		{
			// refreshes interface
			steps = updStepNum;
			xIntf->RefreshAllVForms(FALSE);
			xIntf->CheckForStop();	
		}
	}
	// at the end
	xIntf->RefreshAllVForms(FALSE);
	
	// clears status from ignored stop causes, if needed 
	status = status & stopMask;
}


// This method runs the simulated machine for a number of steps; it stop
// running whenever some conditions are met (breakpoint reached, suspect
// referenced, exception detected) or stepNum steps are performed
void Watch::Step(unsigned int stepNum)
{
	unsigned int steps = updStepNum;
	
	status = RUNNING;
	
	suspLine = 0;
	brkptLine = 0;
	
	// syncs watch status and interface
	if (stepNum > 1)
		// syncs watch status and interface (only if this is not overkill)
		xIntf->RefreshAllVForms(FALSE);
	
	while(!(status & stopMask) && stepNum > 0)
	{
		// events triggered by these procedures are signaled and modify
		// Watch status as a side effect
		sysBus->ClockTick();
		mpsCPU->Cycle();
		stepNum--;
		steps--;
		if (steps == 0 || (dirtyMemReq && dirtyMem && dirtyMemWatch))
		{
			steps = updStepNum;
			xIntf->RefreshAllVForms(FALSE);
			xIntf->CheckForStop();	
		}
	}
	// at end of loop it must look like as if user stopped it 
	if (stepNum == 0)
		SignalUserStop();
	
	xIntf->RefreshAllVForms(FALSE);
	
	// clears status from ignored stop causes, if needed
	status = status & stopMask;
}

	
// This method forces the update of the watch register indexed by num
// and returns TRUE if the register has changed value
Boolean Watch::UpdateReg(unsigned int num)
{
	Boolean update = FALSE;
	Word temp = 0UL;
	
	// decodes register
	if (num < CPUREGNUM)
		temp = (Word) mpsCPU->getGPR(num);
	else
		if (num < (WCP0REGOFFS + CP0REGNUM))
			temp = mpsCPU->getCP0Reg(num - WCP0REGOFFS);
		else 
			// some other register 
			
			switch (num)
			{
				case NEXTPCNUM:
					temp = mpsCPU->getNextPC();
					break;
					
				case SUCCPCNUM:
					temp = mpsCPU->getSuccPC();
					break;
					
				case TODHINUM:
					temp = sysBus->getToDHI();
					break;
					
				case TODLONUM:
					temp = sysBus->getToDLO();
					break;
				
				case TIMERNUM:
					temp = sysBus->getTimer();
					break;
				
				case PREVPPCNUM:
					temp = mpsCPU->getPrevPPC();
					break;
				
				case CURRPPCNUM:
					temp = mpsCPU->getCurrPPC();
					break;
					
				default:
					Panic("Unknown register access in Watch::UpdateReg()");
					Panic("Unknown register access in Watch::UpdateReg()");
					break;
			}
			
	if (temp != watchReg[num])
	{
		update = TRUE;
		watchReg[num] = temp;
	}
	return(update);
}


Word Watch::getReg(unsigned int num)
{
	return(watchReg[num]);
}		


// returns TRUE if operation is impossible		
Boolean Watch::setReg(unsigned int num, Word val)
{
	Boolean retval = FALSE;
	
	if (num == 0 || num > (WCP0REGOFFS + CP0REGNUM))
		// some register other than CPU's or CP0's 
		switch (num)
		{
			case NEXTPCNUM:
				mpsCPU->setNextPC(val);
				break;
					
			case SUCCPCNUM:
				mpsCPU->setSuccPC(val);
				break;
					
			case TODHINUM:
				sysBus->setToDHI(val);
				break;
					
			case TODLONUM:
				sysBus->setToDLO(val);
				break;
				
			case TIMERNUM:
				sysBus->setTimer(val);
				break;

			default:
				// $0, PREVPPCNUM and CURRPPCNUM
				retval = TRUE;
				break;
		}
	else
		if (num < CPUREGNUM)
			mpsCPU->setGPR(num, (SWord) val);
		else
			// it must be between CPUREGNUM and (WCP0REGOFFS + CP0REGNUM): a CP0 reg 
			mpsCPU->setCP0Reg(num - WCP0REGOFFS, val);
			
	if (!retval)
		// update successful
		watchReg[num] = val;
		
	return(retval);
}

			
const char * Watch::getRegName(unsigned int num)
{			
	const char * strp = EMPTYSTR;
	
	if (num < CPUREGNUM)
		strp = RegName(num);
	else
		if (num < (WCP0REGOFFS + CP0REGNUM))
			strp = CP0RegName(num - WCP0REGOFFS);
		else 
			// some other register 
			if (num < WATCHREGNUM)
				strp = othRegName[num - (WCP0REGOFFS + CP0REGNUM)];
	return(strp);
}


void Watch::SignalUserStop(void)
{
	status = status | USERSTOP;
}


void Watch::setSuspStop(Boolean value)
{
	if (value)
	{
		// sets suspect stop bit on and clears corresponding status bit
		// which may be set from a previous masked event
		status = ResetBit(status, SUSPECTBIT);
		stopMask = SetBit(stopMask, SUSPECTBIT);
		
	}
	else
		stopMask = ResetBit(stopMask, SUSPECTBIT);

	setup->setStopMask(stopMask);
}


void Watch::setBrkptStop(Boolean value)
{
	if (value)
	{
		// sets breakpoint stop bit on and clears corresponding status bit
		// which may be set from a previous masked event
		status = ResetBit(status, BRKPTBIT);
		stopMask = SetBit(stopMask, BRKPTBIT);
	}	
	else
		stopMask = ResetBit(stopMask, BRKPTBIT);

	setup->setStopMask(stopMask);
}


void Watch::setExcStop(Boolean value)
{
	if (value)
	{
		// sets exec stop bit on and clears corresponding status bit
		// which may be set from a previous masked event
		status = ResetBit(status, EXCBIT);
		stopMask = SetBit(stopMask, EXCBIT);
	}
	else
		stopMask = ResetBit(stopMask, EXCBIT);

	setup->setStopMask(stopMask);
}


void Watch::setUTLBStopK0(Boolean value)
{
        stopUTLBK0 = value;
        setup->setTLBStop(0, value);
}


void Watch::setUTLBStopK1(Boolean value)
{
        stopUTLBK1 = value;
        setup->setTLBStop(1, value);
}


Boolean Watch::getSuspStop(void)
{
	return(BitVal(stopMask, SUSPECTBIT));
}


Boolean Watch::getUTLBStopK0(void)
{
        return(stopUTLBK0);
}


Boolean Watch::getUTLBStopK1(void)
{
        return(stopUTLBK1);
}


Boolean Watch::getExcStop(void)
{
	return(BitVal(stopMask, EXCBIT));
}


Boolean Watch::getBrkptStop(void)
{
	return(BitVal(stopMask, BRKPTBIT));
}


const char * Watch::getStatusStr(void)
{
	Word asid, pc, instr;
	SWord offset;
	Boolean ld, bd, vm;
	const char * pcpos;
	
	mpsCPU->getCurrStatus(&asid, &pc, &instr, &ld, &bd, &vm);
	
	if (vm)
		pcpos = symTProbe(asid, pc, FALSE, &offset);
	else
		//VM = 0 => symbol has MAXASID asid
		pcpos = symTProbe(MAXASID, pc, FALSE, &offset);
		
	// by bit-testing EXC bit, user is correctly informed of (possibly multiple)
	// status conditions
	
	if (pcpos == NULL)
		if (BitVal(status & stopMask, EXCBIT))
			// must add EXC type string to status string 
			sprintf(strbuf, "%s%s):(UNKNOWN)", statStr[status & stopMask], mpsCPU->getExcCauseStr());
		else
			sprintf(strbuf, "%s:(UNKNOWN)", statStr[status & stopMask]);
	else	
		// pcpos != NULL
		if (BitVal(status & stopMask, EXCBIT))
			// must add EXC type string to status string 	
			sprintf(strbuf, "%s%s):(%s+0x%lX)", statStr[status & stopMask], mpsCPU->getExcCauseStr(), pcpos, offset);
		else
			sprintf(strbuf, "%s:(%s+0x%lX)", statStr[status & stopMask], pcpos, offset);
	return(strbuf);
}


const char * Watch::getCPUStatusStr(Boolean * isLD, Boolean * isBD, Boolean * isVM)
{
	Word asid, pc, instr;

	mpsCPU->getCurrStatus(&asid, &pc, &instr, isLD, isBD, isVM);
	sprintf(strbuf, "    PC: 0x%.8lX : %s",pc, StrInstr(instr));
	return(strbuf);
}


// there is no update control because at every single step, the CPU status
// will change for sure, and full redraws where these strings don't change
// are unfrequent
const char * Watch::getPrevCPUStatusStr(void)
{
	Word pc, instr;

	mpsCPU->getPrevStatus(&pc, &instr);
	sprintf(strbuf, "prevPC: 0x%.8lX : %s",pc, StrInstr(instr));
	return(strbuf);
}

        
void Watch::SignalBusAccess(Word physAddr, Word access)
{
	unsigned int line;
	
	switch (access)
	{
		// bitwise OR is used to overlap multiple status changes
		case READ:
			if ((stopMask & SUSPECTSTOP) && (line = wTable->SuspProbe(MAXASID, physAddr, access)))
			{
				// suspect stop is active and there is a hit
				status = status | SUSPECTSTOP;
				suspLine = line;
			}
			
		case WRITE:
			if ((stopMask & SUSPECTSTOP) && (line = wTable->SuspProbe(MAXASID, physAddr, access)))
			{
				// suspect stop is active and there is a hit
				status = status | SUSPECTSTOP;
				suspLine = line;
			}
			// tests for writes into traced physical memory areas
			// TraceProbe is done only if dirtyMem is not already TRUE
			// and if there is request for it (eg. MemBrowser is shown)
			if (dirtyMemReq && !dirtyMem)
				dirtyMem = wTable->TraceProbe(physAddr);
			break;
	
		case EXEC:
			if ((stopMask & BRKPTSTOP) && (line = wTable->BrkptProbe(MAXASID, physAddr, access)))
			{
				// breakpoint stop is active and there is a hit
				status = status | BRKPTSTOP;
				brkptLine = line;
			}
			break;
	
		default:
			Panic("Unknown access type in Watch::SignalBusAccess()");
			break;
	} 
}


// remember that systembus virtual addresses have no ASID, so 
// they can be monitored only with PHYS mapping, NOT virtual
void Watch::SignalProcVAccess(Word asid, Word vAddr, Word access)
{
	unsigned int line;
	
	switch (access)
	{
		// bitwise OR is used to overlap multiple status changes
		case READ:
		case WRITE:
			if ((stopMask & SUSPECTSTOP) && (line = wTable->SuspProbe(asid, vAddr, access)))
			{
				// suspect stop is active and there is a hit
				status = status | SUSPECTSTOP;
				suspLine = line;
			}
			break;
			
		case EXEC:
			if ((stopMask & BRKPTSTOP) && (line = wTable->BrkptProbe(asid, vAddr, access)))
			{
				// breakpoint stop is active and there is a hit
				status = status | BRKPTSTOP;
				brkptLine = line;
			}
			break;
	
		default:
			Panic("Unknown access type in Watch::SignalProcVAccess()");
			break;
	} 
}


void Watch::SignalProcExc(unsigned int excCause)
{
	// for UTLB exceptions, stop depends on interface settings
	// if UTLB, checks KUp since exception pushed the STATUS stack
	if ((excCause != UTLBLEXCEPTION && excCause != UTLBSEXCEPTION) || \
		(BitVal(mpsCPU->getCP0Reg(STATUS), KUCBITPOS) && stopUTLBK1) || \
		(!BitVal(mpsCPU->getCP0Reg(STATUS), KUCBITPOS) && stopUTLBK0))
	{
		status = status | EXCSTOP;
		// bitwise OR is used to overlap multiple status changes
	}
//	trace("0x%.8lX K0 %d K1 %d \n",mpsCPU->getCP0Reg(STATUS),  stopUTLBK0,  stopUTLBK1);

}


Boolean Watch::MemRead(Word physAddr, Word * datap)
{
	return(sysBus->WatchRead(physAddr, datap));
}


Boolean Watch::MemWrite(Word physAddr, Word data)
{
	if (sysBus->WatchWrite(physAddr, data))
	{
		// write to a read only address
		return(TRUE);
	}
	else
	{
		// regular write: memory has changed and dirtyMem could be affected
		// if it is not TRUE already and if there is request for it
		if (dirtyMemReq && !dirtyMem)
		 	dirtyMem = wTable->TraceProbe(physAddr);
		return(FALSE);
	}
}


// returns TRUE on successful insertion, FALSE otherwise
Boolean Watch::RangeInsert(unsigned int type, Word asid, Word start, Word end, Word access)
{
	if (wTable->RangeIns(type, asid, start, end, access))
	{
		// correct insertion
		
		// must reset line numbers to avoid wrong highlighting
		if (type == SUSP)
			suspLine = 0;
		else 
			if (type == BRKPT)
				brkptLine = 0;
		
		return(TRUE);
	}
	else
		return(FALSE);
}


void Watch::RangeDelete(unsigned int type, unsigned int line)
{
	wTable->RangeDel(type, line);
	
	// must reset line numbers to avoid wrong highlighting
	if (type == SUSP)
		suspLine = 0;
	else 
		if (type == BRKPT)
			brkptLine = 0;
}


unsigned int Watch::getRangeNum(unsigned int type)
{
	return(wTable->getRangeLineNum(type));
}


const char * Watch::getRangeStr(unsigned int type, unsigned int line)
{
	Word asid, start, end, access;
	
	wTable->getRangeLine(type, line, &asid, &start, &end, &access);
	return(rangeToStr(type, asid, start, end, access));
}


void Watch::getRange(unsigned int type, unsigned int line, Word * asidp, Word * startp, Word * endp, Word * accessp)
{
	wTable->getRangeLine(type, line, asidp, startp, endp, accessp);	
}


Boolean Watch::testResetDirtyMem(void)
{
	Boolean temp = dirtyMem;
	dirtyMem = FALSE;
	return(temp);
}


void Watch::setDirtyMemReq(Boolean cond)
{
	dirtyMemReq = cond;
	if (dirtyMemReq == TRUE)
		// this test forces memory dump area refresh and reduces TraceProbe() calls
		dirtyMem = TRUE;
}



Boolean Watch::UpdateTLB(unsigned int eNum)
{
	Word enthi, entlo;
	
	mpsCPU->getTLB(eNum, &enthi, &entlo);
	if (tlbHI[eNum] != enthi || tlbLO[eNum] != entlo)
	{
		tlbHI[eNum]	= enthi;
		tlbLO[eNum] = entlo;
		return(TRUE);
	}
	else 
		return(FALSE);
}


Word Watch::getTLBHI(unsigned int eNum)
{
	return(tlbHI[eNum]);
}


Word Watch::getTLBLO(unsigned int eNum)
{
	return(tlbLO[eNum]);
}


void Watch::setTLB(unsigned int eNum, Word hi, Word lo)
{
	mpsCPU->setTLB(eNum, hi, lo);
}


Word Watch::getTLBSize(void)
{
	return(setup->getTLBSize());
}



unsigned int Watch::getSymbolNum(void)
{
	return(symTab->getSymNum());
}


void Watch::getSymbolData(unsigned int symNum, Word * startp,  Word * endp, Boolean * isFunp)
{
	const char * dummyp;
	
	if (symNum > 0 && symNum <= symTab->getSymNum())
		dummyp = symTab->getSymData(symNum, isFunp, startp, endp);
	else
		Panic("Illegal request for symbol in Watch::getSymbolData()");
}


const char * Watch::getSymbolStr(unsigned int symNum)
{
	Word start, end;
	const char * snamep;
	Boolean isFun;
	const char * objtype;
	
	if (symNum > 0 && symNum <= symTab->getSymNum())
	{
		snamep = symTab->getSymData(symNum, &isFun, &start, &end);
		if (isFun)
			objtype = "FUN";
		else
			objtype = "OBJ";
		
		if (start != end)
			sprintf(strbuf, "%-16.16s:%s:0x%.8lX->0x%.8lX", snamep, objtype, start, end);
		else
			sprintf(strbuf, "%-16.16s:%s:0x%.8lX", snamep, objtype, start);
	}
	else
		Panic("Illegal request for symbol in Watch::getSymbolStr()");

	return(strbuf);
}


unsigned int Watch::getBrkptLine(void)
{
	return(brkptLine);
}


unsigned int Watch::getSuspLine(void)
{
	return(suspLine);
}


unsigned int Watch::getUpdSpeed(void)
{
	return(updSpeed);
}


const char * Watch::getUpdSpeedStr(void)
{
	return(speedNames[updSpeed]);
}


void Watch::setUpdSpeed(unsigned int value)
{
	switch (value)
	{
		case SLOWEST:
			updStepNum = SLOWESTSTEP;
			dirtyMemWatch = TRUE;
			break;
	
		case SLOWER:
			updStepNum = SLOWERSTEP;
			dirtyMemWatch = TRUE;
			break;
	
		case SLOW:
			updStepNum = SLOWSTEP;
			dirtyMemWatch = TRUE;
			break;	
	
		case NORMAL:
			updStepNum = NORMALSTEP;
			dirtyMemWatch = TRUE;
			break;
	
		case FAST:
			updStepNum = FASTSTEP;
			dirtyMemWatch = FALSE;
			break;
	
		case FASTER:
			updStepNum = FASTERSTEP;
			dirtyMemWatch = FALSE;
			break;	
	
		case FASTEST:
			updStepNum = FASTESTSTEP;
			dirtyMemWatch = FALSE;
			break;
		
		default:
			Panic("Illegal speed setting in Watch::setUpdSpeed()");
			break;
	}
	updSpeed = value;
	setup->setUpdSpeed(value);
}


void Watch::LoadSymbolTable(void)
{
	if (symTab != NULL)
		delete symTab;
	symTab = new SymTable(setup->getSymbolTableASID(), setup->getROMFileName(STABINDEX));
}


// This method converts an input range into a string
const char * Watch::rangeToStr(unsigned int type, Word asid, Word start, Word end, Word access)
{
	const char * sbuf;
	const char * ebuf;
	SWord startoffs, endoffs;
	char asidbuf[STRBUFSIZE];
	
	char * accstrp = NULL;
	
	if (asid == MAXASID)
		// physical entry
		sprintf(asidbuf, "PHYS");
	else
		sprintf(asidbuf, "0x%.2lX", asid);
		
	sbuf = symTProbe(asid, start, TRUE, &startoffs);
	if (start != end)
		ebuf = symTProbe(asid, end, TRUE, &endoffs);
	else
		ebuf = NULL;
	
	if (sbuf == NULL || (start != end && ebuf == NULL))			
		// symbol table (maybe partially) unmapped entry
		if (start != end)
			sprintf(strbuf, "(%s)0x%.8lX->0x%.8lX", asidbuf, start, end);
		else
			sprintf(strbuf, "(%s)0x%.8lX", asidbuf, start);
	else
		// fully mapped entry
		if (start != end)
			sprintf(strbuf, "(%s)(%s+0x%lX)->(%s+0x%lX)", asidbuf, sbuf, startoffs, ebuf, endoffs);
		else
			sprintf(strbuf, "(%s)(%s+0x%lX)", asidbuf, sbuf, startoffs);

	// now completing it with access bits if needed
	if (type == SUSP)
	{
		// more overhead, but faster in any other case
		char tbuf[STRBUFSIZE];
		 
		switch (access)
		{
			case READ: 
				accstrp = "R ";
				break;
			case WRITE:
				accstrp = " W";
				break;
			case READWRITE:
				accstrp = "RW";
				break;
			default:
				Panic("Unknown access type in Watch::rangeToStr()");
				break;
		}
		// puts access type and range into return buffer
		sprintf(tbuf, "%s:%s", accstrp, strbuf);
		strcpy(strbuf, tbuf);
	}
	return(strbuf);
}		


// This method probes symbol table looking for a match for virtual
// address given
const char * Watch::symTProbe(Word asid, Word pos, Boolean fullSearch, SWord * offsetp)
{
	if (pos >= RAMBASE)
		// in RAM space
		return(symTab->Probe(asid, pos, fullSearch, offsetp));
	else
		if (pos >= BOOTBASE)
		{
			*offsetp = pos - BOOTBASE;
			return(BOOTAREANAME);
		}
		else
			// in ROM segment, but not in BOOT area
			if (pos >= KSEG0BASE)
			{
				*offsetp = pos - KSEG0BASE;
				return(BIOSAREANAME);
			}
			else
				// I wonder where it is...
				return(NULL);
}



