/*
 * disk.c -- disk simulation
   The following code is thanks to the gimmix project.
   It was ported to fitt the mmix motherboard by Martin Ruckert. 2005

 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#include "resource.h"
#include "win32main.h"
#else
#include <pthread.h>
#include <unistd.h>
#endif
#include "vmb.h"
#include "bus-arith.h"
#include "param.h"
#include "disk.h"


char version[]="$Revision: 1.5 $ $Date: 2008-07-16 15:33:24 $";

char howto[] =
"The disk simulates a disk controller and the disk proper by using a\n"
"file of an appropriate size and organization on the host system. \n"
"\n"
"The disk understands two commands: read a block of data from the disk\n"
"or write a block of data to the disk. The amount of data transferred\n"
"is specified as an integral number of sectors. The sector size is\n"
"fixed at 512 bytes. The number of sectors to read or write is given\n"
"to the controller in the count register. The position of the starting\n"
"sector on the disk is given to the controller in the sector register.\n"
"The disk sectors are numbered starting at 0 through (and including)\n"
"the number of sectors on the disk minus one. The number of sectors on\n"
"the disk can be read by software from the capacity register.\n"
"\n"
"The memory address from where a block of data is read in a write operation\n"
"or to where a block of data is written in a read operation is determined\n"
"by the contents of the DMA register. It holds the physical address of the\n"
"first byte to be transferred. This address must be aligned to an octabyte\n"
"boundary.\n"
"\n"
"a) Control register: 5 bits of this register are used to control the disk\n"
"and get current status information from it. IEN is read-write and enables\n"
"or disables interrupts from the disk. WRT is read-write and must be set to\n"
"0 to read from the disk, or set to 1 to write to the disk. As soon as the\n"
"software writes a 1 to the STRT bit (which is write-only), the next command\n"
"is carried out. BUSY is set by the hardware during command execution. When\n"
"the command is completed (or cannot be completed due to an error), BUSY\n"
"is reset. This condition also raises the disk interrupt if it is enabled.\n"
"The ERR bit is set by the hardware if any error prevented the successful\n"
"completion of the last command. Both BUSY and ERR are read-only.\n"
"\n"
"b) Count register: this register holds the number of disk\n"
"sectors to be transferred in the next command.\n"
"\n"
"c) Sector register: this register holds the disk sector\n"
"number of the first sector to be transferred in the next command.\n"
"\n"
"d) DMA register: this register holds the address of the first\n"
"byte of physical memory where the next transfer takes place. This address\n"
"must be octabyte-aligned.\n"
"\n"
"e) Capacity register: this read-only register holds the total number of\n"
"sectors on the disk.\n"
"\n";


/* Data and auxiliar Functions */
static FILE *diskImage;

static int diskCtrl;
static int diskCnt;
static int diskSct;
static int diskDma_hi;
static int diskDma_lo;
static int diskCap;
static unsigned char mem[8*5] = {0};


/* protecting the variable diskCtrl, to syncronize
   reading and writing the controllregisters and reading and writing 
   the disk with DMA */

#ifdef WIN32
HANDLE haction;
CRITICAL_SECTION   action_section;
#else
static pthread_mutex_t action_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t action_cond = PTHREAD_COND_INITIALIZER;

static void clean_up_action_mutex(void *_dummy)
{ pthread_mutex_unlock(&action_mutex); /* needed if canceled waiting */
}
#endif

void set_diskCtrl(int value)
{ int start;
  vmb_debugi("Setting diskCtrl %X",value);
  start = 0;
#ifdef WIN32
  EnterCriticalSection (&action_section);
#else
  { int rc = pthread_mutex_lock(&action_mutex);
    if (rc) 
    { vmb_errormsg("Locking action mutex failed");
      pthread_exit(NULL);
    }
  }
#endif
  diskCtrl = value;
  if (diskCtrl & DISK_STRT) {
     start = 1;
  }
#ifdef WIN32
  LeaveCriticalSection (&action_section);
  if (start) SetEvent (haction);
#else
  if (start) pthread_cond_signal(&action_cond);
  { int rc;
    rc = pthread_mutex_unlock(&action_mutex);
    if (rc) 
    { vmb_errormsg("Unlocking action mutex failed");
      pthread_exit(NULL);
    }
  }
#endif
  if (start)
    vmb_debug("Action triggered");
}


static unsigned int cancel_wait_for_action = 0;

void wait_for_action(void)
/* waits for setting the start bit or a cancelation
*/
{ 
#ifndef WIN32
  { int rc = pthread_mutex_lock(&action_mutex);
    if (rc) 
    { vmb_errormsg("Locking action mutex failed");
      pthread_exit(NULL);
    }
  }
  pthread_cleanup_push(clean_up_action_mutex,NULL);
#endif
  /* in the meantime the action might have happend */
  while (!(diskCtrl & DISK_STRT) &&
         !cancel_wait_for_action)
#ifdef WIN32
     WaitForSingleObject(haction,INFINITE);
#else
     pthread_cond_wait(&action_cond,&action_mutex);
  pthread_cleanup_pop(1);
#endif
  cancel_wait_for_action=0;
  set_diskCtrl(diskCtrl & ~DISK_STRT);
}


static void register_to_mem(void)
{  memset(mem,0,sizeof(mem));
   inttochar(diskCtrl,mem+DISK_CTRL+4);
   inttochar(diskCnt,mem+DISK_CNT+4);
   inttochar(diskSct,mem+DISK_SCT+4);
   inttochar(diskDma_hi,mem+DISK_DMA);
   inttochar(diskDma_lo,mem+DISK_DMA+4);
   inttochar(diskCap,mem+DISK_CAP+4);
}

static void mem_to_register(int offset, int size)
{  vmb_debugi("mem to registers offset: %d",offset);
   vmb_debugi("mem to registers size: %d",size);

   if (offset < DISK_CTRL+8 && offset+size > DISK_CTRL)
     set_diskCtrl(chartoint(mem+DISK_CTRL+4));
   diskCnt  = chartoint(mem+DISK_CNT+4);
   diskSct  = chartoint(mem+DISK_SCT+4);
   diskDma_hi =  chartoint(mem+DISK_DMA);
   diskDma_lo =  chartoint(mem+DISK_DMA+4);
/* its read only diskCap  = chartoint(mem+DISK_CAP+4); */
}

void inc_DiskDma(int i)
{ unsigned int tmp;
 tmp = (unsigned int) diskDma_lo +i;
 if (tmp < (unsigned int)diskDma_lo)
   diskDma_hi++;
 diskDma_lo = tmp;
}


/* Operating the Disk */


static void diskReset(void)
{ vmb_debug("Disk is Reset");
  diskCnt = 0;
  diskSct = 0;
  diskDma_lo = 0;
  diskDma_hi = 0;
  cancel_wait_for_action=1;
  set_diskCtrl(0);
}

static void diskInit(void) {
  long numBytes;

  diskImage = NULL;
  diskCap = 0;
  vmb_debug("Initializing Disk");
  if (filename != NULL) {
    /* try to install disk */
    diskImage = fopen(filename, "r+b");
    if (diskImage == NULL) {
      vmb_errormsg("cannot open disk image");
    }
    fseek(diskImage, 0, SEEK_END);
    numBytes = ftell(diskImage);
    fseek(diskImage, 0, SEEK_SET);
    diskCap = numBytes / SECTOR_SIZE;
    vmb_debugi("Disk of size %ld sectors installed.", diskCap);
  }
  diskReset();
}



static void diskExit(void) {
  if (diskImage == NULL) 
    /* disk not installed */
    return;
  vmb_debug("Closing Disk");
  fclose(diskImage);
  diskImage = NULL;
  diskCap = 0;
  diskReset();
}

static void diskBussy(void)
{ if ( diskCtrl & DISK_BUSY)
     return;
  vmb_debug("Disk is Bussy");
  set_diskCtrl((diskCtrl | DISK_BUSY) & ~DISK_ERR);
}


static void diskDone(void)
{ vmb_debug("Disk is Idle");
  set_diskCtrl(diskCtrl & ~DISK_BUSY);
  if (diskCtrl & DISK_IEN) {
    vmb_raise_interrupt(interrupt);
    vmb_debug("Raised interrupt");
  }
}

static int diskPosition(void)
{ vmb_debugi("Positioning to sector %d",diskSct);

  if (diskCap > 0 &&
         diskCnt > 0 &&
         diskSct >= 0 &&
         diskSct < diskCap &&
         diskSct + diskCnt <= diskCap) {
         if (fseek(diskImage, diskSct * SECTOR_SIZE, SEEK_SET) != 0) {
           vmb_errormsg("cannot position to sector in disk image");
           set_diskCtrl(diskCtrl | DISK_ERR);
           return 0;
         }
	 else
	 { vmb_debug("Positioned");
           return 1;
	 }
   }
   return 0;
}


static unsigned char sector_buffer[SECTOR_SIZE];
data_address da ={sector_buffer,0,0,SECTOR_SIZE,STATUS_INVALID};

static void diskRead(void) 
{ 
  /* disk --> memory */
  if (!diskPosition())
    return;
  diskBussy();
  while(diskCnt>0) {
    if (fread(sector_buffer, SECTOR_SIZE, 1, diskImage) != 1)  {
          vmb_errormsg("cannot read from disk");
           set_diskCtrl(diskCtrl | DISK_ERR);
          break;
    }
    da.address_lo = diskDma_lo;
    da.address_hi = diskDma_hi;
    da.status = STATUS_VALID;
    vmb_store(&da);
    vmb_debugi("Read sector %d",diskSct);
    inc_DiskDma(SECTOR_SIZE);
    diskCnt--;
    diskSct++;
  }       
  diskDone();
}

static void diskWrite(void) 
{ /* memory --> disk */
  if (!diskPosition())
    return;
  diskBussy();
  while(diskCnt>0) {
    da.address_lo = diskDma_lo;
    da.address_hi = diskDma_hi;
    da.status = STATUS_INVALID;
    vmb_load(&da);
    vmb_wait_for_valid(&da);
    if (da.status!=STATUS_VALID) {
          vmb_errormsg("cannot read memory");
          set_diskCtrl(diskCtrl | DISK_ERR);
          break;
    }
    if (fwrite(sector_buffer, SECTOR_SIZE, 1, diskImage) != 1)  {
          vmb_errormsg("cannot write to disk");
          set_diskCtrl(diskCtrl | DISK_ERR);
          break;
    }
    vmb_debugi("Wrote sector %d",diskSct);
    inc_DiskDma(SECTOR_SIZE);
    diskCnt--;
    diskSct++;
  }
  diskDone();
}

/* connecting to the virtual motherbaord */


unsigned char *vmb_get_payload(unsigned int offset, int size)
     /* read an octabyte  (one of the five registers)*/
{  
   register_to_mem();	
   return mem+offset;
}


void vmb_put_payload(unsigned int offset, int size, unsigned char *payload)
     /* write an octabyte  (one of the five registers)*/
{  
   if ( diskCtrl &DISK_BUSY)
   {  vmb_debug("Write ignored, disk bussy");
      return; /* no writing while we are bussy */
   }
   register_to_mem();
   memmove(mem+offset,payload,size);
   mem_to_register(offset,size);
}


void vmb_poweron(void)
{  diskInit();
#ifdef WIN32
   SendMessage(hMainWnd,WM_USER+1,0,0);
#endif
}


void vmb_poweroff(void)
{ diskExit();
#ifdef WIN32
   SendMessage(hMainWnd,WM_USER+2,0,0);
#endif
}


void vmb_reset(void)
{ diskExit();
  diskInit();
}

void vmb_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{ diskExit();
#ifdef WIN32
   SendMessage(hMainWnd,WM_USER+4,0,0);
#endif
}


void vmb_terminate(void)
/* this function is called when the motherboard politely asks the device to terminate.*/
{ diskExit();
#ifdef WIN32
   PostMessage(hMainWnd,WM_QUIT,0,0);
#endif
}


#ifdef WIN32
#else


int main(int argc, char *argv[])
{ param_init(argc, argv);
  vmb_debugs("%s ",vmb_program_name);
  vmb_debugs("%s ", version);
  vmb_debugs("host: %s ",host);
  vmb_debugi("port: %d ",port);
  vmb_size = 8*5;
  vmb_debugi("address hi: %x",vmb_address_hi);
  vmb_debugi("address lo: %x",vmb_address_lo);
  vmb_debugi("size: %x ",vmb_size);

  vmb_connect(host,port); 
  vmb_register(vmb_address_hi,vmb_address_lo,vmb_size,
               0, 0, vmb_program_name);

  while (vmb_connected)
  {  wait_for_action();
     if (diskCtrl & DISK_WRT)
           diskWrite();
     else
           diskRead();
  }
  return 0;
}

#endif
