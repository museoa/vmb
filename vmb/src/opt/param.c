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

#ifdef WIN32
#include <windows.h>
#include <process.h>
#pragma warning(disable : 4996)
#else
#include <unistd.h>
#endif

#include "param.h"
#include "option.h"
#include "bus-arith.h"
#include "error.h"

#ifdef WIN32
char *programhelpfile = NULL;
#endif

char *host=NULL;
char *filename=NULL;
int port = 9002;
int interrupt = 16;

#ifdef WIN32
#include <windows.h>
extern UINT64 vmb_address;
#else
#include <stdint.h>
extern uint64_t vmb_address;
#endif

uint64_t vmb_address;
unsigned int vmb_size;
char *commands[MAX_EXEC]={0};
#define MAXARG 256

void store_command(char *command)
{ int i;
  vmb_debugs("storing command %s",command);
  for (i=0; i<MAX_EXEC ;i++)
    if (commands[i]!=NULL)
    {  if (strcmp(commands[i],command)==0) 
         return;
       else
         continue;
    }
    else
      { set_option(&commands[i],command);
        return;
      }
  vmb_errormsg("Too many commands");
}



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

void load_configfile(char *name)
{ parse_configfile(name);
}


static int mk_argv(char *argv[MAXARG],char *command)
{ int argc;  
  char *str;
  for (argc=0;argc<MAXARG;argc++)
  { argv[argc]=command;
    if (command==NULL) return 1;
    str=strchr(command,' ');
    if (str==NULL)
      command = NULL;
    else
    { str[0]=0;
      command = str+1;
    }
  }
  vmb_errormsg("Too many arguments in command");
  return 0;
}

void do_commands(void)
{ int i;
  for (i=0; i<MAX_EXEC ;i++)
    if (commands[i]!=NULL)
      { 
        char *argv[MAXARG] = {0};
        vmb_debugs("executing command %s",commands[i]);
        if (!mk_argv(argv,commands[i]))
          continue;
#ifdef WIN32
		{ intptr_t p;
		  p = spawnvp(_P_NOWAIT,argv[0],argv);
		  if (p<0)
		  {  vmb_debugi("could not start %d",errno);
	          vmb_errormsg("Unable to execute command");
		  }
		}
#else
        { pid_t p;
          p = fork();
          if (p<0) vmb_errormsg("Unable to create new process");
          else if (p==0) /* child */
		  { execvp(argv[0],argv);
	        vmb_errormsg("Unable to execute command");
		  }
		}
#endif
	}
    else
      return;
}



void do_argument(int pos, char * arg)
{ 
  vmb_debug("too many arguments"); 
}


#ifdef WIN32
void param_init(void)
#else
void param_init(int argc, char *argv[])
#endif
{ option_defaults();
#ifdef WIN32
  parse_commandstr(GetCommandLine());
#else
  parse_commandline(argc, argv);
#endif
}
