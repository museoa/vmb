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
#include "address.h"

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

int mmix_interact(void)
{ DWORD w;
  breakpoint=0;
  mmix_waiting = 1;
  PostMessage(hMainWnd,WM_MMIX_INTERACT,0,0); 
  w = WaitForSingleObject(hInteract,INFINITE);
  mmix_waiting = 0;
#ifdef VMB
  if (vmb_get_interrupt(&vmb,&new_Q.h,&new_Q.l)==1)
  { g[rQ].h |= new_Q.h; g[rQ].l |= new_Q.l; }
#endif
  return 1;
}

void mmix_stop(void)
{ interrupt=true;
  if (!interacting) halted=true;
}


int mmix_continue(unsigned char command)
{ 
   if (hInteract==NULL) return 0;
   rOlimit=neg_one;
   switch (command)
   { case 'q': /* quit */
       breakpoint|=trace_bit, halted=true;
       break;
     case 'c': /* continue */
	   mmix_status(MMIX_RUNNING);
       break;
	 case 'n': /* next instruction/ step over*/
	   rOlimit=g[rO];
       breakpoint|=trace_bit, interacting=tracing=true; /* trace one inst and break */
       break;
	 case 'o': /* step out */
         rOlimit=incr(g[rO],-1);
       breakpoint|=trace_bit, interacting=tracing=true; /* trace one inst and break */
       break;
	 case 's': /* step instruction */
     default: breakpoint|=trace_bit, interacting=tracing=true; /* trace one inst and break */
       break;
   }
   show_stop_marker(edit_file_no,-1); /* clear stop marker */
   if (mmix_waiting)
     SetEvent (hInteract);
   return 1;
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
		  tracing=false;
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

void mmix_stopped(octa loc)
{ mem_tetra *ll;
  ll=mem_find(loc);
  PostMessage(hMainWnd,WM_MMIX_STOPPED,0,item_data(ll->file_no,ll->line_no));

}

/* this is the plain vmb version of mmix main() */

static int check_rO(void)
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
#define OUTSIDE ((!(loc.h&sign_bit)||show_operating_system) && check_rO())
#else
#define OUTSIDE (check_rO())
#endif

static int check_interact_before(void)
{ 	if (!interacting) { breakpoint=0; return 1; }
    if (break_after|| (breakpoint &(read_bit|write_bit)))
	{ breakpoint &=~(read_bit|write_bit);
	  if (OUTSIDE)
	  {  mmix_stopped(loc); /* display the last stop marker */
	     trace_once=1;
	  }
	}
	else
	{ if (!OUTSIDE) return 1;
	     trace_once=1;
      /* here we still trace the pop instead of the push (or both) with step over */
	  mmix_stopped(loc);
	  return mmix_interact();
	}
    return 1;
}

static int check_interact_after(void)
{ 	if (!interacting) { breakpoint=0; return 1; }
    if (break_after || (breakpoint &(read_bit|write_bit)))
	{ breakpoint &=~(read_bit|write_bit);
	  if (!OUTSIDE) /* no stop inside function */
		  return 1;
#ifdef VMB
	  if ((rOlimit.l&1)&&(!(loc.h&sign_bit)||show_operating_system)) 
#else
	  if (rOlimit.l&1) 
#endif
		  /* this is the case for step and step out but not for step over */
	      mmix_stopped(loc); 
	  return mmix_interact();
	}
    return 1;
}







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
{ g[255].h=0;
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
#ifdef VMB
  if (interacting && (!(loc.h&sign_bit)||show_operating_system))
  { mmix_stopped(loc); breakpoint=trace_bit; }
#else
  goto interact;
#endif
  while (
#ifdef VMB
	  vmb.connected && 
#endif
	  !halted) {
	mmix_fetch_instruction();
#ifndef VMB
interact:
#endif
	if (interrupt) breakpoint|=trace_bit, interacting=true, interrupt=false;
    if (breakpoint && !check_interact_before()) goto end_simulation;
resume:
    mmix_perform_instruction(); 
	mmix_trace();
#ifdef VMB
	mmix_dynamic_trap();
#endif
    if (resuming)
	{ if (op==RESUME)
	    goto resume;
	  else
	    resuming=false;
	}
	if (breakpoint && !check_interact_after()) goto end_simulation;
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

