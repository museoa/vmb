/*
    Copyright 2005 Martin Ruckert
    
    ruckertm@acm.org

    This file is part of the MMIX Motherboard project

    This file is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this software; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#include "resource.h"
#pragma warning(disable : 4996)
extern HWND hMainWnd;
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#include "bus-arith.h"
#include "option.h"
#include "param.h"
#include "vmb.h"



char version[]="$Revision: 1.9 $ $Date: 2010-03-02 10:48:23 $";

char howto[] =
"\n"
"The program will contact the motherboard at host:port\n"
"and register itself with the given satrt address.\n"
"Then, the program will read the file and answer\n"
"read and write requests from the bus.\n"
"\n"
;

static unsigned char *flash=NULL;

#define PAGESIZE (1<<13) /*  8 kbyte */

static int image_changed = 0;

void open_file(void)
{ FILE *f;
  struct stat fs;
  int rc;
  if (filename==NULL)
    vmb_fatal_error(__LINE__,"No filename");
  vmb_debug(VMB_DEBUG_PROGRESS, "reading image file...");
  f = fopen(filename,"rb");
  if (f==NULL) vmb_fatal_error(__LINE__,"Unable to open file");
  if (fstat(fileno(f),&fs)<0) vmb_fatal_error(__LINE__,"Unable to get file size");
  vmb_size = (fs.st_size+PAGESIZE-1)&~(PAGESIZE-1); /* make it a multiple of pages */
  if (vmb_size <= 0)  vmb_fatal_error(__LINE__,"File empty");
  if (flash!=NULL) free(flash);
  flash = malloc(vmb_size);
  if (flash==NULL) vmb_fatal_error(__LINE__,"Out of memory");
  rc=(int)fread(flash,1,vmb_size,f);
  if (rc<0) vmb_fatal_error(__LINE__,"Unable to read file");
  if (rc==0) vmb_fatal_error(__LINE__,"Empty file");
  fclose(f);
  image_changed = 0;
  vmb_debug(VMB_DEBUG_PROGRESS, "done reading image file");
}


void write_file(void)
{ FILE *f;
  int i;
  if (!image_changed) return;
  vmb_debug(VMB_DEBUG_PROGRESS, "writing image file...");
  if (filename==NULL|| filename[0]== 0)
  { vmb_error(__LINE__,"No filename");
    return;
  }
  if (flash==NULL)
  {  vmb_error(__LINE__,"No flash memory");
     return;
  }
  f = fopen(filename,"wb");
  if (f==NULL) 
  { vmb_error(__LINE__,"Unable to open file");
    return;
  }
  i = (int)fwrite(flash,1,vmb_size,f);
  if (i<(int)vmb_size) vmb_error(__LINE__,"Unable to write file");
  else fclose(f);
  vmb_debug(VMB_DEBUG_PROGRESS, "done writing image file");
  image_changed = 0;
}

/* Interface to the virtual motherboard */


unsigned char *flash_get_payload(unsigned int offset,int size)
{
    return flash+offset;
}


void flash_put_payload(unsigned int offset,int size, unsigned char *payload)
{    memmove(flash+offset,payload,size);
     image_changed = 1;
}


void flash_poweron(void)
{ open_file();
  vmb_debugi(VMB_DEBUG_INFO, "size: %d",vmb_size);
  #ifdef WIN32
   PostMessage(hMainWnd,WM_VMB_ON,0,0);
#endif
}

void flash_poweroff(void)
{  write_file();
#ifdef WIN32
   PostMessage(hMainWnd,WM_VMB_OFF,0,0);
#endif
}

void flash_reset(void)
{ write_file();
}


void flash_terminate(void)
/* this function is called when the motherboard politely asks the device to terminate.*/
{  write_file();
#ifdef WIN32
   PostMessage(hMainWnd,WM_QUIT,0,0);
#endif
}


void flash_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{ /* do nothing */
	write_file();
#ifdef WIN32
   PostMessage(hMainWnd,WM_VMB_DISCONNECT,0,0);
#endif
}

void init_device(device_info *vmb)
{ open_file();
  vmb->poweron=flash_poweron;
  vmb->poweroff=flash_poweroff;
  vmb->disconnected=flash_disconnected;
  vmb->reset=flash_reset;
  vmb->terminate=flash_terminate;
  vmb->put_payload=flash_put_payload;
  vmb->get_payload=flash_get_payload;
}


#ifdef WIN32
#else
int main(int argc, char *argv[])
{
 param_init(argc, argv);
 vmb_debugs(VMB_DEBUG_INFO, "%s ",vmb_program_name);
 vmb_debugs(VMB_DEBUG_INFO, "%s ", version);
 vmb_debugs(VMB_DEBUG_INFO, "host: %s ",host);
 vmb_debugi(VMB_DEBUG_INFO, "port: %d ",port);
 close(0); /* stdin */
 init_device(&vmb);
 vmb_debugi(VMB_DEBUG_INFO, "address hi: %x ",vmb_address>>32);
 vmb_debugi(VMB_DEBUG_INFO, "address lo: %x ",vmb_address&0xFFFFFFFF);
 vmb_debugi(VMB_DEBUG_INFO, "size: %x ",vmb_size);
 
 vmb_connect(&vmb,host,port); 

 vmb_register(vmb_address>>32,vmb_address&0xFFFFFFFF,
              vmb_size, 0, 0, vmb_program_name);
 vmb_wait_for_disconnect();
 write_file();
 return 0;
}

#endif
