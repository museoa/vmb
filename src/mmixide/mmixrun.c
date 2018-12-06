#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <setjmp.h>
#include "resource.h"
#include "splitter.h"
#ifdef VMB
#include "vmb.h"
#include "error.h"
#endif
#include "winopt.h"
#include "winmain.h"
#include "mmixlib.h"

#ifdef VMB
#include "mmix-bus.h"
#endif
#include "info.h"
#include "symtab.h"
#include "debug.h"
#include "assembler.h"
#include "winlog.h"
#include "editor.h"
//#include "option.h"
#include "runoptions.h"
#include "breakpoints.h"
#include "mmixrun.h"
#ifdef VMB
#include "address.h"
#endif

extern bool interact_after_break;

typedef enum { dbg_cont, dbg_step, dbg_over, dbg_over_os, dbg_out, dbg_break, dbg_quit} dbg_type;

dbg_type dbg_mode=dbg_cont;

/* Running MMIX */
// maximum mumber of lines the output console should have

#pragma warning(disable : 4996)

#ifdef VMB
device_info vmb;
#endif

extern void vmb_atexit(void);
void mmix_run_init(void);

DWORD dwMMIXThreadId=0;

static HANDLE hInteract=NULL;

/* called by the mmix thread */
static int mmix_waiting = 0;

void mmix_interact(void)
{ DWORD w;
  if (interacting)
  { mmix_stopped(loc);
    mmix_waiting = 1;
    PostMessage(hMainWnd,WM_MMIX_INTERACT,0,0); 
    w = WaitForSingleObject(hInteract,INFINITE);
    mmix_waiting = 0;
  }
#ifdef VMB
  if (vmb_get_interrupt(&vmb,&new_Q.h,&new_Q.l)==1)
  { g[rQ].h |= new_Q.h; g[rQ].l |= new_Q.l; }
#endif
}

void mmix_stop(void)
{ interrupt=true;
  if (!interacting) halted=true;
}


void mmix_continue(unsigned char command)
{ 
   if (hInteract==NULL) return;
   rOlimit=neg_one;
   switch (command)
   { case 'q': /* quit */
 /*      halted=true; */
	   dbg_mode=dbg_quit;
       break;
     case 'c': /* continue */
	   mmix_status(MMIX_RUNNING);
	   dbg_mode=dbg_cont;
       break;
	 case 'n': /* next instruction/ step over*/
	  dbg_mode=dbg_over;
/*	   rOlimit=g[rO];
       breakpoint|=exec_bit, interacting=true;  trace one inst and break */
       break;
	 case 'o': /* step out */
	   dbg_mode=dbg_out;
/*         rOlimit=incr(g[rO],-1);
       breakpoint|=exec_bit, interacting=true;  trace one inst and break */
       break;
	 case 's': /* step instruction */
	   dbg_mode=dbg_step;
	   break;
     default: 
	   dbg_mode=dbg_step;	 	 
/*		 breakpoint|=exec_bit, interacting=true;  trace one inst and break */
       break;
   }
   show_stop_marker(edit_file_no,-1); /* clear stop marker */
   if (mmix_waiting)
     SetEvent (hInteract);
}


extern jmp_buf mmix_exit;


#define sign_bit ((unsigned)0x80000000)

#ifdef VMB

static void mmix_exit_hook(int returncode)
{ longjmp(mmix_exit, returncode);
}
#endif


static DWORD WINAPI MMIXThreadProc(LPVOID dummy)
{ int returncode;
  int argc;
  static char *argv[MAXARG];
#define MAXCOMMAND 256
  static char command[MAXCOMMAND];
  if (hInteract==NULL)
    hInteract =CreateEvent(NULL,FALSE,FALSE,NULL);

  if (run_args!=NULL && run_args[0]!=0)
  { strncpy(command,run_args,MAXCOMMAND);
	argc=mk_argv(argv,command, TRUE);
  }
  else
  { argc=0;
    argv[0]=NULL;
  }

#ifdef VMB 
  vmb_exit_hook = mmix_exit_hook;
#endif
  returncode = mmix_main(argc,argv,NULL);
#ifdef VMB
  vmb_atexit();
  vmb_exit_hook = ide_exit_ignore;
#endif
  PostMessage(hMainWnd,WM_MMIX_STOPPED,0,(LPARAM)-1); 
 
  if (hInteract!=NULL)
   { CloseHandle(hInteract); 
     hInteract=NULL;
   }

  dwMMIXThreadId=0;
  return returncode;
}


int mmix_active(void)
/* returns 1 if the MMIXThread is running 0 otherwise */
{ 
	return dwMMIXThreadId!=0;
}

static void MMIXThread(void)
{ HANDLE hMMIXThread=NULL;
  if (dwMMIXThreadId!=0) return;
  mmix_status(MMIX_RUNNING);
  hMMIXThread = CreateThread(
			NULL,              // default security attributes
            0,                 // use default stack size  
            MMIXThreadProc,        // thread function 
            NULL,             // argument to thread function 
            0,                 // use default creation flags 
            &dwMMIXThreadId);   // returns the thread identifier 
  CloseHandle(hMMIXThread);
}

char full_mmo_name[MAX_PATH+1]={0};

char *get_mmo_name(char *full_mms_name)
{  if (full_mms_name==NULL || full_mms_name[0]==0) return NULL;
   strncpy(full_mmo_name,full_mms_name,MAX_PATH-4);
   full_mmo_name[strlen(full_mmo_name)-1]='o';
   return full_mmo_name;
}

static void init_fake_stdin(void)
{ if (fake_stdin) fclose(fake_stdin);
  mmix_fake_stdin(stdin);
  if (stdin_file!=NULL && stdin_file[0]!=0)
  { fake_stdin=fopen(stdin_file,"r");
    if (!fake_stdin) win32_ferror(__LINE__,"Unable to open file (%s)\r\n",stdin_file);
    else mmix_fake_stdin(fake_stdin);
  }
}

void mmix_run(void)
{		  interacting=false;
#ifdef VMB
		  show_operating_system=false;
#endif
          breakpoint=0;
		  tracing=0;
		  update_symtab();
		  clear_profile();
		  init_fake_stdin();
          MMIXThread();
}


void mmix_reset(void)
{ 	PostMessage(hMainWnd,WM_MMIX_RESET,0,0);
}


void mmix_debug(void)
{		  interacting=true;
          breakpoint=0;
//		  vmb.reset=mmix_reset; currently no need for this.
		  update_symtab();
		  clear_profile();
		  init_fake_stdin();
		  MMIXThread();
}

void show_trace_window(void)
{ if (hLog!=NULL) return;
  sp_create_options(0,0,0.2,0,NULL);
  hLog=CreateLog(hSplitter,hInst);
  CheckMenuItem(hMenu,ID_VIEW_TRACE,MF_BYCOMMAND|MF_CHECKED);
}

char * mmix_status_str[]={"Disconnected", "Connected","Off", "On", "Stopped", "Running", "Halted"};
int mmix_current_status=MMIX_OFF;

void mmix_status(int status)
{ mmix_current_status=status;
  ide_status(mmix_status_str[status]);
}

static void mmix_stopped(octa loc)
{ mem_tetra *ll;
  ll=mem_find(loc);
  PostMessage(hMainWnd,WM_MMIX_STOPPED,0,item_data(ll->file_no,ll->line_no));
}

/* this is the plain vmb version of mmix main() */

static int rO_below_limit(void)
/*used to disable tracing and interaction inside a subroutine. 
  returns true if g[rO]<=rOlimit */
{ return (g[rO].h==rOlimit.h && g[rO].l<=rOlimit.l) || g[rO].h<rOlimit.h;
}

/* checking for interaction is done at two places before and after
   performing (traceing and resumeing) the instruction.
   The parameter after is false when this function is called before the instruction
   and true if this function is called after the instruction.
   Interaction after performing the instruction
   is the "classic" MMIX way of interaction.
   Interaction before executing the instruction is the typical way other debuggers
   do it (you can inspect the register before doing the instruction and after).
   The parameter break_after is true in case the user selects the classical behaviour
   and it is false if the user prefers to interact before the instruction.

*/
#ifdef VMB
#define TRACEABLE ((!(loc.h&sign_bit)||show_operating_system) && rO_below_limit())
#else
#define TRACEABLE (rO_below_limit())
#endif

static bool is_traceable(octa loc)
{ return !(loc.h&sign_bit)||show_operating_system;
}

#ifdef VMB
#define AFTERTRACEABLE ((!(inst_ptr.h&sign_bit)||show_operating_system) && rO_below_limit())
#else
#define AFTERTRACEABLE (rO_below_limit())
#endif
static bool trace_once=false; /* do we trace the next traceable instruction */


static void mmix_load(int file_no)
{ if (!file2loading(file_no)) return;
  mmix_load_file(get_mmo_name(file2fullname(file_no)));
}

#ifndef VMB
void mmix_zero_memory(mem_node *p)
{ if (p->left) mmix_zero_memory(p->left);
  if (p->right) mmix_zero_memory(p->right);
  memset(p->dat,0,sizeof(p->dat));
}
#endif


int mmix_main(int argc, char *argv[],char *mmo_name)
{
  g[255].h=0;
  g[255].l=setjmp(mmix_exit);
  if (g[255].l!=0)
   goto end_simulation;
#ifdef VMB 
  if (!vmb.connected) {win32_message("Not connected"); return 0;}
  if (vmb.power) vmb_raise_reset(&vmb);
#endif
  mmix_initialize();
boot:
#ifdef VMB
  vmb.reset_flag=0;
  win32_log("Power...");
  while (!vmb.power)
  {  vmb_wait_for_power(&vmb);
     if (!vmb.connected){win32_message("Power but not connected"); return 0;}
  }
  win32_log("ON\n");
  Sleep(50); /* give all devices some time to power up before loading the application */
#else
  mmix_zero_memory(mem_root);
#endif
  mmix_boot(); 
  rOlimit=neg_one; /*reset rOlimit */
  for_all_files(mmix_load);
  sync_breakpoints();
  show_breakpoints();
  { 
#ifdef VMB	  
	octa pool;
    pool.h= 0x60000000,pool.l= 0x00;
    if (valid_address(pool))
#endif
      mmix_commandline(argc, argv);
  }
  Sleep(50); /* give all devices some time finish loading the files and commandline */
  new_Q.h=new_Q.l=g[rQ].h=g[rQ].l=0; /* remove error flags */
  trace_once=true;
#ifdef VMB
  if (interacting) 
  { 
	if ((loc.h&sign_bit)&& !show_operating_system) 
		dbg_mode=dbg_over_os;
	else 
		dbg_mode=dbg_step;
  }
  else dbg_mode=dbg_cont;

dbg_mode=dbg_step;

#else
  dbg_mode=dbg_step;
  goto interact;
#endif
  while (
#ifdef VMB
	  vmb.connected && 
#endif
	  !halted) {
	bool fetch_traced=false;
	octa trap_loc;

    breakpoint=0;
	mmix_fetch_instruction();
#ifndef VMB
interact:
#endif

    /* decide on tracing and interacting based on dbg mode*/
	switch (dbg_mode)
	{ 
	  case dbg_quit:	
		halted=true; 
		goto end_simulation; 
	  case dbg_over:
		  if ((inst>>24)==TRAP)
		  {  dbg_mode=dbg_over_os;
		     trap_loc=loc;
             tracing= true; interact_after_break=!break_after;
		  }
		  else
			  goto case_step; 
		  break;
	  case dbg_over_os:	
	    if (loc.h&sign_bit) 
			tracing=interact_after_break=false;
		else
			goto case_step; 
		break;

	  case_step: dbg_mode=dbg_step;
	  case dbg_step:
	    tracing=interact_after_break=true;
	    break;

	  case dbg_cont:	
	  default:
        tracing=interact_after_break=false;
		break;
	}

 	if (breakpoint&exec_bit)  
		tracing=interact_after_break=true;
	if (breakpoint & trace_bit) 
		tracing=true;
	if (interrupt) 
		interacting=tracing=interact_after_break=true, interrupt=false;

	if (tracing) 
	{ mmix_trace_fetch(); fetch_traced=true; Sleep(10);  /* to give the ide time to update the screen while debugging */
     }
 
	if (interact_after_break  && !break_after)
	{   mmix_interact(); 
	    interact_after_break=false; 
	}

resume:

    mmix_perform_instruction(); 
	switch (dbg_mode)
	{ case dbg_over_os:	
	    if ((inst>>24)==RESUME)
		{ fetch_traced=tracing=true, interact_after_break=break_after;
		  loc=trap_loc;
		}
		else 
		  tracing=interact_after_break=false;
        break;
	  default:
		break;
	}
 	if (breakpoint&(read_bit|write_bit))  
		tracing=interact_after_break=true;

	if (tracing)
	{ if (!fetch_traced) 
	  { mmix_trace_fetch(); fetch_traced=true; }
      mmix_trace_perform(); Sleep(10);  /* to give the ide time to update the screen while debugging */
	  if (showing_stats) show_stats(breakpoint);
	}


    if (interact_after_break)
	{  mmix_interact();
	   interact_after_break=false;
	}

	/* mmix_trace(); */
#ifdef VMB
	mmix_dynamic_trap();
#endif
    if (resuming)
	{ if (op==RESUME) /* this is the case if $255 contains the next instruction */
	  { fetch_traced=false;
	    goto resume;
	  }
	  else	/* plain resume without insering an instruction */
	    resuming=false;
  	if (dbg_mode==dbg_over_os && (loc.h&sign_bit)==0)
          interact_after_break=true;

	}
  
	if (
#ifdef VMB
		!vmb.power || vmb.reset_flag ||
#endif
		(g[rQ].l&g[rK].l&RE_BIT))
    { breakpoint|=trace_bit; 
      goto boot;
    }
  }
  end_simulation:
  win32_log("Terminated\n");
  if (interacting || profiling || showing_stats) show_stats(false);
  mmix_finalize();
  return g[255].l;
}


static char logstr[512];

int mmix_vprintf(char *format, va_list vargs)
{   char logstr[512];
  int n; 
  n = vsprintf(logstr,format, vargs);
  win32_log(logstr);
  return n;
}
int mmix_printf(FILE *f, char *format, ...)
{ va_list vargs;
  int n; 
  va_start(vargs,format);
  if (f==stdout||f==stderr)  
    n = mmix_vprintf(format,vargs);
  else
	n=vfprintf(f,format,vargs);

  return n;
}


int mmix_fputc(int c, FILE *f)
{  if (f==stdout||f==stderr)  
   { logstr[0]=c;
     logstr[1]=0;
     win32_log(logstr);
     return 1;
   }
   else
	 return fputc(c,f);
}

