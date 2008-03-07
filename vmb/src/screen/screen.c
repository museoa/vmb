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
extern HWND hMainWnd;
#else
#include <unistd.h>
#endif

#include "bus-arith.h"
#include "option.h"
#include "param.h"
#include "vmb.h"

void display_char(char c);


char version[]="$Revision: 1.3 $ $Date: 2008-03-07 15:00:48 $";

char howto[] =
"The program will contact the motherboard at [host:]port\r\n"
"and register itself with the given address and interrupt.\r\n"
"Then, the program will be ready to receive bytes and display them\r\n"
"by sending them to standard output.\r\n"
"Whenever ready to receive a byte, it will signal an interrupt with the\r\n"
"given interrupt number.\r\n"
"At the given address it will provide a read/write octabyte\r\n"
"in the following format:\r\n"
"\r\n"
"   XX00 00YY 0000 00ZZ\r\n"
"\r\n"
"The XX byte signals errors it will be 0x80 if an error occured\r\n"
"and 0x00 otherwise.\r\n"
"\r\n"
"The YY byte contains the number of characters that were written\r\n"
"to this address since the last output. This should be 0\r\n"
"if the address is ready to receive a character or 1 if the hardware\r\n"
"is bussy with writing a byte. Any other value will indicate that characters were\r\n"
"lost since the last write operation.\r\n"
"\r\n"
"The ZZ byte contains the last character received. It is valid only\r\n"
"if YY is not zero.\r\n"
"\r\n"
"The complete ocatbyte will be reset to zero after a byte is output.\r\n"
"\r\n"
;

static unsigned char data[8];
#define ERROR 0
#define COUNT 3
#define DATA  7


void init_device(void)
{ vmb_debugi("address hi: %x",vmb_address_hi);
  vmb_debugi("address lo: %x",vmb_address_lo);
  vmb_debugi("interrupt: %d",interrupt);
  vmb_size = 8;
#ifndef WIN32
   setvbuf(stdout,NULL,_IONBF,0); /* make ouput unbuffered */
#endif
}

static void display_char(char c)
{ 
#ifdef WIN32	
   SendMessage(hMainWnd,WM_USER+6,(WPARAM)c,0);
#else
    printf("%c",c);
#endif
} 

/* Interface to the virtual motherboard */


unsigned char *vmb_get_payload(unsigned int offset,int size)
{ return data+offset;
}

void vmb_put_payload(unsigned int offset,int size, unsigned char *payload)
{   
    unsigned char cpData[8]; //!< to fix payload smaller then 8
    int i;

    if( offset + size < 8 )
    {
	 for(i=0;i<7;i++) //!< cleanout memory (just in case)
	   cpData[i] = 0;
	 cpData[DATA] = payload[size-1-offset]; //!< copy the char on the right place
	 printf("%d,%d\n",offset,size);
	 payload = cpData;
    }
    if (data[COUNT]<0xFF) data[COUNT]++;
    else data[ERROR]=0x80;
    data[DATA] = payload[7-offset];
    vmb_debugi("(%02X)",data[DATA]);
    display_char(data[DATA]);
    memset(data,0,8);
    vmb_raise_interrupt(interrupt);
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

void vmb_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{ /* do nothing */
#ifdef WIN32
   SendMessage(hMainWnd,WM_USER+4,0,0);
#endif
}


void vmb_terminate(void)
/* this function is called when the motherboard politely asks the device to terminate.*/
{ 
#ifdef WIN32
   PostMessage(hMainWnd,WM_QUIT,0,0);
#endif
}

void vmb_reset(void)
{
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


