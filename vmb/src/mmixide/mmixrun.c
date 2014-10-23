#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <setjmp.h>
#include "splitter.h"
#ifdef VMB
#include "vmb.h"
#include "error.h"
#include "winopt.h"
#endif
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
#include "mmixrun.h"
/* Running MMIX */
// maximum mumber of lines the output console should have

#pragma warning(disable : 4996)

extern void vmb_atexit(void);
void mmix_run_init(void);

DWORD dwMMIXThreadId=0;

static HANDLE hInteract=NULL;

/* called by the mmix thread */
static int mmix_waiting = 0;

int mmix_interact(void)
{ DWORD w;
  breakpoint=false;
  mmix_waiting = 1;
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
  //show_operating_system=true;
  if (!interacting) halted=true;
}


int mmix_continue(unsigned char command)
{ 
   if (hInteract==NULL) return 0;
   rOlimit=neg_one;
   switch (command)
   { case 'q': /* quit */
       breakpoint=halted=true;
       break;
     case 'c': /* continue */
	   mmix_status(MMIX_RUNNING);
       break;
	 case 'n': /* next instruction/ step over*/
	   rOlimit=g[rO];
       interacting=breakpoint=tracing=true; /* trace one inst and break */
       break;
	 case 'o': /* step out */
         rOlimit=incr(g[rO],-1);
       interacting=breakpoint=tracing=true; /* trace one inst and break */
       break;
	 case 's': /* step instruction */
     default: interacting=breakpoint=tracing=true; /* trace one inst and break */
       break;
   }
   show_stop_marker(edit_file_no,-1); /* clear stop marker */
   if (mmix_waiting)
     SetEvent (hInteract);
   return 1;
}


extern jmp_buf mmix_exit;


#define sign_bit ((unsigned)0x80000000)



void mmix_exit_hook(int returncode)
{ longjmp(mmix_exit, returncode);
}



static DWORD WINAPI MMIXThreadProc(LPVOID dummy)
{ int returncode;

  if (hInteract==NULL)
    hInteract =CreateEvent(NULL,FALSE,FALSE,NULL);

#ifdef VMB 
  vmb_exit_hook = mmix_exit_hook;
#endif
  returncode = mmix_main(0,NULL,NULL);
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

void mmix_run(void)
{		  interacting=false;
		  show_operating_system=false;
          breakpoint=false;
		  tracing=false;
		  tracing_exceptions=0;
          stack_tracing=false;

		  update_symtab();
          MMIXThread();
}

int break_at_symbol(int file_no,char *symbol)
{ sym_node *sym=find_symbol(symbol,file_no);
  if (sym!=NULL&& sym->link==DEFINED)
  { loc2bkpt(sym->equiv)|=exec_bit;
	ide_mark_breakpoint(sym->file_no,sym->line_no);
	return 1;
  }
  else
    return 0;
}

void mmix_reset(void)
{ 	PostMessage(hMainWnd,WM_MMIX_RESET,0,0);
}


void mmix_debug(void)
{		  interacting=true;
          breakpoint=true;
		  tracing_exceptions=0x0;
		  stack_tracing=false;
//		  vmb.reset=mmix_reset; currently no need for this.
		  update_symtab();
		  MMIXThread();
}

void show_trace_window(void)
{ if (hLog!=NULL) return;
  sp_create_options(0,0,0.2,0,NULL);
  hLog=CreateLog(hSplitter,hInst);
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
/* returns true if g[rO]<=rOlimit */
{ return (g[rO].h==rOlimit.h && g[rO].l<=rOlimit.l) || g[rO].h<rOlimit.h;
}

static int check_interact(bool after)
{   if (!interrupt && !breakpoint) return 1;
	if (interrupt && !breakpoint) breakpoint=interacting=true, interrupt=false;
	if (!interacting) { breakpoint=false; return 1; }
#ifdef VMB
	if (!after && loc.h==0x80000000 && loc.l==0)  /* boot */
	    return mmix_interact();
#endif
    if (break_after||rw_break)
	{ rw_break=false;
	  if (!after && (!(loc.h&sign_bit)||show_operating_system)&&check_rO())
	  {  mmix_stopped(loc); /* display the last stop marker */
	     trace_once=1;
	  }
	  if (after)
	  { if ((inst_ptr.h&sign_bit) && !show_operating_system) /* no stop in the operating system */  
		  return 1;
	    if (!check_rO()) /* no stop inside function */
		  return 1;
		if ((rOlimit.l&1)&&(!(loc.h&sign_bit)||show_operating_system)) /* this is the case for step and step out but not for step over */
	      mmix_stopped(loc); 
	    return mmix_interact();
      }
	}
	else
	{ if (!after && (!(loc.h&sign_bit)||show_operating_system)&&check_rO())
	  {  mmix_stopped(loc); /* display the last stop marker */
	     trace_once=1;
	  }
	  if (!after)
	  { if (loc.h&sign_bit && 
		  !show_operating_system &&
          !(loc.h==0x80000000 && loc.l==0)) 
		    return 1;
	    if (!check_rO()) 
			return 1; /* here we still trace the pop instead of the push (or both) with step over */
	    mmix_stopped(loc);
	    return mmix_interact();
      }
	}
    return 1;
}



static void mmix_load(int file_no)
{ if (!file2loading(file_no)) return;
  mmix_load_file(get_mmo_name(file2fullname(file_no)));
}

#ifndef VMB
void mmix_zero_memory(mem_node *p)
{ int j;
  if (p->left) mmix_zero_memory(p->left);
  if (p->right) mmix_zero_memory(p->right);
  for (j=0;j<512;j++) 
	  p->dat[j].freq=p->dat[j].tet=0;
}
#endif
int mmix_main(int argc, char *argv[],char *mmo_name)
{ g[255].h=0;
  g[255].l=setjmp(mmix_exit);
  if (g[255].l!=0)
   goto end_simulation;
#ifdef VMB 
  if (!vmb.connected) {win32_message("Not connected") return 0;}
  if (vmb.power) vmb_raise_reset(&vmb);
#endif
  mmix_initialize();
boot:
#ifdef VMB
  vmb.reset_flag=0;
  win32_log("Power...");
  while (!vmb.power)
  {  vmb_wait_for_power(&vmb);
     if (!vmb.connected){win32_message("Power but not connected") return 0;}
  }
  win32_log("ON\n");
  Sleep(50); /* give all devices some time to power up before loading the application */
#else
  mmix_zero_memory(mem_root);
#endif
  mmix_boot(); 
  for_all_files(mmix_load);
  PostMessage(hMainWnd,WM_MMIX_LOAD,0,0);
  mmix_commandline(argc, argv);
#ifndef VMB
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
    if (!check_interact(false)) goto end_simulation;
resume:
    mmix_perform_instruction(); 
	mmix_trace();
	mmix_dynamic_trap();
    if (resuming)
	{ if (op==RESUME)
	    goto resume;
	  else
	    resuming=false;
	}
    if (
#ifdef VMB
		!vmb.power || vmb.reset_flag ||
#endif
		(g[rQ].l&g[rK].l&RE_BIT))
    { breakpoint=true; 
      goto boot;
    }
    if (!check_interact(true)) goto end_simulation;
  }
  end_simulation:
  if (interacting || profiling || showing_stats) show_stats(false);
  mmix_finalize();
  return g[255].l;
}


static char logstr[512];

int mmix_printf(FILE *f, char *format, ...)
{ va_list vargs;
  char logstr[512];
  int n; 
  va_start(vargs,format);	
  if (f==stdout||f==stderr)  
  { n = vsprintf(logstr,format, vargs);
    win32_log(logstr);
  }
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

