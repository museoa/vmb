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

/* specialized version of param.c for mmixide */

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <ctype.h>


#include <windows.h>
#pragma warning(disable : 4996)
#include "winopt.h"
#include "vmb.h"
#include "param.h"
#include "option.h"
#include "bus-arith.h"
#include "info.h"
#include "editor.h"
#include "winmain.h"


char *programhelpfile = NULL;
char *host=NULL;
int port = 9002;
int xpos=0, ypos=0; /* Window position */
int minimized = 0;  /* start the window minimized */


uint64_t vmb_address;
unsigned int vmb_size;




extern option_spec options[];
extern HWND hMainWnd;

void usage(char *message)
{
 if (programhelpfile==NULL)
    WinHelp(hMainWnd,(LPCTSTR)"mmixide.hlp",HELP_CONTENTS, 0L);
  else
    WinHelp(hMainWnd,programhelpfile,HELP_CONTENTS, 0L);
}


void do_argument(int pos, char * arg)
{ int file_no=filename2file(arg,0);
  if (file_no<0) return;
  if (edit_file_no>=0) return;
  set_edit_file(file_no);
  set_application(file_no);
}

int do_define(char *arg)
{ 
 set_option(&defined,"mmixide");
 return 0;
}


int mk_argv(char *argv[MAXARG],char *command, int unquote)
/* splits command into arguments, knows how to handle strings.
   if unquote is true, double-quote characters are removed
   before putting them im the vector
   returns argc.
*/

{ int argc=0;  

  if (command==NULL||*command==0)
  { argv[0]=NULL;
    return argc;
  }
  for (argc=0;argc<MAXARG-1;argc++)
  { 
    while (isspace(*command)) command++;

    if (*command==0)
    { argv[argc]=NULL;
        return argc;
    }

	if( *command=='"')
	{ if (unquote) argv[argc]=++command;
	  else argv[argc]=command++;
      while (*command!='"' && *command!=0) command++;
	  if (*command=='"' && !unquote) command++;
	}
	else
	{ argv[argc]=command;
      while (!isspace(*command) && *command!=0) command++;
	}
    if (*command!=0)
    { *command=0;
      command++;
    }
  }
  argc=MAXARG-1;
  argv[argc]=NULL;
  return argc;
}



void param_init(void)
{ int argc;
  char *argv[MAXARG];
  option_defaults();
  argc=mk_argv(argv,GetCommandLine(),TRUE);
  parse_commandline(argc, argv);
}
