/*!
 * \file systembus.cc
 * \author Martin Hauser <info@martin-hauser.net>
 * \author Mauro Morsiani <mmorsian@cineca.it>
 * \brief implements a dummy SystemBus
 *
 * This module implements the SystemBus object class.
 * 
 * A SystemBus connects a Processor object to memory and devices. 
 * It satisfies the Processor requests to read and write memory positions,
 * and simulates interrupts generation and other external exceptions.
 * SystemBus defines the physical memory map of the system, mapping RamSpace
 * and BiosSpace objects and Device registers to "physical" addresses in
 * memory.  
 * It also implements the system clock and timer, and keeps track of some
 * important system constants.  All this information is mapped in the "bus
 * device register area", at the start of the device register area.
 *
 * On creation, it initializes the device and memory subsystems
 * as set in the simulator configuration (contained in the SetupInfo 
 * object); on destruction, it removes them.  
 * 
 * SystemBus notifies all changes to memory location to Watch controlling
 * object, to track memory accesses for simulation brkpt/susp/trace handling
 *
 * This class has been changed so it uses the external SystemBus and only inherits
 * few of it's former functionality to keep the simulator going.
 * 
 * The changes to this file since Mauro Morsianis Implementation will be listed 
 * below:
 * - fixes
 *  -# add correct mangle prevention (extern "C") for including of C header files
 * - changes
 *  -# Changed ::busWrite to actually ignore device requests from umps and use the 
 *     external bus devices
 *  -# Changed ::busRead to actually ignore device requests and use external bus
 *     device (at the moment through ram class)
 *  -# Changed ::busRead to include functionality of ExtMemRead (purging memspace.cc)
 *  -# Changed ::busWrite to include functionality of ExtMemWrite (purging memspace.cc)
 *  -# added ::readCoreFile (former constructor of RamSpace class)
 */


extern "C" { 
  #include <stdio.h>
  #include "h/bussync.h"
  #include <pthread.h>
  #include "../../bus-util.h"
  #include "../../bus-arith.h"
  #include "h/defaults.h"
  #include <string.h>
}

#include <h/const.h>
#include <h/types.h>
#include <h/blockdev.h>
#include <h/cache.h>


#include <e/utility.e>
#include <e/processor.e>
#include <e/setup.e>
#include <e/watch.e>



// This macro converts a byte address into a word address (minus offset)
//#define CONVERT(ad, bs)	((ad - bs) >> WORDSHIFT)	
#define CONVERT(ad,bs) (ad)


/*!
 * \class SystemBus
 * \author Martin Hauser <info@martin-hauser.net>
 * \author Mauro Morsiani <mmorsian@cineca.it>
 * \brief connects local stuff to the external bus
 *
 * This Class does all the things necersary to interface Mauro Morsiani's Simulator
 * with the external bus System. Several functions were changed from the original Simulator
 * and now just forward data or ignore the original designed devices.
 */

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


// This method builds a SystemBus object and all related structures
SystemBus::SystemBus(SetupInfo * stp, Watch * wch)
{
	unsigned int intl, dnum;
	
	setup = stp;
	watch = wch;
	proc = NULL;

	// init clock, interval timer and device events queue
    timeOfDay = new TimeStamp(0UL, 0UL);
	timer = MAXWORDVAL;
	
	// create RAM and ROM spaces
	
	if (setup->getBootType() == COREBOOT)
		readCoreFile(setup->getROMFileName(COREINDEX));

	intPendMask = 0UL;

	if(enableCache)
        cInstrCache = new Cache();
    else
        cInstrCache = NULL;
}


// This method deletes a SystemBus object and all related structures
SystemBus::~SystemBus(void)
{	
    if(timeOfDay != NULL)
    {
        delete timeOfDay;
        timeOfDay = NULL;
    }
    if(cInstrCache != NULL)
    {
      delete cInstrCache;
      cInstrCache = NULL;
    }
}


// This method links SystemBus to related Processor: it may be used to build 
// multi-processor systems with shared buses
void SystemBus::LinkToProc(Processor * prc)
{
	proc = prc;
}


// This method increments system clock and decrements interval timer;
// on timer underflow (0 -> FFFFFFFF transition) a interrupt is generated.
// Event queue is checked against the current clock value and device operations
// are completed if needed; all memory changes are notified to Watch control
// object
void SystemBus::ClockTick(void)
{
	unsigned int devAddr;
	unsigned int intl, dnum;
	
	timeOfDay->Increase();
	
	// both registers signal "change" because they are conceptually one
	watch->SignalBusAccess((TODHIADDR * WORDLEN) + DEVBASE, WRITE);
	watch->SignalBusAccess((TODLOADDR * WORDLEN) + DEVBASE, WRITE);
	
	if (UnsSub(&timer, timer, 1))
		// timer interrupt is now pending
		intPendMask = SetBit(intPendMask, TIMERINT + IPMASKBASE);
	
	watch->SignalBusAccess((TIMERADDR * WORDLEN) + DEVBASE, WRITE);

	
}


//
// The following methods allow to inspect or modify  TimeofDay Clock and
// Interval Timer (typically for simulation reasons); they are self-explanatory
//

Word SystemBus::getToDHI(void)
{
	return(timeOfDay->getHiTS());
}

Word SystemBus::getToDLO(void)
{
	return(timeOfDay->getLoTS());
}

Word SystemBus::getTimer(void)
{
	return(timer);
}
void SystemBus::setToDHI(Word hi)
{
	timeOfDay->setHiTS(hi);
}
void SystemBus::setToDLO(Word lo)
{
	timeOfDay->setLoTS(lo);
}

void SystemBus::setTimer(Word time)
{
	timer = time;
}


// This method reads a data word from memory at address addr, returning it
// thru datap pointer. It also returns TRUE if the address was invalid and
// an exception was caused, FALSE otherwise, and signals memory access to
// Watch control object
Boolean SystemBus::DataRead(Word addr, Word * datap)
{
	watch->SignalBusAccess(addr, READ);
	
	if (busRead(addr, datap))
	{
		// address invalid: signal exception to processor
		proc->SignalExc(DBEXCEPTION);
		return(TRUE);
	}
	else
		// address was valid
		return(FALSE);
		
}


//
// These methods allow Watch to inspect or modify single memory locations;
// they return TRUE if address is invalid or memory cannot be altered, and
// FALSE otherwise
// 

Boolean SystemBus::WatchRead(Word addr, Word * datap)
{
	return(busRead(addr, datap));
}

Boolean SystemBus::WatchWrite(Word addr, Word data)
{	
	return(busWrite(addr, data));
}


// This method writes the data word at physical addr in RAM memory or device
// register area.  Writes to BIOS or BOOT areas cause a DBEXCEPTION (no
// writes allowed). It returns TRUE if an exception was caused, FALSE
// otherwise, and notifies access to Watch control object
Boolean SystemBus::DataWrite(Word addr, Word data)
{
	watch->SignalBusAccess(addr, WRITE);

	if (busWrite(addr, data))
	{
		// data write is out of valid write bounds
		proc->SignalExc(DBEXCEPTION);
		return(TRUE);
	}
	else
		// write was successful
		return(FALSE);
}



				
// This method reads a istruction from memory at address addr, returning
// it thru istrp pointer. It also returns TRUE if the address was invalid and
// an exception was caused, FALSE otherwise, and notifies Watch

Boolean SystemBus::InstrRead(Word addr, Word* instrp)
{
	watch->SignalBusAccess(addr, EXEC);
	if(enableCache)
    {
        *instrp = cInstrCache->fetch_instr(addr);
        return(FALSE);
    }
    if (busRead(addr, instrp))
    {
    	// address invalid: signal exception to processor
    	proc->SignalExc(IBEXCEPTION);
    	return(TRUE);
    }
    else
      	return(FALSE);
}



// This method returns the current interrupt line status
Word SystemBus::getPendingInt(void)
{    
     unsigned int hi=0, lo=0;
     pthread_mutex_lock(&pmtxBusAccess); /*!< get the mutex for bus access */
     get_interrupt(bus_fd, 0, &hi, &lo);
     pthread_mutex_unlock(&pmtxBusAccess); /*!< we're done with the critic stuff */
     intPendMask = intPendMask | ((lo << 8) & 0xFC00);
     return(intPendMask);
}

Word SystemBus::ackPendingInt(Word wMask)
{
    intPendMask &= wMask;
}

/*!
 * \fn Boolean SystemBus::busRead(Word addr, Word* datap)
 * \author Martin Hauser <info@martin-hauser.net>
 * \author Mauro Morsiani <mmorsian@cineca.it>
 * \return false if everything went okay, true otherwise
 * \param addr The address we write to
 * \param datap The pointer to receive the data.
 * \brief writes data to bus
 * 
 * This function reads data from the external bus without any further checking.
 * Mauro Morisiani's original busRead function had a lot more features, but as 
 * there is nothing to be known about the devices connected to the external bus,
 * the function just sends it's readrequest and hopes that there is something to 
 * respond.
 *
 * As of Milestone 4 this contains the Code of memspace.cc::ExtMemRead as this is
 * now disabled. Also there is a pseudo read supported to the non-existing address
 * 0x10000004 returning the actual size of the ram. This is not actually the most
 * beautiful way to solve this but the history of the System requires it and it's
 * still better then setting the size of the ram per hand.
 */

Boolean SystemBus::busRead(Word addr, Word * datap)
{
    unsigned char cpAddress[8];
    unsigned char cpData[4];
    bool result = true;

    if(addr == 0x10000004)
    {
        *datap = ulRamSize;
        return false;
    }

    inttochar(0L,cpAddress);            /*!< the upper word will always be NULL */
    inttochar(addr,cpAddress+4);    /*!< copy Word Address at the right point */

    pthread_mutex_lock(&pmtxBusAccess); /*!< get the mutex for bus access */
    if (load_bus_data(bus_fd,cpAddress,cpData,WORDLEN)<0)
        result = false;
    
    pthread_mutex_unlock(&pmtxBusAccess); /*!< we're done with the critic stuff */
    if(!result)
        Panic("Illegal memory access in SystemBus::busRead()");
    
    *datap = chartoint(cpData);           /*!< copy Data pointer */  
    return !result;
}




/*!
 * \fn Boolean SystemBus::busWrite(Word addr, Word data)
 * \author Martin Hauser <info@martin-hauser.net>
 * \author Mauro Morsiani <mmorsian@cineca.it>
 * \return false if everything went okay, true otherwise
 * \param addr The address we write to
 * \param data The data to be written
 * \brief writes data to bus
 * 
 * This function writes data to the external bus without any further checking
 * Mauro Morisiani's original busWrite function had a lot more features, but as 
 * there is nothing to be known about the devices connected to the external bus,
 * the function just writes and hopes that there is something to respond.
 *
 * As of Milestone 4 this takes over the function of memspace.cc:ExtMemRead as there is
 * no memspace.cc anymore
 */
 
Boolean SystemBus::busWrite(Word addr, Word data)
{
    unsigned char cpAddress[8];
    unsigned char cpData[4]; 
    bool bRetVal = true;
   
    inttochar(0UL,cpAddress);           /*!< the upper word will always be NULL */
    inttochar(addr,cpAddress+4);    /*!< copy Word Address at the right point */
    inttochar(data,cpData);             /*!< copy Data pointer */

    pthread_mutex_lock(&pmtxBusAccess);
        if(store_bus_data(bus_fd,cpAddress,cpData,WORDLEN) < 0)
            bRetVal = false;
    pthread_mutex_unlock(&pmtxBusAccess);
    
    if(!bRetVal)
        Panic("Illegal memory access in SystemBus::busRead()");
    return !bRetVal;
}



void SystemBus::readCoreFile(const char * fName)
{
	FILE * cFile = NULL;
	Word tag;
	unsigned int i;
	Word addr = RAMBASE;
	Word wData;
		
	if (fName != NULL && !SAMESTRING(fName, EMPTYSTR))
	{        
		// tries to load core file from disk
		if ((cFile = fopen(fName, "r")) == NULL || fread((void *) &tag, WORDLEN, 1, cFile) != 1 || tag != COREFILEID)
		{
			ShowAlertQuit("Unable to load core file", fName, "cannot continue: exiting now...");
		}
		else
		{	
		  /*! loop through core file */
            while(!feof(cFile))
            {
                /*! Read next Chunk from File */
                fread((void *)(&wData),1,WORDLEN,cFile);

                /*! Write data to bus and check whether this was successful, otherwise quit*/
                if(busWrite(addr,wData))
                {
                    ShowAlertQuit("Failure in writing memory to Memory component... ","",
                                " bailing out!\n");
                    break;
                }
                addr += WORDLEN;
            }
		}
		if (cFile != NULL)
			fclose(cFile);
	}
}

