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
#include <unistd.h>
#endif
#include "error.h"
#include "message.h"
#include "bus-arith.h"
#include "bus-util.h"
#include "param.h"
#include "disk.h"
#include "main.h"


static FILE *diskImage;
static long numSectors;

static int diskCtrl;
static int diskCnt;
static int diskSct;
static int diskCap;
static unsigned char diskDma[8];
static unsigned char mem[8*5] = {0};

char version[]="$Revision: 1.1 $ $Date: 2007-08-29 09:19:35 $";

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
"b) Count register: this write-only register holds the number of disk\n"
"sectors to be transferred in the next command.\n"
"\n"
"c) Sector register: this write-only register holds the disk sector\n"
"number of the first sector to be transferred in the next command.\n"
"\n"
"d) DMA register: this write-only register holds the address of the first\n"
"byte of physical memory where the next transfer takes place. This address\n"
"must be octabyte-aligned.\n"
"\n"
"e) Capacity register: this read-only register holds the total number of\n"
"sectors on the disk.\n"
"\n";


static void register_to_mem(void)
{  memset(mem,0,sizeof(mem));
   inttochar(diskCtrl,mem+DISK_CTRL+4);
   inttochar(diskCnt,mem+DISK_CNT+4);
   inttochar(diskSct,mem+DISK_SCT+4);
   memmove(mem+DISK_DMA,diskDma,8);
   inttochar(diskCap,mem+DISK_CAP+4);
}

static void mem_to_register(void)
{  diskCtrl = chartoint(mem+DISK_CTRL+4);
   diskCnt  = chartoint(mem+DISK_CNT+4);
   diskSct  = chartoint(mem+DISK_SCT+4);
   memmove(diskDma,mem+DISK_DMA,8);
/* its read only diskCap  = chartoint(mem+DISK_CAP+4); */
}

unsigned char *get_payload(unsigned int offset, int size)
     /* read an octabyte  (one of the five registers)*/
{  
   register_to_mem();	
   return mem+offset;
}

void diskDone(void)
{ diskCtrl &= ~DISK_BUSY;
  if (diskCtrl & DISK_IEN) {
    set_interrupt(bus_fd, interrupt);
    debug("Raised interrupt");
  }
}

static void diskRead(void) 
{ unsigned char buffer[SECTOR_SIZE];
  /* disk --> memory */
  while(diskCnt>0) {
    if (fread(buffer, SECTOR_SIZE, 1, diskImage) != 1)  {
          errormsg("cannot read from disk");
          diskCtrl |= DISK_ERR;
          break;
    }
    if (store_bus_data(bus_fd, diskDma, buffer, SECTOR_SIZE)!=1) {
          errormsg("cannot write memory");
          diskCtrl |= DISK_ERR;
          break;
    }
    add_offset(diskDma, SECTOR_SIZE, diskDma);
    diskCnt--;
    diskSct++;
    { int fd;
      fd = wait_event(0);
      if (valid_socket(fd)) 
        handle_event(fd);
    }       
  }
  diskDone();
}

static void diskWrite(void) 
{ send_msg(bus_fd,1,TYPE_ADDRESS|TYPE_REQUEST,
          (unsigned char)( SECTOR_SIZE/8 -1),0,ID_READ,0,diskDma,NULL);
}


int reply_payload(unsigned char address[8], int size,unsigned char *buffer)
{ if (equal(address,diskDma) && size == SECTOR_SIZE && diskCnt>0)
  {
    if (fwrite(buffer, SECTOR_SIZE, 1, diskImage) != 1) {
      errormsg("cannot write to disk image");
      diskCtrl |= DISK_ERR;
      return 1;
    }
    add_offset(diskDma, SECTOR_SIZE, diskDma);
    diskCnt--;
    diskSct++;
    if (diskCnt>0) 
      diskWrite();
    else
      diskDone();
  }
  else
    errormsg("Invalid datat transfer to disk");
 return 1;
}


void put_payload(unsigned int offset, int size, unsigned char *payload)
     /* write an octabyte  (one of the five registers)*/
{  
   if ( diskCtrl &DISK_BUSY)
     return; /* no writing while we are bussy */
   register_to_mem();
   memmove(mem+offset,payload,size);
   mem_to_register();
   if (diskCtrl & DISK_STRT) {
     diskCtrl &= ~DISK_STRT;
     if (diskCap > 0 &&
         diskCnt > 0 &&
         diskSct >= 0 &&
         diskSct < diskCap &&
         diskSct + diskCnt <= diskCap) {
         if (fseek(diskImage, diskSct * SECTOR_SIZE, SEEK_SET) != 0) {
           errormsg("cannot position to sector in disk image");
         }
         diskCtrl |= DISK_BUSY;
         diskCtrl &= ~DISK_ERR;
         if (diskCtrl & DISK_WRT)
           diskWrite();
         else
           diskRead();
      }
      else
        errormsg("Illegal disk transfer request");
   }
}


int process_poweron(void)
{       diskInit();
#ifdef WIN32
	SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hon); 
#endif
	return 0;
}


int process_poweroff(void)
{ diskExit();
#ifdef WIN32
  SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hconnect);
#endif
  return 0;
}


int process_reset(void)
{ diskCtrl = 0;
  diskCnt = 0;
  diskSct = 0;
  memset(diskDma,0,8);
  diskCap = numSectors;
  return 0;
}

int process_interrupt(unsigned char interrupt){
  return 0;
}

void process_input(unsigned char interrupt){
}


void diskInit(void) {
  long numBytes;

  if (filename == NULL) {
    /* do not install disk */
    diskImage = NULL;
    numSectors = 0;
  } else {
    /* try to install disk */
    diskImage = fopen(filename, "r+b");
    if (diskImage == NULL) {
      errormsg("cannot open disk image");
    }
    fseek(diskImage, 0, SEEK_END);
    numBytes = ftell(diskImage);
    fseek(diskImage, 0, SEEK_SET);
    numSectors = numBytes / SECTOR_SIZE;
    debugi("Disk of size %ld sectors installed.", numSectors);
  }
  process_reset();
}



void init_device(void)
{  debugs("address: %s",hexaddress);
   diskInit();
   size = 8*5;
   close(0);
}

void diskExit(void) {
  if (diskImage == NULL) {
    /* disk not installed */
    return;
  }
  fclose(diskImage);
}
