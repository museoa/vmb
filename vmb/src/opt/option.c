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


/* this file 
   explains the usage
   contains the defaults
   and processes the commandline
   ( not implemented: reads the secondary configuration file (if any))
 */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "option.h"
#include "error.h"
#ifdef WIN32
#pragma warning(disable : 4996)
#endif

 
/* a string to store teporarily an option */

char tmp_option[MAXTMPOPTION]= {0};
char *programpath = NULL;
char *defined=NULL;

void set_option(char **option, char *str)
/* deallocate *option if necessary, allocate if necesarry, and fill with the the given string */
{ int n;
  n = (int)strlen(str);
  if (*option != NULL && (int)strlen(*option) < n)  
	{ free(*option);
      *option = NULL;
	}
  if (*option == NULL)
  { *option = malloc(n+1);
	if (*option == NULL)
	{ vmb_fatal_error(__LINE__,"Out of Memory for option");
	  return;
	}
  }
  *option = malloc(n+1);
  if (*option == NULL)
  { vmb_fatal_error(__LINE__,"Out of Memory for option");
    return;
  }
  strcpy(*option, str);
}

#define hexdigit(c) ((c)<10? (c)+'0':(c)+'A')

void uint64tohex(uint64_t u, char *c)
/* converts to hex respresetnation. c needs at least 19 characters */
{ int i;
  c[18] = 0;
  for (i=17;i>1;i--)
  {  c[i] = hexdigit((unsigned char)(u&0xF));
     u = u>> 4;
  }
  c[1] = 'x';
  c[0] = '0';
}


uint64_t strtouint64(char *arg)
{ uint64_t r = 0;
  while(isspace(*arg)) arg++;
  if (strncmp(arg,"0x",2)==0 || strncmp(arg,"0X",2)==0) /* hex */
  { arg = arg+2;
	while (isxdigit(*arg))
	{ unsigned int x;
	  if (isdigit(*arg)) x = *arg - '0'; 
	  else if (isupper(*arg)) x = *arg - 'A';
	  else x = *arg -'a';
	  r = (r<<4) + x;
	  arg++;
	}
  }
  else /* decimal */
      while (isdigit(*arg))
	  {unsigned int d;
       d = *arg -'0';
	   r = r*10+d;
	   arg++;
	  }
  return r;
}
static 
int do_option(option_spec *p, char *arg)
{ unsigned int n;
  vmb_debug(0, "processing option:");
  vmb_debug(0, p->longopt);
  switch (p->kind)
  { case str_arg: 
      if (*(p->handler.str)!=NULL)
        free(*(p->handler.str));
      if (arg==NULL)
	  { vmb_error(__LINE__,"Argument expected");
	    return 1;
	  }
	  n = (int)strlen(arg);
	  while (n>0 && isspace(arg[n-1]))
	  { n--; arg[n]=0;}
	  if (n>0)
      { *(p->handler.str) =  malloc(n+1);
	    if (*(p->handler.str)==NULL)
			vmb_error(__LINE__,"Out of memory"); 
		else
	        strcpy(*(p->handler.str), arg);
	  }
	  else
		*(p->handler.str) = NULL;
      return 1;
  case int_arg:
	  if (arg==NULL)
		  vmb_error(__LINE__,"Argument expected");
	  else
          *(p->handler.i)=atoi(arg);
      return 1;
   case uint64_arg:
      if (arg==NULL)
		  vmb_error(__LINE__,"Argument expected");
	  else
          *(p->handler.u)=strtouint64(arg);
      return 1;
    case tgl_arg:
      if (arg==NULL)
	vmb_error(__LINE__,"Argument expected");
      else if (strcmp(arg,"on")==0)
      { *(p->handler.i)=1;
        return 1;
      } else if (strcmp(arg,"off")==0)
      { *(p->handler.i)=0;
        return 1;
      }
      *(p->handler.i)= !(*(p->handler.i));
      return 0;
    case inc_arg:
      (*(p->handler.i))++;
      return 0;
    case on_arg:
      if (arg==NULL)
	 vmb_error(__LINE__,"Argument expected");
      else if (strcmp(arg,"on")==0)
      { *(p->handler.i)=1;
        return 1;
      } else if (strcmp(arg,"off")==0)
      { *(p->handler.i)=0;
        return 1;
      }
      *(p->handler.i)= 1;
      return 0;
    case off_arg:
      if (arg==NULL)
	 vmb_error(__LINE__,"Argument expected");
      else if (strcmp(arg,"on")==0)
      { *(p->handler.i)=1;
        return 1;
      } else if (strcmp(arg,"off")==0)
      { *(p->handler.i)=0;
        return 1;
      }
      *(p->handler.i)= 0;
      return 0;
    case fun_arg:
      vmb_debug(0, "calling handler");
      return (p->handler.f)(arg);
    default:
      /* ignore unknown options */
      return 0;
  }
}

static
int  do_option_long(char *cmd,char *arg)
/* returns 1 if argument was used 0 otherwise */
{  int i;
   static char msg[100];
   vmb_debug(0, "searching for option:");
   vmb_debug(0, cmd);
   i=0;
   while (1)
   { if (options[i].description==NULL)
      { vmb_debug(1, "option ignored:");
        strncpy(msg,cmd,99);
        vmb_debug(1, msg);
        return 0;
      }
      if (cmd[strlen(options[i].longopt)] == '=')
      { if (strncmp(cmd,options[i].longopt,strlen(options[i].longopt)) == 0)
        { do_option(options + i, cmd + strlen(options[i].longopt)+1);
          return 0;
        }
      }
      else if (strcmp(cmd,options[i].longopt)== 0)
        return  do_option(options + i, arg);
      i++;
    }
}

static
int  do_option_short(char cmd,char *arg)
/* returns 1 if argument was used 0 otherwise */
{  int i;
   static char cmdstr[] = "- ";

   i=0;   
   cmdstr[1] = cmd;
   vmb_debug(0, "searching for option:");
   vmb_debug(0, cmdstr);
   
   while (1)
   { if (options[i].description==NULL)
      { vmb_debug(1, "option ignored:");
        vmb_debug(1, cmdstr);
        return 0;
      }
      if (cmd==options[i].shortopt)
        return  do_option(options + i, arg);
      i++;
    }
}

void option_usage(FILE *out)
{ option_spec *p;
  int i;
  
  i=0;
  p= options;
  while(p->description!=NULL)
  { fprintf(out,"-%c  --%s",p->shortopt, p->longopt);
    if (p->arg_name != NULL)
      fprintf(out," <%s>", p->arg_name);
    fprintf(out,"\n\t%s ",p->description);
    if (p->default_str != NULL)
      fprintf(out,"\t(default = %s)",p->default_str);
    fprintf(out,"\n");

    p++;
  }
}

void option_defaults(void)
{ option_spec *p;
  int i;
  
  i=0;
  p= &options[i];
  while(p->description!=NULL)
  { if (p->kind == on_arg)
      *(p->handler.i) = 0;
    else if (p->kind == off_arg)
      *(p->handler.i) = 1;
    else if (p->default_str != NULL)
      do_option(p,p->default_str);
    p = &options[++i];
  }
}

#define MAXLINE 4*1024

int write_configfile(char *filename)
{ FILE *out;
  option_spec *p;

  if (filename==NULL)
  { vmb_error(__LINE__,"Filename expected");
    return 0;
  }
  vmb_debug(0, "writing configfile");
  vmb_debug(0, filename);
  out=fopen(filename,"w");
  if (out==NULL) 
	  {  vmb_error(__LINE__,"Could not write configuration file");
	     return 0;
	  }
  fprintf(out,"#m3w configfile: %s\n\n",filename);
  for (p= options; p->description!=NULL; p++)
  switch (p->kind)
  { case fun_arg:
    case tgl_arg: 
      continue;
    case str_arg:
      fprintf(out,"\n#%s\n",p->description);
      fprintf(out,"%s ", p->longopt);
	  if (*(p->handler.str)!=NULL)
        fprintf(out,"%s",*(p->handler.str));
	  fprintf(out,"\n");
	  continue;
    case on_arg:
      fprintf(out,"\n#%s\n",p->description);
	  if (!(*(p->handler.i))) 
		fprintf(out,"#");
	  fprintf(out,"%s\n", p->longopt);
      continue;
    case off_arg:
      fprintf(out,"\n#%s\n",p->description);
	  if (*(p->handler.i)) 
		fprintf(out,"#");
	  fprintf(out,"%s\n", p->longopt);
      continue;
    case inc_arg:
      fprintf(out,"\n#%s\n",p->description);
	  { int i;
	    if (*(p->handler.i)<=0)
		  fprintf(out,"#%s\n", p->longopt);
		else
	      for (i=0; i<*(p->handler.i); i++) 
    	    fprintf(out,"%s\n", p->longopt);
	  }
      continue;
    default:
      fprintf(out,"\n#%s\n",p->description);
      fprintf(out,"%s %d\n", p->longopt, *(p->handler.i));
      continue;
  }
  fclose(out);
  vmb_debug(0, "done writing configfile");
  return 1;
}

int parse_configfile(char *filename)
{ FILE *in;
  static char line[MAXLINE];
  char *cmd, *arg, *p;    
  int conditional=0;
  int i;

  if(filename==NULL)
  { vmb_error(__LINE__,"Argument expected");
    return 0;
  }
  vmb_debug(0, "reading configfile");
  vmb_debug(0, filename);
  in=fopen(filename,"r");
  if (in==NULL)
	return 0;
  while(!feof(in))
  {  fgets(line,MAXLINE,in);
     if(feof(in)) break;

     p =line; 

     /* trim end of line */
     i = (int)strlen(line)-1;
     while(i>=0 && isspace((int)(p[i])))
     { p[i]=0;
       i--; 
     }
     
     /* skip spaces */
     while(isspace((int)(p[0])))
       p++;

     /* ignore empty lines*/
     if (p[0] == 0) 
       continue;

    /* handle conditionals */
     if (strncmp(p,"#if",3)==0 && isspace(p[3])) 
     { p=p+4;
       while(isspace((int)(p[0]))) p++;
       if ((defined!=NULL && strncmp(p,defined,strlen(defined))==0))
         conditional = 1;
       else
       { while (!feof(in))
         { fgets(line,MAXLINE,in);
           if(feof(in)) break;
           p =line; 
           while(isspace((int)(p[0]))) p++;
           if (strncmp(p,"#endif",6)==0) break;
         }
       }
       continue;
     }

     /* handle end of conditionals */
     if (strncmp(p,"#endif",6)==0)
     { if (conditional ==1) 
         conditional=0;
       else
         vmb_debug(1, "Unmatched #endif"); 
       continue;
     }
  
     

    /* ignore comments */
     if (p[0]=='#') 
       continue;

     /* command found */
     cmd = p; 
     /* convert command to lowercase */
     while(isalpha((int)(*p)))
     { *p = tolower(*p);
       p++;
     }

     /* skip space */
     if (p[0]!=0)
     { p[0]=0; /* terminate command */
       p++;
       while(isspace((int)(p[0])))
         p++;
     }
     arg = p;

	 
     /* here we have cmd and arg pointing to the right places */
     
     do_option_long(cmd,arg);
  }
  fclose(in);
  vmb_debug(0, "done configfile");
  return 1;
}

static 
char *parse_argument(char **str)
/* makes *str point past the argument and returns the argument */
{ char *p, *arg;
  p = *str;
  if (*p == '\0') 
    arg = NULL;
  else
  { /* skip spaces */
    while(isspace((int)(p[0])))
      p++;
    if (*p=='\0' || *p=='-')
      arg = NULL;
    else
    { arg = p;
      if (*arg == '"')
      { arg++; p++;
        while(*p!= 0 && *p != '"')
          p++;
      }
      else
      { while(*p!= 0 && !isspace((int)(*p)))
          p++;
      }
      if (*p != 0)
      {*p = 0;
        p++;
      }
    }
  }
  *str = p;
  return arg;
}


static void do_program(char * arg)
{ int i,n;
  if (arg == NULL)
    return;
  i = 0;
  n = 0;
  while (arg[i]!=0)
  { if (arg[i] == '\\' || arg[i]=='/') 
      n = i+1;
    i++;
  }
  programpath = malloc(n  + 1); /* path + '0' */
  if (programpath==NULL) 
  { vmb_fatal_error(__LINE__,"Out of memory");
    return;
  }
  strncpy(programpath,arg,n);
  programpath[n]=0;
  i = (int)strlen(arg+n) + 1;
  vmb_program_name = malloc(i); /* name + '0' */
  if (vmb_program_name==NULL) 
  { vmb_fatal_error(__LINE__,"Out of memory");
    return;
  }
  strcpy(vmb_program_name,arg+n);
  defined = malloc(i); /* name + '0' */
  if (defined==NULL) 
  { vmb_fatal_error(__LINE__,"Out of memory");
    return;
  }
  strcpy(defined,vmb_program_name);
#ifdef WIN32
  if (strncmp(defined+i-5,".exe",4)== 0 || strncmp(defined+i-5,".EXE",4)== 0 )
	  defined[i-5]=0;
#endif 
}

static int do_define(char *arg)
{ 
 if (arg==NULL || arg[0]=='-')
    return 0;
 set_option(&defined,arg);
 return 1;
}

static void do_configfile(void)
{   char *configfile;
    int n;
    n = (int)strlen(programpath);
    configfile = malloc(n  + 11 + 1); /* arg + default.mmc  + '0'*/
    if (configfile==NULL) 
    { vmb_fatal_error(__LINE__,"Out of memory");
      return;
    }
    strcpy(configfile,programpath);
    strcat(configfile,"default.mmc");
    parse_configfile(configfile); /* global configfile first */
    parse_configfile("default.mmc"); /* next local configfile */
    free(configfile);
}

void parse_commandstr(char *p)
{ int arguments;
  char *cmd, *arg;    
  vmb_debug(0, "reading commandstr");
  do_program(parse_argument(&p));
  arguments=do_define(parse_argument(&p));
  arguments++;
  do_configfile();
  while(*p != 0)
  {  while(isspace((int)(p[0]))) p++; /* skip spaces */
     if (p[0] == 0) /* done? */
       break;
     if (p[0] == '-')
     { p++;
       if (*p == 0 || isspace((int)(*p))) /* single - */
         do_argument(arguments++, "-");
       else if (*p == '-') /* double -- */
       { p++;
         if (*p == 0 || isspace((int)(*p))) /* single -- */
           do_argument(arguments++, "--");
         else  /* long command found */
         { cmd = p; 
           /* convert command to lowercase */
           while(isalpha((int)(*p)))
           { *p = tolower(*p);
             p++;
           }
           if (p[0]!=0)
           { p[0]=0; /* terminate command */
              p++;
           }
           arg = parse_argument(&p);
           if (do_option_long(cmd,arg)==0 && arg != NULL)
             do_argument(arguments++, arg);
         }
       }
       else 
       { char c;
         while(*p!=0 && !isspace((int)(*p)))
         { c = *p;
           p++;
           if (*p == 0)
 	       do_option_short(c, NULL);
           else if (isspace((int)(*p)))
           { arg = parse_argument(&p);
             if (do_option_short(c, arg)==0 && arg != NULL)
               do_argument(arguments++, arg);
             break;
           }
           else
 	       do_option_short(c, NULL);
         }
       }
     }
     else
       do_argument(arguments++, parse_argument(&p));
  }
  vmb_debug(0, "done commandstr");
}

void parse_commandline(int argc, char *argv[])
{ int i,j;

  vmb_debug(0, "parsing commandline");
  do_program(argv[0]);
  i=do_define(argv[1]);
  do_configfile();
  while (++i < argc)
  { if (argv[i][0] == '-' && argv[i][1] != 0)
    {  if (argv[i][1] == '-' && argv[i][2] != 0)
	 i = i+ do_option_long(argv[i]+2, argv[i+1]);
       else
       { for (j=1;argv[i][j]!=0 && argv[i][j+1]!=0;j++)
	   do_option_short(argv[i][j], NULL);
	 i = i+ do_option_short(argv[i][j], argv[i+1]);
       }
    }
    else
      do_argument(i, argv[i]);

  }
  vmb_debug(0, "done commandline");
}

