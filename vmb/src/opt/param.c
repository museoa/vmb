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

#include <sys/types.h>
#include <ctype.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable : 4996)
#include "winopt.h"
#else
#include <unistd.h>
#include <stdint.h>
#endif
#include "vmb.h"
#include "param.h"
#include "option.h"
#include "bus-arith.h"


#ifdef WIN32
char *programhelpfile = NULL;
#endif

char *host=NULL;
char *filename=NULL;
int port = 9002;
int interrupt = 16;
int disable_interrupt = 0;
int xpos=0, ypos=0; /* Window position */
int minimized = 0;  /* start the window minimized */


uint64_t vmb_address;
unsigned int vmb_size;




extern option_spec options[];

#ifdef WIN32
extern HWND hMainWnd;
#endif

void usage(char *message)
{
#ifdef WIN32
 if (programhelpfile==NULL)
    WinHelp(hMainWnd,(LPCTSTR)"mmixmb.hlp",HELP_CONTENTS, 0L);
  else
    WinHelp(hMainWnd,programhelpfile,HELP_CONTENTS, 0L);
#else
  printf("Version %s\n", version);
  printf("%s\n",howto);
  option_usage(stdout);
  printf("\n");
#endif
}


void do_argument(int pos, char * arg)
{ 
  vmb_debug(VMB_DEBUG_ERROR, "too many arguments"); 
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


#ifdef WIN32
void param_init(void)
{ int argc;
  char *argv[MAXARG];
#else
void param_init(int argc, char *argv[])
{
#endif 
  option_defaults();
#ifdef WIN32
//    MessageBox(hMainWnd,GetCommandLine(),"Commandline",MB_OK);
    argc=mk_argv(argv,GetCommandLine(),TRUE);
//  argv = CommandLineToArgvW( GetCommandLine(),&argc);
//  parse_commandstr(GetCommandLine());
    parse_commandline(argc, argv);
  if (vmb_verbose_flag) vmb_debug_mask=0; 
  CheckMenuItem(hMenu,ID_VERBOSE,MF_BYCOMMAND|(vmb_debug_mask==0?MF_CHECKED:MF_UNCHECKED));
#else
  parse_commandline(argc, argv);
  if (vmb_verbose_flag) vmb_debug_mask=0; 
#endif
}
