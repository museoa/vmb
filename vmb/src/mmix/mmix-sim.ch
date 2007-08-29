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

\bull
@y
@z

@x
is equivalent to \.{-eff}, tracing all eight exceptions.
@y
is equivalent to \.{-eff}, tracing all eight exceptions.

\bull \.{-g<n>}\quad Connect to the gdb debugger on port n. Without
this option there is only (for the time being) the internal debugger available.
Sooner or later there will be no debugger at all if this option is omitted.

\bull \.{-B[host:]port}\quad Connect to the bus on the given host and port.
This option is required. Without a bus, the processor has no power suply, no
memory and no other devices. It will not function. The hostname is separated with
a colon from the port number. The hostname, together with the colon,
can be ommited; in this case the simulator will try a connection to localhost.
@z


@x
@* Simulated memory. Chunks of simulated memory, 2048 bytes each,
are kept in a tree structure organized as a {\it treap},
following ideas of Vuillemin, Aragon, and Seidel
@^Vuillemin, Jean Etienne@>
@^Aragon, Cecilia Rodriguez@>
@^Seidel, Raimund@>
[{\sl Communications of the ACM\/ \bf23} (1980), 229--239;
{\sl IEEE Symp.\ on Foundations of Computer Science\/ \bf30} (1989), 540--546].
Each node of the treap has two keys: One, called |loc|, is the
base address of 512 simulated tetrabytes; it follows the conventions
of an ordinary binary search tree, with all locations in the left subtree
less than the |loc| of a node and all locations in the right subtree
greater than that~|loc|. The other, called |stamp|, can be thought of as the
time the node was inserted into the tree; all subnodes of a given node
have a larger~|stamp|. By assigning time stamps at random, we maintain
a tree structure that almost always is fairly well balanced.

Each simulated tetrabyte has an associated frequency count and
source file reference.

@<Type...@>=
typedef struct {
  tetra tet; /* the tetrabyte of simulated memory */
  tetra freq; /* the number of times it was obeyed as an instruction */
  unsigned char bkpt; /* breakpoint information for this tetrabyte */
  unsigned char file_no; /* source file number, if known */
  unsigned short line_no; /* source line number, if known */
} mem_tetra;
@#
typedef struct mem_node_struct {
  octa loc; /* location of the first of 512 simulated tetrabytes */
  tetra stamp; /* time stamp for treap balancing */
  struct mem_node_struct *left, *right; /* pointers to subtrees */
  mem_tetra dat[512]; /* the chunk of simulated tetrabytes */
} mem_node;

@ The |stamp| value is actually only pseudorandom, based on the
idea of Fibonacci hashing [see {\sl Sorting and Searching}, Section~6.4].
This is good enough for our purposes, and it guarantees that
no two stamps will be identical.

@<Sub...@>=
mem_node* new_mem @,@,@[ARGS((void))@];@+@t}\6{@>
mem_node* new_mem()
{
  register mem_node *p;
  p=(mem_node*)calloc(1,sizeof(mem_node));
  if (!p) panic("Can't allocate any more memory");
@.Can't allocate...@>
  p->stamp=priority;
  priority+=0x9e3779b9; /* $\lfloor2^{32}(\phi-1)\rfloor$ */
  return p;
}

@ Initially we start with a chunk for the pool segment, since
the simulator will be putting command-line information there before
it runs the program.

@<Initialize...@>=
mem_root=new_mem();
mem_root->loc.h=0x40000000;
last_mem=mem_root;

@ @<Glob...@>=
tetra priority=314159265; /* pseudorandom time stamp counter */
mem_node *mem_root; /* root of the treap */
mem_node *last_mem; /* the memory node most recently read or written */

@ The |mem_find| routine finds a given tetrabyte in the simulated
memory, inserting a new node into the treap if necessary.

@<Sub...@>=
mem_tetra* mem_find @,@,@[ARGS((octa))@];@+@t}\6{@>
mem_tetra* mem_find(addr)
  octa addr;
{
  octa key;
  register int offset;
  register mem_node *p=last_mem;
  key.h=addr.h;
  key.l=addr.l&0xfffff800;
  offset=addr.l&0x7fc;
  if (p->loc.l!=key.l || p->loc.h!=key.h)
    @<Search for |key| in the treap,
        setting |last_mem| and |p| to its location@>;
  return &p->dat[offset>>2];
}

@ @<Search for |key| in the treap...@>=
{@+register mem_node **q;
  for (p=mem_root; p; ) {
    if (key.l==p->loc.l && key.h==p->loc.h) goto found;
    if ((key.l<p->loc.l && key.h<=p->loc.h) || key.h<p->loc.h) p=p->left;
    else p=p->right;
  }
  for (p=mem_root,q=&mem_root; p && p->stamp<priority; p=*q) {
    if ((key.l<p->loc.l && key.h<=p->loc.h) || key.h<p->loc.h) q=&p->left;
    else q=&p->right;
  }
  *q=new_mem();
  (*q)->loc=key;
  @<Fix up the subtrees of |*q|@>;
  p=*q;
found: last_mem=p;
}

@ At this point we want to split the binary search tree |p| into two
parts based on the given |key|, forming the left and right subtrees
of the new node~|q|. The effect will be as if |key| had been inserted
before all of |p|'s nodes.

@<Fix up the subtrees of |*q|@>=
{
  register mem_node **l=&(*q)->left,**r=&(*q)->right;
  while (p) {
    if ((key.l<p->loc.l && key.h<=p->loc.h) || key.h<p->loc.h)
      *r=p, r=&p->left, p=*r;
    else *l=p, l=&p->right, p=*l;
  }
  *l=*r=NULL;
}  
@y  
@* Simulated memory. 
We now read memory using some external simulator.
We provide the |extern| function |load_dat| to get
data and |store_data| to store data.
Both functions use virtual addresses. The adresses
are rounded down to a multiple of 1,2,4, or 8 for alignment. 
The the |extern| function |load_instruction| loads
an instruction (possibly using a different cache). The
address is rounded down to a multiple of 4.

@<Type...@>=
extern int load_instruction(tetra *data, octa address);
extern int load_data(int size, octa *data, octa address, int signextension);
extern int store_data(int size, octa data, octa address);
extern void clear_all_data_vtc(void);
extern void clear_all_instruction_vtc(void);
extern octa update_vtc(octa key);
extern void write_all_data_cache(void);
extern void clear_all_data_cache(void);
extern void clear_all_instruction_cache(void);
extern void store_data_translation(octa *virt, octa *phys);
extern void store_exec_translation(octa *virt, octa *phys);
extern void write_data(octa address,int size);
extern void delete_data(octa address,int size);
extern void delete_instruction(octa address,int size);
extern void read_instruction(octa address,int size);
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
@y
@ We do not load the symbol table. (A more ambitious simulator could
implement \.{MMIXAL}-style expressions for interactive debugging,
but such enhancements are left to the interested reader.)

@<Load object file@>=
if (mmo_file_name!=NULL)
{ mmo_file=fopen(mmo_file_name,"rb");
  if (!mmo_file) {
  register char *alt_name=(char*)calloc(strlen(mmo_file_name)+5,sizeof(char));
  if (!alt_name) panic("Can't allocate file name buffer");
@z

@x
@ In a normal situation, the newly read tetrabyte is simply supposed
to be loaded into the current location. We load not only the current
location but also the current file position, if |cur_line| is nonzero
and |cur_loc| belongs to segment~0.

@d mmo_load(loc,val) ll=mem_find(loc), ll->tet^=val

@<Load |tet| as a normal item@>=
{
  mmo_load(cur_loc,tet);
  if (cur_line) {
    ll->file_no=cur_file;
    ll->line_no=cur_line;
    cur_line++;
  }
  cur_loc=incr(cur_loc,4);@+ cur_loc.l &=-4;
}

@ @<Glob...@>=
octa cur_loc; /* the current location */
int cur_file=-1; /* the most recently selected file number */
int cur_line; /* the current position in |cur_file|, if nonzero */
octa tmp; /* an octabyte of temporary interest */
tetra obj_time; /* when the object file was created */

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
@ In a normal situation, the newly read tetrabyte is simply supposed
to be loaded into the current location. 

@<Sub...@>=
void mmo_load @,@,@[ARGS((octa,tetra))@];@+@t}\6{@>
void mmo_load (loc, val)
  octa loc;
  tetra val;
{
	octa x;
        load_data(4,&x,loc,0);
        x.h = 0;
        x.l = x.l^val;
	store_data(4,x,loc);
}

@ @<Load |tet| as a normal item@>=
{
  mmo_load(cur_loc,tet);
  cur_loc=incr(cur_loc,4);@+ cur_loc.l &=-4;
}

@ @<Glob...@>=
octa cur_loc; /* the current location */
octa tmp; /* an octabyte of temporary interest */
tetra obj_time; /* when the object file was created */

@ @<Load object file@>=
cur_loc.h=cur_loc.l=0;
@<Load the preamble@>;
do @<Load the next item@>@;@+while (!postamble);
@<Load the postamble@>;
fclose(mmo_file);
write_all_data_cache();
clear_all_instruction_cache();
}
@z

@x
case lop_fixo:@+if (zbyte==2) {
   j=ybyte;@+ read_tet();@+ tmp.h=(j<<24)+tet;
 }@+else if (zbyte==1) tmp.h=ybyte<<24;
 else mmo_err;
 read_tet();@+ tmp.l=tet;
 mmo_load(tmp,cur_loc.h);
 mmo_load(incr(tmp,4),cur_loc.l);
 continue;
@y
case lop_fixo:@+if (zbyte==2) {
   j=ybyte;@+ read_tet();@+ tmp.h=(j<<24)+tet;
 }@+else if (zbyte==1) tmp.h=ybyte<<24;
 else mmo_err;
 read_tet();@+ tmp.l=tet;
 store_data(8,tmp,cur_loc);
 continue;
@z

@x
@ The space for file names isn't allocated until we are sure we need it.

@<Cases for lopcodes...@>=
case lop_file:@+if (file_info[ybyte].name) {
   if (zbyte) mmo_err;
   cur_file=ybyte;
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
 cur_line=0;@+continue;
case lop_line:@+if (cur_file<0) mmo_err;
 cur_line=yzbytes;@+continue;
@y
@ The space for file names isn't allocated until we are sure we need it.

@<Cases for lopcodes...@>=
case lop_file:
   for (j=zbyte; j>0; j--) {/* skip file name */
     read_tet();
   }
 continue;
case lop_line:
 continue;
@z

@x
@ Since a chunk of memory holds 512 tetrabytes, the |ll| pointer in the
following loop stays in the same chunk (namely, the first chunk
of segment~3, also known as \.{Stack\_Segment}).
@:Stack_Segment}\.{Stack\_Segment@>
@:Pool_Segment}\.{Pool\_Segment@>

@<Load the postamble@>=
aux.h=0x60000000;@+ aux.l=0x18;
ll=mem_find(aux);
(ll-1)->tet=2; /* this will ultimately set |rL=2| */
(ll-5)->tet=argc; /* and $\$0=|argc|$ */
(ll-4)->tet=0x40000000;
(ll-3)->tet=0x8; /* and $\$1=\.{Pool\_Segment}+8$ */
G=zbyte;@+ L=0;
for (j=G+G;j<256+256;j++,ll++,aux.l+=4) read_tet(), ll->tet=tet;
inst_ptr.h=(ll-2)->tet, inst_ptr.l=(ll-1)->tet; /* \.{Main} */
(ll+2*12)->tet=G<<24;
g[255]=incr(aux,12*8); /* we will |UNSAVE| from here, to get going */

@* Loading and printing source lines.
The loaded program generally contains cross references to the lines
of symbolic source files, so that the context of each instruction
can be understood. The following sections of this program
make such information available when it is desired.

Source file data is kept in a \&{file\_node} structure:

@<Type...@>=
typedef struct {
  char *name; /* name of source file */
  int line_count; /* number of lines in the file */
  long *map; /* pointer to map of file positions */
} file_node;

@ In partial preparation for the day when source files are in
Unicode, we define a type \&{Char} for the source characters.

@<Type...@>=
typedef char Char; /* bytes that will become wydes some day */

@ @<Glob...@>=
file_node file_info[256]; /* data about each source file */
int buf_size; /* size of buffer for source lines */
Char *buffer;

@ As in \.{MMIXAL}, we prefer source lines of length 72 characters or less,
but the user is allowed to increase the limit. (Longer lines will silently
be truncated to the buffer size when the simulator lists them.)

@<Initialize...@>=
if (buf_size<72) buf_size=72;
buffer=(Char*)calloc(buf_size+1,sizeof(Char));
if (!buffer) panic("Can't allocate source line buffer");
@.Can't allocate...@>

@ The first time we are called upon to list a line from a given source
file, we make a map of starting locations for each line. Source files
should contain at most 65535 lines. We assume that they contain
no null characters.

@<Sub...@>=
void make_map @,@,@[ARGS((void))@];@+@t}\6{@>
void make_map()
{
  long map[65536];
  register int k,l;
  register long*p;
  @<Check if the source file has been modified@>;
  for (l=1;l<65536 && !feof(src_file);l++) {
    map[l]=ftell(src_file);
   loop:@+if (!fgets(buffer,buf_size,src_file)) break;
    if (buffer[strlen(buffer)-1]!='\n') goto loop;
  }
  file_info[cur_file].line_count=l;
  file_info[cur_file].map=p=(long*)calloc(l,sizeof(long));
  if (!p) panic("No room for a source-line map");
@.No room...@>
  for (k=1;k<l;k++) p[k]=map[k];
}

@ We want to warn the user if the source file has changed since the
object file was written. The standard \CEE/ library doesn't provide
the information we need; so we use the \UNIX/ system function |stat|,
in hopes that other operating systems provide a similar way to do the job.
@^system dependencies@>

@<Preprocessor macros@>=
#include <sys/types.h>
#include <sys/stat.h>

@ @<Check if the source file has been modified@>=
@^system dependencies@>
{
  struct stat stat_buf;
  if (stat(file_info[cur_file].name,&stat_buf)>=0)
    if ((tetra)stat_buf.st_mtime > obj_time)
      fprintf(stderr,
         "Warning: File %s was modified; it may not match the program!\n",
@.File...was modified@>
         file_info[cur_file].name);
}

@ Source lines are listed by the |print_line| routine, preceded by
12 characters containing the line number. If a file error occurs,
nothing is printed---not even an error message; the absence of
listed data is itself a message.

@<Sub...@>=
void print_line @,@,@[ARGS((int))@];@+@t}\6{@>
void print_line(k)
  int k;
{
  char buf[11];
  if (k>=file_info[cur_file].line_count) return;
  if (fseek(src_file,file_info[cur_file].map[k],SEEK_SET)!=0) return;
  if (!fgets(buffer,buf_size,src_file)) return;
  sprintf(buf,"%d:    ",k);
  printf("line %.6s %s",buf,buffer);
  if (buffer[strlen(buffer)-1]!='\n') printf("\n");
  line_shown=true;
}

@ @<Preprocessor macros@>=
#ifndef SEEK_SET
#define SEEK_SET 0 /* code for setting the file pointer to a given offset */
#endif

@ The |show_line| routine is called when we want to output line |cur_line|
of source file number |cur_file|, assuming that |cur_line!=0|. Its job
is primarily to maintain continuity, by opening or reopening the |src_file|
if the source file changes, and by connecting the previously output
lines to the new one. Sometimes no output is necessary, because the
desired line has already been printed.

@<Sub...@>=
void show_line @,@,@[ARGS((void))@];@+@t}\6{@>
void show_line()
{
  register int k;
  if (shown_file!=cur_file) @<Prepare to list lines from a new source file@>@;
  else if (shown_line==cur_line) return; /* already shown */
  if (cur_line>shown_line+gap+1 || cur_line<shown_line) {
    if (shown_line>0)
      if (cur_line<shown_line) printf("--------\n"); /* indicate upward move */
      else printf("     ...\n"); /* indicate the gap */
    print_line(cur_line);
  }@+else@+ for (k=shown_line+1;k<=cur_line;k++) print_line(k);
  shown_line=cur_line;
}
    
@ @<Glob...@>=
FILE *src_file; /* the currently open source file */
int shown_file=-1; /* index of the most recently listed file */
int shown_line; /* the line most recently listed in |shown_file| */
int gap; /* minimum gap between consecutively listed source lines */
bool line_shown; /* did we list anything recently? */
bool showing_source; /* are we listing source lines? */
int profile_gap; /* the |gap| when printing final frequencies */
bool profile_showing_source; /* |showing_source| within final frequencies */

@ @<Prepare to list lines from a new source file@>=
{
  if (!src_file) src_file=fopen(file_info[cur_file].name,"r");
  else freopen(file_info[cur_file].name,"r",src_file);
  if (!src_file) {
    fprintf(stderr,"Warning: I can't open file %s; source listing omitted.\n",
@.I can't open...@>
               file_info[cur_file].name);
    showing_source=false;
    return;
  }
  printf("\"%s\"\n",file_info[cur_file].name);
  shown_file=cur_file;
  shown_line=0;
  if (!file_info[cur_file].map) make_map();
}

@ Here is a simple application of |show_line|. It is a recursive routine that
prints the frequency counts of all instructions that occur in a
given subtree of the simulated memory and that were executed at least once.
The subtree is traversed in symmetric order; therefore the frequencies
appear in increasing order of the instruction locations.

@<Sub...@>=
void print_freqs @,@,@[ARGS((mem_node*))@];@+@t}\6{@>
void print_freqs(p)
  mem_node *p;
{
  register int j;
  octa cur_loc;
  if (p->left) print_freqs(p->left);
  for (j=0;j<512;j++) if (p->dat[j].freq)
    @<Print frequency data for location |p->loc+4*j|@>;
  if (p->right) print_freqs(p->right);
}

@ An ellipsis (\.{...}) is printed between frequency data for nonconsecutive
instructions, unless source line information intervenes.

@<Print frequency data...@>=
{
  cur_loc=incr(p->loc,4*j);
  if (showing_source && p->dat[j].line_no) {
    cur_file=p->dat[j].file_no, cur_line=p->dat[j].line_no;
    line_shown=false;
    show_line();
    if (line_shown) goto loc_implied;
  }
  if (cur_loc.l!=implied_loc.l || cur_loc.h!=implied_loc.h)
    if (profile_started) printf("         0.        ...\n");
 loc_implied: printf("%10d. %08x%08x: %08x (%s)\n",
      p->dat[j].freq, cur_loc.h, cur_loc.l, p->dat[j].tet,
      info[p->dat[j].tet>>24].name);
  implied_loc=incr(cur_loc,4);@+ profile_started=true;
}
    
@ @<Glob...@>=
octa implied_loc; /* location following the last shown frequency data */
bool profile_started; /* have we printed at least one frequency count? */

@ @<Print all the frequency counts@>=
{
  printf("\nProgram profile:\n");
  shown_file=cur_file=-1;@+ shown_line=cur_line=0;
  gap=profile_gap;
  showing_source=profile_showing_source;
  implied_loc=neg_one;
  print_freqs(mem_root);
}
@y
@ We load the postamble into the beginning
of segment~3, also known as \.{Stack\_Segment}).
@:Stack_Segment}\.{Stack\_Segment@>
@:Pool_Segment}\.{Pool\_Segment@>
The stack segment is set up to be used with an unsave instruction.
On the stack, we have, the local registers (argc and argv) and the value of rL, then the global 
registers and the special registers rB, rD, rE, rH, rJ, rM, rR, rP, rW, rX, rY, and rZ,
followed by rG and rA packed into eight byte

@<Load the postamble@>=
aux.h=0x60000000;
{ octa x;
  x.h=0;@+x.l=argc;@+aux.l=0x00;@+store_data(8,x,aux); /* and $\$0=|argc|$ */
  x.h=0x40000000;@+x.l=0x8;@+aux.l=0x08;@+store_data(8,x,aux); /* and $\$1=\.{Pool\_Segment}+8$ */
  x.h=0;@+x.l=2;@+aux.l=0x10;@+store_data(8,x,aux); /* this will ultimately set |rL=2| */
  G=zbyte;@+ L=0;
  aux.l=0x18;
  for (j=G;j<256;j++,aux.l+=8) 
  { read_tet(); x.h=tet;
    read_tet(), x.l=tet;
    store_data(8,x,aux);
  }
  g[rWW] = x;  /* last octa stored is address of \.{Main} */
  g[rXX].h = 0; g[rXX].l = 0xFB0000FF; /* |UNSAVE| \$255 */
  g[rBB]=aux=incr(aux,12*8); /* we can |UNSAVE| from here, to get going */
  x.h=G<<24; x.l=0 /* rA */; 
  store_data(8,x,aux);
}
@z


@x
@* The main loop. Now let's plunge in to the guts of the simulator,
the master switch that controls most of the action.

@<Perform one instruction@>=
{
  if (resuming) loc=incr(inst_ptr,-4), inst=g[rX].l;
  else @<Fetch the next instruction@>;
  op=inst>>24;@+xx=(inst>>16)&0xff;@+yy=(inst>>8)&0xff;@+zz=inst&0xff;
  f=info[op].flags;@+yz=inst&0xffff;
  x=y=z=a=b=zero_octa;@+ exc=0;@+ old_L=L;
  if (f&rel_addr_bit) @<Convert relative address to absolute address@>;
  @<Install operand fields@>;
  if (f&X_is_dest_bit) @<Install register~X as the destination,
          adjusting the register stack if necessary@>;
  w=oplus(y,z);
  if (loc.h>=0x20000000) goto privileged_inst;
  switch(op) {
  @t\4@>@<Cases for individual \MMIX\ instructions@>;
  }
  @<Check for trip interrupt@>;
  @<Update the clocks@>;
  @<Trace the current instruction, if requested@>;
  if (resuming && op!=RESUME) resuming=false;
}

@y
@* The main loop. Now let's plunge in to the guts of the simulator,
the master switch that controls most of the action.

@<Perform one instruction@>=
{
  if (!resuming) @<Fetch the next instruction@>;
  op=inst>>24;@+xx=(inst>>16)&0xff;@+yy=(inst>>8)&0xff;@+zz=inst&0xff;
  f=info[op].flags;@+yz=inst&0xffff;
  x=y=z=a=b=zero_octa;@+ exc=0;@+ old_L=L;
  if (f&rel_addr_bit) @<Convert relative address to absolute address@>;
  {@+ @<Install operand fields@>;@+}
  if (f&X_is_dest_bit){ @<Install register~X as the destination,
          adjusting the register stack if necessary@>; }
  w=oplus(y,z);
  switch(op) {
  @t\4@>@<Cases for individual \MMIX\ instructions@>;
  }
  @<Check for trap and trip interrupt@>;
  @<Update the clocks@>;
  @<Trace the current instruction, if requested@>;
  if (resuming && op!=RESUME) resuming=false;
}


@z

@x
bool interacting; /* are we in interactive mode? */
@y
bool interacting; /* are we in interactive mode? */
bool stepping; /* should we pause after the next instruction? */
bool remotegdb=false; /* are we connected to a remote gdb? */
int gdbport; /* on which port are we connected to gdb ? */
int busport=-1; /* on which port to connect to the bus */
char *bushost=NULL; /* on which host to connect to the bus */
@z


@x
register int i,j,k; /* miscellaneous indices */
register mem_tetra *ll; /* current place in the simulated memory */
register char *p; /* current place in a string */

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
register int i,j,k; /* miscellaneous indices */
register char *p; /* current place in a string */

@ @<Fetch the next instruction@>=
{ loc=inst_ptr;
  load_instruction(&inst,loc);
  inst_ptr=incr(inst_ptr,4);
}
@z


@x
@ @<Info for arithmetic commands@>=
{"TRAP",0x0a,255,0,5,"%r"},@|
@y
@ @<Info for arithmetic commands@>=
{"TRAP",0x0a,255,0,5,"%#z"},@|
@z

@x
{"UNSAVE",0x82,0,20,1,"%#z: rG=%x, ..., rL=%a"},@|
{"SYNC",0x01,0,0,1,""},@|
@y
{"UNSAVE",0x82,0,20,1,"%#z: rG=%x, ..., rL=%a"},@|
{"SYNC",0x01,0,0,1,"%z"},@|
@z


@x
@ @<Set |b| from register X@>=
{
  if (xx>=G) b=g[xx];
  else if (xx<L) b=l[(O+xx)&lring_mask];
}
  
@ @<Local...@>=
register int G,L,O; /* accessible copies of key registers */

@ @<Glob...@>=
octa g[256]; /* global registers */
@y
@ @<Set |b| from register X@>=
{
  if (xx>=G) b=g[xx];
  else if (xx<L) b=l[(O+xx)&lring_mask];
}
  

@ @<Glob...@>=
int G=1,L=0,O=0; /* accessible copies of key registers */
octa g[256]; /* global registers */
@z

@x
@d SUBSUBVERSION 1 /* further qualification to version number */

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
@d SUBSUBVERSION 1 /* further qualification to version number */

@<Initialize...@>=
@<Boot the machine@>@;
if (lring_size<256) lring_size=256;
lring_mask=lring_size-1;
if (lring_size&lring_mask)
  panic("The number of local registers must be a power of 2");
@.The number of local...@>
l=(octa*)calloc(lring_size,sizeof(octa));
if (!l) panic("No room for the local registers");
@.No room...@>

@ @<Boot the machine@>=
g[rK].h=g[rK].l=0;
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
@d test_store_bkpt(ll) if ((ll)->bkpt&write_bit) breakpoint=tracing=true

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

@ The |stack_load| routine is essentially the inverse of |stack_store|.

@d test_load_bkpt(ll) if ((ll)->bkpt&read_bit) breakpoint=tracing=true

@<Sub...@>=
void stack_load @,@,@[ARGS((void))@];@+@t}\6{@>
void stack_load()
{
  register mem_tetra *ll;
  register int k;
  S--, g[rS]=incr(g[rS],-8);
  ll=mem_find(g[rS]);
  k=S&lring_mask;
  l[k].h=ll->tet;@+test_load_bkpt(ll);
  l[k].l=(ll+1)->tet;@+test_load_bkpt(ll+1);
  if (stack_tracing) {
    tracing=true;
    if (cur_line) show_line();
    printf("             rS-=8, l[%d]=M8[#%08x%08x]=#%08x%08x\n",
              k,g[rS].h,g[rS].l,l[k].h,l[k].l);
  }
}
@y
@d test_store_bkpt(a) if (get_break(a)&write_bit) breakpoint=tracing=true

@<Sub...@>=
void stack_store @,@,@[ARGS((void))@];@+@t}\6{@>
void stack_store()
{
  register int k=S&lring_mask;
  store_data(8,l[k],g[rS]);@+test_store_bkpt(g[rS]);
  if (stack_tracing) {
    tracing=true;
    printf("             M8[#%08x%08x]=l[%d]=#%08x%08x, rS+=8\n",
              g[rS].h,g[rS].l,k,l[k].h,l[k].l);
  }
  g[rS]=incr(g[rS],8),  S++;
}

@ The |stack_load| routine is essentially the inverse of |stack_store|.

@d test_load_bkpt(a) if (get_break(a)&read_bit) breakpoint=tracing=true

@<Sub...@>=
void stack_load @,@,@[ARGS((void))@];@+@t}\6{@>
void stack_load()
{
  register int k;
  S--, g[rS]=incr(g[rS],-8);
  k=S&lring_mask;
  load_data(8,&l[k],g[rS],0);@+test_load_bkpt(g[rS]);
  if (stack_tracing) {
    tracing=true;
    printf("             rS-=8, l[%d]=M8[#%08x%08x]=#%08x%08x\n",
              k,g[rS].h,g[rS].l,l[k].h,l[k].l);
  }
}
@z

@x
case LDB: case LDBI: case LDBU: case LDBUI:@/
 i=56;@+j=(w.l&0x3)<<3; goto fin_ld;
case LDW: case LDWI: case LDWU: case LDWUI:@/
 i=48;@+j=(w.l&0x2)<<3; goto fin_ld;
case LDT: case LDTI: case LDTU: case LDTUI:@/
 i=32;@+j=0;@+ goto fin_ld;
case LDHT: case LDHTI: i=j=0;
fin_ld: ll=mem_find(w);@+test_load_bkpt(ll);
 x.h=ll->tet;
 x=shift_right(shift_left(x,j),i,op&0x2);
check_ld:@+if (w.h&sign_bit) goto privileged_inst;
 goto store_x;
case LDO: case LDOI: case LDOU: case LDOUI: case LDUNC: case LDUNCI:
 w.l&=-8;@+ ll=mem_find(w);
 test_load_bkpt(ll);@+test_load_bkpt(ll+1);
 x.h=ll->tet;@+ x.l=(ll+1)->tet;
 goto check_ld;
case LDSF: case LDSFI: ll=mem_find(w);@+test_load_bkpt(ll);
 x=load_sf(ll->tet);@+ goto check_ld;

@ @<Cases for ind...@>=
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

@ The |CSWAP| operation has elements of both loading and storing.
We shuffle some of
the operands around so that they will appear correctly in the trace output.

@<Cases for ind...@>=
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
 goto check_ld;
@y
case LDB: case LDBI:
 if (!load_data(1,&x,w,1)) goto page_fault;
 goto check_ld;
case LDBU: case LDBUI:@/
 if (!load_data(1,&x,w,0)) goto page_fault;
 goto check_ld;
case LDW: case LDWI:
 if (!load_data(2,&x,w,1)) goto page_fault;
 goto check_ld;
case LDWU: case LDWUI:@/
 if (!load_data(2,&x,w,0)) goto page_fault;
 goto check_ld;
case LDT: case LDTI: 
 if (!load_data(4,&x,w,1)) goto page_fault;
 goto check_ld;
case LDTU: case LDTUI:@/
 if (!load_data(4,&x,w,0)) goto page_fault;
 goto check_ld;
case LDHT: case LDHTI:
 if (!load_data(4,&x,w,0)) goto page_fault;
 x.h=x.l;
 x.l = 0;
 goto check_ld;
case LDO: case LDOI: 
case LDOU: case LDOUI: 
case LDUNC: case LDUNCI:
 if (!load_data(8,&x,w,0)) goto page_fault;
 goto check_ld;
case LDSF: case LDSFI: 
 if (!load_data(4,&x,w,0)) goto page_fault;
 x=load_sf(x.l);
check_ld:
 test_load_bkpt(w);
 if ((w.h&sign_bit) && !(loc.h&sign_bit)) goto privileged_inst;
 goto store_x;
page_fault:
 if ((g[rK].h & g[rQ].h) != 0 || (g[rK].l & g[rQ].l) != 0) 
 { x.h=0, x.l=inst;
   @<Initiate a trap interrupt@>
   inst_ptr=g[rTT];
 }
 break;

@ @<Cases for ind...@>=
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
fin_st:@+ if ((w.h&sign_bit) && !(loc.h&sign_bit)) goto privileged_inst;
 if (!store_data(j,b,w)) goto page_fault;
 test_store_bkpt(w);
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
case STUNC: case STUNCI:
 j = 8;
 goto fin_st;

@ The |CSWAP| operation has elements of both loading and storing.
We shuffle some of
the operands around so that they will appear correctly in the trace output.

The locking of the bus still needs to be implemented!

@<Cases for ind...@>=
case CSWAP: case CSWAPI:
 if (!load_data(8,&a,w,0)) goto page_fault;
 if (g[rP].h==a.h && g[rP].l==a.l) {
   x.h=0, x.l=1;
   test_store_bkpt(w);
   if (!store_data(8,b,w)) goto page_fault;
   strcpy(rhs,"M8[%#w]=%#b");
 }@+else {
   b=a;
   a = g[rP];
   g[rP]=b;
   x.h=0, x.l=0;
   strcpy(rhs,"rP=%#b");
 }
 goto check_ld;
@z


@x
case PUT: case PUTI:@+ if (yy!=0 || xx>=32) goto illegal_inst;
  strcpy(rhs,"%z = %#z");
  if (xx>=8) {
    if (xx<=11) goto illegal_inst; /* can't change rC, rN, rO, rS */
    if (xx<=18) goto privileged_inst;
    if (xx==rA) @<Get ready to update rA@>@;
    else if (xx==rL) @<Set $L=z=\min(z,L)$@>@;
    else if (xx==rG) @<Get ready to update rG@>;
  }
  g[xx]=z;@+zz=xx;@+break;
@y
case PUT: case PUTI:@+ if (yy!=0 || xx>=32) goto illegal_inst;
  strcpy(rhs,"%z = %#z");
  if (xx>=8) {
    if (xx<=9) goto illegal_inst; /* can't change rC, rN */
    if (xx<=18 && !(loc.h&sign_bit)) goto privileged_inst;
    if (xx==rA) @<Get ready to update rA@>@;
    else if (xx==rL) @<Set $L=z=\min(z,L)$@>@;
    else if (xx==rG) @<Get ready to update rG@>;
  }
  g[xx]=z;@+zz=xx;@+break;
@z

@x
case SAVE:@+if (xx<G || yy!=0 || zz!=0) goto illegal_inst;
 l[(O+L)&lring_mask].l=L++;
@y
case SAVE:@+if (xx<G || yy!=0 || zz!=0) goto illegal_inst;
 l[(O+L)&lring_mask].l=L, L++;
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
@y
@<Store |g[k]| in the register stack...@>=
if (k==rZ+1) x.h=G<<24, x.l=g[rA].l;
else x=g[k];
if (!store_data(8,x,g[rS])) goto page_fault;
test_store_bkpt(g[rS]);
if (stack_tracing) {
  tracing=true;
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
test_load_bkpt(g[rS]);
if (k==rZ+1) 
{ if (!load_data(8,&a,g[rS],0)) goto page_fault;
  x.l=G=g[rG].l=a.h>>24;
  a.l=g[rA].l=a.l&0x3ffff;
}
else
 if (!load_data(8,&(g[k]),g[rS],0)) goto page_fault;
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
 /* a bit artificial, since we have no pipeline and can not use the
    time until the instructions are loaded */
 read_instruction(w,xx+1);
 break;
case PRELD: case PRELDI: x=incr(w,xx);@+break;
@z

@x
case SYNC:@+if (xx!=0 || yy!=0 || zz>7) goto illegal_inst;
 if (zz<=3) break;
case LDVTS: case LDVTSI: privileged_inst: strcpy(lhs,"!privileged");
 goto break_inst;
illegal_inst: strcpy(lhs,"!illegal");
break_inst: breakpoint=tracing=true;
 if (!interacting && !interact_after_break) halted=true;
 break;
@y
case SYNC:@+if (xx!=0 || yy!=0 || zz>7) goto illegal_inst;
/* should give a privileged instruction interrupt in case zz  >3 */
 else if (zz==4) /* power save mode */
     get_interrupt(bus_fd, 1, &g[rQ].h,&g[rQ].l);
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
}
break;
privileged_inst: strcpy(lhs,"!privileged");
g[rQ].h |= 0x10; /* set the k bit */
 goto break_inst;
illegal_inst: strcpy(lhs,"!illegal");
g[rQ].h |= 0x20; /* set the b bit */
break_inst: breakpoint=tracing=true;
 if (!interacting && !interact_after_break) halted=true;
 break;
case SWYM: @+break;
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

@ @<Glob...@>=
char arg_count[]={1,3,1,3,3,3,3,2,2,2,1};
char *trap_format[]={
"Halt(%z)",
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

@ @<Prepare memory arguments...@>=
if (arg_count[yy]==3) {
  ll=mem_find(b);@+test_load_bkpt(ll);@+test_load_bkpt(ll+1);
  mb.h=ll->tet, mb.l=(ll+1)->tet;
  ll=mem_find(a);@+test_load_bkpt(ll);@+test_load_bkpt(ll+1);
  ma.h=ll->tet, ma.l=(ll+1)->tet;
}

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
@<Cases for ind...@>=
case TRIP: exc|=H_BIT;@+break;
case TRAP:
 if (inst==0) gdb_signal=-1, breakpoint=true; /* Halt */
 x.h=sign_bit, x.l=inst;
 @<Initiate a trap interrupt@>
 inst_ptr=g[rT];
 break;
@z

@x
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
int mmgetchars(buf,size,addr,stop)
  char *buf;
  int size;
  octa addr;
  int stop;
{
  register char *p;
  register int m;
  octa x;
  octa a;
  for (p=buf,m=0,a=addr; m<size;) {
    if ((a.l&0x7) || m+8>size) @<Read and store one byte; |return| if done@>@;
    else @<Read and store eight bytes; |return| if done@>@;
  }
  return size;
}

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
  octa x;
  octa a;
  for (p=buf,m=0,a=addr; m<size;) {
    test_store_bkpt(a);
    if ((a.l&0x7) || m+8>size) @<Load and write one byte@>@;
    else @<Load and write eight bytes@>;
  }
}

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
@ Just after executing each instruction, we do the following.
Underflow that is exact and not enabled is ignored. (This applies
also to underflow that was triggered by |RESUME_SET|.)

@<Check for trip interrupt@>=
if ((exc&(U_BIT+X_BIT))==U_BIT && !(g[rA].l&U_BIT)) exc &=~U_BIT;
if (exc) {
  if (exc&tracing_exceptions) tracing=true;
  j=exc&(g[rA].l|H_BIT); /* find all exceptions that have been enabled */
  if (j) @<Initiate a trip interrupt@>;
  g[rA].l |= exc>>8;
}
@y
@ Just after executing each instruction, we do the following.
Underflow that is exact and not enabled is ignored. (This applies
also to underflow that was triggered by |RESUME_SET|.)


@<Check for trap and trip interrupt@>=
if (!resuming)
{ if (0>get_interrupt(bus_fd, 0, &g[rQ].h,&g[rQ].l))
    panic("Unable to read rQ");
  if (bus_reset) { breakpoint=true; bus_reset=0; goto boot;}
  if (remotegdb && gdb_interrupt(-1)) breakpoint=true;
  if ((g[rK].h & g[rQ].h) != 0 || (g[rK].l & g[rQ].l) != 0) 
  { /*this is a dynamic trap */
    gdb_signal=3, breakpoint=true;
    x.h=sign_bit, x.l=inst;
    @<Initiate a trap interrupt@>
    inst_ptr=g[rTT];
  }
  else
  {
    if ((exc&(U_BIT+X_BIT))==U_BIT && !(g[rA].l&U_BIT)) exc &=~U_BIT;
    if (exc) {
      if (exc&tracing_exceptions) tracing=true;
      j=exc&(g[rA].l|H_BIT); /* find all exceptions that have been enabled */
      if (j) @<Initiate a trip interrupt@>;
      g[rA].l |= exc>>8;
    }
  }
}

@ We need this:

@<Sub...@>=
 extern int bus_fd;
 extern int gdb_signal;
 extern void wsa_init(void);
 extern void gdb_wait(int socket);
 extern int gdb_interrupt(int socket);
     
@ @<Initiate a trap interrupt@>=
 g[rWW]=inst_ptr;
 g[rXX]=x;
 g[rYY]=y;
 g[rZZ]=z;
 z.h=0, z.l=inst&0xFFFFFF;
 g[rK].h = g[rK].l = 0;
 g[rBB]=g[255];
 g[255]=g[rJ];
@z

@x
case RESUME:@+if (xx || yy || zz) goto illegal_inst;
inst_ptr=z=g[rW];
b=g[rX];
if (!(b.h&sign_bit)) @<Prepare to perform a ropcode@>;
break;

@ Here we check to see if the ropcode restrictions hold.
If so, the ropcode will actually be obeyed on the next fetch phase.

@d RESUME_AGAIN 0 /* repeat the command in rX as if in location $\rm rW-4$ */
@d RESUME_CONT 1 /* same, but substitute rY and rZ for operands */
@d RESUME_SET 2 /* set r[X] to rZ */

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
case RESUME:@+if (xx || yy) goto illegal_inst;
if ( zz == 0)
{ inst_ptr=z=g[rW];
  loc=incr(inst_ptr,-4);
  b=g[rX];
  inst=g[rX].l;
}
else if ( zz == 1)
{ 
  if (!(loc.h&sign_bit)) goto privileged_inst;
  inst_ptr=z=g[rWW];
  loc=incr(inst_ptr,-4);
  b=g[rXX];
  inst=g[rXX].l;
  g[rK]=g[255];
  g[255]=g[rBB];
  resuming=(b.h<0);
}
else goto illegal_inst;
if (!(b.h&sign_bit)) @<Prepare to perform a ropcode@>;
break;

@ Here we check to see if the ropcode restrictions hold.
If so, the ropcode will actually be obeyed on the next fetch phase.

@d RESUME_AGAIN 0 /* repeat the command in rX as if in location $\rm rW-4$ */
@d RESUME_CONT 1 /* same, but substitute rY and rZ for operands */
@d RESUME_SET 2 /* set r[X] to rZ */
@d RESUME_TRANS 3 /* install $\rm(rY,rZ)$ into IT-cache or DT-cache,
        then |RESUME_AGAIN| */

@<Prepare to perform a ropcode@>=
{
  rop=b.h>>24; /* the ropcode is the leading byte of rX */
  switch (rop) {
 case RESUME_CONT:@+if ((1<<(b.l>>28))&0x8f30) goto illegal_inst;
 case RESUME_SET: k=(b.l>>16)&0xff;
   if (k>=L && k<G) goto illegal_inst;
 case RESUME_AGAIN:@+if ((b.l>>24)==RESUME) goto illegal_inst;
   break;
 case RESUME_TRANS:@+if (zz==0) goto illegal_inst;
   break;
 default: goto illegal_inst;
  }
  resuming=true;
}

@ @<Install special operands when resuming an interrupted operation@>=
if (zz == 0)
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
  if (showing_source && cur_line) show_line();
  @<Print the frequency count, the location, and the instruction@>;
  @<Print a stream-of-consciousness description of the instruction@>;
  if (showing_stats || breakpoint) show_stats(breakpoint);
  just_traced=true;
}@+else if (just_traced) {
  printf(" ...............................................\n");
  just_traced=false;
  shown_line=-gap-1; /* gap will not be filled */
}
@y
  @<Print the frequency count, the location, and the instruction@>;
  @<Print a stream-of-consciousness description of the instruction@>;
  if (showing_stats || breakpoint) show_stats(breakpoint);
  just_traced=true;
}@+else if (just_traced) {
  printf(" ...............................................\n");
  just_traced=false;
}
@z

@x
@ @<Print the frequency count, the location, and the instruction@>=
if (resuming && op!=RESUME) {
  switch (rop) {
 case RESUME_AGAIN: printf("           (%08x%08x: %08x (%s)) ",
                     loc.h,loc.l,inst,info[op].name);@+break;
 case RESUME_CONT: printf("           (%08x%08x: %04xrYrZ (%s)) ",
                     loc.h,loc.l,inst>>16,info[op].name);@+break;
 case RESUME_SET: printf("           (%08x%08x: ..%02x..rZ (SET)) ",
                     loc.h,loc.l,(inst>>16)&0xff);@+break;
  }
}@+else {
  ll=mem_find(loc);
  printf("%10d. %08x%08x: %08x (%s) ",ll->freq,loc.h,loc.l,inst,info[op].name);
}
@y
@ @<Print the frequency count, the location, and the instruction@>=
if (resuming && op!=RESUME) {
  switch (rop) {
 case RESUME_AGAIN: printf("           (%08x%08x: %08x (%s)) ",
                     loc.h,loc.l,inst,info[op].name);@+break;
 case RESUME_CONT: printf("           (%08x%08x: %04xrYrZ (%s)) ",
                     loc.h,loc.l,inst>>16,info[op].name);@+break;
 case RESUME_SET: printf("           (%08x%08x: ..%02x..rZ (SET)) ",
                     loc.h,loc.l,(inst>>16)&0xff);@+break;
  }
}@+else {
  printf("%08x%08x: %08x (%s) ",loc.h,loc.l,inst,info[op].name);
}
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
#include "abstime.h"
@y
#include "abstime.h"
#include "message.h"
#include "bus-arith.h"
#include "bus-util.h"
@z

@x
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

@ Here we process the command-line options; when we finish, |*cur_arg|
should be the name of the object file to be loaded and simulated.

@d mmo_file_name *cur_arg

@<Process the command line@>=
myself=argv[0];
for (cur_arg=argv+1;*cur_arg && (*cur_arg)[0]=='-'; cur_arg++)
  scan_option(*cur_arg+1,true);
if (!*cur_arg) scan_option("?",true); /* exit with usage note */
argc -= cur_arg-argv; /* this is the |argc| of the user program */
@y
int main(argc,argv)
  int argc;
  char *argv[];
{ 
  @<Local registers@>;
  @<Process the command line@>;

  if (bushost==NULL) panic("No Bus given. Use Option -B[host:]port");
#ifdef WIN32
  wsa_init();
#endif
  bus_fd=bus_connect(bushost,busport);
  if (bus_fd<0) panic("Unable to connect to Bus.");
  if (0>bus_register(bus_fd,NULL,NULL,0xFFFFFFFF,0xFFFFFFFF,"MMIX CPU"))
    panic("Unable to register with the bus.");
  breakpoint = true;
boot:
  @<Initialize everything@>;

  while (bus_connected)
  {
    while (!bus_power) 
    { if (remotegdb) 
      { gdb_wait(bus_fd);
        bus_poweron(bus_fd,0); /* non blocking */
      }
      else
        bus_poweron(bus_fd,1); /* blocking */
      if (!bus_connected) goto end_simulation;
      if (bus_power) {
        fprintf(stderr,"Power ON\n");
        @<Boot the machine@>;
        @<Load object file@>;
        @<Load the command line arguments@>;
        breakpoint = true;
      }
    }
    do { 
      { unsigned char b;
        b = get_break(inst_ptr);
        if (b&exec_bit) gdb_signal=5, breakpoint=true;
        tracing=breakpoint||(b&trace_bit);
        if (interrupt && !breakpoint) 
          gdb_signal=3, breakpoint=interacting=true, interrupt=false;
      }
      if (breakpoint)
      { breakpoint = false;
        if (remotegdb) gdb_wait(-1);
        else  if (interacting) {
          @<Interact with the user@>;
        }
      }
      if (halted) break;
      do @<Perform one instruction@>@;
      while (resuming);
      if (interact_after_break) interacting=true, interact_after_break=false;
      if (stepping) breakpoint=true,stepping=false;
    } while (bus_power);
  }
  end_simulation:
    if (interacting || profiling || showing_stats) show_stats(true);
  if (bus_fd>=0)
  { bus_unregister(bus_fd);
    bus_disconnect(bus_fd); 
    bus_fd=-1; 
  }
  return g[255].l; /* provide rudimentary feedback for non-interactive runs */
}

@ Here we process the command-line options; when we finish, |*cur_arg|
should be the name of the object file to be loaded and simulated.

@d mmo_file_name *cur_arg

@<Process the command line@>=
myself=argv[0];
for (cur_arg=argv+1;*cur_arg && (*cur_arg)[0]=='-'; cur_arg++)
  scan_option(*cur_arg+1,true);
argc -= cur_arg-argv; /* this is the |argc| of the user program */
@z


@x
 case 's': showing_stats=true;@+return;
 case 'l':@+if (!*(arg+1)) gap=3;
  else if (sscanf(arg+1,"%d",&gap)!=1) gap=0;
  showing_source=true;@+return;
 case 'L':@+if (!*(arg+1)) profile_gap=3;
  else if (sscanf(arg+1,"%d",&profile_gap)!=1) profile_gap=0;
  profile_showing_source=true;
 case 'P': profiling=true;@+return;
 case 'v': trace_threshold=0xffffffff;@+ tracing_exceptions=0xff;
  stack_tracing=true; @+ showing_stats=true;
  gap=10, showing_source=true;
  profile_gap=10, profile_showing_source=true, profiling=true;
  return;
 case 'q': trace_threshold=tracing_exceptions=0;
  stack_tracing=showing_stats=showing_source=false;
  profiling=profile_showing_source=false;
  return;
 case 'i': interacting=true;@+return;
 case 'I': interact_after_break=true;@+return;
 case 'b':@+if (sscanf(arg+1,"%d",&buf_size)!=1) buf_size=0;@+return;
@y
 case 's': showing_stats=true;@+return;
 case 'g': remotegdb=true; breakpoint=true; gdbport=atoi(arg+1);@+ return;
 case 'B': 
  { char *p;
    p = strchr(arg+1,':');
    if (p==NULL)
    { static char localhost[]="localhost";
      bushost=localhost;
      busport = atoi(arg+1);
    }   
    else
    { busport = atoi(p+1);
      bushost = malloc(p+1-arg+1);
      if (bushost==NULL) panic("No room for hostname");
      strncpy(bushost,arg+1,p-arg-1);
      bushost[p-arg-1]=0;
    }
    return;
  } 
 case 'P': profiling=true;@+return;
 case 'v': trace_threshold=0xffffffff;@+ tracing_exceptions=0xff;
  stack_tracing=true; @+ showing_stats=true;
  profiling=true;
  return;
 case 'q': trace_threshold=tracing_exceptions=0;
  stack_tracing=showing_stats=false;
  profiling=false;
  return;
 case 'i': interacting=true;@+return;
 case 'I': interact_after_break=true;@+return;
@z

@x
"-r    trace hidden details of the register stack\n",@|
"-l<n> list source lines when tracing, filling gaps <= n\n",@|
"-s    show statistics after each traced instruction\n",@|
"-P    print a profile when simulation ends\n",@|
"-L<n> list source lines with the profile\n",@|
@y
"-r    trace hidden details of the register stack\n",@|
"-g<n> connect to gdb on port <n>\n",@|
"-B<n> connect to Bus on port <n>\n",@|
"-s    show statistics after each traced instruction\n",@|
"-P    print a profile when simulation ends\n",@|
@z

@x
else mmix_fake_stdin(fake_stdin);
@y
else fprintf(stderr,"Sorry, I can't fake stdin\n");
@z

@x
  case '\n': case 'n': breakpoint=tracing=true; /* trace one inst and break */
  case 'c': goto resume_simulation; /* continue until breakpoint */
@y
  case '\n': case 'n': breakpoint=tracing=true; goto resume_simulation; /* trace one inst and break */
  case 'c': breakpoint=false; goto resume_simulation; /* continue until breakpoint */
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
@ @<Cases that set and clear tracing and breakpoints@>=
case '@@': inst_ptr=scan_hex(p+1,cur_seg);@+ p=next_char;
 halted=false;@+break;
case 't': case 'u': k=*p;
 val=scan_hex(p+1,cur_seg);@+ p=next_char;
 if (val.h<0x20000000) {
   ll=mem_find(val);
   if (k=='t') ll->bkpt |= trace_bit;
   else ll->bkpt &=~trace_bit;
 }
 break;
case 'b':@+ for (k=0,p++; !isxdigit(*p); p++)
   if (*p=='r') k|=read_bit;
   else if (*p=='w') k|=write_bit;
   else if (*p=='x') k|=exec_bit;
 val=scan_hex(p,cur_seg);@+ p=next_char;
 if (!(val.h&sign_bit)) {
   ll=mem_find(val);
   ll->bkpt=(ll->bkpt&-8)|k;
 }
 break;
case 'T': cur_seg.h=0;@+goto passit;
case 'D': cur_seg.h=0x20000000;@+goto passit;
case 'P': cur_seg.h=0x40000000;@+goto passit;
case 'S': cur_seg.h=0x60000000;@+goto passit;
case 'B': show_breaks(mem_root);
passit: p++;@+break;

@ @<Sub...@>=
void show_breaks @,@,@[ARGS((mem_node*))@];@+@t}\6{@>
void show_breaks(p)
  mem_node *p;
{
  register int j;
  octa cur_loc;
  if (p->left) show_breaks(p->left);
  for (j=0;j<512;j++) if (p->dat[j].bkpt) {
    cur_loc=incr(p->loc,4*j);
    printf("  %08x%08x %c%c%c%c\n",cur_loc.h,cur_loc.l,@|
             p->dat[j].bkpt&trace_bit? 't': '-',
             p->dat[j].bkpt&read_bit? 'r': '-',
             p->dat[j].bkpt&write_bit? 'w': '-',
             p->dat[j].bkpt&exec_bit? 'x': '-');
  }
  if (p->right) show_breaks(p->right);
}

@ We put pointers to the command-line strings in
M$[\.{Pool\_Segment}+8*(k+1)]_8$ for $0\le k<|argc|$;
the strings themselves are octabyte-aligned, starting at
M$[\.{Pool\_Segment}+8*(|argc|+2)]_8$. The location of the first free
octabyte in the pool segment is placed in M$[\.{Pool\_Segment}]_8$.
@:Pool_Segment}\.{Pool\_Segment@>
@^command line arguments@>

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

@ @<Get ready to \.{UNSAVE} the initial context@>=
x.h=0, x.l=0x90;
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
@ @<Type...@>=
extern unsigned char get_break(octa a);
extern void set_break(octa a, unsigned char b);
extern void show_breaks(void);

@ @<Cases that set and clear tracing and breakpoints@>=
case '@@': inst_ptr=scan_hex(p+1,cur_seg);@+ p=next_char;
 halted=false;@+break;
case 't': case 'u': k=*p;
 val=scan_hex(p+1,cur_seg);@+ p=next_char;
 if (val.h<0x20000000) {
   if (k=='t') set_break(val,get_break(val)|trace_bit);
   else set_break(val,get_break(val)&~trace_bit);
 }
 break;
case 'b':@+ for (k=0,p++; !isxdigit(*p); p++)
   if (*p=='r') k|=read_bit;
   else if (*p=='w') k|=write_bit;
   else if (*p=='x') k|=exec_bit;
 val=scan_hex(p,cur_seg);@+ p=next_char;
 set_break(val,k);
 break;
case 'T': cur_seg.h=0;@+goto passit;
case 'D': cur_seg.h=0x20000000;@+goto passit;
case 'P': cur_seg.h=0x40000000;@+goto passit;
case 'S': cur_seg.h=0x60000000;@+goto passit;
case 'B': show_breaks();
passit: p++;@+break;


@ We put pointers to the command-line strings in
M$[\.{Pool\_Segment}+8*(k+1)]_8$ for $0\le k<|argc|$;
the strings themselves are octabyte-aligned, starting at
M$[\.{Pool\_Segment}+8*(|argc|+2)]_8$. The location of the first free
octabyte in the pool segment is placed in M$[\.{Pool\_Segment}]_8$.
@:Pool_Segment}\.{Pool\_Segment@>
@^command line arguments@>

@<Load the command line arguments@>=
x.h=0x40000000, x.l=0x8;
aux=incr(x,8*(argc+1));
for (k=0; k<argc; k++,cur_arg++) {
  store_data(8,aux,x);
  mmputchars((unsigned char *)*cur_arg,strlen(*cur_arg),aux);
  x.l+=8, aux.l+=8+(strlen(*cur_arg)&-8);
}
x.l=0;@+ store_data(8,aux,x);


@ We make sure that the processor boots properly.
A user porgram (if loaded) can be started with UNSAVE \$255.

@<Boot the machine@>=
loc.h=inst_ptr.h=0x80000000;
loc.l=inst_ptr.l=0x00000000;
g[rJ].h=g[rJ].l =0xFFFFFFFF;
resuming=false;
@z

