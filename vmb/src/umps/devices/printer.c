/*!
  * \file printer.c
  * \author Martin Hauser <info@martin-hauser.net>
  * \brief implements a simulation of the old umps printer
  * 
  * The functions in this file attempt to simulate the old umps printer using
  * the new virtual bus. Some features have been discarded as they were not needed
  * anymore (interrupts for example, as they were only passed in simulator code).
 */

#include <stdlib.h>
#include "../../message.h"
#include "../../bus-arith.h"
#include "../../bus-util.h"
#include "../../option.h"
#include "../../param.h"
#include "../../error.h"
#include "../../main.h"
#include <pthread.h>

typedef unsigned long Word;

char version[]="$Revision: 1.1 $ $Date: 2008-05-04 15:46:57 $";

char howto[] =
"\n"
"Programm attempts to simulate the printer provided with the old umps simulator.\n"
"This means the device in question is a charprinting device similar to the umps \n"
"Terminal device. It'll just receive Data at the given Addresses                \n"
"(addr == STATUS,addr + 4 == COMMAND, addr + 8 == DATA)\n"
"\n";

#define WORDLEN      4

#define CMDPRINTCHR	 2

#define READY        1
#define BUSY         3
#define PERROR       4

#define MAXSLOTS     2
#define STATUS       0
#define COMMAND      1
#define DATA0        2


// delays

// 10 second till char is printed
#define PRINTTIME 10


FILE* fLogFile = NULL;


pthread_mutex_t pmtxStatusLock,pmtxLFileLock; //!< mutexes to make the terminal threadsafe
pthread_t pthrPrintWorker;                    //!< printthread

Word wpStatus[MAXSLOTS+1] = {READY,0,0}; //!< the status array
char tmpArr[WORDLEN];  //!< temporary array used in reply calls



/*!
 * \fn unsigned char *get_payload(unsigned int offset, int size)
 * \author Martin Hauser <info@martin-hauser.net>
 * \return the payload
 * \param offset the offset from the base address of the device
 * \param size the amound of bytes read
 * \brief replies to read requests from the bus
 *
 * get_payload is called whenever there is a read request send towards
 * the device. It'll check whether the call is out of bounds and if not
 * from the parameters a slot in the ::wpStatus Array will be calculated,
 * whichs value will be returned.
 */

unsigned char *get_payload(unsigned int offset, int size)
{  
    if(offset/size > MAXSLOTS || offset < 0 || size !=4)
        inttochar(-1,tmpArr);
    else
        inttochar(wpStatus[offset/size],tmpArr);
    return tmpArr;
}

/*!
 * \fn int reply_payload(unsigned char address[8], int size,unsigned char *payload)
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief dummy callback
 */

int reply_payload(unsigned char address[8], int size,unsigned char *payload)
{ return 1;
}

/*!
 * \fn void statusChange(unsigned int uiSlot, Word wData)
 * \author Martin Hauser <info@martin-hauser.net>
 * \param uiSlot the slot to write to
 * \param wData the data to write
 * \brief changes values of the ::wpStatus Array
 *
 * statusChange applies changes into the ::wpStatus array, which is protected
 * by a mutex. If the given slot is out of bounds, the whole call is discarded
 * and no changes are made.
 */

void statusChange(unsigned int uiSlot,Word wData)
{
    if(uiSlot < 0 || uiSlot > MAXSLOTS)
        return;
    
    pthread_mutex_lock(&pmtxStatusLock);
    wpStatus[uiSlot] = wData;
    pthread_mutex_unlock(&pmtxStatusLock);
}

/*!
 * \fn Word getStatus(unsigned int uiSlot)
 * \author Martin Hauser <info@martin-hauser.net>
 * \param uiSlot the slot to be read
 * \return the data stored in the given slot or -1 if read was not appropriate
 * \brief reads the status register
 *
 * This function reads the ::wpStatus array which is protected by a mutex. If the
 * given slotindex exceeds the bounds of the array, it returns -1 to indicate 
 * the slot can't be read.
 */

Word getStatus(unsigned int uiSlot)
{
    Word wData;
    if(uiSlot < 0 || uiSlot > MAXSLOTS)
        return -1;
    
    pthread_mutex_lock(&pmtxStatusLock);
    wData = wpStatus[uiSlot];
    pthread_mutex_unlock(&pmtxStatusLock);
    return wData;
}

/*!
 * \fn void logPrint(char cData)
 * \author Martin Hauser <info@martin-hauser.net>
 * \param cData the char to be printed
 * \brief prints a char into the logFile
 *
 * logPrint prints the given char into the logfile and flushes it
 * afterwards, so that the new char may appear imediatily. The access
 * to the logfile is restricted through the use of a mutex.
 */

void logPrint(char cData)
{
   pthread_mutex_lock(&pmtxLFileLock);
   fprintf(fLogFile,"%c",cData);
   fflush(fLogFile);
   pthread_mutex_unlock(&pmtxLFileLock);
}


/*!
 * \fn void* printWorker()
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief controlls print to logfile (using ::logPrint)
 *
 * printWorker() is launched as seperate thread once a 
 * command has been received by ::put_payload . printWorker() will
 * check upon launch whether the received command was correct and if
 * so, it'll wait the time specified by ::PRINTTIME to simulate a physical 
 * delay and finally call ::logPrint to get the char out to the logfile. 
 * Before the function returns, it'll reset the status registers involved
 * in printing the char.
 */

void* printWorker()
{
    char cData = ((getStatus(DATA0)) & 0xFF);
    if(getStatus(COMMAND) != CMDPRINTCHR || getStatus(STATUS) != BUSY)
    {
        statusChange(STATUS,PERROR);
        return;
    }
    sleep(PRINTTIME);
    logPrint(cData);
    statusChange(DATA0,0);
    statusChange(STATUS,READY);
}

/*!
 * \fn void put_payload(unsigned int offset, int size, unsigned char* payload)
 * \author Martin Hauser <info@martin-hauser.net>
 * \param offset The offset to the address written to relative to the one given by the ::address variable.
 * \param size The number of bytes written to the terminal
 * \param payload the actual data written as char array
 * \brief processes data written to the device
 *
 * This function checks first whether the device is busy and if so rejects the data
 * written to it. If not, it then analyses the offset whether it was written to the
 * right address. Finally the payload is converted into Word format and a check for
 * the right command word is performed. If all this fits, the device status changes 
 * to busy and a new pthread is launched, performing the print (see ::printWorker for
 * detail).
 */


void put_payload(unsigned int offset, int size, unsigned char *payload)
{
    Word wData = 0;
    
    if(size != 4 || getStatus(STATUS) == BUSY) //!< check whether request is legitimate
        return;
        
    offset /= size;
    if(offset == DATA0) //!< was request sent to right address? 
    {
        wData = chartoint(payload);         
        statusChange(DATA0,wData);
    }
    else if (offset == COMMAND)
    {
        statusChange(STATUS,BUSY);       
        pthread_create(&pthrPrintWorker,0,printWorker,0); //!< launch print thread
    }
}

/*!
 * \fn static void prepare_file()
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief setups log file
 *
 * This function does everything neccersary to ensure the logfile used
 * by the program is open and writeable. It is called once at bootup of 
 * the terminal.
 */

static void prepare_file()
{
    if(filename == NULL || (fLogFile = fopen(filename,"w")) == NULL)
        fatal_error(__LINE__,"failed to open File!"); 
}

/*!
 * \fn init_device()
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief callback for device startup
 *
 * This function is called on bootup (see it as replacement main function)
 * and sets up the size,address and the mutexes used by other functions to
 * synchronize data element access. 
 */

void init_device(void)
{  
    size       =  (MAXSLOTS+1)*4;
    hexaddress = "100001D0";
    
    prepare_file();
    
    pthread_mutex_init(&pmtxStatusLock, NULL); // init mutex  
    pthread_mutex_init(&pmtxLFileLock,  NULL); // init mutex
    
    debugs("address:  %s",hexaddress);
    debugi("size:     %d",size);
    debugs("logFile:  %s",filename);
    
}


/*!
 * \fn void process_input(unsigned char c)
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief dummy callback
 */


void process_input(unsigned char c) 
{ 
  
}

/*!
 * \fn int process_poweroff(unsigned char interrupt)
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief dummy callback
 */


int process_interrupt(unsigned char interrupt)
{ return 0;
}  

/*!
 * \fn int process_poweron(void)
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief dummy callback for poweron
 */


int process_poweron(void)
{ 
  return 0;
}

/*!
 * \fn int process_poweroff(void)
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief dummy callback for poweroff
 */


int process_poweroff(void)
{  
  return 0;
}

/*!
 * \fn int process_reset(void)
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief Reset callback implementation
 *
 * This is the reset callback for response to a reset by the 
 * virtual motherboard. It'll just call the thread function 
 * ::printWorker and make sure that everything is back to
 * default values
 */

int process_reset(void)
{ 
    pthread_join(pthrPrintWorker,NULL);
    if(getStatus(STATUS) == BUSY)
        printWorker();
        
    statusChange(STATUS,READY);
    return 0;
}

