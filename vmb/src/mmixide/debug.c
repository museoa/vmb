#include <windows.h>
#include "splitter.h"
#include "vmb.h"
#include "winmain.h"
#include "error.h"
#include "inspect.h"
#include "bus-arith.h"
#include "mmix-internals.h"
#include "mmixlib.h"
#include "debug.h"


#define MAXMEM 5
static HWND hMemory[MAXMEM]={0};
inspector_def memory_insp[];

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
  MemoryDialogUpdate(hMemory[segment],&memory_insp[segment],offset,size);
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
{ 	{"Text Segment",0x1000,get_text_mem,load_text_mem,store_text_mem,hex_format, tetra_chunk,-1,8,0,NULL,0,0ULL<<61},
	{"Data Segment",0x1000,get_data_mem,load_data_mem,store_data_mem,hex_format, octa_chunk,-1,8,0,NULL,0,1ULL<<61},
	{"Pool Segment",0x1000,get_pool_mem,load_pool_mem,store_pool_mem,hex_format, octa_chunk,-1,8,0,NULL,0,2ULL<<61},
	{"Stack Segment",0x1000,get_stack_mem,load_stack_mem,store_stack_mem,hex_format, octa_chunk,-1,8,0,NULL,0,3ULL<<61},
	{"Negative Segemnt",0x1000,get_neg_mem,load_neg_mem,store_neg_mem,hex_format, tetra_chunk,-1,8,0,NULL,0,4ULL<<61},
	{NULL}
};




void new_memory_view(int i)
{ if (mmix_status==MMIX_DISCONNECTED) return;
  if (i<0 || i>=MAXMEM) return;
  if (hMemory[i]!=NULL) return;
  hMemory[i] = CreateMemoryDialog(hInst,hSplitter);
  SetInspector(hMemory[i], &memory_insp[i]);
  ShowWindow(hMemory[i],SW_SHOW);
}

/* REgisters */

#define REG_LOCAL 0
#define REG_GLOBAL 1
#define REG_STACK 2
#define REG_SPECIAL 3
#define MAXREG 4
static HWND hRegisters[MAXREG]={0};
struct register_def reg_names[256]={0};
struct inspector_def register_insp[];

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
{ int i;
  for (i=0;i<32;i++)
  { special_reg_names[i].name=special_name[i];
    special_reg_names[i].offset=i*8;
    special_reg_names[i].size=8;
    special_reg_names[i].chunk=user_chunk;
    special_reg_names[i].format=user_format;
  }
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
  MemoryDialogUpdate(hRegisters[REG_LOCAL],&register_insp[REG_LOCAL],register_insp[REG_LOCAL].regs[from].offset,8*(to-from));
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
  	  MemoryDialogUpdate(hRegisters[REG_SPECIAL],&register_insp[REG_SPECIAL],register_insp[REG_SPECIAL].regs[from].offset,8*(to-from));
  else if (to>=G)
  	  MemoryDialogUpdate(hRegisters[REG_GLOBAL],&register_insp[REG_GLOBAL],register_insp[REG_GLOBAL].regs[from].offset,8*(to-from));
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
  {"Registerstack",256*8,get_local_mem,load_local_mem,store_local_mem,hex_format, octa_chunk,-1,8,0,reg_names},
  {"Special Registers",32*8,get_global_mem,load_global_mem,store_global_mem,hex_format, octa_chunk,-1,8,32,special_reg_names},
 {NULL}
};

void set_register_inspectors(void)
{ register_insp[REG_LOCAL].num_regs=L;
  register_insp[REG_GLOBAL].num_regs=256-G;
  register_insp[REG_GLOBAL].regs=&reg_names[G];
}

void new_register_view(int i)
{ int k;
  if (i<0 || i>=MAXREG) return;
  if (reg_names[0].name==NULL){
	reg_names_init();
	set_special_reg_name();
  }
  if (hRegisters[i]!=NULL) return;
  for (k=i-1;k>=0&&hRegisters[k]==NULL;k--)
	  continue;
  if (k<0)
    for (k=i+1;k<MAXREG&&hRegisters[k]==NULL;k++)
	  continue;
  if (k>=MAXREG)
	 sp_create_options(0,1,0.2,NULL);
  else if (k<i)
	 sp_create_options(0,0,0.5,hRegisters[k]);
  else
	 sp_create_options(1,0,0.5,hRegisters[k]);
  hRegisters[i] = CreateMemoryDialog(hInst,hSplitter);
  SetInspector(hRegisters[i], &register_insp[i]);
  set_register_inspectors();
  ShowWindow(hRegisters[i],SW_SHOW);
}




void memory_update(void)
{ int i;
  for (i=0; i<MAXMEM; i++)
	if(hMemory[i])
	  MemoryDialogUpdate(hMemory[i],&memory_insp[i], 0,memory_insp[i].size );
  set_register_inspectors();
  for (i=0; i<MAXREG; i++)
	if(hRegisters[i])
	  MemoryDialogUpdate(hRegisters[i],&register_insp[i], 0,register_insp[i].size );
}


#define MAXLINE (1<<11)
static octa linetab[MAXLINE];

octa line_to_loc(unsigned char file, int line)
/* returns -1 if no location was found 
   currently only for file==0 and line_no < (1<<11)
*/
{ octa loc;
  if (file!=0 || line > MAXLINE)
	  loc.h=loc.l=0xFFFFFFFF;
  else
	  loc = linetab[line];
  return loc;
}

void add_line_loc(unsigned char file, int line, octa loc)
{  if (file!=0 || line > MAXLINE) return;
   if (linetab[line].h<loc.h ||
		(linetab[line].h==loc.h &&linetab[line].l<loc.l))
		return;
   linetab[line]=loc;
}

int set_breakpoint(unsigned char file, int line_no)
/* return true if breakpoint could be set */
{ octa loc;
  mem_tetra *ll;
  loc= line_to_loc(file,line_no);
  if (loc.h==0xFFFFFFFF&&loc.l==0xFFFFFFFF)
	  return 0;
  if (mem_root==NULL || last_mem==NULL)
	  return 0;
  ll= mem_find(loc);
  ll->bkpt |= exec_bit;
  return 1;
}
int del_breakpoint(unsigned char file, int line_no)
{ octa loc;
  mem_tetra *ll;
  loc= line_to_loc(file,line_no);
  if (loc.h==0xFFFFFFFF&&loc.l==0xFFFFFFFF)
	  return 0;
  ll= mem_find(loc);
  ll->bkpt &= ~exec_bit;
  return 1;
}

void clear_breakpoints(unsigned char file)
{ int i;
  if (file!=0) return;
  for (i=0;i<MAXLINE;i++)
	  linetab[i].h=linetab[i].l=0xFFFFFFFF;
}