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
#include <stdio.h>
#if defined(WIN32)
#pragma warning(disable : 4996)
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#endif

#include "string.h"
#include "bus-arith.h"
#include "message.h"
#include "error.h"

unsigned int vmb_debug_flag = 0;
char *vmb_program_name = "Unknown";

#if defined(WIN32)
void win32_message(char *msg)
{
	MessageBox(NULL,msg,"Message",MB_OK);
}
void (*vmb_message_hook)(char *msg) = win32_message;
#else
void (*vmb_message_hook)(char *msg) = NULL;
#endif


/* thhooks can be used to change the apperance of messages and debug output */
void (*vmb_debug_hook)(char *msg) = NULL;

#if defined(WIN32)
#define MAX_DEBUG_LINES 500
#define MAX_DEBUG_COLUMNS 500
static FILE orig_stdout, orig_stdin, orig_stderr;
static int debug_on = 0;

/* two functions to switch on and off debugging by creating a console window */
void vmb_debug_on(void)
{ int hConHandle;
  HANDLE hStd;
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  FILE *fp;
  
  if (debug_on) return;

  if (!AllocConsole()) return;

  hStd = GetStdHandle(STD_OUTPUT_HANDLE);

  GetConsoleScreenBufferInfo(hStd, &coninfo);
  coninfo.dwSize.Y = MAX_DEBUG_LINES;
  coninfo.dwSize.X = MAX_DEBUG_COLUMNS;
  SetConsoleScreenBufferSize(hStd, coninfo.dwSize);

  /* redirect unbuffered STDOUT to the console */
  hConHandle = _open_osfhandle((intptr_t)hStd, _O_TEXT);
  fp = _fdopen( hConHandle, "w" );
  orig_stdout = *stdout;
  *stdout = *fp;
  setvbuf( stdout, NULL, _IONBF, 0 );

  /* redirect unbuffered STDERR to the console */

  hStd = GetStdHandle(STD_ERROR_HANDLE);
  hConHandle = _open_osfhandle((intptr_t)hStd, _O_TEXT);
  fp = _fdopen( hConHandle, "w" );
  orig_stderr = *stderr;
  *stderr = *fp;
  setvbuf( stderr, NULL, _IONBF, 0 );

#ifdef REDIRECT_STDIN
  /* redirect unbuffered STDIN to the console */
  hStd = GetStdHandle(STD_INPUT_HANDLE);
  hConHandle = _open_osfhandle((intptr_t)hStd, _O_TEXT);
  fp = _fdopen( hConHandle, "r" );
  orig_stdin = *stdin;
  *stdin = *fp;
  setvbuf( stdin, NULL, _IONBF, 0 );
#endif	

	vmb_debug_flag = 1;
	debug_on = 1;
}

void vmb_debug_off(void)
{ 
  if (!debug_on) return;
  FreeConsole();

  *stdout = orig_stdout;
  *stderr = orig_stderr;
#ifdef REDIRECT_STDIN
  *stdin = orig_stdin;
#endif
  vmb_debug_flag = 0;
  debug_on = 0;
}
#endif


void vmb_message(char *message)
{  if (vmb_message_hook != NULL)
     vmb_message_hook(message);
   else
 	 fprintf(stderr,"%s\r\n", message);
}

void vmb_fatal_error(int line, char *message)
{   vmb_error(line, message);
    exit(1);
}


void vmb_error(int line, char *message)
{ static char tmp[1000];
  sprintf(tmp,"ERROR (%s, %d): %s\r\n",vmb_program_name,line,message);
  vmb_message(tmp);
}


void vmb_debug(char *msg)
{ 
  if (!vmb_debug_flag) return;
  if (vmb_debug_hook == NULL)
	fprintf(stderr,"DEBUG (%s): %s\r\n",vmb_program_name, msg);
  else  
	vmb_debug_hook(msg);
}

void vmb_debugi(char *msg, int i)
/* a function to call to display debug messages */
{ 
  static char tmp[1000];
  if (!vmb_debug_flag) return;
  sprintf(tmp,msg,i);
  vmb_debug(tmp);
}


void vmb_debugs(char *msg, char *s)
/* a function to call to display debug messages */
{ 
  static char tmp[1000];
  if (!vmb_debug_flag) return;
  sprintf(tmp,msg,s);
  vmb_debug(tmp);
}


void vmb_debugx(char *msg, unsigned char *s, int n)
     /* a function to call to display debug messages */
{ int i; 
#define HEXMAX 256
 
 static char hex[HEXMAX*2+ HEXMAX/8 +1];/*each character 2 hex digits,
                                          one space every eight' digit, 
                                          one zero byte */
 char *p;
 if (!vmb_debug_flag) return;
 if (n>HEXMAX) n = HEXMAX;

 i = 0;   
 hex[0] = 0;
 p = hex;
 while (n-i>=8)
   { chartohex(s+i,p,8);
   p=p+8*2;
   *p = ' ';
   p = p+1;
   i = i+8;
   }
 if (n-i>0)
   { chartohex(s+i,p,n-i);
   p = p + (n-i)*2;
   }
 *p = 0;
  vmb_debugs(msg,hex);
}



static char *
debug_type (unsigned char mtype)
{
  static char typestr[100];
  typestr[0] = 0;
  if (mtype & TYPE_BUS)
    strcat (typestr, "BUS ");
  if (mtype & TYPE_TIME)
    strcat (typestr, "TIME ");
  if (mtype & TYPE_ADDRESS)
    strcat (typestr, "ADDRESS ");
  if (mtype & TYPE_REQUEST)
    strcat (typestr, "REQUEST ");
  if (mtype & TYPE_ROUTE)
    strcat (typestr, "ROUTE ");
  if (mtype & TYPE_PAYLOAD)
    strcat (typestr, "PAYLOAD ");
  if (mtype & TYPE_LOCK)
    strcat (typestr, "LOCK ");
  if (mtype & TYPE_UNUSED)
    strcat (typestr, "UNUSED ");
  return typestr;
}
static char *
debug_id (unsigned char id)
{
  switch (id)
  {
  case ID_IGNORE:
    return "IGNORE";
  case ID_READ:
    return "READ";
  case ID_WRITE:
    return "WRITE";
  case ID_READREPLY:
    return "READREPLY";
  case ID_NOREPLY:
    return "NOREPLY";
  case ID_READBYTE:
    return "READBYTE";
  case ID_READWYDE:
    return "READWYDE";
  case ID_READTETRA:
    return "READTETRA";
  case ID_WRITEBYTE:
    return "WRITEBYTE";
  case ID_WRITEWYDE:
    return "WRITEWYDE";
  case ID_WRITETETRA:
    return "WRITETETRA";
  case ID_BYTEREPLY:
    return "BYTEREPLY";
  case ID_WYDEREPLY:
    return "WYDEREPLY";
  case ID_TETRAREPLY:
    return "TETRAREPLY";
/* predefined IDs for BUS messages */
  case ID_REGISTER:
    return "REGISTER";
  case ID_UNREGISTER:
    return "UNREGISTER";
  case ID_INTERRUPT:
    return "INTERRUPT";
  case ID_TERMINATE:
    return "TERMINATE";
  case ID_RESET:
    return "RESET";
  case ID_POWEROFF:
    return "POWEROFF";
  case ID_POWERON:
    return "POWERON";
  default:
    return NULL;
  }
}

void vmb_debugm(unsigned char mtype,unsigned char msize, unsigned char mslot,unsigned char mid,
                   unsigned char maddress[8], unsigned char *mpayload)
{ if (!vmb_debug_flag) return;
  vmb_debugs ("\ttype:    %s", debug_type (mtype));
  vmb_debugi ("\tsize:    %d", msize);
  vmb_debugi ("\tslot:    %d", mslot);
  {
    char *id_str = debug_id (mid);
    if (id_str == NULL)
      vmb_debugx ("\tid:      %s", &mid, 1);
    else
      vmb_debugs ("\tid:      %s", id_str);
  }
#if 0
  if (mtype & TYPE_TIME)
    vmb_debugi ("\ttime:    %d", mtime);
#endif
  if (mtype & TYPE_ADDRESS)
    vmb_debugx ("\taddress: %s", maddress, 8);
  if (mtype & TYPE_PAYLOAD)
    vmb_debugx ("\tpayload: %s", mpayload, 8 * (msize + 1));
}
