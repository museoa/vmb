/* File: $Id: setup.e,v 1.2 2008-05-04 15:46:59 mbbh Exp $ */

/****************************************************************************
 *
 * External declarations from the corresponding module .cc
 *
 ****************************************************************************/


// This class implements SetupInfo objects. Each contains all configuration
// information for every other object in the program which may need it: see
// object contents.

class SetupInfo
{
	public:
		// This method builds a new SetupInfo object and set ROM file
		// configuration depending on boot type. If boot type is "core
		// boot", a kernel image must be loaded in RAM memory; for any other
		// kind of boot type, no kernel image needs to be loaded, but a
		// different Bootstrap ROM does.  Other system configuration
		// parameters are set to default, and devices are activated or
		// deactivated accordingly with file availability and default
		// configuration
		SetupInfo(unsigned int bType, const char * bootFile);

		// This method deletes all SetupInfo associated structures
		~SetupInfo(void);


		// the following methods allow to get the current configuration
		// values; their meaning and arguments are self-explanatory
		
		unsigned int getSpeed(void);
		Word getRamSize(void);
		Word getTLBSize(void);
		const char * getROMFileName(unsigned int index);
		const char * DevIsActiveStr(unsigned int intl, unsigned int dnum);
		unsigned int getBootType(void);
		Word getSymbolTableASID(void);
		
		
		// all these methods allow to update the current configuration values;
		// their meaning and arguments are almost self-explanatory
		
		void setSpeed(unsigned int val);
		void setRamSize(Word val);
		Boolean setROMFileName(unsigned int index, const char * fName, Boolean atBoot);
				
		void setBootType(unsigned int bType);
		void setSymbolTableASID(Word asid);
		unsigned int getUpdSpeed(void);
		void setUpdSpeed(unsigned int value);
		Boolean getExpertMode(void);
		Word getStopMask(void);
		void setStopMask(Word mask);
		Boolean getTLBStop(Word k);
		void setTLBStop(Word k, Boolean value);
		
	private:
	
		// processor speed in MHz
		unsigned int speed;
		
		// RAM size (in 4KB frames) 
		Word ramSize;

		// TLB size 
		Word tlbSize;
		
		// ASID associated to current symbol table		
		Word symTabASID;

		// boot type ("core", from disk, from tape)		
		unsigned int bootType;
		
		
		// ROM & symbol table file names 
		char * romFName[ROMSNUM];
		
		unsigned int updSpeed;
		Boolean expertMode;
		Word stopMask;
		Boolean TLBStop[2];
		
		
		// This method tests device file validity. A device file is valid
		// when: if it is a tape or disk file, it contains the correct magic
		// number; if it is another kind of device, if file is writable
		// (since it is just a log file).  It returns TRUE or FALSE
		// according to file validity
		Boolean validFileMagic(Word tag, const char * fName);
        unsigned long ipRomPids[2];
};
