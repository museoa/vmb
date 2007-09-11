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
#include "error.h"
#include "option.h"
#include "param.h"
#include "bus-arith.h"

#ifdef WIN32
#include <windows.h>
#include "win32main.h"


void fatal_error(int line, char *message)
{   debugi("ERROR (%d):",line);
    debug(message);
	MessageBox(NULL,message,"Error",MB_ICONEXCLAMATION |MB_OK);
	SendMessage(hMainWnd,WM_QUIT,(WPARAM)1,0);
	MessageBox(NULL,"We should not get here","Error",MB_ICONEXCLAMATION |MB_OK);
    exit(1);
}

void message(char *message)
{   debug(message);
	MessageBox(NULL,message,"Message",MB_OK);
}

void errormsg(char *message)
{   debug("ERROR:");
    debug(message);
	MessageBox(NULL,message,"Error",MB_ICONEXCLAMATION |MB_OK);
}

void debug(char *msg)
{ 
  static char nl[] ="\r\n";	
  int n;
  if (hDebug==NULL)
      return;
   n = SendDlgItemMessage(hDebug,IDC_DEBUG,EM_GETLINECOUNT,0,0);
   if (n>100)
   { n = SendDlgItemMessage(hDebug,IDC_DEBUG,EM_LINELENGTH,0,0);
     SendDlgItemMessage(hDebug,IDC_DEBUG,EM_SETSEL,0,n+2);
     SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)"");
     n = SendDlgItemMessage(hDebug,IDC_DEBUG,WM_GETTEXTLENGTH,0,0);
     SendDlgItemMessage(hDebug,IDC_DEBUG,EM_SETSEL,n,n);
   }
   SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)msg);
   SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)nl);
}

void debugi(char *msg, int i)
/* a function to call to display debug messages */
{ 
	static char nl[] ="\r\n";
	static char tmp[1000];
   sprintf(tmp,msg,i);
   debug(tmp);
}

void debugs(char *msg, char *s)
/* a function to call to display debug messages */
{ 
	static char nl[] ="\r\n";
	static char tmp[1000];
  if (hDebug==NULL)
      return;
   sprintf(tmp,msg,s);
   debug(tmp);
}



#else
 
void fatal_error(int line,char *msg)
{ fprintf(stderr,"ERROR (%s,%d): %s\r\n",programname,line, msg);
  exit(1);
}

void message(char *msg)
/* a function to call to display messages */
{ printf("%s\r\n",msg);
}

void errormsg(char *msg)
/* a function to call to display errormessages */
{ fprintf(stderr,"ERROR (%s): %s\r\n",programname, msg);
}

void debug(char *msg)
/* a function to call to display debug messages */
{ if (debugflag)
    fprintf(stderr,"DEBUG (%s): %s\r\n",programname, msg);
}
void debugi(char *msg, int i)
/* a function to call to display debug messages */
{ if (debugflag)
  {  fprintf(stderr,"DEBUG (%s): ",programname);
     fprintf(stderr,msg,i);
     fprintf(stderr,"\r\n");
  }
}
void debugs(char *msg, char *s)
/* a function to call to display debug messages */
{ if (debugflag)
  {  fprintf(stderr,"DEBUG (%s): ",programname);
     fprintf(stderr,msg,s);
     fprintf(stderr,"\r\n");
  }
}

#endif


void debugx(char *msg, char *s, int n)
     /* a function to call to display debug messages */
{ int i; 
#define HEXMAX 256
 
 static char hex[HEXMAX*2+1];
 char *p;
 if (n>HEXMAX) n = HEXMAX;

 i = 0;   
 hex[0] = 0;
 p = hex;
 while (n-i>8)
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
  debugs(msg,hex);
}
