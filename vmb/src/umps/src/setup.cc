/* File: $Id: setup.cc,v 1.1 2007-08-29 09:19:37 ruckert Exp $ */

/****************************************************************************
 *
 * This module contains the SetupInfo class definition. Inside a SetupInfo
 * object, all configuration paramenters relevant to simulation have their
 * place.
 *
 ****************************************************************************/

/****************************************************************************/
/* Inclusion of header files.                                               */
/****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>	// kMcC ;)

#include <h/const.h>
#include <h/types.h>
#include <h/blockdev.h>
#include <h/aout.h>
extern "C" {
    #include "h/defaults.h"
    #include <signal.h>
    #include <wait.h>
}


/****************************************************************************/
/* Inclusion of imported declarations.                                      */
/****************************************************************************/

#include <e/utility.e>

/****************************************************************************/
/* Declarations strictly local to the module.                               */
/****************************************************************************/


// default .rom file names (Bootstrap ROM excluded)
HIDDEN char * defltROMFName[ROMSNUM] =	{	
											"kernel",
											"kernel"
										};

										
// default .rom file extension										
HIDDEN char * romFileExt[ROMSNUM] =	{
										COREFILETYPE,
										STABFILETYPE
									};	


// device status
HIDDEN char * active[2] =	{	"INACTIVE",
								"ACTIVE  "
							};


HIDDEN char* cpExtRomDefaults[2][2] = {
                                        {"00000000","exec.rom.umps"},
                                        {"1FC00000","coreboot.rom.umps"}
                                };


// static buffer 
#define STRBUFSIZE	256
HIDDEN char strbuf[STRBUFSIZE];

#define UMPSRCFILE 	"umpsrc"

HIDDEN Word computeTLBSize(Word val);
HIDDEN long forkNexecROM(char* cpAddress,char* cpFileName);
HIDDEN void killRoms(unsigned long* ipRomPids);
/****************************************************************************/
/* Definitions to be exported.                                              */
/****************************************************************************/

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

		// boot type ("core", "no core")		
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
		
        unsigned long ipRomPids[NUMEXTROMS];
};

// This method builds a new SetupInfo object and sets ROM file configuration
// depending on boot type. If boot type is "core boot", a kernel image must
// be loaded in RAM memory; for any other kind of boot type, no kernel image
// needs to be loaded, but a different Bootstrap ROM does.
// Other system configuration parameters are set to default, and devices are
// activated or deactivated accordingly with file availability and default
// configuration
SetupInfo::SetupInfo(unsigned int bType, const char * bootFile)
{
	unsigned int i, j, c, dnum;
	char * strp, * eqpos, * dstrp;
	char fileName[STRBUFSIZE];
	char fileDir[STRBUFSIZE];
	char umpsrc[STRBUFSIZE];
	char line[STRBUFSIZE];
	FILE * testF = NULL;
	Boolean rcFound = FALSE, eol;
	Word val;
	
	getcwd(fileDir, STRBUFSIZE);


	// sets boot type
	if (bType < BOOTTYPES)
		bootType = bType;
	else
		Panic("Invalid boot type in SetupInfo::SetupInfo()");
	
	// sets default initial parameters
	
	speed = MINSPEED;
	ramSize = STARTRAM;
	tlbSize = TLBDFLSIZE;
	symTabASID = MAXASID;
	updSpeed = FASTEST;
	expertMode = FALSE;
	stopMask = DFLSTOPMASK;
	
	// default ROM file configuration
	for (i = 0; i < ROMSNUM; i++)
	{
		sprintf(fileName, "%s/%s%s", fileDir, defltROMFName[i], romFileExt[i]);					

		romFName[i] = new char[strlen(fileName) + 1];
		strcpy(romFName[i], fileName);
	}
	// default 'rom' pid config
    for(i = 0; i< NUMEXTROMS; i++)
        ipRomPids[i] = 0;
    			
	// tries to locate .umpsrc file
	
	// looks in current dir 
	sprintf(fileName, "%s/.%s",fileDir, UMPSRCFILE);
	if ((testF = fopen(fileName, "r")) != NULL)
	{
		fclose (testF);
		strcpy(umpsrc, fileName);
		rcFound = TRUE;
	}
	else
	{
		// looks in HOME dir
		sprintf(fileName, "%s/.%s",getenv("HOME"), UMPSRCFILE);
		if ((testF = fopen(fileName, "r")) != NULL)
		{
			fclose (testF);
			strcpy(umpsrc, fileName);
			rcFound = TRUE;
		}
		else
		{
			// looks in /etc
			sprintf(fileName, "/etc/%s", UMPSRCFILE);
			if ((testF = fopen(fileName, "r")) != NULL)
			{
				fclose (testF);
				strcpy(umpsrc, fileName);
				rcFound = TRUE;
			}	
		}
	}
	
	if (rcFound)
	{		
		// found it: tries to load non-default config parameters from it

		testF = fopen(fileName, "r");
		
		while (!feof(testF) && ! ferror(testF))
		{
			// read single line
			eol = FALSE;
			c = 0;
			while (!eol && c < STRBUFSIZE)
			{
				if (fread((void *)&(line[c]), sizeof(char), 1, testF) == 0 || line[c] == '\n' )
					eol = TRUE;
				else
					if (line[c] != ' ')
					{
						c++;
					}
			}
			if (c >= STRBUFSIZE)
			{
				ShowAlertQuit(umpsrc,"line too long","exiting now...");
			}
			else
				if (c > 0) 
				{
					// line is ok: terminate it
					line[c] = EOS;
					if (line[0] != '#' && strlen(line) != 0)
					{
						// not blank
						if ((eqpos = strchr(line, '=')) == NULL)
							ShowAlert(umpsrc,line,"unknown format: ignoring line");
						else
						{
							// separate right from left 
							* eqpos = EOS;
							eqpos++; 
							if (strcmp(line,"LOADCOREFILE") == 0)
							{
								if (!StrToWord(eqpos, (Word *) &bootType) || bootType < NOCOREBOOT || bootType > COREBOOT)
									ShowAlertQuit(umpsrc,"LOADCOREFILE out of bounds","exiting now...");
							}
							else if(strcmp(line,"EXECROM") == 0)
							{
							    if(enableRomFork)
                                    ipRomPids[EXTEXECINDEX] = forkNexecROM(cpExtRomDefaults[EXTEXECINDEX][0],eqpos);
                                else
                                    fprintf(stderr,"EXECROM setting disabled via -dr switch, doing nothing here.\n");
							}
							else if(strcmp(line,"BOOTROM") == 0 && bootType == COREBOOT)
							{
							    if(enableRomFork)
                                    ipRomPids[EXTBOOTINDEX] = forkNexecROM(cpExtRomDefaults[EXTBOOTINDEX][0],eqpos);
                                else
                                    fprintf(stderr,"BOOTROM setting disabled via -dr switch, doing nothing here.\n");
                                
							}
							else if (strcmp(line,"COREFILE") == 0)
							{
									delete romFName[COREINDEX];
									romFName[COREINDEX] = new char[strlen(eqpos) + 1]; 
									strcpy(romFName[COREINDEX],eqpos);
							}
							else if (strcmp(line,"STABFILE") == 0)
							{
								     delete romFName[STABINDEX];
								romFName[STABINDEX] = new char[strlen(eqpos) + 1];
								strcpy(romFName[STABINDEX],eqpos);
							}
							else if (strcmp(line,"RAMPAGESIZE") == 0)
							{ 
								if (!StrToWord(eqpos, (Word *) &ramSize) || ramSize < MINRAM || ramSize > MAXRAM)
									ShowAlertQuit(umpsrc,"RAMSIZE out of bounds","exiting now...");
								if(ramSize*FRAMEKB > ulRamSize)
                                    ulRamSize = ramSize*FRAMEKB;
							}
							else if (strcmp(line,"PROCSPEED") == 0)
							{
								if (!StrToWord(eqpos, (Word *) &speed) || speed < MINSPEED || speed > MAXSPEED)
									ShowAlertQuit(umpsrc,"PROCSPEED out of bounds","exiting now...");
							}
							else if (strcmp(line,"TLBSIZE") == 0)
							{
								if (!StrToWord(eqpos, &val) || val < TLBMINSIZE || val > TLBMAXSIZE)
									ShowAlertQuit(umpsrc,"TLBSIZE out of bounds","exiting now...");								
								else
									tlbSize = computeTLBSize(val);
							}		
							else if (strcmp(line,"SIMSPEED") == 0)
							{ 
								if (!StrToWord(eqpos, (Word *) &updSpeed) || updSpeed < SLOWEST || updSpeed > FASTEST)
									ShowAlertQuit(umpsrc,"SIMSPEED out of bounds","exiting now...");
							}
							else if (strcmp(line,"EXCSTOP") == 0)
							{ 
								if (!StrToWord(eqpos, &val) || val < 0 || val > 1)
									ShowAlertQuit(umpsrc,"EXCSTOP out of bounds","exiting now...");
								else
									if (val) 
										stopMask = SetBit(stopMask, EXCBIT);
									else
										stopMask = ResetBit(stopMask, EXCBIT);
							}
							else if (strcmp(line,"BRKPTSTOP") == 0)							
							{ 
								if (!StrToWord(eqpos, &val) || val < 0 || val > 1)
									ShowAlertQuit(umpsrc,"BRKPTSTOP out of bounds","exiting now...");
								else
									if (val) 
										stopMask = SetBit(stopMask, BRKPTBIT);
									else
										stopMask = ResetBit(stopMask, BRKPTBIT);
							}
							else if (strcmp(line,"SUSPSTOP") == 0)
							{ 
								if (!StrToWord(eqpos, &val) || val < 0 || val > 1)
									ShowAlertQuit(umpsrc,"SUSPSTOP out of bounds","exiting now...");
								else
									if (val) 
										stopMask = SetBit(stopMask, SUSPECTBIT);
									else
										stopMask = ResetBit(stopMask, SUSPECTBIT);
							}
							else if (strcmp(line,"TLBK0STOP") == 0)
							{ 
								if (!StrToWord(eqpos, &val) || val < 0 || val > 1)
									ShowAlertQuit(umpsrc,"TLBK0STOP out of bounds","exiting now...");
								else
									 TLBStop[0]= val;
							}
							else if (strcmp(line,"TLBK1STOP") == 0)
							{ 
								if (!StrToWord(eqpos, &val) || val < 0 || val > 1)
									ShowAlertQuit(umpsrc,"TLBK1STOP out of bounds","exiting now...");
								else
									 TLBStop[1]= val;
							}
							else if (strcmp(line,"EXPERTMODE") == 0)
							{ 
								if (!StrToWord(eqpos, &val) || val < FALSE || val > TRUE)
									ShowAlertQuit(umpsrc,"EXPERTMODE out of bounds","exiting now...");
								else
								{
									expertMode = val;
								}
							}
							else
								ShowAlert(umpsrc,line,"unknown format: ignoring line");
						}
					}
				}
		}
		if (ferror(testF))
			ShowAlertQuit(umpsrc,"error accessing file","exiting now...");

		fclose(testF);
	}
		
	if (bootFile != NULL)
	{
		// bootFile is set (overrides rc and defaults)
	
		// sets COREINDEX ROM file name
		sprintf(fileName, "%s",bootFile);
		delete romFName[COREINDEX];
		romFName[COREINDEX] = new char[strlen(fileName) + 1];
		strcpy(romFName[COREINDEX], fileName);
		
		// sets STABINDEX ROM file name if possible
		
		// identifies kernel core extension
		strcpy(strbuf, bootFile);
		if ((strp = strstr(strbuf, romFileExt[COREINDEX])) != NULL)
		{
			*strp = EOS;
			sprintf(fileName, "%s%s", strbuf, romFileExt[STABINDEX]);
			delete romFName[STABINDEX];
			romFName[STABINDEX] = new char[strlen(fileName) + 1];
			strcpy(romFName[STABINDEX], fileName);
		}
		// else it leaves as it is	
	}
	
	// checks for ROM and device files validity
	for (i = 0; i < ROMSNUM; i++)
	{	
		strcpy(fileName, romFName[i]);
		setROMFileName(i, fileName, TRUE);
	}
    for(i=0;i < NUMEXTROMS;i++)
    {   
        if(ipRomPids[i] <= 0 && enableRomFork)
        {
            ipRomPids[i] = forkNexecROM(cpExtRomDefaults[i][0],cpExtRomDefaults[i][1]);
            if(ipRomPids[i] == -1)
            {
                fprintf(stderr,"Failsafe rom fork didn't work. Make sure rom executeable is \n"
                               "in your PATH and that BOOTROM/EXECROM are set right or the  \n"
                               "default exec.rom.umps and coreboot.rom.umps are present in  \n"
                               "the current directory. Second solution to this problem is   \n"
                               "starting the simulator with the -dr option and launching the\n"
                               "rom executeable manually!\n"
                               "bailing out....\n");
                killRoms(ipRomPids);
                exit(5);
            }
        }
    }
}


// This method deletes all SetupInfo associated structures
SetupInfo::~SetupInfo(void)
{
	unsigned int i, j;
	
	for (i = 0; i < ROMSNUM; i++)
		if (romFName[i] != NULL)
			delete (romFName[i]);

    killRoms(ipRomPids);
}

//
// the following methods allow to get the current configuration values, or
// to update them; their meaning and arguments are self-explanatory
//                

unsigned int SetupInfo::getSpeed(void)
{
	return(speed);
}

void SetupInfo::setSpeed(unsigned int val)
{
	speed = val;
}

Word SetupInfo::getRamSize(void)
{
	return(ramSize);
}

Word SetupInfo::getTLBSize(void)
{
	return(tlbSize);
}

void SetupInfo::setRamSize(Word val)
{
	ramSize = val;
}



	 
const char * SetupInfo::getROMFileName(unsigned int index)
{
	if (index < ROMSNUM)
		return(romFName[index]);
	else
	{
		Panic("Unknown ROM file index in SetupInfo::getROMFileName()");
		return(NULL);
	}
}

Boolean SetupInfo::setROMFileName(unsigned int index, const char * fName, Boolean atBoot)
{
	Word tag = 0;
		
	// switch tests index validity too
	switch (index)
	{
		case COREINDEX:
			tag = COREFILEID;
			break;
			
		case STABINDEX:
			tag = STABFILEID;
			break;
		
		default:
			Panic("Unknown ROM file index in SetupInfo::setROMFileName()");
			break;
	}
	if (validFileMagic(tag, fName) || (index == COREINDEX && bootType == NOCOREBOOT))
	{
		if (romFName[index] != NULL)
			delete (romFName[index]);
		romFName[index] = new char[strlen(fName) + 1];
		strcpy(romFName[index], fName);
		return(TRUE);
	}
	else
	{
		// invalid file: at boot the program quits, else does not load the file
		if (atBoot)
			ShowAlertQuit("ROM file not found or invalid:",fName,"cannot continue: exiting now...");
		else
			ShowAlert("ROM file not found or invalid", "file name:",fName);
			
		return(FALSE);
	}
}

unsigned int SetupInfo::getBootType(void)
{
	return(bootType);
}

void SetupInfo::setBootType(unsigned int bType)
{
	// char fileName[STRBUFSIZE], * mpsdirp;
	
	if (bootType < BOOTTYPES)
	{
		bootType = bType;

// the following is not requires since BOOT ROM file name has to be set in .umpsrc
//
		// tries to locate boot ROM file that matches bootType and load it: if fails, sets to consistent state 
//	if ((mpsdirp = getenv("UMPSDIR")) != NULL)
//		sprintf(fileName, "%s/%s%s", mpsdirp, bootROMFName[bootType], romFileExt[BOOTINDEX]);				
//	else
//	{
//			getcwd(strbuf, STRBUFSIZE);
//			sprintf(fileName, "%s/%s%s", strbuf, bootROMFName[bootType], romFileExt[BOOTINDEX]);
//	}
//		if (!setROMFileName(BOOTINDEX, fileName, TRUE))
//			setROMFileName(BOOTINDEX, NULL, TRUE);
	}
	else
		Panic("Invalid boot type in SetupInfo::setBootType()");

}

Word SetupInfo::getSymbolTableASID(void)
{
	return(symTabASID);
}

void SetupInfo::setSymbolTableASID(Word asid)
{
	symTabASID = asid;
}


// This method tests device file validity. A device file is valid when: if
// it is a tape or disk file, it contains the correct magic number; if it is
// another kind of device, if file is writable (since it is simply a log
// file)
// It returns TRUE or FALSE accordingly to file validity

Boolean SetupInfo::validFileMagic(Word tag, const char * fName)
{
	Word fTag;
	FILE * testF = NULL;
	Boolean ret = TRUE;
	
	if (tag == 0UL)
		// just testing file write access, without changing the file itself
	{
			if ((testF = fopen(fName, "a")) == NULL)
			// file is not accessible
			ret = FALSE;
		// else file is ok
	}	
	else
		// tag != 0UL
		if ((testF = fopen(fName, "r")) == NULL || fread((void *) &fTag, WORDLEN, 1, testF) != 1 || fTag != tag)
			// file empty/invalid
			ret = FALSE;
		// else file matches tag
		
	if (testF != NULL)
		fclose(testF);
			
	return(ret);
}
 
 
unsigned int SetupInfo::getUpdSpeed(void)
{
	return updSpeed;
}


void SetupInfo::setUpdSpeed(unsigned int value)
{
	updSpeed = value;
}


Boolean SetupInfo::getExpertMode(void)
{
	return expertMode;
}


Word SetupInfo::getStopMask(void)
{
	return stopMask;
}


void SetupInfo::setStopMask(Word mask)
{
	stopMask = mask;
}



Boolean SetupInfo::getTLBStop(Word k)
{
	if (k < 0 || k > 1)
		Panic("Invalid value in SetupInfo::getTLBStop()");
	return TLBStop[k];
}


void SetupInfo::setTLBStop(Word k, Boolean value)
{
	if (k < 0 || k > 1)
		Panic("Invalid value in SetupInfo::setTLBStop()");
	TLBStop[k] = value;
}


HIDDEN Word computeTLBSize(Word val)
{
	Word i;
	
	for (i = (TLBMINSIZE << 1); (i <= TLBMAXSIZE) && (i < val); i = i << 1)
	{
		// only a power of 2 is a valid TLB size
	}
			
	if (i == val)
		return(val);
	else
		// nearest power of two rounding down 
		return(i >> 1);
}

HIDDEN long forkNexecROM(char* cpAddress, char* cpFileName)
{
    long ulPid;
    int iStatus;
    char* cpArgs[] = {"rom","-a",0,"-d","1","-f",0,(char*)0};
    cpArgs[6] = cpFileName;
    cpArgs[2] = cpAddress;
    ulPid = fork(); // we just do fork n exec, use vfork for faster stuff
    
    if(ulPid < 0)
    {
        fprintf(stderr,"failed to fork new process!\n");
        exit(6);
    }
    else if(ulPid == 0)
    {
        execvp("rom",cpArgs);
        exit(0);
    }
    else
    {
        waitpid(ulPid,&iStatus,WNOHANG); // check whether child is alive
        if(WIFEXITED(iStatus))
        {
            ulPid = -1;
            fprintf(stderr,"%s:(%d): failed to exec child!\n",__FILE__,__LINE__);
        }
    }
    return(ulPid);
}

HIDDEN void killRoms(unsigned long* ipRomPids)
{
    int i;
    for(i=0;i < NUMEXTROMS; i++)
    {
        if(ipRomPids[i] > 0)
        {
            if(debugflag)
                fprintf(stderr,"%s:(%d): killing external ROM with pid %d\n",__FILE__,__LINE__,ipRomPids[i]);
            kill(ipRomPids[i],SIGTERM);
            if(debugflag)
                fprintf(stderr,"%s:(%d): waiting for pid %d to terminate.\n",__FILE__,__LINE__,ipRomPids[i]);
            waitpid(ipRomPids[i],NULL,0);
        } 
    }
}
