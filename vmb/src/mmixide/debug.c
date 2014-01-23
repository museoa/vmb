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

int show_debug_local=1;
int show_debug_global=1;
int show_debug_special=0;
int show_debug_regstack=0;
int show_debug_text=0;
int show_debug_data=0;
int show_debug_pool=0;
int show_debug_stack=0;
int show_debug_neg=0;




int break_at_Main = 1;
int break_after = 1;

#define MAX_DEBUG_WINDOWS 9
INT_PTR CALLBACK    
OptionDebugDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ 
  switch ( message )
  { case WM_INITDIALOG:
      { 
        CheckDlgButton(hDlg,IDC_SHOW_LOCAL,(show_debug_local)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hDlg,IDC_SHOW_GLOBAL,(show_debug_global)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hDlg,IDC_SHOW_SPECIAL,(show_debug_special)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hDlg,IDC_SHOW_REGSTACK,(show_debug_regstack)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hDlg,IDC_SHOW_TEXT,(show_debug_text)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hDlg,IDC_SHOW_DATA,(show_debug_data)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hDlg,IDC_SHOW_POOL,(show_debug_pool)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hDlg,IDC_SHOW_STACK,(show_debug_stack)?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(hDlg,IDC_SHOW_NEG,(show_debug_neg)?BST_CHECKED:BST_UNCHECKED);

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
      { 
		show_debug_local=IsDlgButtonChecked(hDlg,IDC_SHOW_LOCAL);
		show_debug_global=IsDlgButtonChecked(hDlg,IDC_SHOW_GLOBAL);
		show_debug_special=IsDlgButtonChecked(hDlg,IDC_SHOW_SPECIAL);
		show_debug_regstack=IsDlgButtonChecked(hDlg,IDC_SHOW_REGSTACK);
		show_debug_text=IsDlgButtonChecked(hDlg,IDC_SHOW_TEXT);
		show_debug_data=IsDlgButtonChecked(hDlg,IDC_SHOW_DATA);
		show_debug_pool=IsDlgButtonChecked(hDlg,IDC_SHOW_POOL);
		show_debug_stack=IsDlgButtonChecked(hDlg,IDC_SHOW_STACK);
		show_debug_neg=IsDlgButtonChecked(hDlg,IDC_SHOW_NEG);

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
{ if (show_debug_local) new_register_view(0);
  if (show_debug_global) new_register_view(1);
  if (show_debug_special) new_register_view(2);
  if (show_debug_text) new_memory_view(0);
  if (show_debug_data) new_memory_view(1);
  if (show_debug_pool) new_memory_view(2);
  if (show_debug_stack) new_memory_view(3);
  if (show_debug_neg) new_memory_view(4);
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

#define MAXREG 3
struct register_def reg_names[256]={0};
struct register_def reg_stack[256]={0};
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



static char *regnames[256] = {
	"$0","$1","$2","$3","$4","$5","$6","$7","$8","$9",
	"$10","$11","$12","$13","$14","$15","$16","$17","$18","$19",
	"$20","$21","$22","$23","$24","$25","$26","$27","$28","$29",
	"$30","$31","$32","$33","$34","$35","$36","$37","$38","$39",
	"$40","$41","$42","$43","$44","$45","$46","$47","$48","$49",
	"$50","$51","$52","$53","$54","$55","$56","$57","$58","$59",
	"$60","$61","$62","$63","$64","$65","$66","$67","$68","$69",
	"$70","$71","$72","$73","$74","$75","$76","$77","$78","$79",
	"$80","$81","$82","$83","$84","$85","$86","$87","$88","$89",
	"$90","$91","$92","$93","$94","$95","$96","$97","$98","$99",
	"$100","$101","$102","$103","$104","$105","$106","$107","$108","$109",
	"$110","$111","$112","$113","$114","$115","$116","$117","$118","$119",
	"$120","$121","$122","$123","$124","$125","$126","$127","$128","$129",
	"$130","$131","$132","$133","$134","$135","$136","$137","$138","$139",
	"$140","$141","$142","$143","$144","$145","$146","$147","$148","$149",
	"$150","$151","$152","$153","$154","$155","$156","$157","$158","$159",
	"$160","$161","$162","$163","$164","$165","$166","$167","$168","$169",
	"$170","$171","$172","$173","$174","$175","$176","$177","$178","$179",
	"$180","$181","$182","$183","$184","$185","$186","$187","$188","$189",
	"$190","$191","$192","$193","$194","$195","$196","$197","$198","$199",
	"$200","$201","$202","$203","$204","$205","$206","$207","$208","$209",
	"$210","$211","$212","$213","$214","$215","$216","$217","$218","$219",
	"$220","$221","$222","$223","$224","$225","$226","$227","$228","$229",
	"$230","$231","$232","$233","$234","$235","$236","$237","$238","$239",
	"$240","$241","$242","$243","$244","$245","$246","$247","$248","$249",
	"$250","$251","$252","$253","$254","$255"};


void reg_names_init(void)
{ int i;
  for (i=0;i<256;i++)
  { register_def *r;
    r= &reg_names[i];
	r->name=regnames[i];
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
static unsigned char *local_mem=NULL;

void locals_to_mem(int from, int to)
{ int i;
  for (i=from; i<to;i++)
    { int k = i&lring_mask;
	  octa *o= &l[k];
	  inttochar(o->h,local_mem+(k*8));
	  inttochar(o->l,local_mem+(k*8)+4);
    }
}
void mem_to_locals(int from, int to)
{ int i;
  int max = O-S+L;
  if (to<max) max=to;
  for (i=from; i<max;i++)
  { int k= i&lring_mask;
    octa *o= &l[k];
    o->h=chartoint(local_mem+(k*8));
    o->l=chartoint(local_mem+(k*8)+4);
  }
  if (to>max)
   memset(local_mem+max*8,0,to-max*8);
  MemoryDialogUpdate(register_insp[REG_LOCAL].hWnd,&register_insp[REG_LOCAL],to*8,(to-from)*8);
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


struct inspector_def register_insp[MAXREG+1]= /* array sorted by REG_LOCAL,REG_GLOBAL,REG_SPECIAL */
{ {"Local Registers",256*8,get_local_mem,load_local_mem,store_local_mem,hex_format, octa_chunk,-1,8,0,reg_names},
  {"Global Registers",256*8,get_global_mem,load_global_mem,store_global_mem,hex_format, octa_chunk,-1,8,0,&reg_names[255]},
  {"Special Registers",32*8,get_global_mem,load_global_mem,store_global_mem,hex_format, octa_chunk,-1,8,32,special_reg_names},
{NULL}
};

void set_register_inspectors(void)
{ if (register_insp[REG_LOCAL].hWnd!=NULL)
  { int i, n, r, b, opt;
    if (show_debug_regstack)
	{ n = (O-S)+L;
	  b = S;
	}
	else 
	{ n=L;
	  b=O;
	}
	if (n>0xFF) n=0xFF;
    register_insp[REG_LOCAL].num_regs=n;
	opt=0;
	for (r=L-1,i=n-1;i>=0;i--,r--)/* registerstack */
	{ char *name;
	  static char empty[]="";
	  if (r<0) 
	  { r=l[(b+i)&lring_mask].l;
	    name = empty;
		opt=REG_OPT_DISABLED;
	  }
	  else
		name = regnames[r];
	  reg_names[i].name=name;
	  reg_names[i].offset=(b+i)*8;
	  reg_names[i].options=opt;
	}
    adjust_mem_display(&register_insp[REG_LOCAL]);
  }
  if (register_insp[REG_GLOBAL].num_regs!=256-G)
  { register_insp[REG_GLOBAL].num_regs=256-G;
    register_insp[REG_GLOBAL].regs=&reg_names[G];
    adjust_mem_display(&register_insp[REG_GLOBAL]);
  }
  else
	  MemoryDialogUpdate(register_insp[REG_GLOBAL].hWnd,&register_insp[REG_GLOBAL], register_insp[REG_GLOBAL].regs[0].offset,(256-G)*8);
   MemoryDialogUpdate(register_insp[REG_SPECIAL].hWnd,&register_insp[REG_SPECIAL], 0,register_insp[REG_SPECIAL].size );
}

void debug_init(void)
{ 	if (local_mem!=NULL) free(local_mem);
    local_mem=calloc(lring_size,8);
	if (local_mem==NULL)  vmb_fatal_error(__LINE__,"Out of Memory for local_mem");
	reg_names_init();
	set_special_reg_name();
	set_mem_font_metrics();
}

void new_register_view(int i)
{ int k;
  HWND h;
  if (i<0 || i>=MAXREG) return;
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
//  for (i=0; i<MAXREG; i++)
//	if(register_insp[i].hWnd)
//	  MemoryDialogUpdate(register_insp[i].hWnd,&register_insp[i], 0,register_insp[i].size );
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