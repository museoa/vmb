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
#include "inspect.h"


char version[]="$Revision: 1.16 $ $Date: 2011-07-27 18:33:13 $";

char howto[] =
"\n"
"The program will contact the motherboard at host:port\n"
"and register itself with the given satrt address.\n"
"Then, the program will read the file and answer\n"
"read and write requests from the bus.\n"
"\n"
;

static unsigned char *flash=NULL;

#define PAGESIZE (1<<10) /*  1 kbyte */

static int image_changed = 0;

void open_file(void)
{ FILE *f;
  struct stat fs;
  f=NULL;
  vmb_size=0;
  if (filename==NULL || strcmp(filename,"") == 0)
    vmb_error(__LINE__,"No filename for image file given");
  else
  { vmb_debugs(VMB_DEBUG_PROGRESS, "Reading image file: %s",filename);
	f = vmb_fopen(filename,"rb");
	if (f==NULL) 
		vmb_error2(__LINE__,"Unable to open image file", filename);
	else
	{ if (fstat(fileno(f),&fs)<0) 
	    vmb_error(__LINE__,"Unable to get file size");
	  else
	  { vmb_size = (fs.st_size+PAGESIZE-1)&~(PAGESIZE-1); /* make it a multiple of pages */
	    if (vmb_size <= 0)  vmb_error(__LINE__,"File empty");
	    else
	   { if (flash!=NULL) free(flash);
	     flash = malloc(vmb_size);
		 if (flash==NULL) vmb_fatal_error(__LINE__,"Out of memory");
		 vmb_size=(int)fread(flash,1,vmb_size,f);
		 if (vmb_size<0) vmb_error(__LINE__,"Unable to read file");
		 if (vmb_size==0) vmb_error(__LINE__,"Empty file");
	   }
	  }
	fclose(f);
	}
  }
  if (vmb_size<=0)
  { vmb_size = PAGESIZE;
    flash = malloc(vmb_size);
      if (flash==NULL) vmb_fatal_error(__LINE__,"Out of memory");
  }
  inspector[0].address=vmb_address;
  inspector[0].size=vmb_size;
  mem_update(0,0,vmb_size);
  image_changed = 0;
  vmb_debug(VMB_DEBUG_PROGRESS, "Done reading image file");
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
	 mem_update(0,offset,size);
}


void flash_poweron(void)
{ open_file();
  mem_update(0,0,vmb_size);
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
   PostMessage(hMainWnd,WM_CLOSE,0,0);
#endif
}


void flash_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{ 
  write_file();
#ifdef WIN32
   PostMessage(hMainWnd,WM_VMB_DISCONNECT,0,0);
#endif
}

static int flash_read(unsigned int offset,int size,unsigned char *buf)
{ if (offset>vmb_size) return 0;
  if (offset+size>vmb_size) size =vmb_size-offset;
  memmove(buf,flash+offset,size);
  return size;
}

struct inspector_def inspector[2] = {
    /* name size get_mem address num_regs regs */
	{"Memory",0,flash_read,0,0,NULL},
	{0}
};


void init_device(device_info *vmb)
{ vmb_debuga(VMB_DEBUG_INFO, "address: %#08x %08x",vmb_address); 
  vmb->poweron=flash_poweron;
  vmb->poweroff=flash_poweroff;
  vmb->disconnected=flash_disconnected;
  vmb->reset=flash_reset;
  vmb->terminate=flash_terminate;
  vmb->put_payload=flash_put_payload;
  vmb->get_payload=flash_get_payload;
  open_file();
}


