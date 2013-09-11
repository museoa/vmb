#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include "vmb.h"
#include "error.h"
#include "winopt.h"
#include "winmain.h"
#include "mmixrun.h"
#include "mmix-internals.h"
#include "mmixlib.h"
#include "mmix-bus.h"
#include "debug.h"


/* Assemble MMIX */
void mmix_run_init(void);

void free_sym(sym_node *sym)
{ if (((unsigned long long int)sym&~0x3)==0) return;
  free_sym(sym->link);
  sym->link=NULL;
  free(sym);
}
  

void free_tree(trie_node *root)
{ if (root==NULL) return;
  if (root->left!=NULL) free_tree(root->left);
  root->left=NULL;
  if (root->mid!=NULL) free_tree(root->mid);
  root->mid=NULL;
  if (root->right!=NULL) free_tree(root->right);
  root->right=NULL;
  free_sym(root->sym);
  free(root);
}

DWORD dwMMIXALThreadId=0;
static void mmixal_finish(void)
{ int i;
	
  dwMMIXALThreadId=0;
  if (listing_file!=NULL) 
  { fclose(listing_file);
    listing_file=NULL;
  }
  if (obj_file!=NULL) 
  { fclose(obj_file);
    obj_file=NULL;
  }
  if (src_file!=NULL) 
  { fclose(src_file);
    src_file=NULL;
  }
  cur_file=0;
  line_no=0;
  long_warning_given=0;
  cur_loc.h=cur_loc.l=0;
  listing_loc.h=listing_loc.l=0;
  spec_mode= 0;
  spec_mode_loc= 0;
  err_count=0;
  serial_number=0;
  free(buffer); buffer=NULL;
  free(lab_field); lab_field=NULL;
  free(op_field); op_field=NULL;
  free(operand_list); operand_list=NULL;
  free(err_buf);err_buf=NULL;
  free(op_stack); op_stack=NULL;
  free(val_stack);val_stack=NULL;
  filename[0]=NULL;
  filename_passed[0]=0;
  for (i=1;i<filename_count;i++)
  { free(filename[i]);
    filename[i]=NULL;
	filename_passed[i]=0;
  }
  filename_count=0;
  free_tree(trie_root); 
  trie_root=NULL;
  free_sym(sym_avail);
  sym_avail=NULL;

}

void mmixal_init(void)
{ int i;
  cur_file=0;
  line_no=0;
  long_warning_given=0;
  cur_loc.h=cur_loc.l=0;
  listing_loc.h=listing_loc.l=0;
  spec_mode= 0;
  spec_mode_loc= 0;
  mmo_ptr=0;
  err_count=0;
  serial_number=0;
  filename_count=0;
  for (i=0;i<10;i++)
  { forward_local[i].link=0;
    backward_local[i].link=0;
  }
  greg= 255;
  lreg= 32;
  halted = 0;
}


void mmixal_exit(int returncode)
{ mmixal_finish();
  ExitThread(returncode);
}

void mmixal_error(char *message, int line_no, int status)
/* status = 0 normal, 1 warning, -1 fatal */
{ if (status<0) ide_status(message);
  else
	 ide_add_error(message,line_no);
}



static DWORD WINAPI AssemblerThreadProc(LPVOID file)
{   
/* variables set by the command line */	
	mmixal_init();
	ide_status("mmixal running ...");
    mmixal();
    mmixal_finish();
    ide_status("mmixal done.");
	return 0;
}

void mmix_assemble(int file_no)
{ HANDLE h;
  if (dwMMIXALThreadId!=0) return;
  if (hError==NULL) new_errorlist();
  ide_clear_error();
		  clear_linetab(file_no);
		  mem_clear_breaks(file_no);
  	src_file_name = fullname;
    obj_file_name[0]=0; /* -o */
	listing_name[0]=0; /* -l */
	expanding=0; /* -x */
    buf_size=500; /* -b */
  h = CreateThread(
			NULL,              // default security attributes
            0,                 // use default stack size  
            AssemblerThreadProc,        // thread function 
            NULL,             // argument to thread function 
            0,                 // use default creation flags 
            &dwMMIXALThreadId);   // returns the thread identifier 
    CloseHandle(h);
}


/* Running MMIX */
// maximum mumber of lines the output console should have

static const WORD MAX_CONSOLE_LINES = 500;
extern void vmb_atexit(void);


typedef struct {
	int (*proc)(void *);
	void *param; } console_params;

DWORD dwMMIXThreadId=0;

void mmix_run_finish(void)
{ int i;
  for (i=0; i<256;i++)
  {   if (file_info[i].name!=NULL)
       { free(file_info[i].name);
         file_info[i].name=NULL;
       }
  }
  vmb_atexit();
#ifdef MMIXLIB
  set_mmix_status(MMIX_OFF);
#endif 
}




void mmix_exit(int returncode)
{ mmix_run_finish();
  FreeConsole();
  dwMMIXThreadId=0;
  ExitThread(returncode);
}



static DWORD WINAPI ConsoleThreadProc(LPVOID cp)
{ int hConHandle;
  HANDLE lStdHandle;
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  FILE *fp;
  FILE stdoutOld, stdinOld, stderrOld;
  int returncode;
  int (*proc)(void*) = ((console_params*)cp)->proc;
  void *param=((console_params*)cp)->param;
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

  mmix_run_init();

  returncode = (*proc)(param);
  *stderr=stderrOld;
  *stdin=stdinOld;
  *stdout=stdoutOld;
  
  mmix_run_finish();
  FreeConsole();
  vmb_atexit();
  vmb_exit_hook = ide_exit_ignore;
  dwMMIXThreadId=0;
  return returncode;
}

static void ConsoleThread(int proc(void *), void *param)
{ HANDLE hMMIXThread=NULL;
  static console_params cp;
  cp.param=param;
  cp.proc=proc;
  if (dwMMIXThreadId!=0) return;
  hMMIXThread = CreateThread(
			NULL,              // default security attributes
            0,                 // use default stack size  
            ConsoleThreadProc,        // thread function 
            &cp,             // argument to thread function 
            0,                 // use default creation flags 
            &dwMMIXThreadId);   // returns the thread identifier 
  CloseHandle(hMMIXThread);
}

char full_mmo_name[MAX_PATH+1]={0};

char *get_mmo_name(void)
{  if (fullname[0]==0) return NULL;
   strncpy(full_mmo_name,fullname,MAX_PATH-4);
   full_mmo_name[strlen(full_mmo_name)-1]='o';
   return full_mmo_name;
}

void mmix_run_init(void)
{ 
  if (!vmb.connected)
  { ide_status("connecting ...");
    init_mmix_bus(host, port, "MMIX IDE");
    if (!vmb.connected)
    { ide_status("unable to connect to motherboard");
    return;
    }
	ide_status("connected");
  }
    vmb_exit_hook = mmix_exit;
//  vmb_debug_hook = win32_debug;
//	vmb_error_init_hook = win32_error_init;
  strncpy(full_mmo_name,fullname,MAX_PATH-4);
  full_mmo_name[strlen(full_mmo_name)-1]='o';
  }

void mmix_run(void)
{		  interacting=false;
		  show_operating_system=false;
          breakpoint=false;
		  tracing=false;
		  tracing_exceptions=0;
          stack_tracing=false;
          ConsoleThread(mmix_main,NULL);
}

void mmix_debug(void)
{		  interacting=true;
		  show_operating_system=false;
          breakpoint=true;
		  tracing=true;
		  tracing_exceptions=0xFFFF;
		  stack_tracing=false;
		  ConsoleThread(mmix_main,NULL);
}

void mmix_stop(void)
{ interrupt=true;
  if (!interacting) halted=true;
}

char * mmix_status_str[]={"Disconnected", "Connected","Off", "On", "Stopped", "Running", "Halted"};

void set_mmix_status(int status)
{ mmix_status = status;
  ide_status(mmix_status_str[mmix_status]);
}

void mmix_stopped(octa loc)
{ mem_tetra *ll;
  ide_status(mmix_status_str[mmix_status]);
  ll=mem_find(loc);
  if (ll->file_no==0)
    PostMessage(hMainWnd,WM_MMIX_STOPPED,ll->line_no,0);
}