#include <windows.h>
#include "splitter.h"
#include "vmb.h"
#include "winmain.h"
#include "error.h"
#include "inspect.h"
#include "bus-arith.h"
#include "mmix-internals.h"
#include "mmixlib.h"
#include "info.h"
#include "debug.h"
#include "resource.h"
#include "mmixrun.h"



#define MAXMEM 5
inspector_def memory_insp[];

#define WIN_LOCAL                  (1<<0)
#define WIN_GLOBAL                 (1<<1)
#define WIN_SPECIAL                (1<<2)
#define WIN_REGSTACK               (1<<3)
#define WIN_TEXT                   (1<<4)
#define WIN_DATA                   (1<<5)
#define WIN_POOL                   (1<<6)
#define WIN_STACK                  (1<<7)

unsigned int show_debug_windows = WIN_LOCAL|WIN_GLOBAL; 
int break_at_Main = 1;
int break_after = 1;

#define MAX_DEBUG_WINDOWS 9
INT_PTR CALLBACK    
OptionDebugDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ 
  switch ( message )
  { case WM_INITDIALOG:
      { int i;
	    for (i=0;i<MAX_DEBUG_WINDOWS;i++)
		{	CheckDlgButton(hDlg,IDC_SHOW_LOCAL+i,
			   (show_debug_windows&(1<<i))?BST_CHECKED:BST_UNCHECKED);
		}
        CheckDlgButton(hDlg,IDC_CHECK_MAIN,break_at_Main?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hDlg,IDC_CHECK_TRACE,tracing?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hDlg,IDC_CHECK_OS,show_operating_system?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_RADIO_BREAK_AFTER,break_after?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_RADIO_BREAK_BEFORE,!break_after?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hDlg,IDC_CHECK_EXCEPTIONS,tracing_exceptions!=0?BST_CHECKED:BST_UNCHECKED);
      }
      return TRUE;
    case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
      { int i;
	    for (i=0;i<MAX_DEBUG_WINDOWS;i++)
			if (IsDlgButtonChecked(hDlg,IDC_SHOW_RA+i))
			  show_debug_windows|=1<<i;
			else
			  show_debug_windows&=~(1<<i);
		break_at_Main=IsDlgButtonChecked(hDlg,IDC_CHECK_MAIN);
		tracing=IsDlgButtonChecked(hDlg,IDC_CHECK_TRACE);
		show_operating_system=IsDlgButtonChecked(hDlg,IDC_CHECK_OS);
		break_after=IsDlgButtonChecked(hDlg,IDC_RADIO_BREAK_AFTER);
		if (IsDlgButtonChecked(hDlg,IDC_CHECK_EXCEPTIONS))
			tracing_exceptions=-1;
		else
			tracing_exceptions=0;
        EndDialog(hDlg, TRUE);
        return TRUE;
      } else if (wparam==IDCANCEL)
	  { EndDialog(hDlg, TRUE);
        return TRUE;
	  } else if (wparam==IDC_SELECT_SPECIALS)
	  { DialogBox(hInst,MAKEINTRESOURCE(IDD_SHOW_SPECIAL),hDlg,OptionSpecialDialogProc);
        return TRUE;
	  }
     break;
  }
  return FALSE;
}


void set_debug_windows(void)
{
  if (show_debug_windows&(1<<0)) new_register_view(0);
  if (show_debug_windows&(1<<1)) new_register_view(1);
  if (show_debug_windows&(1<<2)) new_register_view(2);
  if (show_debug_windows&(1<<3)) new_register_view(3);
  if (show_debug_windows&(1<<4)) new_memory_view(0);
  if (show_debug_windows&(1<<5)) new_memory_view(1);
  if (show_debug_windows&(1<<6)) new_memory_view(2);
  if (show_debug_windows&(1<<7)) new_memory_view(3);
  if (show_debug_windows&(1<<8)) new_memory_view(4);
}

/* generic routines */
static int get_mem(uint64_t address, int size, unsigned char *buf)
{ octa addr;
  addr.h=(tetra)((address>>32)&0xFFFFFFFF);
  addr.l=(tetra)(address&0xFFFFFFFF);
  return mmgetchars(buf, size, addr, -1);
}

static unsigned char *load_mem(uint64_t address, int size)
{ static unsigned char load_memory[8];
  if (size>8) size=8;	
  get_mem(address,size,load_memory);
  return load_memory;
}

static void store_mem(int segment, unsigned int offset, int size, unsigned char *buf)
{ octa addr;
  addr.h=segment<<29;
  addr.l=offset;
  mmputchars(buf, size, addr);
  MemoryDialogUpdate(memory_insp[segment].hWnd,&memory_insp[segment],offset,size);
}

/* specialized routines for the various segments */

static int get_text_mem(unsigned int offset, int size, unsigned char *buf)
{ return get_mem(0ULL+offset,size,buf);
}
static unsigned char * load_text_mem(unsigned int offset, int size)
{ return load_mem(0ULL+offset,size);
}
static void store_text_mem(unsigned int offset, int size, unsigned char *buf)
{ store_mem(0,offset,size,buf);
}
static int get_data_mem(unsigned int offset, int size, unsigned char *buf)
{return get_mem((1ULL<<61)+offset,size,buf);
}
static unsigned char * load_data_mem(unsigned int offset, int size)
{ return load_mem((1ULL<<61)+offset,size);
}
static void store_data_mem(unsigned int offset, int size, unsigned char *buf)
{ store_mem(1,offset,size,buf);
}
static int get_pool_mem(unsigned int offset, int size, unsigned char *buf)
{return get_mem((2ULL<<61)+offset,size,buf);
}
static unsigned char * load_pool_mem(unsigned int offset, int size)
{ return load_mem((2ULL<<61)+offset,size);
}
static void store_pool_mem(unsigned int offset, int size, unsigned char *buf)
{ store_mem(2,offset,size,buf);
}
static int get_stack_mem(unsigned int offset, int size, unsigned char *buf)
{return get_mem((3ULL<<61)+offset,size,buf);
}
static unsigned char * load_stack_mem(unsigned int offset, int size)
{ return load_mem((3ULL<<61)+offset,size);
}
static void store_stack_mem(unsigned int offset, int size, unsigned char *buf)
{ store_mem(3,offset,size,buf);
}
static int get_neg_mem(unsigned int offset, int size, unsigned char *buf)
{return get_mem((4ULL<<61)+offset,size,buf);
}
static unsigned char * load_neg_mem(unsigned int offset, int size)
{ return load_mem((4ULL<<61)+offset,size);
}
static void store_neg_mem(unsigned int offset, int size, unsigned char *buf)
{ store_mem(4,offset,size,buf);
}



static inspector_def memory_insp[MAXMEM+1]=
{ 	{"Text Segment",0xffffffff,get_text_mem,load_text_mem,store_text_mem,hex_format, tetra_chunk,-1,8,0,NULL,0,0ULL<<61},
	{"Data Segment",0xffffffff,get_data_mem,load_data_mem,store_data_mem,hex_format, octa_chunk,-1,8,0,NULL,0,1ULL<<61},
	{"Pool Segment",0xffffffff,get_pool_mem,load_pool_mem,store_pool_mem,hex_format, octa_chunk,-1,8,0,NULL,0,2ULL<<61},
	{"Stack Segment",0xffffffff,get_stack_mem,load_stack_mem,store_stack_mem,hex_format, octa_chunk,-1,8,0,NULL,0,3ULL<<61},
	{"Negative Segemnt",0xffffffff,get_neg_mem,load_neg_mem,store_neg_mem,hex_format, tetra_chunk,-1,8,0,NULL,0,4ULL<<61},
	{NULL}
};


int mmix_current_status;

void new_memory_view(int i)
{ HWND h;
  int k;
  if (mmix_current_status==MMIX_DISCONNECTED) return;
  if (i<0 || i>=MAXMEM) return;
  if (memory_insp[i].hWnd!=NULL) return;
  for (k=i-1;k>=0&&memory_insp[k].hWnd==NULL;k--)
	  continue;
  if (k<0)
    for (k=i+1;k<MAXMEM&&memory_insp[k].hWnd==NULL;k++)
	  continue;
  if (k>=MAXMEM)
	 sp_create_options(0,0,0.0,mem_min_width,NULL);
  else if (k<i)
	 sp_create_options(0,0,0.5,0,memory_insp[k].hWnd);
  else
	 sp_create_options(1,0,0.5,0,memory_insp[k].hWnd);

  h = CreateMemoryDialog(hInst,hSplitter);
  SetInspector(h, &memory_insp[i]);
  ShowWindow(h,SW_SHOW);
}

/* REgisters */


unsigned int show_special_registers = 0xf03980da; /* bits correspond to registers from rZZ=31 to rB=0 */ 
#define REG_LOCAL 0
#define REG_GLOBAL 1
#define REG_SPECIAL 2
#define REG_STACK 3
#define MAXREG 4
struct register_def reg_names[256]={0};
struct inspector_def register_insp[];
void set_special_reg_name(void);
char *long_special_name[32]= {"rB bootstrap register (trip)",
"rD dividend register","rE epsilon register","rH himult register",
"rJ return-jump register","rM multiplex mask register",
"rR remainder register","rBB bootstrap register (trap)",
"rC continuation register","rN serial number","rO register stack offset",
"rS register stack pointer","rI interval counter","rT trap address register",
"rTT dynamic trap address register","rK interrupt mask register",
"rQ interrupt request register","rU usage counter","rV virtual translation register",
"rG global threshold register","rL local threshold register",
"rA arithmetic status register","rF failure location register",
"rP prediction register","rW where-interrupted register (trip)",
"rX execution register (trip)","rY Y operand (trip)",
"rZ Z operand (trip)","rWW where-interrupted register (trap)",
"rXX execution register (trap)","rYY Y operand (trap)","rZZ Z operand (trap)"};
INT_PTR CALLBACK    
OptionSpecialDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ 
  switch ( message )
  { case WM_INITDIALOG:
      { int i;
	    for (i=0;i<32;i++)
		{	CheckDlgButton(hDlg,IDC_SHOW_RA+i,
			   (show_special_registers&(1<<i))?BST_CHECKED:BST_UNCHECKED);
		    SetDlgItemText(hDlg,IDC_SHOW_RA+i,long_special_name[i]);
		}
      }
      return TRUE;
    case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
      { int i;
	    for (i=0;i<32;i++)
			if (IsDlgButtonChecked(hDlg,IDC_SHOW_RA+i))
			  show_special_registers|=1<<i;
			else
			  show_special_registers&=~(1<<i);
		if (register_insp[REG_SPECIAL].hWnd!=NULL)
		{ set_special_reg_name();
		  adjust_mem_display(&register_insp[REG_SPECIAL]);
		}
        EndDialog(hDlg, TRUE);
        return TRUE;
      } else if (wparam==IDCANCEL)
	  { EndDialog(hDlg, TRUE);
        return TRUE;
	  }
     break;
  }
  return FALSE;
}






void reg_names_init(void)
{ int i;
  static char names[256*5];
  for (i=0;i<256;i++)
  { register_def *r;
    r= &reg_names[i];
	r->name=names+i*5;
	sprintf(r->name,"$%d",i);
	r->offset=i*8;
	r->size=8;
	r->chunk=user_chunk;
	r->format=user_format;
  }
}

struct register_def special_reg_names[32]={0};

void set_special_reg_name(void)
{ int i,k;
  for (i=k=0;i<32;i++)
  if (show_special_registers&(1<<i))
  { special_reg_names[k].name=special_name[i];
    special_reg_names[k].offset=i*8;
    special_reg_names[k].size=8;
    special_reg_names[k].chunk=user_chunk;
    special_reg_names[k].format=user_format;
	k++;
  }
  register_insp[REG_SPECIAL].num_regs=k;
}


/* local registers */
static unsigned char local_mem[256*8]={0};

void locals_to_mem(int from, int to)
{ int i;
  for (i=from; i<to;i++)
    if (i<L)
    { octa *o= &l[(O+i)&lring_mask];
	  inttochar(o->h,local_mem+(i*8));
	  inttochar(o->l,local_mem+(i*8)+4);
    }
    else
	  memset(local_mem+(i*8),0,8);
}
void mem_to_locals(int from, int to)
{ int i;
  for (i=from; i<to;i++)
    if (i<L)
    { octa *o= &l[(O+i)&lring_mask];
	  o->h=chartoint(local_mem+(i*8));
	  o->l=chartoint(local_mem+(i*8)+4);
    }
  MemoryDialogUpdate(register_insp[REG_LOCAL].hWnd,&register_insp[REG_LOCAL],register_insp[REG_LOCAL].regs[from].offset,8*(to-from));
}


static int get_local_mem(unsigned int offset, int size, unsigned char *buf)
{ locals_to_mem(offset/8,(offset+size+7)/8);
  memmove(buf,&local_mem[offset],size);
  return size;
}
static unsigned char * load_local_mem(unsigned int offset, int size)
{ locals_to_mem(offset/8,(offset+size+7)/8);
  return local_mem+offset;
}
static void store_local_mem(unsigned int offset, int size, unsigned char *buf)
{ memmove(local_mem+offset,buf,size);
  mem_to_locals(offset/8,(offset+size+7)/8);
}

/* global and special registers */

static unsigned char global_mem[256*8]={0};

void globals_to_mem(int from, int to)
{ int i;
  for (i=from; i<to;i++)
    if (i>=G || i<32)
    { octa *o= &g[i];
	  inttochar(o->h,global_mem+(i*8));
	  inttochar(o->l,global_mem+(i*8)+4);
    }
    else
	  memset(global_mem+(i*8),0,8);
}
void mem_to_globals(int from, int to)
{ int i;
  for (i=from; i<to;i++)
    if (i>=G || i<32)
    { octa *o= &g[i];
	  o->h=chartoint(global_mem+(i*8));
	  o->l=chartoint(global_mem+(i*8)+4);
    }
  if (from<32)
	  MemoryDialogUpdate(register_insp[REG_SPECIAL].hWnd,&register_insp[REG_SPECIAL],from*8,8*(to-from));
  else if (to>=G)
  	  MemoryDialogUpdate(register_insp[REG_GLOBAL].hWnd,&register_insp[REG_GLOBAL],register_insp[REG_GLOBAL].regs[from-G].offset,8*(to-from));
}


static int get_global_mem(unsigned int offset, int size, unsigned char *buf)
{ globals_to_mem(offset/8,(offset+size+7)/8);
  memmove(buf,&global_mem[offset],size);
  return size;
}
static unsigned char * load_global_mem(unsigned int offset, int size)
{ globals_to_mem(offset/8,(offset+size+7)/8);
  return global_mem+offset;
}
static void store_global_mem(unsigned int offset, int size, unsigned char *buf)
{ memmove(global_mem+offset,buf,size);
  mem_to_globals(offset/8,(offset+size+7)/8);
}


struct inspector_def register_insp[MAXREG+1]=
{ {"Local Registers",256*8,get_local_mem,load_local_mem,store_local_mem,hex_format, octa_chunk,-1,8,1,reg_names},
  {"Global Registers",256*8,get_global_mem,load_global_mem,store_global_mem,hex_format, octa_chunk,-1,8,1,&reg_names[255]},
  {"Special Registers",32*8,get_global_mem,load_global_mem,store_global_mem,hex_format, octa_chunk,-1,8,32,special_reg_names},
  {"Registerstack",256*8,get_local_mem,load_local_mem,store_local_mem,hex_format, octa_chunk,-1,8,0,reg_names},
{NULL}
};

void set_register_inspectors(void)
{ if (register_insp[REG_LOCAL].num_regs!=L)
  { register_insp[REG_LOCAL].num_regs=L;
    adjust_mem_display(&register_insp[REG_LOCAL]);
  }
  if (register_insp[REG_GLOBAL].num_regs!=256-G)
  { register_insp[REG_GLOBAL].num_regs=256-G;
    register_insp[REG_GLOBAL].regs=&reg_names[G];
    adjust_mem_display(&register_insp[REG_GLOBAL]);
  }
}

void new_register_view(int i)
{ int k;
  HWND h;
  if (i<0 || i>=MAXREG) return;
  if (reg_names[0].name==NULL){
	reg_names_init();
	set_special_reg_name();
	set_mem_font_metrics();
  }
  if (register_insp[i].hWnd!=NULL) return;
  for (k=i-1;k>=0&&register_insp[k].hWnd==NULL;k--)
	  continue;
  if (k<0)
    for (k=i+1;k<MAXREG&&register_insp[k].hWnd==NULL;k++)
	  continue;
  if (k>=MAXREG)
	 sp_create_options(0,1,0.0,mem_min_width,NULL);
  else if (k<i)
	 sp_create_options(0,0,0.5,0,register_insp[k].hWnd);
  else
	 sp_create_options(1,0,0.5,0,register_insp[k].hWnd);
  h = CreateMemoryDialog(hInst,hSplitter);
  SetInspector(h, &register_insp[i]);
  set_register_inspectors();
  ShowWindow(h,SW_SHOW);
}




void memory_update(void)
{ int i;
  for (i=0; i<MAXMEM; i++)
	if(memory_insp[i].hWnd)
	  MemoryDialogUpdate(memory_insp[i].hWnd,&memory_insp[i], 0,memory_insp[i].size );
  set_register_inspectors();
  for (i=0; i<MAXREG; i++)
	if(register_insp[i].hWnd)
	  MemoryDialogUpdate(register_insp[i].hWnd,&register_insp[i], 0,register_insp[i].size );
}


static void set_x_break(octa loc)
{ mem_find(loc)->bkpt |= exec_bit;
}

int set_breakpoint(int file_no, int line_no)
/* return true if breakpoint could be set */
{ for_all_loc(file_no, line_no, set_x_break);
  return 1;
}	

static void del_x_break(octa loc)
{ mem_find(loc)->bkpt &= ~exec_bit;
}


int del_breakpoint(int file_no, int line_no)
{ for_all_loc(file_no, line_no, del_x_break);
  return 1;
}	