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
#define _WIN32_WINNT 0x0500 
#pragma warning(disable : 4996)
#include <windows.h>
#include <wincon.h>
#include <fcntl.h>
#include <io.h>
#endif

#include "string.h"
#include "bus-arith.h"
#include "message.h"
#include "error.h"

unsigned int vmb_debug_flag = 0;


int vmb_verbose_level = 1;
char *vmb_program_name = "Unknown";
void (*vmb_message_hook)(char *msg) = NULL;
void (*vmb_debug_hook)(char *msg) = NULL;


void vmb_debug_on(void)
{  vmb_debug_flag = 1;
}

void vmb_debug_off(void)
{  vmb_debug_flag = 0;
}



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


void vmb_debug(int level, char *msg)
{ 
  if (!vmb_debug_flag) return;
  if (level<vmb_verbose_level) return;
  if (vmb_debug_hook == NULL)
	fprintf(stderr,"DEBUG (%s): %s\r\n",vmb_program_name, msg);
  else  
	vmb_debug_hook(msg);
}

void vmb_debugi(int level, char *msg, int i)
/* a function to call to display debug messages */
{ 
  static char tmp[1000];
  if (!vmb_debug_flag) return;
  if (level<vmb_verbose_level) return;
  sprintf(tmp,msg,i);
  vmb_debug(level, tmp);
}


void vmb_debugs(int level, char *msg, char *s)
/* a function to call to display debug messages */
{ 
  static char tmp[1000];
  if (!vmb_debug_flag) return;
  if (level<vmb_verbose_level) return;
  sprintf(tmp,msg,s);
  vmb_debug(level, tmp);
}


void vmb_debugx(int level, char *msg, unsigned char *s, int n)
     /* a function to call to display debug messages */
{ int i; 
#define HEXMAX (256*8) 
 
  static char hex[HEXMAX*2+ HEXMAX/8 +1];/*each character 2 hex digits,
                                          one space every eight' digit, 
                                          one zero byte */
  char *p;
  if (!vmb_debug_flag) return;
  if (level<vmb_verbose_level) return;
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
  vmb_debugs(level, msg,hex);
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

void vmb_debugm(int level, unsigned char mtype,unsigned char msize, unsigned char mslot,unsigned char mid,
                   unsigned char maddress[8], unsigned char *mpayload)
{ if (!vmb_debug_flag) return;
  if (level<vmb_verbose_level) return;
  vmb_debugs(level,"\ttype:    %s", debug_type (mtype));
  vmb_debugi(level,"\tsize:    %d", msize);
  vmb_debugi(level,"\tslot:    %d", mslot);
  {
    char *id_str = debug_id (mid);
    if (id_str == NULL)
      vmb_debugx(level,"\tid:      %s", &mid, 1);
    else
      vmb_debugs(level,"\tid:      %s", id_str);
  }
#if 0
  if (mtype & TYPE_TIME)
    vmb_debugi(level,"\ttime:    %d", mtime);
#endif
  if (mtype & TYPE_ADDRESS)
    vmb_debugx(level,"\taddress: %s", maddress, 8);
  if (mtype & TYPE_PAYLOAD)
    vmb_debugx(level,"\tpayload: %s", mpayload, 8 * (msize + 1));
}
