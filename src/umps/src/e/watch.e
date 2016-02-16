
/* File: $Id: watch.e,v 1.2 2008-05-04 15:46:59 mbbh Exp $ */

/****************************************************************************
 *
 * External declarations from the corresponding module .cc
 *
 ****************************************************************************/

class SetupInfo;
class XInterface;
class SystemBus;
class Processor;
class SymTable;


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
		
		//
		// The following methods are used to monitor the simulated system
		// state, and most have a direct counterpart in SystemBus, Processor
		// and Device classes methods. Their description and arguments are
		// most self-explanatory; just remember the double-buffering issue
		//
		
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
		// This method allows to test if device status has changed to know
		// if data displayed should be updated 
		Boolean UpdateDev(unsigned int intL, unsigned int dNum);
		Boolean MemRead(Word physAddr, Word * datap); Boolean
		MemWrite(Word physAddr, Word data);
		Boolean getDevNotWorking(unsigned int intL, unsigned int dNum);
		const char * getDevStatStr(unsigned int intL, unsigned int dNum);
		const char * getDevCTimeStr(unsigned int intL, unsigned int dNum);				
		Boolean setDevNotWorking(unsigned int intL, unsigned int dNum, Boolean cond);
		void TermInput(unsigned int intL, unsigned int tNum, const char * inputL);		
		Boolean TapeAvailable(unsigned int intL, unsigned int dNum);
		void LoadTape(unsigned int intL, unsigned int dNum, const char * fName);
		
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

