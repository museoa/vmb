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




char version[]="$Revision: 1.4 $ $Date: 2008-03-07 15:00:48 $";

char howto[] =
"\n"
"The program will contact the motherboard at host:port\n"
"and register itself with the given satrt address.\n"
"Then, the program will read the file and answer read requests from the bus.\n"
"\n"
;

static unsigned char *rom=NULL;
typedef unsigned long Word;

#define PAGESIZE (1<<10) /* one kbyte */
#define BIOSFILEID	0x0253504D
#define WORDLEN 4

/*!
 * \fn void processUMPSFile(Word* biosBuf,long lSize)
 * \author Martin Hauser <info@martin-hauser.net>
 * \param biosBuf Buff filled with command words
 * \param lSize the size of the buffer
 * \brief read words into rom specificed by biosBuf
 * \warning Make sure that biosBuf is a malloc'ed structure, nothing local!
 *
 * This function will essentially walk through all the words of biosBuf and
 * attempt to convert them one by one into the char structure of ::rom. Then
 * it'll calculate the size to be passed on to the rom file and finally free
 * the buffer turned over.
 */

void processUMPSFile(Word* biosBuf,long lSize)
{
	int i,k;
	int j = 0;
	
	rom = malloc(lSize * WORDLEN); /*!< allocate real rom buffer (is of char) */
	
	for(i = 0; i < lSize; i++) /*!< walk through biosBuf */
	{  
	    for(k=0;k<4;k++)    
		    rom[j + k] = (unsigned char)((biosBuf[i] >> (32 - (k+1)*8))&0xFF); /*!< do an int to char conversion */
		
		j += WORDLEN;
	}
	free(biosBuf);
	size = lSize * WORDLEN; /*!< calc real size of rom */
}

/*
 * \fn void readUMPSFile(File* bFile)
 * \author Martin Hauser <info@martin-hauser.net>
 * \param bFile The file to read from
 * \brief does some safty checks on the file and turns the results over to ::processUMPSFile
 *
 * This function will take any File descriptor as argument, read the BIOS tag from it and if that's 
 * okay it'll turn over to read the size. If both are okay the rest of the file will be read into
 * a freshly allocated buffer, which will then be turned over to ::processUMPSFile where it will
 * be freed.
 */

void readUMPSFile(FILE* bFile)
{
    Word tag;
	Word * biosBuf;
    
    unsigned long lSize;
	unsigned long* sizep = &lSize;
	
	/*! check whether tag and size can be read from the file */
    if (fread((void *) &tag, WORDLEN, 1, bFile) != 1 || tag != BIOSFILEID || \
         fread((void *) sizep, WORDLEN, 1, bFile) != 1)
	{
	    vmb_fatal_error(__LINE__,"couldn't read umps file.\n");
	}
	
	/*! alloc buffer and read data into it */
	biosBuf = malloc(lSize * WORDLEN); 
	if (fread((void *) biosBuf, WORDLEN, *sizep, bFile) != *sizep)
	{
		// wrong file format
		free(biosBuf);
        vmb_fatal_error(__LINE__,"wrong UMPS file format.\n");
	}
	/*! turn data over */
	processUMPSFile(biosBuf,lSize);
	
}

void open_file(void)
{ 
    FILE *f;
    char *c;
    struct stat fs;
    
    if (filename==NULL || strcmp(filename,"") == 0 || (f = fopen(filename, "rb")) == NULL)
        vmb_fatal_error(__LINE__,"No filename or file");
        
    c = strrchr(filename,'.');
    if(strcmp(c,".umps") == 0)
    {
        readUMPSFile(f);
    }
    else
    {
       
        if (fstat(fileno(f),&fs)<0)
            vmb_fatal_error(__LINE__,"Unable to get file size");
    
        size = ((fs.st_size+7)/8)*8; /* make it a multiple of octas */
        if (rom!=NULL) free(rom);
        rom = malloc(size);
        if (rom==NULL) vmb_fatal_error(__LINE__,"Out of memory");
        size=(int)fread(rom,1,size,f);
        if (size < 0) vmb_fatal_error(__LINE__,"Unable to read file");
        if (size == 0) vmb_fatal_error(__LINE__,"Empty file");
        size = (size+PAGESIZE-1)&~(PAGESIZE-1); /*round up to whole pages*/
       
    }
    fclose(f);
}


void init_device(void)
{  debugi("address hi: %x",vmb_address_hi);
   debugi("address lo: %x",vmb_address_lo);
   open_file();
   vmb_debugi("size: %d",vmb_size);
   close(0);
}

/* Interface to the virtual motherboard */


unsigned char *vmb_get_payload(unsigned int offset,int size)
{
    return rom+offset;
}


void vmb_poweron(void)
{ 
#ifdef WIN32
   SendMessage(hMainWnd,WM_USER+1,0,0);
#endif
}

void vmb_poweroff(void)
{  
#ifdef WIN32
   SendMessage(hMainWnd,WM_USER+2,0,0);
#endif
}

void vmb_terminate(void)
/* this function is called when the motherboard politely asks the device to terminate.*/
{ 
#ifdef WIN32
   PostMessage(hMainWnd,WM_QUIT,0,0);
#endif
}


void vmb_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{ /* do nothing */
#ifdef WIN32
   SendMessage(hMainWnd,WM_USER+4,0,0);
#endif
}
#ifdef WIN32
#else
int main(int argc, char *argv[])
{
 param_init(argc, argv);
 vmb_debugs("%s ",vmb_program_name);
 vmb_debugs("%s ", version);
 vmb_debugs("host: %s ",host);
 vmb_debugi("port: %d ",port);
 close(0); /* stdin */
 init_device();
 vmb_debugi("address hi: %x",vmb_address_hi);
 vmb_debugi("address lo: %x",vmb_address_lo);
 vmb_debugi("size: %x ",vmb_size);
 
 vmb_connect(host,port); 
 vmb_register(vmb_address_hi,vmb_address_lo,vmb_size, 0, 0, vmb_program_name);
 vmb_wait_for_disconnect();
 return 0;
}

#endif
