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
#include "win32main.h"
#else
#include <unistd.h>
#endif
#include "param.h"
#include "option.h"
#include "message.h"
#include "bus-arith.h"
#include "error.h"
#include "main.h"

#ifdef WIN32
char *programhelpfile = NULL;
#endif

char *host=NULL;
char *filename=NULL;
int port = 9002;
int interrupt = 16;
char *hexaddress=NULL;
unsigned char address[8] = {0};
unsigned char limit[8] = {0};
unsigned int size;
int debugflag = 0;
char *commands[MAX_EXEC]={0};
#define MAXARG 256


static void usage(char *message)
{
#ifdef WIN32
 if (programhelpfile==NULL)
    WinHelp(hMainWnd,"mmixmb.hlp",HELP_CONTENTS, 0L);
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
  hextochar(hexaddress,address,8);
}

static void store_command(char *command)
{ int i;
  debugs("storing command %s",command);
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
  errormsg("Too many commands");
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
  errormsg("Too many arguments in command");
  return 0;
}

void do_commands(void)
{ int i;
  for (i=0; i<MAX_EXEC ;i++)
    if (commands[i]!=NULL)
      { 
        char *argv[MAXARG] = {0};
        debugs("executing command %s",commands[i]);
        if (!mk_argv(argv,commands[i]))
          continue;
#ifdef WIN32
		{ int p;
		  p = spawnvp(_P_NOWAIT,argv[0],argv);
		  if (p<0)
		  {  debugi("could not start %d",errno);
	          errormsg("Unable to execute command");
		  }
		}
#else
        { pid_t p;
          p = fork();
          if (p<0) errormsg("Unable to create new process");
          else if (p==0) /* child */
		  { execvp(argv[0],argv);
	        errormsg("Unable to execute command");
		  }
		}
#endif
	}
    else
      return;
}


option_spec options[] = {
/* description short long kind default handler */
{"the host where the bus is located", 'h',   "host",    "host",          str_arg, "localhost", {&host}},
{"the port where the bus is located",   'p', "port",    "port",          int_arg, "9002", {&port}},
{"to generate debug output",            'd', "debug",   "debugflag",     on_arg, NULL, {&debugflag}},
{"to define a name for conditionals",   'D', "define",  "conditional",   str_arg, NULL, {&defined}},
{"address whre the resource is located",'a', "address", "hex address",   str_arg, "8000000000000000", {&hexaddress}},
{"size of address range in octas",      's', "size",    "size in octas", int_arg, "1", {&size}},
{"interrupt send by device",            'i', "interrupt", "interrupt number", int_arg, "8", {&interrupt}},
{"filename for input file",             'f', "file",    "file name",     str_arg, NULL, {&filename}},
{"command to execute",                   'x', "exec",    "command line",     fun_arg, NULL, {store_command}},

{"filename for a configuration file",    'c', "config", "file",          fun_arg, NULL, {parse_configfile}},
{"to print usage information",           '?', "help",   NULL,            fun_arg, NULL,{usage}},
{NULL}
};


void do_argument(int pos, char * arg)
{ 
  debug("too many arguments"); 
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
  hextochar(hexaddress,address,8);
}
