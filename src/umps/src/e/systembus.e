/* File: $Id: systembus.e,v 1.2 2008-05-04 15:46:59 mbbh Exp $ */
/****************************************************************************
 *
 * External declarations from the corresponding module .cc
 *
 ****************************************************************************/

class SetupInfo;
class Watch;
class Processor;
class TimeStamp;
class Cache;
 
class SystemBus
{
	public:
	
		// This method builds a SystemBus object and all related structures
		SystemBus(SetupInfo * stp, Watch * wch);
		// This method deletes a SystemBus object and all related structures
		~SystemBus(void);
		
		// This method links SystemBus to related Processor: it may be used
		// to build multi-processor systems with shared buses
		void LinkToProc(Processor * prc);		

		// This method increments system clock and decrements interval
		// timer; on timer underflow (0 -> FFFFFFFF transition) a interrupt
		// is generated.  Event queue is checked against the current clock
		// value and device operations are completed if needed; all memory
		// changes are notified to Watch control object
		void ClockTick(void);

		// This method reads a data word from memory at physical address
		// addr, returning it thru datap pointer. It also returns TRUE if
		// the address was invalid and an exception was caused, FALSE
		// otherwise, and signals memory access to Watch control object
		Boolean DataRead(Word addr, Word * datap);
		
		// This method writes the data word at physical addr in RAM memory or device
		// register area.  Writes to BIOS or BOOT areas cause a DBEXCEPTION (no
		// writes allowed). It returns TRUE if an exception was caused, FALSE
		// otherwise, and notifies access to Watch control object
		Boolean DataWrite(Word addr, Word data);	
		
		// This method reads a istruction from memory at physical address addr,
		// returning it thru istrp pointer. It also returns TRUE if the
		// address was invalid and an exception was caused, FALSE otherwise,
		// and notifies Watch
		Boolean InstrRead(Word addr, Word * instp);
		
		
		// This method returns the current interrupt line status
		Word getPendingInt(void);
		
		
		
		// These methods allow to inspect or modify  TimeofDay Clock and
		// Interval Timer (typically for simulation reasons)
		
		Word getToDHI(void);
		Word getToDLO(void);
		Word getTimer(void);
		void setToDHI(Word hi);
		void setToDLO(Word lo);
		void setTimer(Word time);
		
		// These methods allow Watch to inspect or modify single memory
		// locations; they return TRUE if address is invalid or cannot be
		// changed, and FALSE otherwise
		 
		Boolean WatchRead(Word addr, Word * datap);
		Boolean WatchWrite(Word addr, Word data);
		Word ackPendingInt(Word wMask);

	private:

        // read config file
        void readCoreFile(const char * fName);
        
		// configuration information object
		SetupInfo * setup;
		
		// watch interface object
		Watch * watch;
		
        Cache* cInstrCache;
		
		// Processor object
		Processor * proc;

		// system clock & interval timer
		TimeStamp * timeOfDay;
		Word timer;
				
		// pending interrupts on lines: this word is packed into MIPS Cause
		// Register IP field format for easy masking
		Word intPendMask;        
		
		
		// This method read the data at physical address addr, and
		// passes it back thru the datap pointer. It also return FALSE if
		// the addr is valid, and TRUE otherwise
		Boolean busRead(Word addr, Word * datap);
		
		// This method returns the value for the device field addressed in
		// the "bus register area"
		Word busRegRead(Word addr);
		
		// This method writes the data at physical address addr, and
		// passes it back thru the datap pointer. It also return FALSE if
		// the addr is valid and writable, and TRUE otherwise
		Boolean busWrite(Word addr, Word data);
		
		// This method accesses the system configuration and constructs
		// the devices needed, linking them to SystemBus object
		
};

