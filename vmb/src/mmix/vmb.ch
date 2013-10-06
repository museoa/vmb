
@x
Simulation applies only to user programs, not to an operating system kernel.
Thus, all addresses must be nonnegative; ``privileged'' commands such as
\.{PUT}~\.{rK,z} or \.{RESUME}~\.1 or \.{LDVTS}~\.{x,y,z} are not allowed;
instructions should be executed only from addresses in segment~0
(addresses less than \Hex{2000000000000000}).
Certain special registers remain constant: $\rm rF=0$,
$\rm rK=\Hex{ffffffffffffffff}$,
$\rm rQ=0$;
$\rm rT=\Hex{8000000500000000}$,
$\rm rTT=\Hex{8000000600000000}$,
$\rm rV=\Hex{369c200400000000}$.

\bull
No trap interrupts are implemented, except for a few special cases of \.{TRAP}
that provide rudimentary input-output.
@^interrupts@>
@y
@z

@x
is equivalent to \.{-eff}, tracing all eight exceptions.
@y
is equivalent to \.{-eff}, tracing all eight exceptions.

\bull \.{-B[host:]port}\quad Connect to the bus on the given host and port.
Without a bus, the processor has no power suply, no
memory and no other devices. It will not function. The hostname is separated with
a colon from the port number. The hostname, together with the colon,
can be ommited; in this case the simulator will try a connection to localhost.
If the option is omited altogether, localhost is contacted at port 9002.
@z

@x
#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif
@y
#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif
#define _MMIX_SIM_

#pragma warning(disable : 4146 4267)

@ @<Glob...@>=
#include <time.h>

#include "address.h"
#include "mmix-bus.h"
#include "vmb.h"
#ifdef MMIXLIB
#include <setjmp.h>
#include "mmixlib.h"
#endif
@z

@x
@ @<Sub...@>=
void print_hex @,@,@[ARGS((octa))@];@+@t}\6{@>
void print_hex(o)
  octa o;
@y
@ @(libprint.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
//#include "address.h"
//#include "mmix-bus.h"
#include "mmixlib.h"

void print_hex(octa o)
@z





The external definitions of mmix-arith
should also go int a header file.

@x
@<Sub...@>=
extern octa zero_octa; /* |zero_octa.h=zero_octa.l=0| */
@y
@<Sub...@>=
#include "libarith.h"

@ @(libarith.h@>=
extern octa zero_octa; /* |zero_octa.h=zero_octa.l=0| */
@z

Fatal errors need to be handled.

@x
@d panic(m) {@+fprintf(stderr,"Panic: %s!\n",m);@+exit(-2);@+}
@<Initialize...@>=
if (shift_left(neg_one,1).h!=0xffffffff)
  panic("Incorrect implementation of type tetra");
@.Incorrect implementation...@>
@y
@d panic(m) {@+vmb_fatal_error(__LINE__,m);@+}
@<Set up persistent data@>=
if (shift_left(neg_one,1).h!=0xffffffff)
  return -1;
@.Incorrect implementation...@>
@z

@x
@<Sub...@>=
void print_int @,@,@[ARGS((octa))@];@+@t}\6{@>
void print_int(o)
  octa o;
@y
@(libprint.c@>=

void print_int(octa o)
@z




@x
Each simulated tetrabyte has an associated frequency count and
source file reference.
@y
Each simulated tetrabyte has an associated frequency count and
source file reference.

We now read memory using some external simulator.
We provide |extern| functions defined in a separate file.
The functions we use are partly concerned with virtual
to physical address translation and contained in the
file address.h and address.c;
with access to a cache, contained in cache.h and cache.c;
and with access to the virtual bus contained in
mmix-bus.h and mmix-bus.c. 
At a later point the interface 
should be included here as a literate program.
@z

The tet field of the mem_tetra is therefore eliminated

@x
  tetra tet; /* the tetrabyte of simulated memory */
@y
@z

@x
@<Sub...@>=
mem_node* new_mem @,@,@[ARGS((void))@];@+@t}\6{@>
mem_node* new_mem()
@y
@(libmem.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libglobals.h"
#include "vmb.h"
#include "mmixlib.h"

mem_node* new_mem(void)
@z

@x
@<Initialize...@>=
mem_root=new_mem();
mem_root->loc.h=0x40000000;
last_mem=mem_root;
@y
@<Set up persistent data@>=
mem_root=new_mem();
mem_root->loc.h=0x40000000;
last_mem=mem_root;
@z

@x
@<Sub...@>=
mem_tetra* mem_find @,@,@[ARGS((octa))@];@+@t}\6{@>
mem_tetra* mem_find(addr)
  octa addr;
@y
@(libmem.c@>=
mem_tetra* mem_find(octa addr)
@z



@x
@d mm 0x98 /* the escape code of \.{mmo} format */
@y
@d mmo_esc 0x98 /* the escape code of \.{mmo} format */
@z

@x
@ We do not load the symbol table. (A more ambitious simulator could
implement \.{MMIXAL}-style expressions for interactive debugging,
but such enhancements are left to the interested reader.)

@<Initialize everything@>=
mmo_file=fopen(mmo_file_name,"rb");
if (!mmo_file) {
  register char *alt_name=(char*)calloc(strlen(mmo_file_name)+5,sizeof(char));
  if (!alt_name) panic("Can't allocate file name buffer");
@.Can't allocate...@>
  sprintf(alt_name,"%s.mmo",mmo_file_name);
  mmo_file=fopen(alt_name,"rb");
  if (!mmo_file) {
    fprintf(stderr,"Can't open the object file %s or %s!\n",
@.Can't open...@>
               mmo_file_name,alt_name);
    exit(-3);
  }
  free(alt_name);
}
@y
@ We do not load the symbol table. (A more ambitious simulator could
implement \.{MMIXAL}-style expressions for interactive debugging,
but such enhancements are left to the interested reader.)

@<Load object file@>=
#ifdef MMIXLIB
if (mmo_file_name!=NULL && mmo_file_name[0]!=0)
{ mmo_file=fopen(mmo_file_name,"rb");
  if (!mmo_file) {panic("Can't open mmo file");}
#else
#define mmo_file_name *cur_arg
if (mmo_file_name!=NULL && mmo_file_name[0]!=0)
{ mmo_file=fopen(mmo_file_name,"rb");
  if (!mmo_file) {
    register char *alt_name=(char*)calloc(strlen(mmo_file_name)+5,sizeof(char));
    if (!alt_name) panic("Can't allocate file name buffer");
@.Can't allocate...@>
    sprintf(alt_name,"%s.mmo",mmo_file_name);
    mmo_file=fopen(alt_name,"rb");
    if (!mmo_file) {
      fprintf(stderr,"Can't open the object file %s or %s!\n",
@.Can't open...@>
               mmo_file_name,alt_name);
      exit(-3);
    }
    free(alt_name);
  }
#endif
@z

@x
@d mmo_err {
     fprintf(stderr,"Bad object file! (Try running MMOtype.)\n");
@.Bad object file@>
     exit(-4);
   }
@y   
@d mmo_err {panic("Bad object file! (Try running MMOtype.)");}
@z

@x
@<Sub...@>=
void read_tet @,@,@[ARGS((void))@];@+@t}\6{@>
void read_tet()
@y
@(libload.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
#include "address.h"
#include "mmix-bus.h"
#include "mmixlib.h"

void read_tet(void)
@z

@x
if (buf[0]!=mm || buf[1]!=lop_pre) mmo_err;
@y
if (buf[0]!=mmo_esc || buf[1]!=lop_pre) mmo_err;
@z

@x
 loop:@+if (buf[0]==mm) switch (buf[1]) {
@y
 loop:@+if (buf[0]==mmo_esc) switch (buf[1]) {
@z

@x
@d mmo_load(loc,val) ll=mem_find(loc), ll->tet^=val
@y
@d mmo_load(loc,val) ll=load_mem_tetra(loc,val)

@(libload.c@>=
static mem_tetra *load_mem_tetra(octa loc, tetra val)  
{ octa x; 
  mem_tetra *ll=mem_find(loc);           
  load_data(4,&x,loc,0);     
  x.l = x.l^val;             
  if (!store_data(4,x,loc))  
  panic("Unable to store mmo file to RAM");
  return ll;
}

@ This function is used here:

@z

before we increment the line number,
we reset the frequency.

@x
    cur_line++;
@y
    ll->freq=0;
    cur_line++;
@z

The next lines of code complete loading the object file.

@x
@ @<Initialize...@>=
cur_loc.h=cur_loc.l=0;
cur_file=-1;
cur_line=0;
@<Load the preamble@>;
do @<Load the next item@>@;@+while (!postamble);
@<Load the postamble@>;
fclose(mmo_file);
cur_line=0;
@y
@ @<Load object file@>=
cur_loc.h=cur_loc.l=0;
cur_file=-1;
cur_line=0;
postamble=0;
@<Load the preamble@>;
do @<Load the next item@>@;@+while (!postamble);
@<Load the postamble@>;
fclose(mmo_file);
cur_line=0;
write_all_data_cache();
clear_all_instruction_cache();
}
@z

we have to map the file number stored in
the mmo file as the ybyte to the 
filenumbers used inside the library.

First the case of known files.

@x
case lop_file:@+if (file_info[ybyte].name) {
   if (zbyte) mmo_err;
   cur_file=ybyte;
@y
case lop_file:
   if (ybyte2file_no[ybyte]>=0) {
   if (zbyte) mmo_err;
   cur_file=ybyte2file_no[ybyte];
@z

Now we handle new files.

@x
 }@+else {
   if (!zbyte) mmo_err;
   file_info[ybyte].name=(char*)calloc(4*zbyte+1,1);
   if (!file_info[ybyte].name) {
     fprintf(stderr,"No room to store the file name!\n");@+exit(-5);
@.No room...@>
   }
   cur_file=ybyte;
   for (j=zbyte,p=file_info[ybyte].name; j>0; j--,p+=4) {
     read_tet();
     *p=buf[0];@+*(p+1)=buf[1];@+*(p+2)=buf[2];@+*(p+3)=buf[3];
   }
 }
@y
 }@+else {
   char *name;
   if (!zbyte) mmo_err;
   name=(char*)calloc(4*zbyte+1,1);
   if (!name) {
     panic("No room to store the file name!\n");
@.No room...@>
   }   
   for (j=zbyte,p=name; j>0; j--,p+=4) {
     read_tet();
     *p=buf[0];@+*(p+1)=buf[1];@+*(p+2)=buf[2];@+*(p+3)=buf[3];
   }
   cur_file=filename2file(name);
   ybyte2file_no[ybyte]=cur_file;
   file_info[cur_file].name=name;
 }
@z


@x
   if (buf[0]==mm) {
@y
   if (buf[0]==mmo_esc) {
@z


@x
@ Since a chunk of memory holds 512 tetrabytes, the |ll| pointer in the
following loop stays in the same chunk (namely, the first chunk
of segment~3, also known as \.{Stack\_Segment}).
@:Stack_Segment}\.{Stack\_Segment@>
@:Pool_Segment}\.{Pool\_Segment@>
@y
@ We load the postamble into the beginning
of segment~3, also known as \.{Stack\_Segment}).
@:Stack_Segment}\.{Stack\_Segment@>
@:Pool_Segment}\.{Pool\_Segment@>
The stack segment is set up to be used with an unsave instruction.
On the stack, we have, the local registers (argc and argv) and the value of rL, then the global 
registers and the special registers rB, rD, rE, rH, rJ, rM, rR, rP, rW, rX, rY, and rZ,
followed by rG and rA packed into eight byte.
@z

@x
@<Load the postamble@>=
aux.h=0x60000000;@+ aux.l=0x18;
ll=mem_find(aux);
(ll-1)->tet=2; /* this will ultimately set |rL=2| */
(ll-5)->tet=argc; /* and $\$0=|argc|$ */
(ll-4)->tet=0x40000000;
(ll-3)->tet=0x8; /* and $\$1=\.{Pool\_Segment}+8$ */
G=zbyte;@+ L=0;@+ O=0;
for (j=G+G;j<256+256;j++,ll++,aux.l+=4) read_tet(), ll->tet=tet;
inst_ptr.h=(ll-2)->tet, inst_ptr.l=(ll-1)->tet; /* \.{Main} */
(ll+2*12)->tet=G<<24;
g[255]=incr(aux,12*8); /* we will |UNSAVE| from here, to get going */
@y
@<Load the postamble@>=
aux.h=0x60000000;
{ octa x;
  x.h=0;@+x.l=0;@+aux.l=0x00;
  if (!store_data(8,x,aux)) /* $\$0=|argc|$ */
     panic("Unable to store mmo file to RAM");
  x.h=0x40000000;@+x.l=0x8;@+aux.l=0x08;
  if (!store_data(8,x,aux)) /* and $\$1=\.{Pool\_Segment}+8$ */
     panic("Unable to store mmo file to RAM");
  x.h=0;@+x.l=2;@+aux.l=0x10;
  if (!store_data(8,x,aux)) /* this will ultimately set |rL=2| */
     panic("Unable to store mmo file to RAM");
  G=zbyte;@+ L=0;@+ O=0;
  aux.l=0x18;
  for (j=G;j<256;j++,aux.l+=8) 
  { read_tet(); x.h=tet;
    read_tet(), x.l=tet;
    if (!store_data(8,x,aux))
       panic("Unable to store mmo file to RAM");
  }
  g[rWW] = x;  /* last octa stored is address of \.{Main} */
//  if (interacting) set_break(x,exec_bit);
  g[rXX].h = 0; g[rXX].l = 0xFB0000FF; /* |UNSAVE| \$255 */
  g[rBB]=aux=incr(aux,12*8); /* we can |UNSAVE| from here, to get going */
  x.h=G<<24; x.l=0 /* rA */; 
  if (!store_data(8,x,aux))
     panic("Unable to store mmo file to RAM");
}
@z

Because mmo files have there own notion of
file numbers, we have to map mmo file numbers
to file numbers used in mem and in file_nodes.

@x
file_node file_info[256]; /* data about each source file */
@y
file_node file_info[256]; /* data about each source file */
int ybyte2file_no[256]; /* mapping internal to external files */
@z


@x
@<Initialize...@>=
if (buf_size<72) buf_size=72;
buffer=(Char*)calloc(buf_size+1,sizeof(Char));
if (!buffer) panic("Can't allocate source line buffer");
@.Can't allocate...@>
@y
@<Set up persistent data@>=
if (buf_size<72) buf_size=72;
buffer=(Char*)calloc(buf_size+1,sizeof(Char));
if (!buffer) return -1;
@z

@x
@<Sub...@>=
void make_map @,@,@[ARGS((void))@];@+@t}\6{@>
void make_map()
@y
@(libshowline.c@>=
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
@h
@<Preprocessor macros@>@;
@<Showline macros@>@;
@<Type declarations@>@;
//#include "libarith.h"
#include "libglobals.h"
//#include "address.h"
#include "vmb.h"
#include "mmixlib.h"

void make_map(void)
@z

these include files are needed only in libshowline.c

@x
@<Preprocessor macros@>=
@y
@<Showline macros@>=
@z

@x
@<Sub...@>=
void print_line @,@,@[ARGS((int))@];@+@t}\6{@>
void print_line(k)
  int k;
@y
@(libshowline.c@>=
void print_line(int k)
@z


@x
@ @<Preprocessor macros@>=
@y
@ @<Showline macros@>=
@z

@x
@<Sub...@>=
void show_line @,@,@[ARGS((void))@];@+@t}\6{@>
void show_line()
@y
@(libshowline.c@>=
void show_line(void)
@z

@x
@<Sub...@>=
void print_freqs @,@,@[ARGS((mem_node*))@];@+@t}\6{@>
void print_freqs(p)
  mem_node *p;
@y
@(libprofile.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
#include "address.h"
#include "mmixlib.h"

void print_freqs(mem_node *p)
@z

@x
 loc_implied: printf("%10d. %08x%08x: %08x (%s)\n",
      p->dat[j].freq, cur_loc.h, cur_loc.l, p->dat[j].tet,
      info[p->dat[j].tet>>24].name);
@y
 loc_implied:
 { tetra inst; 
   load_instruction(&inst,cur_loc);
   printf("%10d. %08x%08x: %08x (%s)\n",
      p->dat[j].freq, cur_loc.h, cur_loc.l, inst,
      info[inst>>24].name);
 }
@z

For the mmixlib, we split performing the
instruction in three parts: 
resuming,fetching, and executing.

@x
@<Perform one instruction@>=
{
  if (resuming) loc=incr(inst_ptr,-4), inst=g[rX].l;
  else @<Fetch the next instruction@>;
@y
@<Perform one instruction@>=
{
  if (resuming)
  { loc=incr(inst_ptr,-4), inst=g[rzz?rXX:rX].l;
    if (rzz==0) /* RESUME 0 */
    { if ((loc.h&sign_bit) != (inst_ptr.h&sign_bit))
      { resuming = false;
        goto protection_violation;
      }
      @<Check for security violation@>
    }
  }
@z

This restriction is no longer necessary.

@x
  if (loc.h>=0x20000000) goto privileged_inst;
@y
@z

the next two lines are not a propper part of
performing an instruction and move to a separate 
function respectively to the main loop.
But we check for traps.
@x
  @<Trace the current instruction, if requested@>;
  if (resuming && op!=RESUME) resuming=false;
@y
@z

@x
int rop; /* ropcode of a resumed instruction */
@y
int rop; /* ropcode of a resumed instruction */
int rzz; /* Z field of a resumed instruction */
@z

@x
bool interacting; /* are we in interactive mode? */
@y
bool interacting; /* are we in interactive mode? */
bool show_operating_system = false; /* do we show negative addresses */
bool interact_after_resume = false;
#ifdef MMIXLIB
extern int port; /* on which port to connect to the bus */
extern char *host; /* on which host to connect to the bus */
#else
char localhost[]="localhost";
int port=9002; /* on which port to connect to the bus */
char *host=localhost; /* on which host to connect to the bus */
#endif
@z

We make some more variables global.

@x
@ @<Local...@>=
register mmix_opcode op; /* operation code of the current instruction */
register int xx,yy,zz,yz; /* operand fields of the current instruction */
register tetra f; /* properties of the current |op| */
@y
@ @<Glob...@>=
mmix_opcode op; /* operation code of the current instruction */
tetra f; /* properties of the current |op| */
int xx,yy,zz,yz; /* operand fields of the current instruction */

@ @<Local...@>=
@z

and p is no longer needed in performing an instruction.

@x
register char *p; /* current place in a string */
@y
@z


@x
@ @<Fetch the next instruction@>=
{
  loc=inst_ptr;
  ll=mem_find(loc);
  inst=ll->tet;
  cur_file=ll->file_no;
  cur_line=ll->line_no;
  ll->freq++;
  if (ll->bkpt&exec_bit) breakpoint=true;
  tracing=breakpoint||(ll->bkpt&trace_bit)||(ll->freq<=trace_threshold);
  inst_ptr=incr(inst_ptr,4);
}
@y
@ @<Fetch the next instruction@>=
{ loc=inst_ptr;
  ll=mem_find(loc);
  cur_file=ll->file_no;
  cur_line=ll->line_no;
  ll->freq++;
  if (ll->bkpt&exec_bit) breakpoint=true;
  tracing=breakpoint||(ll->bkpt&trace_bit)||(ll->freq<=trace_threshold);
  inst=SWYM<<24; /* default SWYM */
  @<Check for security violation@>
  if(!load_instruction(&inst,loc)) 
    goto page_fault;
  inst_ptr=incr(inst_ptr,4);
  if ((inst_ptr.h&sign_bit) && !(loc.h&sign_bit))
    goto protection_violation;
}
@z

We change how to display certein instructions.

@x
{"RESUME",0x00,0,0,5,"{%#b} -> %#z"},@|
{"SAVE",0x20,0,20,1,"%l = %#x"},@|
{"UNSAVE",0x82,0,20,1,"%#z: rG=%x, ..., rL=%a"},@|
{"SYNC",0x01,0,0,1,""},@|
{"SWYM",0x00,0,0,1,""},@|
@y
{"RESUME",0x00,0,0,5,"{%#b}, $255 = %x, -> %#z"},@|
{"SAVE",0x20,0,20,1,"%l = %#x"},@|
{"UNSAVE",0x82,0,20,1,"%#z: rG=%x, ..., rL=%a"},@|
{"SYNC",0x01,0,0,1,"%z"},@|
{"SWYM",0x01,0,0,1,"%r"},@|
@z


@x
@ @<Local...@>=
register int G,L,O; /* accessible copies of key registers */
@y
@ @<Glob...@>=
int G=255,L=0,O=0; /* accessible copies of key registers */
@z

Initialization needs to be done at each reboot.

@x
@<Initialize...@>=
g[rK]=neg_one;
g[rN].h=(VERSION<<24)+(SUBVERSION<<16)+(SUBSUBVERSION<<8);
g[rN].l=ABSTIME; /* see comment and warning above */
g[rT].h=0x80000005;
g[rTT].h=0x80000006;
g[rV].h=0x369c2004;
if (lring_size<256) lring_size=256;
lring_mask=lring_size-1;
if (lring_size&lring_mask)
  panic("The number of local registers must be a power of 2");
@.The number of local...@>
l=(octa*)calloc(lring_size,sizeof(octa));
if (!l) panic("No room for the local registers");
@.No room...@>
cur_round=ROUND_NEAR;
@y
@<Set up persistent data@>=
if (lring_size<256) lring_size=256;
lring_mask=lring_size-1;
if (lring_size&lring_mask) return -1;
l=(octa*)calloc(lring_size,sizeof(octa));
if (!l) return -1;

@ @<Initialize...@>=
sclock.l=sclock.h=0;
profile_started=false;
halted=false;
stdin_buf_start=stdin_buf_end=NULL;
good_guesses=bad_guesses=0;
profiling=false;
interrupt=false;

@ @<Boot the machine@>=
clear_all_data_vtc();
clear_all_instruction_vtc();
clear_all_data_cache();
clear_all_instruction_cache();
memset(l,0,sizeof(l));
memset(g,0,sizeof(g));
L=O=S=0;
g[rN].h=(VERSION<<24)+(SUBVERSION<<16)+(SUBSUBVERSION<<8);
g[rN].l=ABSTIME; /* see comment and warning above */
g[rT].h=0x80000000;g[rT].l=0x00000000;
g[rTT].h=0x80000000;g[rTT].l=0x00000000;
G=g[rG].l=255;
g[rV].h=0x12340D00;
g[rV].l=0x00002000;
cur_round=ROUND_NEAR;
@z

@x
  if (((S-O-L)&lring_mask)==0) stack_store();
@y
  if (((S-O-L)&lring_mask)==0) stack_store(l[S&lring_mask]);
@z

stack_store must implement the rC register.

@x
@<Sub...@>=
void stack_store @,@,@[ARGS((void))@];@+@t}\6{@>
void stack_store()
{
  register mem_tetra *ll=mem_find(g[rS]);
  register int k=S&lring_mask;
  ll->tet=l[k].h;@+test_store_bkpt(ll);
  (ll+1)->tet=l[k].l;@+test_store_bkpt(ll+1);
  if (stack_tracing) {
    tracing=true;
    if (cur_line) show_line();
    printf("             M8[#%08x%08x]=l[%d]=#%08x%08x, rS+=8\n",
              g[rS].h,g[rS].l,k,l[k].h,l[k].l);
  }
  g[rS]=incr(g[rS],8),  S++;
}
@y
@<Stack store@>=
void stack_store @,@,@[ARGS((octa))@];@+@t}\6{@>
void stack_store(x)
  octa x;
{ unsigned int pw_bit, new_pw_bit;
  mem_tetra *ll;
  pw_bit=g[rQ].h&PW_BIT;
  new_pw_bit=new_Q.h&PW_BIT;
  if(!store_data(8,x,g[rS])) /* implementing the rC register */
  {   /* set CP_BIT */
      g[rQ].l |= CP_BIT;
      new_Q.l |= CP_BIT;
      if (g[rC].l&0x02)  /* Write bit */
      {  int s;
         octa address, base, offset,mask;
         mask.h=mask.l=0xFFFFFFFF;
         g[rQ].h &=~PW_BIT;  /* restore PW_BIT */
         new_Q.h &=~PW_BIT;
         g[rQ].h |= pw_bit;
         new_Q.h |= new_pw_bit;
         s    = (g[rV].h>>8)&0xFF;  /* extract the page size from rV */
         mask = shift_left(mask,s);
         offset.h = g[rS].h&~mask.h,offset.l = g[rS].l&~mask.l;
         mask.h &= 0x0000FFFF;     /* reduce mask to 48 bits */
         base.h = g[rC].h&mask.h,base.l = g[rC].l&mask.l;
         address.h=base.h|offset.h,address.l=base.l|offset.l;
         store_data(8,x,address);
      }
  }
  ll=mem_find(g[rS]);
  test_store_bkpt(ll);
  test_store_bkpt(ll+1);
  if (stack_tracing) {
    tracing=true;
    printf("             M8[#%08x%08x]=#%08x%08x, rS+=8\n",
              g[rS].h,g[rS].l,x.h,x.l);
  }
  g[rS]=incr(g[rS],8),  S++;
}

@ @<Sub...@>=
@<Stack store@>@;
@z

Same with stack load.
@x
@<Sub...@>=
void stack_load @,@,@[ARGS((void))@];@+@t}\6{@>
@y
@<Sub...@>=
@<Stack load@>@;

@ @<Stack load@>=
void stack_load @,@,@[ARGS((void))@];@+@t}\6{@>
@z

@x
  l[k].h=ll->tet;@+test_load_bkpt(ll);
  l[k].l=(ll+1)->tet;@+test_load_bkpt(ll+1);
@y
  test_load_bkpt(ll);@+test_load_bkpt(ll+1);
  load_data(8,&l[k],g[rS],0);
@z

@x
    if (cur_line) show_line();
@y
@z

@x
@<Sub...@>=
int register_truth @,@,@[ARGS((octa,mmix_opcode))@];@+@t}\6{@>
@y
@<Sub...@>=
@<Register truth@>@;

@ @<Register truth@>=
int register_truth @,@,@[ARGS((octa,mmix_opcode))@];@+@t}\6{@>
@z

@x
   inst_ptr=z;
@y
   if ((z.h&sign_bit) && !(loc.h&sign_bit))
   goto protection_violation;
   inst_ptr=z;
@z


@x
fin_ld: ll=mem_find(w);@+test_load_bkpt(ll);
 x.h=ll->tet;
 x=shift_right(shift_left(x,j),i,op&0x2);
check_ld:@+if (w.h&sign_bit) goto privileged_inst; 
@y
fin_ld: ll=mem_find(w);@+test_load_bkpt(ll);
 if ((w.h&sign_bit) && !(loc.h&sign_bit)) goto translation_bypassed_inst;
 if (!load_data(4,&x,w,0)) goto page_fault;
 x=shift_right(shift_left(x,j+32),i,op&0x2);
check_ld:@+ if ((w.h&sign_bit) && !(loc.h&sign_bit)) goto translation_bypassed_inst;
 goto store_x;
page_fault:
 if ((g[rK].h & g[rQ].h) != 0 || (g[rK].l & g[rQ].l) != 0) 
 { x.h=0, x.l=inst;
   y = w;
   z = zero_octa;
   @<Initiate a trap interrupt@>
   inst_ptr=y=g[rTT];
 }
 break;
@z

@x
case LDO: case LDOI: case LDOU: case LDOUI: case LDUNC: case LDUNCI:@/
 w.l&=-8;@+ ll=mem_find(w);
 test_load_bkpt(ll);@+test_load_bkpt(ll+1);
 x.h=ll->tet;@+ x.l=(ll+1)->tet;
 goto check_ld;
case LDSF: case LDSFI: ll=mem_find(w);@+test_load_bkpt(ll);
 x=load_sf(ll->tet);@+ goto check_ld;
@y
case LDO: case LDOI: case LDOU: case LDOUI: 
 w.l&=-8;@+ ll=mem_find(w);
 test_load_bkpt(ll);@+test_load_bkpt(ll+1);
 if ((w.h&sign_bit) && !(loc.h&sign_bit)) goto translation_bypassed_inst;
 if (!load_data(8,&x,w,0)) goto page_fault;
 goto check_ld;
case LDUNC: case LDUNCI:
 w.l&=-8;@+ ll=mem_find(w);
 test_load_bkpt(ll);@+test_load_bkpt(ll+1);
 if ((w.h&sign_bit) && !(loc.h&sign_bit)) goto translation_bypassed_inst;
 if (!load_data_uncached(8,&x,w,0)) goto page_fault;
 goto check_ld;
case LDSF: case LDSFI: ll=mem_find(w);@+test_load_bkpt(ll);
 if ((w.h&sign_bit) && !(loc.h&sign_bit)) goto translation_bypassed_inst;
 if (!load_data(4,&x,w,0)) goto page_fault;
 x=load_sf(x.l);@+ goto check_ld;
@z


@x
case STB: case STBI: case STBU: case STBUI:@/
 i=56;@+j=(w.l&0x3)<<3; goto fin_pst;
case STW: case STWI: case STWU: case STWUI:@/
 i=48;@+j=(w.l&0x2)<<3; goto fin_pst;
case STT: case STTI: case STTU: case STTUI:@/
 i=32;@+j=0;
fin_pst: ll=mem_find(w);
 if ((op&0x2)==0) {
   a=shift_right(shift_left(b,i),i,0);
   if (a.h!=b.h || a.l!=b.l) exc|=V_BIT;
 }
 ll->tet^=(ll->tet^(b.l<<(i-32-j))) & ((((tetra)-1)<<(i-32))>>j);
 goto fin_st;
case STSF: case STSFI: ll=mem_find(w);
 ll->tet=store_sf(b);@+exc=exceptions;
 goto fin_st;
case STHT: case STHTI: ll=mem_find(w);@+ ll->tet=b.h;
fin_st: test_store_bkpt(ll);
 w.l&=-8;@+ll=mem_find(w);
 a.h=ll->tet;@+ a.l=(ll+1)->tet; /* for trace output */
 goto check_st; 
case STCO: case STCOI: b.l=xx;
case STO: case STOI: case STOU: case STOUI: case STUNC: case STUNCI:
 w.l&=-8;@+ll=mem_find(w);
 test_store_bkpt(ll);@+ test_store_bkpt(ll+1);
 ll->tet=b.h;@+ (ll+1)->tet=b.l;
check_st:@+if (w.h&sign_bit) goto privileged_inst;
 break;
@y
case STB: case STBI: case STBU: case STBUI:@/
 i=56;@+j=1; goto fin_pst;
case STW: case STWI: case STWU: case STWUI:@/
 i=48;@+j=2; goto fin_pst;
case STT: case STTI: case STTU: case STTUI:@/
 i=32;@+j=4; goto fin_pst;
fin_pst: 
 if ((op&0x2)==0) {
   a=shift_right(shift_left(b,i),i,0);
   if (a.h!=b.h || a.l!=b.l) exc|=V_BIT;
 }
fin_st:@+  if ((w.h&sign_bit) && !(loc.h&sign_bit)) goto translation_bypassed_inst;
 if (!store_data(j,b,w)) goto page_fault;
 ll=mem_find(w); test_store_bkpt(ll);
 if(j>4) test_store_bkpt(ll+1);
 break;
case STSF: case STSFI: 
 b.l = store_sf(b);@+exc=exceptions;
 j=4;
 goto fin_st;
case STHT: case STHTI: 
  b.l=b.h;
  j=4;
  goto fin_st;
case STCO: case STCOI: b.h=0; b.l=xx;
case STO: case STOI: case STOU: case STOUI: 
 j = 8;w.l&=-8;
 goto fin_st;
case STUNC: case STUNCI:
 j = 8;w.l&=-8;
 if ((w.h&sign_bit) && !(loc.h&sign_bit)) goto translation_bypassed_inst;
 if (!store_data_uncached(j,b,w)) goto page_fault;
 ll=mem_find(w); test_store_bkpt(ll);test_store_bkpt(ll+1);
 break;
@z

@x
case CSWAP: case CSWAPI: w.l&=-8;@+ll=mem_find(w);
 test_load_bkpt(ll);@+test_load_bkpt(ll+1);
 a=g[rP];
 if (ll->tet==a.h && (ll+1)->tet==a.l) {
   x.h=0, x.l=1;
   test_store_bkpt(ll);@+test_store_bkpt(ll+1);
   ll->tet=b.h, (ll+1)->tet=b.l;
   strcpy(rhs,"M8[%#w]=%#b");
 }@+else {
   b.h=ll->tet, b.l=(ll+1)->tet;
   g[rP]=b;
   strcpy(rhs,"rP=%#b");
 }
@y
case CSWAP: case CSWAPI: w.l&=-8;@+ll=mem_find(w);
 test_load_bkpt(ll);@+test_load_bkpt(ll+1);
 if ((w.h&sign_bit) && !(loc.h&sign_bit)) goto  translation_bypassed_inst;
 if (!load_data(8,&a,w,0)) goto page_fault;
 if (g[rP].h==a.h && g[rP].l==a.l) {
   x.h=0, x.l=1;
   test_store_bkpt(ll);@+test_store_bkpt(ll+1);
   if (!store_data(8,b,w)) goto page_fault;
   strcpy(rhs,"M8[%#w]=%#b");
 }@+else {
   b=a;
   a = g[rP];
   g[rP]=b;
   x.h=0, x.l=0;
   strcpy(rhs,"rP=%#b");
 }
@z


@x
case GET:@+if (yy!=0 || zz>=32) goto illegal_inst;
  x=g[zz];
  goto store_x;
case PUT: case PUTI:@+ if (yy!=0 || xx>=32) goto illegal_inst;
  strcpy(rhs,"%z = %#z");
  if (xx>=8) {
    if (xx<=11 && xx!=8) goto illegal_inst; /* can't change rN, rO, rS */
    if (xx<=18) goto privileged_inst;
    if (xx==rA) @<Get ready to update rA@>@;
    else if (xx==rL) @<Set $L=z=\min(z,L)$@>@;
    else if (xx==rG) @<Get ready to update rG@>;
  }
  g[xx]=z;@+zz=xx;@+break;
@y
case GET:@+if (yy!=0 || zz>=32) goto illegal_inst;
  x=g[zz];
  if (zz==rQ) { 
      new_Q.h = new_Q.l = 0;
  }
  goto store_x;
case PUT: case PUTI:@+ if (yy!=0 || xx>=32) goto illegal_inst;
  strcpy(rhs,"%z = %#z");
  if (xx>=8) {
    if (xx==9) goto illegal_inst; /* can't change rN */
    if (xx<=18 && !(loc.h&sign_bit)) goto privileged_inst;
    if (xx==rA) @<Get ready to update rA@>@;
    else if (xx==rL) @<Set $L=z=\min(z,L)$@>@;
    else if (xx==rG) @<Get ready to update rG@>@;
    else if (xx==rQ)
    { new_Q.h |= z.h &~ g[rQ].h;@+
      new_Q.l |= z.l &~ g[rQ].l;
      z.l |= new_Q.l;@+
      z.h |= new_Q.h;@+
    }
  }
  g[xx]=z;@+zz=xx;@+break;
@z

@x
case PUSHGO: case PUSHGOI: inst_ptr=w;@+goto push;
case PUSHJ: case PUSHJB: inst_ptr=z;
push:@+if (xx>=G) {
   xx=L++;
   if (((S-O-L)&lring_mask)==0) stack_store();
 }
@y
case PUSHGO: case PUSHGOI: 
if ((w.h&sign_bit) && !(loc.h&sign_bit))
goto protection_violation;
inst_ptr=w;@+goto push;
case PUSHJ: case PUSHJB: 
if ((z.h&sign_bit) && !(loc.h&sign_bit))
goto protection_violation;  
inst_ptr=z;
push:@+if (xx>=G) {
   xx=L++;
   if (((S-O-L)&lring_mask)==0) stack_store(l[S&lring_mask]);
 }
@z

@x
 y=g[rJ];@+ z.l=yz<<2;@+ inst_ptr=oplus(y,z);
@y
 y=g[rJ];@+ z.l=yz<<2;
 { octa tmp;
   tmp=oplus(y,z);
   if ((tmp.h&sign_bit) && !(loc.h&sign_bit))
   goto protection_violation;  
   inst_ptr = tmp;
}
@z

@x
case SAVE:@+if (xx<G || yy!=0 || zz!=0) goto illegal_inst;
 l[(O+L)&lring_mask].l=L, L++;
 if (((S-O-L)&lring_mask)==0) stack_store();
@y
case SAVE:@+if (xx<G || yy!=0 || zz!=0) goto illegal_inst;
 l[(O+L)&lring_mask].l=L, L++;
 if (((S-O-L)&lring_mask)==0) stack_store(l[S&lring_mask]);
@z

@x
 while (g[rO].l!=g[rS].l) stack_store();
@y
 while (g[rO].l!=g[rS].l) stack_store(l[S&lring_mask]);
@z

@x
@<Store |g[k]| in the register stack...@>=
ll=mem_find(g[rS]);
if (k==rZ+1) x.h=G<<24, x.l=g[rA].l;
else x=g[k];
ll->tet=x.h;@+test_store_bkpt(ll);
(ll+1)->tet=x.l;@+test_store_bkpt(ll+1);
if (stack_tracing) {
  tracing=true;
  if (cur_line) show_line();
  if (k>=32) printf("             M8[#%08x%08x]=g[%d]=#%08x%08x, rS+=8\n",
            g[rS].h,g[rS].l,k,x.h,x.l);
  else printf("             M8[#%08x%08x]=%s=#%08x%08x, rS+=8\n",
            g[rS].h,g[rS].l,k==rZ+1? "(rG,rA)": special_name[k],x.h,x.l);
}
S++, g[rS]=incr(g[rS],8);
@y
@<Store |g[k]| in the register stack...@>=
if (k==rZ+1) x.h=G<<24, x.l=g[rA].l;
else x=g[k];
stack_store(x);
@z

@x
@ @<Load |g[k]| from the register stack@>=
g[rS]=incr(g[rS],-8);
ll=mem_find(g[rS]);
test_load_bkpt(ll);@+test_load_bkpt(ll+1);
if (k==rZ+1) x.l=G=g[rG].l=ll->tet>>24, a.l=g[rA].l=(ll+1)->tet&0x3ffff;
else g[k].h=ll->tet, g[k].l=(ll+1)->tet;
if (stack_tracing) {
  tracing=true;
  if (cur_line) show_line();
  if (k>=32) printf("             rS-=8, g[%d]=M8[#%08x%08x]=#%08x%08x\n",
            k,g[rS].h,g[rS].l,ll->tet,(ll+1)->tet);
  else if (k==rZ+1) printf("             (rG,rA)=M8[#%08x%08x]=#%08x%08x\n",
            g[rS].h,g[rS].l,ll->tet,(ll+1)->tet);
  else printf("             rS-=8, %s=M8[#%08x%08x]=#%08x%08x\n",
            special_name[k],g[rS].h,g[rS].l,ll->tet,(ll+1)->tet);
}
@y
@ @<Load |g[k]| from the register stack@>=
g[rS]=incr(g[rS],-8);
ll=mem_find(g[rS]);
test_load_bkpt(ll);@+test_load_bkpt(ll+1);
if (k==rZ+1) 
{ if (!load_data(8,&a,g[rS],0)) { w=g[rS]; goto page_fault; }
  x.l=G=g[rG].l=a.h>>24;
  a.l=g[rA].l=a.l&0x3ffff;
}
else
 if (!load_data(8,&(g[k]),g[rS],0))  { w=g[rS]; goto page_fault; }
if (stack_tracing) {
  tracing=true;
  if (k>=32) printf("             rS-=8, g[%d]=M8[#%08x%08x]=#%08x%08x\n",
            k,g[rS].h,g[rS].l,g[k].h,g[k].l);
  else if (k==rZ+1) printf("             (rG,rA)=M8[#%08x%08x]=#%08x%08x\n",
            g[rS].h,g[rS].l,g[k].h,g[k].l);
  else printf("             rS-=8, %s=M8[#%08x%08x]=#%08x%08x\n",
            special_name[k],g[rS].h,g[rS].l,g[k].h,g[k].l);
}
@z

@x
@ The cache maintenance instructions don't affect this simulation,
because there are no caches. But if the user has invoked them, we do
provide a bit of information when tracing, indicating the scope of the
instruction.

@<Cases for ind...@>=
case SYNCID: case SYNCIDI: case PREST: case PRESTI:
case SYNCD: case SYNCDI: case PREGO: case PREGOI:
case PRELD: case PRELDI: x=incr(w,xx);@+break;
@y
@ The cache maintenance instructions do affect this simulation.

@<Cases for ind...@>=
case SYNCID: case SYNCIDI:
 delete_instruction(w,xx+1);
 if (loc.h&sign_bit)
   delete_data(w,xx+1);
 else
   write_data(w,xx+1);
 break;
case PREST: case PRESTI: x=incr(w,xx);@+break;
case SYNCD: case SYNCDI:  
 write_data(w,xx+1);
 if (loc.h&sign_bit)
   delete_data(w,xx+1);
 break;
case PREGO: case PREGOI:
 prego_instruction(w,xx+1);
 break;
case PRELD: case PRELDI:
 preload_data_cache(w,xx+1);
 x=incr(w,xx);@+break;
@z

@x
case GO: case GOI: x=inst_ptr;@+inst_ptr=w;@+goto store_x;
case JMP: case JMPB: inst_ptr=z;
case SWYM: break;
case SYNC:@+if (xx!=0 || yy!=0 || zz>7) goto illegal_inst;
 if (zz<=3) break;
case LDVTS: case LDVTSI: privileged_inst: strcpy(lhs,"!privileged");
 goto break_inst;
illegal_inst: strcpy(lhs,"!illegal");
break_inst: breakpoint=tracing=true;
 if (!interacting && !interact_after_break) halted=true;
 break;
@y
case GO: case GOI: 
   if ((w.h&sign_bit) && !(loc.h&sign_bit))
   goto protection_violation;  
   x=inst_ptr;@+inst_ptr=w;@+goto store_x;
case JMP: case JMPB: 
   if ((z.h&sign_bit) && !(loc.h&sign_bit))
   goto protection_violation;  
   inst_ptr=z;@+break;
case SYNC:@+if (xx!=0 || yy!=0 || zz>7) goto illegal_inst;
/* should give a privileged instruction interrupt in case zz  >3 */
 else if (zz==4) /* power save mode */
 {  const unsigned int cycle_speed = 1000000; /* cycles per ms */
    const unsigned int max_wait = 100;
    int d, ms;
    if (g[rI].h!=0 || g[rI].l>max_wait*cycle_speed) /* large rI values */
      ms = max_wait;
    else
      ms = g[rI].l/cycle_speed;
    if (ms>0)
    {  d = vmb_wait_for_event_timed(&vmb,ms);
       g[rI]=incr(g[rI],-(ms-d)*cycle_speed);
     }
     else if (g[rI].l>1000)
        g[rI].l = g[rI].l-1000;
     else if (g[rI].l>100)
        g[rI].l = g[rI].l-100;
     else if (g[rI].l>10)
        g[rI].l = g[rI].l-10;
 }
 else if (zz==5) /* empty write buffer */
   write_all_data_cache();
 else if (zz==6) /* clear VAT cache */
 { clear_all_data_vtc();
   clear_all_instruction_vtc();
 }
 else if (zz==7) /* clear instruction and data cache */
 { clear_all_data_cache();
   clear_all_instruction_cache();
 }
 break;
case LDVTS: case LDVTSI:   
{ if (!(loc.h&sign_bit)) goto privileged_inst;
  if (w.h&sign_bit) goto illegal_inst;
  x = update_vtc(w);
  goto store_x;
}
break;
case SWYM:
 if ((inst&0xFFFFFF)!=0) 
 {   char buf[256+1];
     int n;
     strcpy(rhs,"$%x,%z");
     z.h=0, z.l=yz;
     x.h=0, x.l=xx;
     tracing=interacting;
     breakpoint=true;
     interrupt=false;
     @<Set |b| from register X@>;
     n=mmgetchars((unsigned char *)buf,256,b,0);
     buf[n]=0;
     if (n>6 && strncmp(buf,"DEBUG ",6)==0) 
     { fprintf(stdout,"\n%s!\n",buf+6);
       sprintf(rhs,"rF=#%08X%08X\n",g[rF].h, g[rF].l);
       tracing= true;
     }
 }
 else
   strcpy(rhs,"");
break;
translation_bypassed_inst: strcpy(lhs,"!absolute address");
g[rQ].h |= N_BIT; new_Q.h |= N_BIT; /* set the n bit */
 goto break_inst;
privileged_inst: strcpy(lhs,"!kernel only");
g[rQ].h |= K_BIT; new_Q.h |= K_BIT; /* set the k bit */
 goto break_inst;
illegal_inst: strcpy(lhs,"!broken");
g[rQ].h |= B_BIT; new_Q.h |= B_BIT; /* set the b bit */
 goto break_inst;
protection_violation: strcpy(lhs,"!protected");
g[rQ].h |= P_BIT; new_Q.h |= P_BIT; /* set the p bit */
 goto break_inst;
security_inst: strcpy(lhs,"!insecure");
break_inst: breakpoint=tracing=true;
 if (!interacting && !interact_after_break) halted=true;
break;
@z

@x
The |TRAP| instruction is not simulated, except for the system calls
mentioned in the introduction.

@<Cases for ind...@>=
case TRIP: exc|=H_BIT;@+break;
case TRAP:@+if (xx!=0 || yy>max_sys_call) goto privileged_inst;
 strcpy(rhs,trap_format[yy]);
 g[rWW]=inst_ptr;
 g[rXX].h=sign_bit, g[rXX].l=inst;
 g[rYY]=y, g[rZZ]=z;
 z.h=0, z.l=zz;
 a=incr(b,8);
 @<Prepare memory arguments $|ma|={\rm M}[a]$ and $|mb|={\rm M}[b]$ if needed@>;
 switch (yy) {
case Halt: @<Either halt or print warning@>;@+g[rBB]=g[255];@+break;
case Fopen: g[rBB]=mmix_fopen((unsigned char)zz,mb,ma);@+break;
case Fclose: g[rBB]=mmix_fclose((unsigned char)zz);@+break;
case Fread: g[rBB]=mmix_fread((unsigned char)zz,mb,ma);@+break;
case Fgets: g[rBB]=mmix_fgets((unsigned char)zz,mb,ma);@+break;
case Fgetws: g[rBB]=mmix_fgetws((unsigned char)zz,mb,ma);@+break;
case Fwrite: g[rBB]=mmix_fwrite((unsigned char)zz,mb,ma);@+break;
case Fputs: g[rBB]=mmix_fputs((unsigned char)zz,b);@+break;
case Fputws: g[rBB]=mmix_fputws((unsigned char)zz,b);@+break;
case Fseek: g[rBB]=mmix_fseek((unsigned char)zz,b);@+break;
case Ftell: g[rBB]=mmix_ftell((unsigned char)zz);@+break;
}
 x=g[255]=g[rBB];@+break;

@ @<Either halt or print warning@>=
if (!zz) halted=breakpoint=true;
else if (zz==1) {
  if (loc.h || loc.l>=0x90) goto privileged_inst;
  print_trip_warning(loc.l>>4,incr(g[rW],-4));
}@+else goto privileged_inst;
@y
The |TRAP| instruction prints nicely for some system calls.

@<Cases for ind...@>=
case TRIP: exc|=H_BIT;@+break;
case TRAP:@+if (xx==0 && yy<=max_sys_call) 
      { strcpy(rhs,trap_format[yy]);
        a=incr(b,8);
        @<Prepare memory arguments $|ma|={\rm M}[a]$ and $|mb|={\rm M}[b]$ if needed@>;
      }
     else strcpy(rhs, "%#x -> %#y");
 if (tracing && !show_operating_system) interact_after_resume = true;    
 x.h=sign_bit, x.l=inst;
 @<Initiate a trap interrupt@>
 inst_ptr=y=g[rT];
 break;
@z


@x
"$255 = Fopen(%!z,M8[%#b]=%#q,M8[%#a]=%p) = %x",
"$255 = Fclose(%!z) = %x",
"$255 = Fread(%!z,M8[%#b]=%#q,M8[%#a]=%p) = %x",
"$255 = Fgets(%!z,M8[%#b]=%#q,M8[%#a]=%p) = %x",
"$255 = Fgetws(%!z,M8[%#b]=%#q,M8[%#a]=%p) = %x",
"$255 = Fwrite(%!z,M8[%#b]=%#q,M8[%#a]=%p) = %x",
"$255 = Fputs(%!z,%#b) = %x",
"$255 = Fputws(%!z,%#b) = %x",
"$255 = Fseek(%!z,%b) = %x",
"$255 = Ftell(%!z) = %x"};
@y
"$255 = Fopen(%!z,M8[%#b]=%#q,M8[%#a]=%p) -> %#y",
"$255 = Fclose(%!z) -> %#y",
"$255 = Fread(%!z,M8[%#b]=%#q,M8[%#a]=%p) -> %#y",
"$255 = Fgets(%!z,M8[%#b]=%#q,M8[%#a]=%p) -> %#y",
"$255 = Fgetws(%!z,M8[%#b]=%#q,M8[%#a]=%p) -> %#y",
"$255 = Fwrite(%!z,M8[%#b]=%#q,M8[%#a]=%p) -> %#y",
"$255 = Fputs(%!z,%#b) -> %#y",
"$255 = Fputws(%!z,%#b) -> %#y",
"$255 = Fseek(%!z,%b) -> %#y",
"$255 = Ftell(%!z) -> %#y"};
@z



@x
@ @<Prepare memory arguments...@>=
if (arg_count[yy]==3) {
  ll=mem_find(b);@+test_load_bkpt(ll);@+test_load_bkpt(ll+1);
  mb.h=ll->tet, mb.l=(ll+1)->tet;
  ll=mem_find(a);@+test_load_bkpt(ll);@+test_load_bkpt(ll+1);
  ma.h=ll->tet, ma.l=(ll+1)->tet;
}
@y
@ @<Prepare memory arguments...@>=
if (arg_count[yy]==3) {
   load_data(8,&mb,b,0);
   load_data(8,&ma,a,0);
}
@z

@x
@ The input/output operations invoked by \.{TRAP}s are
done by subroutines in an auxiliary program module called {\mc MMIX-IO}.
Here we need only declare those subroutines, and write three primitive
interfaces on which they depend.

@ @<Glob...@>=
extern void mmix_io_init @,@,@[ARGS((void))@];
extern octa mmix_fopen @,@,@[ARGS((unsigned char,octa,octa))@];
extern octa mmix_fclose @,@,@[ARGS((unsigned char))@];
extern octa mmix_fread @,@,@[ARGS((unsigned char,octa,octa))@];
extern octa mmix_fgets @,@,@[ARGS((unsigned char,octa,octa))@];
extern octa mmix_fgetws @,@,@[ARGS((unsigned char,octa,octa))@];
extern octa mmix_fwrite @,@,@[ARGS((unsigned char,octa,octa))@];
extern octa mmix_fputs @,@,@[ARGS((unsigned char,octa))@];
extern octa mmix_fputws @,@,@[ARGS((unsigned char,octa))@];
extern octa mmix_fseek @,@,@[ARGS((unsigned char,octa))@];
extern octa mmix_ftell @,@,@[ARGS((unsigned char))@];
extern void print_trip_warning @,@,@[ARGS((int,octa))@];
extern void mmix_fake_stdin @,@,@[ARGS((FILE*))@];
@y
@z

@x
@<Sub...@>=
int mmgetchars @,@,@[ARGS((char*,int,octa,int))@];@+@t}\6{@>
int mmgetchars(buf,size,addr,stop)
  char *buf;
  int size;
  octa addr;
  int stop;
{
  register char *p;
  register int m;
  register mem_tetra *ll;
  register tetra x;
  octa a;
  for (p=buf,m=0,a=addr; m<size;) {
    ll=mem_find(a);@+test_load_bkpt(ll);
    x=ll->tet;
    if ((a.l&0x3) || m>size-4) @<Read and store one byte; |return| if done@>@;
    else @<Read and store up to four bytes; |return| if done@>@;
  }
  return size;
}
@y
@(libmmget.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "address.h"
#include "vmb.h"
#include "mmixlib.h"

int mmgetchars(buf,size,addr,stop)
  unsigned char *buf;
  int size;
  octa addr;
  int stop;
{
  register unsigned char *p;
  register int m;
  octa x;
  octa a;
  for (p=buf,m=0,a=addr; m<size;) {
    if ((a.l&0x7) || m+8>size) @<Read and store one byte; |return| if done@>@;
    else @<Read and store eight bytes; |return| if done@>@;
  }
  return size;
}
@z

@x
@ @<Read and store one byte...@>=
{
  *p=(x>>(8*((~a.l)&0x3)))&0xff;
  if (!*p && stop>=0) {
    if (stop==0) return m;
    if ((a.l&0x1) && *(p-1)=='\0') return m-1;
  }
  p++,m++,a=incr(a,1);
}

@ @<Read and store up to four bytes...@>=
{
  *p=x>>24;
  if (!*p && (stop==0 || (stop>0 && x<0x10000))) return m;
  *(p+1)=(x>>16)&0xff;
  if (!*(p+1) && stop==0) return m+1;
  *(p+2)=(x>>8)&0xff;
  if (!*(p+2) && (stop==0 || (stop>0 && (x&0xffff)==0))) return m+2;
  *(p+3)=x&0xff;
  if (!*(p+3) && stop==0) return m+3;
  p+=4,m+=4,a=incr(a,4);
}
@y
@ @<Read and store one byte...@>=
{ load_data(1,&x,a,0);
    *p=x.l&0xff;
  if (!*p && stop>=0) {
    if (stop==0) return m;
    if ((a.l&0x1) && *(p-1)=='\0') return m-1;
  }
  p++,m++,a=incr(a,1);
}

@ @<Read and store eight bytes...@>=
{ load_data(8,&x,a,0);
  *p=x.h>>24;
  if (!*p && (stop==0 || (stop>0 && x.h<0x10000))) return m;
  *(p+1)=(x.h>>16)&0xff;
  if (!*(p+1) && stop==0) return m+1;
  *(p+2)=(x.h>>8)&0xff;
  if (!*(p+2) && (stop==0 || (stop>0 && (x.h&0xffff)==0))) return m+2;
  *(p+3)=x.h&0xff;
  if (!*(p+3) && stop==0) return m+3;
  p+=4,m+=4,a=incr(a,4);
  *p=x.l>>24;
  if (!*p && (stop==0 || (stop>0 && x.l<0x10000))) return m;
  *(p+1)=(x.l>>16)&0xff;
  if (!*(p+1) && stop==0) return m+1;
  *(p+2)=(x.l>>8)&0xff;
  if (!*(p+2) && (stop==0 || (stop>0 && (x.l&0xffff)==0))) return m+2;
  *(p+3)=x.l&0xff;
  if (!*(p+3) && stop==0) return m+3;
  p+=4,m+=4,a=incr(a,4);
}
@z

@x      
@ The subroutine |mmputchars(buf,size,addr)| puts |size| characters
into the simulated memory starting at address |addr|.

@<Sub...@>=
void mmputchars @,@,@[ARGS((unsigned char*,int,octa))@];@+@t}\6{@>
void mmputchars(buf,size,addr)
  unsigned char *buf;
  int size;
  octa addr;
{
  register unsigned char *p;
  register int m;
  register mem_tetra *ll;
  octa a;
  for (p=buf,m=0,a=addr; m<size;) {
    ll=mem_find(a);@+test_store_bkpt(ll);
    if ((a.l&0x3) || m>size-4) @<Load and write one byte@>@;
    else @<Load and write four bytes@>;
  }
}
@y
@ The subroutine |mmputchars(buf,size,addr)| puts |size| characters
into the simulated memory starting at address |addr|.

@(libmmput.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "address.h"
#include "vmb.h"
#include "mmixlib.h"

void mmputchars(unsigned char *buf,int size,octa addr)
{
  register unsigned char *p;
  register int m;
  octa x;
  octa a;
  for (p=buf,m=0,a=addr; m<size;) {
    if ((a.l&0x7) || m+8>size) @<Load and write one byte@>@;
    else @<Load and write eight bytes@>;
  }
}
@z

@x
@ @<Load and write one byte@>=
{
  register int s=8*((~a.l)&0x3);
  ll->tet^=(((ll->tet>>s)^*p)&0xff)<<s;
  p++,m++,a=incr(a,1);
}

@ @<Load and write four bytes@>=
{
  ll->tet=(*p<<24)+(*(p+1)<<16)+(*(p+2)<<8)+*(p+3);
  p+=4,m+=4,a=incr(a,4);
}
@y
@ @<Load and write one byte@>=
{
  x.l=*p;
  x.h=0;
  store_data(1,x,a);
  p++,m++,a=incr(a,1);
}

@ @<Load and write eight bytes@>=
{ x.h=(*p<<24)+(*(p+1)<<16)+(*(p+2)<<8)+*(p+3);
  p+=4;
  x.l=(*p<<24)+(*(p+1)<<16)+(*(p+2)<<8)+*(p+3);
  p+=4;
  store_data(8,x,a);
  m+=8,a=incr(a,8);
}
@z

@x
@ We are finally ready for the last case.
@y
@ We do similar things for a trap interrupt.

Interrupt bits in rQ might be lost if they are set between a \.{GET}
and a~\.{PUT}. Therefore we don't allow \.{PUT} to zero out bits that
have become~1 since the most recently committed \.{GET}.

@<Glob...@>=
octa new_Q; /* when rQ increases in any bit position, so should this */

@ Now we can implement external interrupts.

@<Check for trap interrupt@>=
if (!resuming)
{ if (vmb_get_interrupt(&vmb,&new_Q.h,&new_Q.l)==1)
  { g[rQ].h |= new_Q.h; g[rQ].l |= new_Q.l; 
    if (tracing)
    printf("Interrupt: rQ=%08x%08x rK=%08x%08x\n",
            g[rQ].h, g[rQ].l, g[rK].h, g[rK].l);
  }
  if ((g[rK].h & g[rQ].h) != 0 || (g[rK].l & g[rQ].l) != 0) 
  { /*this is a dynamic trap */
    x.h=sign_bit, x.l=inst;
    if (tracing)
      printf("Dynamic TRAP: rQ=%08x%08x rK=%08x%08x\n",
            g[rQ].h, g[rQ].l, g[rK].h, g[rK].l);
    @<Initiate a trap interrupt@>
    inst_ptr=y=g[rTT];
  }
}

@ An instruction will not be executed if it violates the basic
security rule of \MMIX: An instruction in a nonnegative location
should not be performed unless all eight of the internal interrupts
have been enabled in the interrupt mask register~rK.
Conversely, an instruction in a negative location should not be performed
if the |P_BIT| is enabled in~rK.

The nonnegative-location case turns on the |S_BIT| of both rK and~rQ\null,
leading to an immediate interrupt.

@<Check for security violation@>=
{
  if (inst_ptr.h&sign_bit)
  { if (g[rK].h&P_BIT) 
    { g[rQ].h |= P_BIT;
      new_Q.h |= P_BIT;
      goto security_inst;
    }
  }
  else
  { if ((g[rK].h&0xff)!=0xff)
    { g[rQ].h |= S_BIT;
      new_Q.h |= S_BIT;
      g[rK].h |= S_BIT;
      goto security_inst;
    }
  }
}


@ Here are the bit codes that affect traps. The first eight
cases apply to the upper half of~rQ the next eight to the lower half.

@d P_BIT (1<<0) /* instruction in privileged location */
@d S_BIT (1<<1) /* security violation */
@d B_BIT (1<<2) /* instruction breaks the rules */
@d K_BIT (1<<3) /* instruction for kernel only */
@d N_BIT (1<<4) /* virtual translation bypassed */
@d PX_BIT (1<<5) /* permission lacking to execute from page */
@d PW_BIT (1<<6) /* permission lacking to write on page */
@d PR_BIT (1<<7) /* permission lacking to read from page */

@d PF_BIT (1<<0) /* power fail */
@d MP_BIT (1<<1) /* memory parity error */
@d NM_BIT (1<<2) /* non existent memory */
@d YY_BIT (1<<3) /* unassigned */
@d RE_BIT (1<<4) /* rebooting */
@d CP_BIT (1<<5) /* page fault */
@d PT_BIT (1<<6) /* page table error */
@d IN_BIT (1<<7) /* interval counter rI reaches zero */

@ We need this:
    
@ @<Initiate a trap interrupt@>=
 g[rWW]=inst_ptr;
 g[rXX]=x;
 g[rYY]=y;
 g[rZZ]=z;
 z.h=0, z.l=zz;
 g[rK].h = g[rK].l = 0;
 g[rBB]=g[255];
 g[255]=g[rJ];

@ We are finally ready for the last case.
@z

@x
case RESUME:@+if (xx || yy || zz) goto illegal_inst;
inst_ptr=z=g[rW];
b=g[rX];
if (!(b.h&sign_bit)) @<Prepare to perform a ropcode@>;
break;
@y
case RESUME:@+if (xx || yy) goto illegal_inst;
rzz=zz;
if ( rzz == 0)
{ if (!(loc.h&sign_bit) && (g[rW].h&sign_bit)) 
  goto protection_violation;
  inst_ptr=z=g[rW];
  b=g[rX];
}
else if ( rzz == 1)
{ 
  if (!(loc.h&sign_bit)) goto privileged_inst;
  inst_ptr=z=g[rWW];
  b=g[rXX];
  g[rK]=g[255];
  x=g[255]=g[rBB];
  @<Check for security violation@>
  if (interact_after_resume)
  { breakpoint = true;
    interact_after_resume = false;
  }
}
else goto illegal_inst;
if (!(b.h&sign_bit)) @<Prepare to perform a ropcode@>
break;
@z


@x
@d RESUME_AGAIN 0 /* repeat the command in rX as if in location $\rm rW-4$ */
@d RESUME_CONT 1 /* same, but substitute rY and rZ for operands */
@d RESUME_SET 2 /* set r[X] to rZ */
@y
@d RESUME_AGAIN 0 /* repeat the command in rX as if in location $\rm rW-4$ */
@d RESUME_CONT 1 /* same, but substitute rY and rZ for operands */
@d RESUME_SET 2 /* set r[X] to rZ */
@d RESUME_TRANS 3 /* install $\rm(rY,rZ)$ into IT-cache or DT-cache,
        then |RESUME_AGAIN| */
@z

@x
@<Prepare to perform a ropcode@>=
{
  rop=b.h>>24; /* the ropcode is the leading byte of rX */
  switch (rop) {
 case RESUME_CONT:@+if ((1<<(b.l>>28))&0x8f30) goto illegal_inst;
 case RESUME_SET: k=(b.l>>16)&0xff;
   if (k>=L && k<G) goto illegal_inst;
 case RESUME_AGAIN:@+if ((b.l>>24)==RESUME) goto illegal_inst;
   break;
 default: goto illegal_inst;
  }
  resuming=true;
}

@ @<Install special operands when resuming an interrupted operation@>=
if (rop==RESUME_SET) {
    op=ORI;
    y=g[rZ];
    z=zero_octa;
    exc=g[rX].h&0xff00;
    f=X_is_dest_bit;
}@+else { /* |RESUME_CONT| */
  y=g[rY];
  z=g[rZ];
}
@y
@<Prepare to perform a ropcode@>=
{
  rop=b.h>>24; /* the ropcode is the leading byte of rX */
  switch (rop) {
 case RESUME_CONT:@+if ((1<<(b.l>>28))&0x8f30) goto illegal_inst;
 case RESUME_SET: k=(b.l>>16)&0xff;
   if (k>=L && k<G) goto illegal_inst;
 case RESUME_AGAIN:@+if ((b.l>>24)==RESUME) goto illegal_inst;
   break;
 case RESUME_TRANS:@+if (rzz==0) goto illegal_inst;
   break;
 default: goto illegal_inst;
  }
  resuming=true;
}

@ @<Install special operands when resuming an interrupted operation@>=
if (rzz == 0)
{ if (rop==RESUME_SET) {
    op=ORI;
    y=g[rZ];
    z=zero_octa;
    exc=g[rX].h&0xff00;
    f=X_is_dest_bit;
  }@+else if (rop == RESUME_CONT) {
  y=g[rY];
  z=g[rZ];
  }
}
else
{ if (rop==RESUME_SET) {
    op=ORI;
    y=g[rZZ];
    z=zero_octa;
    exc=g[rXX].h&0xff00;
    f=X_is_dest_bit;
  } else if (rop==RESUME_TRANS)
  {  if ((b.l>>24)==SWYM) 
       store_exec_translation(&g[rYY], &g[rZZ]);
     else
       store_data_translation(&g[rYY], &g[rZZ]);
  }@+else if (rop == RESUME_CONT) {
  y=g[rYY];
  z=g[rZZ];
  }
}
@z


@x
  if (g[rI].l==0 && g[rI].h==0) tracing=breakpoint=true;
@y
  if (g[rI].l==0 && g[rI].h==0) g[rQ].l |= IN_BIT, new_Q.l |= IN_BIT; /* set the i bit */
@z


@x
if (tracing) {
@y
if (tracing && (!(loc.h&0x80000000) || show_operating_system)) {
@z


@x
@<Print a stream-of-consciousness description of the instruction@>=
if (lhs[0]=='!') printf("%s instruction!\n",lhs+1); /* privileged or illegal */
@y
@<Print a stream-of-consciousness description of the instruction@>=
if (lhs[0]=='!') { printf("%s instruction!\n",lhs+1); /* privileged or illegal */
  lhs[0]='\0';
}
@z

@x
@ @<Sub...@>=
fmt_style style;
char *stream_name[]={"StdIn","StdOut","StdErr"};
@.StdIn@>
@.StdOut@>
@.StdErr@>
@#
void trace_print @,@,@[ARGS((octa))@];@+@t}\6{@>
void trace_print(o)
  octa o;
@y
@ @(libtrace.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
//#include "address.h"
//#include "mmix-bus.h"
#include "mmixlib.h"

static fmt_style style;
static char *stream_name[]={"StdIn","StdOut","StdErr"};
@.StdIn@>
@.StdOut@>
@.StdErr@>
@#
void trace_print(octa o)
@z
  
  

@x
char switchable_string[48]; /* holds |rhs|; position 0 is ignored */
 /* |switchable_string| must be able to hold any |trap_format| */
@y
char switchable_string[300] ={0}; /* holds |rhs|; position 0 is ignored */
 /* |switchable_string| must be able to hold any debug message */
@z

@x
@ @<Sub...@>=
void show_stats @,@,@[ARGS((bool))@];@+@t}\6{@>
void show_stats(verbose)
  bool verbose;
@y
@ @(libstats.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
#include "mmixlib.h"

void show_stats(bool verbose)
@z


@x
@* Running the program. Now we are ready to fit the pieces together into a
working simulator.

@c
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include "abstime.h"
@<Preprocessor macros@>@;
@<Type declarations@>@;
@<Global variables@>@;
@<Subroutines@>@;
@#
int main(argc,argv)
  int argc;
  char *argv[];
{
  @<Local registers@>;
  mmix_io_init();
  @<Process the command line@>;
  @<Initialize everything@>;
  @<Load the command line arguments@>;
  @<Get ready to \.{UNSAVE} the initial context@>;
  while (1) {
    if (interrupt && !breakpoint) breakpoint=interacting=true, interrupt=false;
    else {
      breakpoint=false;
      if (interacting) @<Interact with the user@>;
    }
    if (halted) break;
    do @<Perform one instruction@>@;
    while ((!interrupt && !breakpoint) || resuming);
    if (interact_after_break) interacting=true, interact_after_break=false;
  }
 end_simulation:@+if (profiling) @<Print all the frequency counts@>;
  if (interacting || profiling || showing_stats) show_stats(true);
  return g[255].l; /* provide rudimentary feedback for non-interactive runs */
}
@y
@* Making the library. Now we are ready to write the different pieces of
a working simulator to separate files of a library.

@(liblibinit.c@>=
#include <stdlib.h>
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
#include "mmixlib.h"

int mmix_lib_initialize(void)
{
   @<Set up persistent data@>;
   return 0;
}

@ @(libinit.c@>=
#include <stdio.h>
#include <signal.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
//#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
#include "mmixlib.h"

#ifdef WIN32
BOOL CtrlHandler(DWORD fdwCtrlType);
#else
void catchint(int n);
#endif

int mmix_initialize(void)
{  @<Initialize everything@>;
  cur_seg.h=cur_seg.l=0; /* the Text segment is current */
  return 0;
}

@ @(libboot.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "abstime.h"
#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
#include "address.h"
#include "mmix-bus.h"
#include "mmixlib.h"

void mmix_boot(void)
{  @<Boot the machine@>;
}


@ @(libload.c@>=
int mmix_load_file(char *mmo_file_name)
{ int j; /* miscellaneous indices */
  mem_tetra *ll; /* current place in the simulated memory */
  char *p; /* current place in a string */
  free_file_info();
   @<Load object file@>;
   return 0;
}

@ @(libcommand.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
#include "address.h"
//#include "mmix-bus.h"
#include "mmixlib.h"

int mmix_commandline(int argc, char *argv[])
{ int k;
  @<Load the command line arguments@>;
  g[rQ].h=g[rQ].l=new_Q.h=new_Q.l=0; /*hide problems with loading the command line*/
  return 0;
}

@ @(libinteract.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
#include "address.h"
//#include "mmix-bus.h"
#include "mmixlib.h"

static octa scan_hex(char *s, octa offset);

int	mmix_interact(void)      
/* return zero to end the simulation */
{ int j,k; /* miscellaneous indices */
  mem_tetra *ll; /* current place in the simulated memory */
  char *p; /* current place in a string */
  @<Interact with the user@>;
  return 1;
end_simulation:
  return 0;
}

@ @(libfetch.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
#include "address.h"
//#include "mmix-bus.h"
#include "mmixlib.h"

int mmix_fetch_instruction(void)
/* return zero if no instruction was loaded */
{ mem_tetra *ll; /* current place in the simulated memory */
  @<Fetch the next instruction@>;
  return 1;

protection_violation: 
  strcpy(lhs,"!protected");
  g[rQ].h |= P_BIT; new_Q.h |= P_BIT; /* set the p bit */
  return 0;

security_inst: 
  strcpy(lhs,"!insecure");
  return 0;
  
page_fault: 
  strcpy(lhs,"!not fetched");
  return 0;
}

@ @(libperform.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "address.h"
#include "mmix-bus.h"
#include "vmb.h"
#include "mmixlib.h"

static bool interact_after_resume= false;

@<Stack store@>@;
@<Stack load@>@;
@<Register truth@>@;

int mmix_perform_instruction(void)  
{ @<Local registers@>;
  @<Perform one instruction@>@;
  return 1;
}

@ @(libtrace.c@>=

void mmix_trace(void)
{ mem_tetra *ll; /* current place in the simulated memory */
  char *p; /* current place in a string */

  @<Trace the current instruction, if requested@>;
} 

@ @(libdtrap.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
#include "mmixlib.h"

void mmix_dynamic_trap(void)
{  
   @<Check for trap interrupt@>;
}

@ @(libprofile.c@>=


void mmix_profile(void)
{
  @<Print all the frequency counts@>;
}


@ @(libfinal.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
#include "mmixlib.h"

int mmix_finalize(void)
{ free_file_info();
  return 0;
}

@ @(liblibfinal.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "vmb.h"
#include "mmixlib.h"

int mmix_lib_finalize(void)
{ return 0;
}

@ @c
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include "abstime.h"
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
@<Global variables@>@;
@<Subroutines@>@;
@#
int main(argc,argv)
  int argc;
  char *argv[];
{
  char **boot_cur_arg;
  int boot_argc;
  @<Set up persistent data@>;
  @<Process the command line@>;
  if (host==NULL) panic("No Bus given. Use Option -B[host:]port");
  init_mmix_bus(host,port,"MMIX CPU");
  atexit(vmb_atexit);
 
  boot_cur_arg = cur_arg;
  boot_argc = argc;
  if (vmb.power)  
    vmb_raise_reset(&vmb);
  mmix_initialize();

boot:

  argc = boot_argc;
  cur_arg = boot_cur_arg;
  mmix_boot();
     
  fprintf(stderr,"Power...");
  while (!vmb.power)
  {  vmb_wait_for_power(&vmb);
     if (!vmb.connected) goto end_simulation;
  }
  fprintf(stderr,"ON\n");
  
  mmix_load_file(*cur_arg);
  mmix_commandline(argc, argv);
  while (vmb.connected) {
    if (interrupt && !breakpoint) breakpoint=interacting=true, interrupt=false;
    else if (!(inst_ptr.h&sign_bit) || show_operating_system || 
          (inst_ptr.h==0x80000000 && inst_ptr.l==0))
    { breakpoint=false;
      if (interacting) { 
		if (!mmix_interact()) goto end_simulation;
      }
    }
    if (halted) break;
    do   
    { if (!resuming)
        mmix_fetch_instruction();
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
  end_simulation:@+if (profiling) mmix_profile();
  if (interacting || profiling || showing_stats) show_stats(true);
  mmix_finalize();
  return g[255].l; /* provide rudimentary feedback for non-interactive runs */
}  
@z

mmo_file_name becomes a variable not an alias.

@x
@d mmo_file_name *cur_arg
@y
@z

@x
if (!*cur_arg) scan_option("?",true); /* exit with usage note */
argc -= cur_arg-argv; /* this is the |argc| of the user program */
@y
#ifndef MMIXLIB
if (!*cur_arg) scan_option("?",true); /* exit with usage note */
#endif
argc -= (int)(cur_arg-argv); /* this is the |argc| of the user program */
@z

@x
@<Subr...@>=
void scan_option @,@,@[ARGS((char*,bool))@];@+@t}\6{@>
void scan_option(arg,usage)
  char *arg; /* command-line argument (without the `\.-') */
  bool usage; /* should we exit with usage note if unrecognized? */
@y
@(libsoption.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
#include "mmixlib.h"

void scan_option(char *arg, /* command-line argument (without the `\.-') */
                 bool usage) /* should we exit with usage note if unrecognized? */
@z

@x
 case 'l':@+if (!*(arg+1)) gap=3;
  else if (sscanf(arg+1,"%d",&gap)!=1) gap=0;
  showing_source=true;@+return;
 case 'L':@+if (!*(arg+1)) profile_gap=3;
  else if (sscanf(arg+1,"%d",&profile_gap)!=1) profile_gap=0;
  profile_showing_source=true;
@y
@z

@x
  gap=10, showing_source=true;
  profile_gap=10, profile_showing_source=true, profiling=true;
@y
  gap=10;
  profile_gap=10, profiling=true;
@z

@x
  stack_tracing=showing_stats=showing_source=false;
  profiling=profile_showing_source=false;
@y
  stack_tracing=showing_stats=false;
  profiling=false;
@z


@x
 case 'b':@+if (sscanf(arg+1,"%d",&buf_size)!=1) buf_size=0;@+return;
@y
#ifndef MMIXLIB 
 case 'B': 
  { char *p;
    p = strchr(arg+1,':');
    if (p==NULL)
    { host=localhost;
      port = atoi(arg+1);
    }   
    else
    { port = atoi(p+1);
      host = malloc(p+1-arg+1);
      if (host==NULL) panic("No room for hostname");
      strncpy(host,arg+1,p-arg-1);
      host[p-arg-1]=0;
    }
  }
  return; 
 #endif
 case 'O': show_operating_system=true;@+return;
 case 'o': show_operating_system=false;@+return;
@z

we need to replace all exits.

@x
    exit(-1);
@y
#ifdef MMIXLIB
    mmix_exit(-1);
#else
	exit(-1);
#endif    
@z    

@x
bool interrupt; /* has the user interrupted the simulation recently? */
bool profiling; /* should we print the profile at the end? */
@y
bool interrupt=0; /* has the user interrupted the simulation recently? */
bool profiling=0; /* should we print the profile at the end? */
@z

@x
"-s    show statistics after each traced instruction\n",@|
@y
"-O    trace inside the operating system\n",@|
"-o    disable trace inside the operating system\n",@|
"-B<n> connect to Bus on port <n>\n",@|
"-s    show statistics after each traced instruction\n",@|
@z

@x
"T         set current segment to Text_Segment\n",@|
"D         set current segment to Data_Segment\n",@|
"P         set current segment to Pool_Segment\n",@|
"S         set current segment to Stack_Segment\n",@|
@y
"T         set current segment to Text_Segment\n",@|
"D         set current segment to Data_Segment\n",@|
"P         set current segment to Pool_Segment\n",@|
"S         set current segment to Stack_Segment\n",@|
"N         set current segment to Negative Addresses\n",@|
"O         enable tracing inside the operating system\n",@|
"o         disable tracing inside the operating system\n",@|
@z

@x
else mmix_fake_stdin(fake_stdin);
@y
else fprintf(stderr,"Sorry, I can't fake stdin\n");
@z


@x
@ @<Initialize...@>=
signal(SIGINT,catchint); /* now |catchint| will catch the first interrupt */

@ @<Subr...@>=
void catchint @,@,@[ARGS((int))@];@+@t}\6{@>
void catchint(n)
  int n;
{ if (n!=SIGINT) return;
  interrupt=true;
  signal(SIGINT,catchint); /* now |catchint| will catch the next interrupt */
}
@y
@ @<Initialize...@>=
#ifdef WIN32
SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE );
#else
signal(SIGINT,catchint); /* now |catchint| will catch the first interrupt */
#endif

@ @(libinit.c@>=
#ifdef WIN32
BOOL CtrlHandler( DWORD fdwCtrlType ) 
{ interrupt=true;
  vmb_cancel_wait_for_event(&vmb);
  show_operating_system=true;
  if (fdwCtrlType==CTRL_C_EVENT || fdwCtrlType==CTRL_BREAK_EVENT )
  { 
    printf("Ctrl-C received\n");
  }
  else
  { printf("Closing MMIX\n");
#ifdef MMIXLIB
    halted=1;
    /* will not work if waiting for input */
#else
    FreeConsole();
#endif   
  }
  return TRUE;
}
#else
void catchint @,@,@[ARGS((int))@];@+@t}\6{@>
void catchint(n)
  int n;
{
  interrupt=true;
  vmb_cancel_wait_for_event(&vmb);
  show_operating_system=true;
  printf("Ctrl-C received\n");
  signal(SIGINT,catchint); /* now |catchint| will catch the next interrupt */
}
#endif
@z

@x
 interact: @<Put a new command in |command_buf|@>;
@y
 interact: @<Put a new command in |command_buf|@>;
 if (vmb_get_interrupt(&vmb,&new_Q.h,&new_Q.l)==1)
  { g[rQ].h |= new_Q.h; g[rQ].l |= new_Q.l; }
@z

@x
@ @<Subr...@>=
octa scan_hex @,@,@[ARGS((char*,octa))@];@+@t}\6{@>
octa scan_hex(s,offset)
  char *s;
  octa offset;
@y
@ @(libinteract.c@>=
octa scan_hex(char *s, octa offset)
@z

@x
 case 'M':@+if (!(cur_disp_addr.h&sign_bit)) {
    ll=mem_find(cur_disp_addr);
    ll->tet=val.h;@+ (ll+1)->tet=val.l;
  }@+break;
@y
 case 'M':
  store_data(8,val,cur_disp_addr);
  @+break;
@z

@x
 case 'M':@+if (cur_disp_addr.h&sign_bit) aux=zero_octa;
  else {
    ll=mem_find(cur_disp_addr);
    aux.h=ll->tet;@+ aux.l=(ll+1)->tet;
  }
@y
 case 'M':
    load_data(8,&aux,cur_disp_addr,0);
@z


@x
@ @<Subr...@>=
void print_string @,@,@[ARGS((octa))@];@+@t}\6{@>
void print_string(o)
  octa o;
@y
@ @(libprint.c@>=
void print_string(octa o)
@z


We allow tracing in the Textsegement and in the negative Segment.

@x
 if (val.h<0x20000000) {
@y
 if (val.h<0x20000000|| val.h>=0x80000000) {
@z

We allow breakpoints at all addresses.

@x
 if (!(val.h&sign_bit)) {
@y
 {
@z

@x
case 'B': show_breaks(mem_root);
@y
case 'N': cur_seg.h=0x80000000;@+goto passit;
case 'B': show_breaks(mem_root);@+goto passit;
case 'O': show_operating_system=true;@+goto passit;
case 'o': show_operating_system=false;@+goto passit;
@z

@x
@ @<Sub...@>=
void show_breaks @,@,@[ARGS((mem_node*))@];@+@t}\6{@>
void show_breaks(p)
  mem_node *p;
@y
@ @(libshowbreaks.c@>=
#include <stdio.h>
@h
@<Preprocessor macros@>@;
@<Type declarations@>@;
#include "libarith.h"
#include "libglobals.h"
#include "vmb.h"
//#include "address.h"
//#include "mmix-bus.h"
#include "mmixlib.h"


void show_breaks(mem_node *p)
@z


@x
@<Load the command line arguments@>=
x.h=0x40000000, x.l=0x8;
loc=incr(x,8*(argc+1));
for (k=0; k<argc; k++,cur_arg++) {
  ll=mem_find(x);
  ll->tet=loc.h, (ll+1)->tet=loc.l;
  ll=mem_find(loc);
  mmputchars((unsigned char *)*cur_arg,strlen(*cur_arg),loc);
  x.l+=8, loc.l+=8+(strlen(*cur_arg)&-8);
}
x.l=0;@+ll=mem_find(x);@+ll->tet=loc.h, (ll+1)->tet=loc.l;
@y
@<Load the command line arguments@>=
x.h=0x60000000, x.l=0x00;
aux.h=0, aux.l=argc;
store_data(8,aux,x); /* and $\$0=|argc|$ */
x.h=0x40000000, x.l=0x8;
aux=incr(x,8*(argc+1));
for (k=0; k<argc && *argv!=NULL; k++,argv++) {
  store_data(8,aux,x);
  mmputchars((unsigned char *)*argv,strlen(*argv),aux);
  x.l+=8, aux.l+=8+(tetra)(strlen(*argv)&-8);
}
if (argc>0) 
{ x.l=0;
  store_data(8,aux,x);
}
@z

@x
@ @<Get ready to \.{UNSAVE} the initial context@>=
x.h=0, x.l=0xf0;
ll=mem_find(x);
if (ll->tet) inst_ptr=x;
@^subroutine library initialization@>
@^initialization of a user program@>
resuming=true;
rop=RESUME_AGAIN;
g[rX].l=((tetra)UNSAVE<<24)+255;
if (dump_file) {
  x.l=1;
  dump(mem_root);
  dump_tet(0),dump_tet(0);
  exit(0);
}
@y
@ We make sure that the processor boots properly.
A user porgram (if loaded) can be started with UNSAVE \$255.

@<Boot the machine@>=
loc.h=inst_ptr.h=0x80000000;
loc.l=inst_ptr.l=0x00000000;
g[rJ].h=g[rJ].l =0xFFFFFFFF;
resuming=false;
@z

we do not support the -D dump option.


@x
@ The special option `\.{-D<filename>}' can be used to prepare binary files
needed by the \MMIX-in-\MMIX\ simulator of Section 1.4.3\'{}. This option
puts big-endian octa\-bytes into a given file; a location~$l$ is followed
by one or more nonzero octabytes M$_8[l]$, M$_8[l+8]$, M$_8[l+16]$, \dots,
followed by zero. The simulated simulator knows how to load programs
in such a format (see exercise 1.4.3\'{}--20), and so does
the meta-simulator {\mc MMMIX}.

@<Sub...@>=
void dump @,@,@[ARGS((mem_node*))@];@+@t}\6{@>
void dump_tet @,@,@[ARGS((tetra))@];@+@t}\6{@>
void dump(p)
  mem_node *p;
{
  register int j;
  octa cur_loc;
  if (p->left) dump(p->left);
  for (j=0;j<512;j+=2) if (p->dat[j].tet || p->dat[j+1].tet) {
    cur_loc=incr(p->loc,4*j);
    if (cur_loc.l!=x.l || cur_loc.h!=x.h) {
      if (x.l!=1) dump_tet(0),dump_tet(0);
      dump_tet(cur_loc.h);@+dump_tet(cur_loc.l);@+x=cur_loc;
    }
    dump_tet(p->dat[j].tet);
    dump_tet(p->dat[j+1].tet);
    x=incr(x,8);
  }
  if (p->right) dump(p->right);
}

@ @<Sub...@>=
void dump_tet(t)
  tetra t;
{
  fputc(t>>24,dump_file);
  fputc((t>>16)&0xff,dump_file);
  fputc((t>>8)&0xff,dump_file);
  fputc(t&0xff,dump_file);
}
@y
@z

