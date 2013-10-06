#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <setjmp.h>
#include "vmb.h"
#include "error.h"
#include "winopt.h"
#include "winmain.h"
#include "mmix-internals.h"
#include "mmixlib.h"
#include "mmix-bus.h"
#include "info.h"
#include "symtab.h"
#include "debug.h"
#include "assembler.h"
#include "mmixrun.h"
/* Running MMIX */
// maximum mumber of lines the output console should have

static const WORD MAX_CONSOLE_LINES = 500;
extern void vmb_atexit(void);
void mmix_run_init(void);
int application_file_no=-1;
DWORD dwMMIXThreadId=0;

static HANDLE hInteract=NULL;

/* called by the mmix thread */
static int mmix_waiting = 0;

int mmix_interact(void)
{ DWORD w;
  mmix_stopped(loc);
  printf("----------------------------------------------------\n");
  mmix_waiting = 1;
  w = WaitForSingleObject(hInteract,INFINITE);
  mmix_waiting = 0;
  if (vmb_get_interrupt(&vmb,&new_Q.h,&new_Q.l)==1)
  { g[rQ].h |= new_Q.h; g[rQ].l |= new_Q.l; }
  return 1;
}

void mmix_stop(void)
{ interrupt=true;
  //show_operating_system=true;
  if (!interacting) halted=true;
}
 

int mmix_continue(unsigned char command)
{  if (hInteract==NULL) return 0;
   switch (command)
   { case 'q': /* quit */
       breakpoint=halted=true;
       break;
     case 'c': /* continue */
	   mmix_status(MMIX_RUNNING);
       break;
	 case 'n': /* next instruction */
     default: interacting=breakpoint=tracing=true; /* trace one inst and break */
       break;
   }
   show_stop_marker(edit_file_no,-1); /* clear stop marker */
   if (mmix_waiting)
     SetEvent (hInteract);
   return 1;
}

static DWORD WINAPI MMIXThreadProc(LPVOID dummy)
{ int hConHandle;
  HANDLE lStdHandle;
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  FILE *fp;
  FILE stdoutOld, stdinOld, stderrOld;
  int returncode;

  if (AllocConsole()==0)
    vmb_debugi(VMB_DEBUG_FATAL,"Unable to allocate Console (%X)",GetLastError());
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&coninfo);
  coninfo.dwSize.Y = MAX_CONSOLE_LINES;
  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),coninfo.dwSize);

  lStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
  fp = _fdopen( hConHandle, "w" );
  stdoutOld=*stdout;
  *stdout = *fp;
  setvbuf( stdout, NULL, _IONBF, 0 );

  lStdHandle = GetStdHandle(STD_INPUT_HANDLE);
  hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
  fp = _fdopen( hConHandle, "r" );
  stdinOld=*stdin;
  *stdin = *fp;
  setvbuf( stdin, NULL, _IONBF, 0 );

  lStdHandle =  GetStdHandle(STD_ERROR_HANDLE);
  hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
  fp = _fdopen( hConHandle, "w" );
  stderrOld=*stderr;
  *stderr = *fp;
  setvbuf( stderr, NULL, _IONBF, 0 );

  if (hInteract==NULL)
    hInteract =CreateEvent(NULL,FALSE,FALSE,NULL);

  vmb_exit_hook = mmix_exit;
  returncode = mmix_main(0,NULL,get_mmo_name(file2fullname(application_file_no)));
  application_file_no=-1;
  vmb_atexit();
  vmb_exit_hook = ide_exit_ignore;

  PostMessage(hMainWnd,WM_MMIX_STOPPED,0,(LPARAM)-1); 
 
  if (hInteract!=NULL)
   { CloseHandle(hInteract); 
     hInteract=NULL;
   }

  *stderr=stderrOld;
  *stdin=stdinOld;
  *stdout=stdoutOld;

  FreeConsole();
  dwMMIXThreadId=0;
  return returncode;
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

void mmix_run(int file_no)
{		  interacting=false;
		  show_operating_system=false;
          breakpoint=false;
		  tracing=false;
		  tracing_exceptions=0;
          stack_tracing=false;
		  application_file_no=file_no;
		  update_symtab();
          MMIXThread();
}

int break_at_symbol(char *symbol)
{ sym_node *sym=find_symbol(symbol,application_file_no);
  if (sym!=NULL&& sym->link==DEFINED)
  { loc2bkpt(sym->equiv)|= exec_bit;
	ide_mark_breakpoint(sym->file_no,sym->line_no);
	return 1;
  }
  else
    return 0;
}

void mmix_reset(void)
{ 	PostMessage(hMainWnd,WM_MMIX_RESET,0,0);
}


void mmix_debug(int file_no)
{		  interacting=true;
		  show_operating_system=show_os;
          breakpoint=true;
		  tracing=trace;
		  tracing_exceptions=0x0;
		  stack_tracing=false;
//		  vmb.reset=mmix_reset; currently no need for this.
		  application_file_no=file_no;
		  update_symtab();
		  MMIXThread();
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

jmp_buf error_exit;
device_info vmb;

#define panic(m) vmb_fatal_error(__LINE__,m)
#define sign_bit ((unsigned)0x80000000)



void mmix_exit(int returncode)
{ longjmp(error_exit, returncode);
}

/* this is the plain vmb version of mmix main() */

int check_interact(void)
{   if (!interrupt && !breakpoint) return 1;
	if (interrupt && !breakpoint) breakpoint=interacting=true, interrupt=false;
    else if (!(inst_ptr.h&sign_bit) || show_operating_system || 
          (inst_ptr.h==0x80000000 && inst_ptr.l==0))
    { breakpoint=false;
      if (interacting) { 
		return mmix_interact();
      }
    }
   return 1;
}

int mmix_main(int argc, char *argv[],char *mmo_name)
{ g[255].h=0;
  g[255].l=setjmp(error_exit);
  if (g[255].l!=0)
   goto end_simulation;
 
  if (!vmb.connected) panic("Not connected");
  if (vmb.power)  
    vmb_raise_reset(&vmb);
  mmix_initialize();
boot:
  mmix_boot(); 
  fprintf(stderr,"Power...");
  while (!vmb.power)
  {  vmb_wait_for_power(&vmb);
     if (!vmb.connected) goto end_simulation;
  }
  fprintf(stderr,"ON\n");
  mmix_load_file(mmo_name);
  PostMessage(hMainWnd,WM_MMIX_LOAD,0,0);
  mmix_commandline(argc, argv);
  while (vmb.connected) {
	if (!break_after && !check_interact()) goto end_simulation;
    if (halted) break;
    do   
    { if (!resuming)
        mmix_fetch_instruction();
      if (break_after && !check_interact()) goto end_simulation;
      mmix_perform_instruction(); 
	  mmix_trace();
	  mmix_dynamic_trap();
      if (resuming && op!=RESUME) resuming=false; 
    } while ((vmb.connected && vmb.power && !vmb.reset_flag &&
              !interrupt && !breakpoint) || 
              resuming);
    if (interact_after_break) 
       interacting=true, interact_after_break=false;
    if (!vmb.power|| vmb.reset_flag)
    { breakpoint=true; 
      vmb.reset_flag=0; 
      goto boot;
    }
  }
  end_simulation:
  if (interacting || profiling || showing_stats) show_stats(true);
  mmix_finalize();
  return g[255].l;
}
