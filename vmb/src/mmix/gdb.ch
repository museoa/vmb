@x
#include "vmb.h"
@y
#include "vmb.h"
#include "gdb.h"
@z


@x
@ We do not load the symbol table. (A more ambitious simulator could
implement \.{MMIXAL}-style expressions for interactive debugging,
but such enhancements are left to the interested reader.)

@<Load object file@>=
#ifdef MMIXLIB
if (full_mmo_name!=NULL && full_mmo_name[0]!=0)
{ mmo_file=fopen(full_mmo_name,"rb");
  if (!mmo_file) {panic("Can't open mmo file");}
#else
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
@y
@z



@x
  ll=mem_find(loc);
  cur_file=ll->file_no;
  cur_line=ll->line_no;
  ll->freq++;
@y
@z

@x
  if (ll->bkpt&exec_bit) breakpoint=true;
  tracing=breakpoint||(ll->bkpt&trace_bit)||(ll->freq<=trace_threshold);
@y
@z

@x
@d test_store_bkpt(ll) if ((ll)->bkpt&write_bit) breakpoint=tracing=true
@y
@d do_store_bkpt breakpoint=tracing=true,gdb_signal=TARGET_SIGNAL_TRAP
@d test_store_bkpt(ll) if ((ll)->bkpt&write_bit) do_store_bkpt
@z

@x
@d test_load_bkpt(ll) if ((ll)->bkpt&read_bit) breakpoint=tracing=true
@y
@d do_load_bkpt breakpoint=tracing=true,gdb_signal=TARGET_SIGNAL_TRAP
@d test_load_bkpt(ll) if ((ll)->bkpt&read_bit) do_load_bkpt
@z


@x
     breakpoint=true;
@y
     breakpoint=true;
     gdb_signal=yz;
@z

@x
break_inst: breakpoint=tracing=true;
@y
break_inst: breakpoint=tracing=true;
  gdb_signal=TARGET_SIGNAL_ILL;
@z

@x
  for (p=buf,m=0,a=addr; m<size;) {
@y
  if (addr.h==0xFFFFFFFF || !valid_address(addr) ) /* gdb goes over segment boundaries */
  { memset(buf,0,size);
    return size;
  }
  for (p=buf,m=0,a=addr; m<size;) {
@z

@x
  for (p=buf,m=0,a=addr; m<size;) {
@y
  if (addr.h==0xFFFFFFFF || !valid_address(addr) ) /* gdb goes over segment boundaries */
    return;
  for (p=buf,m=0,a=addr; m<size;) {
@z


@x
  { breakpoint=true; 
@y
  { breakpoint=true; 
    gdb_signal=TARGET_SIGNAL_PWR;
@z

@x
  { breakpoint = true;
@y
  { breakpoint = true;
    gdb_signal=TARGET_SIGNAL_TRAP;
@z

@x
  init_mmix_bus(host,port,"MMIX CPU");
@y
  init_mmix_bus(host,port,"MMIX CPU");
  if (interacting && gdb_init(gdbport)) 
  { breakpoint = true;
    gdb_signal=TARGET_SIGNAL_TRAP;
  }
@z


@x
  @<Load object file@>;
  @<Load the command line arguments@>;
  g[rQ].h=g[rQ].l=new_Q.h=new_Q.l=0; /*hide problems with loading the command line*/
@y
@z

@x
    if (interrupt && !breakpoint) breakpoint=interacting=true, interrupt=false;
@y
    if (interrupt && !breakpoint) 
    { breakpoint=interacting=true;
      interrupt=false;
      gdb_signal=TARGET_SIGNAL_INT;
    }
@z

@x
        @<Interact with the user@>;
@y
            if (interact_with_gdb()==0)
            { interacting=false;
              goto end_simulation;
            }
@z

@x
    do @<Perform one instruction@>@;
    while ((!interrupt && !breakpoint) || resuming);
    if (interact_after_break) 
       interacting=true, interact_after_break=false;
@y
    do
    {
      @<Perform one instruction@>@;
      { unsigned char b;
        b = get_break(inst_ptr);
        if (b&exec_bit) 
        { breakpoint=true;
          gdb_signal=TARGET_SIGNAL_TRAP;
        }
        tracing=breakpoint||(b&trace_bit);
      }
    } while ((!interrupt && !breakpoint) || resuming);
    if (interact_after_break) interacting=true, interact_after_break=false;
    if (stepping) 
    { breakpoint=true;
      gdb_signal=TARGET_SIGNAL_TRAP;
      stepping=false;
    }
@z

@x
  end_simulation:  
@y
  end_simulation:
  if (interacting) { gdb_signal=-1; interact_with_gdb(); }
@z

@x
 case 'f': @<Open a file for simulated standard input@>;@+return;
@y
@z

@x
"-f<filename> use given file to simulate standard input\n",@|
@y
@z



@x
char *interactive_help[]={@/
"The interactive commands are:\n",@|
"<return>  trace one instruction\n",@|
"n         trace one instruction\n",@|
"c         continue until halt or breakpoint\n",@|
"q         quit the simulation\n",@|
"s         show current statistics\n",@|
"l<n><t>   set and/or show local register in format t\n",@|
"g<n><t>   set and/or show global register in format t\n",@|
"rA<t>     set and/or show register rA in format t\n",@|
"$<n><t>   set and/or show dynamic register in format t\n",@|
"M<x><t>   set and/or show memory octabyte in format t\n",@|
"+<n><t>   set and/or show n additional octabytes in format t\n",@|
" <t> is ! (decimal) or . (floating) or # (hex) or \" (string)\n",@|
"     or <empty> (previous <t>) or =<value> (change value)\n",@|
"@@<x>      go to location x\n",@|
"b[rwx]<x> set or reset breakpoint at location x\n",@|
"t<x>      trace location x\n",@|
"u<x>      untrace location x\n",@|
"T         set current segment to Text_Segment\n",@|
"D         set current segment to Data_Segment\n",@|
"P         set current segment to Pool_Segment\n",@|
"S         set current segment to Stack_Segment\n",@|
"N         set current segment to Negative Addresses\n",@|
"O         enable tracing inside the operating system\n",@|
"o         disable tracing inside the operating system\n",@|
"B         show all current breakpoints and tracepoints\n",@|
"i<file>   insert commands from file\n",@|
"-<option> change a tracing/listing/profile option\n",@|
"-?        show the tracing/listing/profile options  \n",@|
""};

@ @<Open a file for simulated standard input@>=
if (fake_stdin) fclose(fake_stdin);
fake_stdin=fopen(arg+1,"r");
if (!fake_stdin) fprintf(stderr,"Sorry, I can't open file %s!\n",arg+1);
@.Sorry, I can't open...@>
else fprintf(stderr,"Sorry, I can't fake stdin\n");
@y
@z

@x
@ @<Interact with the user@>=
{@+register int repeating;
 interact: @<Put a new command in |command_buf|@>;
 if (vmb_get_interrupt(&vmb,&new_Q.h,&new_Q.l)==1)
  { g[rQ].h |= new_Q.h; g[rQ].l |= new_Q.l; }
  p=command_buf;
  repeating=0;
  switch (*p) {
  case '\n': case 'n': breakpoint=tracing=true; /* trace one inst and break */
  case 'c': goto resume_simulation; /* continue until breakpoint */
  case 'q': goto end_simulation;
  case 's': show_stats(true);@+goto interact;
  case '-': k=strlen(p);@+if (p[k-1]=='\n') p[k-1]='\0';
    scan_option(p+1,false);@+goto interact;
  @t\4@>@<Cases that change |cur_disp_mode|@>;
  @t\4@>@<Cases that define |cur_disp_type|@>;
  @t\4@>@<Cases that set and clear tracing and breakpoints@>;
  default: what_say: k=strlen(command_buf);
    if (k<10 && command_buf[k-1]=='\n') command_buf[k-1]='\0';
    else strcpy(command_buf+9,"...");
    printf("Eh? Sorry, I don't understand `%s'. (Type h for help)\n",
         command_buf);
    goto interact;
  case 'h':@+ for (k=0;interactive_help[k][0];k++) printf("%s",interactive_help[k]);
    goto interact;
  }
  if (*p!='\n') {
   if (!*p) incomplete_str: printf("Syntax error: Incomplete command!\n");
   else {
     p[strlen(p)-1]='\0';
     printf("Syntax error; I'm ignoring `%s'!\n",p);
   }
 }
 while (repeating) @<Display and/or set the value of the current octabyte@>;
 goto interact;
resume_simulation:;
}

@ @<Put a new command...@>=
{@+register bool ready=false;
 incl_read:@+ while (incl_file && !ready)
    if (!fgets(command_buf,command_buf_size,incl_file)) {
      fclose(incl_file);
      incl_file=NULL;
    }@+else if (command_buf[0]!='\n' && command_buf[0]!='i' &&
              command_buf[0]!='%') {
      if (command_buf[0]==' ') printf("%s",command_buf);@+
      else ready=true;@+
    }
  while (!ready) {
    printf("mmix> ");@+fflush(stdout);
@.mmix>@>
    if (!fgets(command_buf,command_buf_size,stdin)) command_buf[0]='q';
    if (command_buf[0]!='i') ready=true;
    else {
      command_buf[strlen(command_buf)-1]='\0';
      incl_file=fopen(command_buf+1,"r");
      if (incl_file) goto incl_read;
      if (isspace(command_buf[1])) incl_file=fopen(command_buf+2,"r");
      if (incl_file) goto incl_read;
      printf("Can't open file `%s'!\n",command_buf+1);
    }
  }
}
@y
@z

@x
@ @d command_buf_size 1024 /* make it plenty long, for floating point tests */
@y
@z

@x
@<Glob...@>=
char command_buf[command_buf_size];
FILE *incl_file; /* file of commands included by `\.i' */
char cur_disp_mode='l'; /* |'l'| or |'g'| or |'$'| or |'M'| */
char cur_disp_type='!'; /* |'!'| or |'.'| or |'#'| or |'"'| */
bool cur_disp_set; /* was the last \.{<t>} of the form \.{=<val>}? */
octa cur_disp_addr; /* the |h| half is relevant only in mode |'M'| */
octa cur_seg; /* current segment offset */
char spec_reg_code[]={rA,rB,rC,rD,rE,rF,rG,rH,rI,rJ,rK,rL,rM,
      rN,rO,rP,rQ,rR,rS,rT,rU,rV,rW,rX,rY,rZ};
char spec_regg_code[]={0,rBB,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,rTT,0,0,rWW,rXX,rYY,rZZ};

@ @<Cases that change |cur_disp_mode|@>=
case 'l': case 'g': case '$': cur_disp_mode=*p++;
 for (cur_disp_addr.l=0; isdigit(*p); p++)
   cur_disp_addr.l=10*cur_disp_addr.l + *p - '0';
 goto new_mode;
case 'r': p++;@+ cur_disp_mode='g';
 if (*p<'A' || *p>'Z') goto what_say;
 if (*(p+1)!=*p) cur_disp_addr.l=spec_reg_code[*p-'A'],p++;
 else if (spec_regg_code[*p-'A']) cur_disp_addr.l=spec_regg_code[*p-'A'],p+=2;
 else goto what_say;
 goto new_mode;
case 'M': cur_disp_mode=*p;
 cur_disp_addr=scan_hex(p+1,cur_seg);@+ cur_disp_addr.l&=-8;@+ p=next_char;
new_mode: cur_disp_set=false; /* the `\.=' is remembered only by `\.+' */
 repeating=1;
 goto scan_type;
case '+':@+ if (!isdigit(*(p+1))) repeating=1;
 for (p++; isdigit(*p); p++)
   repeating=10*repeating + *p - '0';
 if (repeating) {
   if (cur_disp_mode=='M') cur_disp_addr=incr(cur_disp_addr,8);
   else cur_disp_addr.l++;
 }
 goto scan_type;

@ @<Cases that define |cur_disp_type|@>=
case '!': case '.': case '#': case '"': cur_disp_set=false;
 repeating=1;
set_type: cur_disp_type=*p++;@+break;
scan_type:@+ if (*p=='!' || *p=='.' || *p=='#' || *p=='"') goto set_type;
 if (*p!='=') break;
 goto scan_eql;
case '=': repeating=1;
scan_eql: cur_disp_set=true;
 val=zero_octa;
 if (*++p=='#') cur_disp_type=*p, val=scan_hex(p+1,zero_octa);
 else if (*p=='"' || *p=='\'') goto scan_string;
 else cur_disp_type=(scan_const(p)>0? '.': '!');
 p=next_char;
 if (*p!=',') break;
 val.h=0;@+ val.l&=0xff;
scan_string: cur_disp_type='"';
 @<Scan a string constant@>;@+break;

@ @<Subr...@>=
octa scan_hex @,@,@[ARGS((char*,octa))@];@+@t}\6{@>
octa scan_hex(s,offset)
  char *s;
  octa offset;
{
  register char *p;
  octa o;
  o=zero_octa;
  for (p=s;isxdigit(*p);p++) {
    o=incr(shift_left(o,4),*p-'0');
    if (*p>='a') o=incr(o,'0'-'a'+10);
    else if (*p>='A') o=incr(o,'0'-'A'+10);
  }
  next_char=p;
  return oplus(o,offset);
}

@ @<Scan a string constant@>=
while (*p==',') {
  if (*++p=='#') {
    aux=scan_hex(p+1,zero_octa), p=next_char;
    val=incr(shift_left(val,8),aux.l&0xff);
  }@+else if (isdigit(*p)) {
    for (k=*p++ - '0';isdigit(*p);p++) k=(10*k + *p - '0')&0xff;
    val=incr(shift_left(val,8),k);
  }
  else if (*p=='\n') goto incomplete_str;
}
if (*p=='\'' && *(p+2)==*p) *p=*(p+2)='"';
if (*p=='"') {
  for (p++;*p && *p!='\n' && *p!='"'; p++)
    val=incr(shift_left(val,8),*p);
  if (*p && *p++=='"')
    if (*p==',') goto scan_string;
}

@ @<Display and/or set the value of the current octabyte@>=
{
  if (cur_disp_set) @<Set the current octabyte to |val|@>;
  @<Display the current octabyte@>;
  fputc('\n',stdout);
  repeating--;
  if (!repeating) break;
  if (cur_disp_mode=='M') cur_disp_addr=incr(cur_disp_addr,8);
  else cur_disp_addr.l++;
}

@ @<Set the current octabyte to |val|@>=
switch (cur_disp_mode) {
 case 'l': l[cur_disp_addr.l&lring_mask]=val;@+break;
 case '$': k=cur_disp_addr.l&0xff;
  if (k<L) l[(O+k)&lring_mask]=val;@+else if (k>=G) g[k]=val;
  break;
 case 'g': k=cur_disp_addr.l&0xff;
  if (k<32) @<Set |g[k]=val| only if permissible@>;
  g[k]=val;@+break;
 case 'M':
  store_data(8,val,cur_disp_addr);
  @+break;
}
@y
@z

@x
@ Here we essentially simulate a |PUT| command, but we simply |break|
if the |PUT| is illegal or privileged.

@<Set |g[k]=val| only if permissible@>=
if (k>=9 && k!=rI) {
  if (k<=19) break;
  if (k==rA) {
    if (val.h!=0 || val.l>=0x40000) break;
    cur_round=(val.l>=0x10000? val.l>>16: ROUND_NEAR);
  }@+else if (k==rG) {
    if (val.h!=0 || val.l>(unsigned int)255 || val.l<(unsigned int)L || val.l<32) break;
    for (j=val.l; j<G; j++) g[j]=zero_octa;
    G=val.l;
  }@+else if (k==rL) {
    if (val.h==0 && val.l<(unsigned int)L) L=val.l;
    else break;
  }
}
@y
@z

@x
@ @<Display the current octabyte@>=
switch (cur_disp_mode) {
 case 'l': k=cur_disp_addr.l&lring_mask;
  printf("l[%d]=",k);@+ aux=l[k];@+ break;
 case '$': k=cur_disp_addr.l&0xff;
  if (k<L) printf("$%d=l[%d]=",k,(O+k)&lring_mask), aux=l[(O+k)&lring_mask];
  else if (k>=G) printf("$%d=g[%d]=",k,k), aux=g[k];
  else printf("$%d=",k), aux=zero_octa;
  break;
 case 'g': k=cur_disp_addr.l&0xff;
  printf("g[%d]=",k);@+ aux=g[k];@+ break;
 case 'M':
    load_data(8,&aux,cur_disp_addr,0);
  printf("M8[#");@+ print_hex(cur_disp_addr);@+ printf("]=");@+break;
}
switch (cur_disp_type) {
 case '!': print_int(aux);@+break;
 case '.': print_float(aux);@+break;
 case '#': fputc('#',stdout);@+print_hex(aux);@+break;
 case '"': print_string(aux);@+break;
}
@y
@z

@x
@ @<Subr...@>=
void print_string @,@,@[ARGS((octa))@];@+@t}\6{@>
void print_string(o)
  octa o;
{
  register int k, state, b;
  for (k=state=0; k<8; k++) {
    b=((k<4? o.h>>(8*(3-k)): o.l>>(8*(7-k))))&0xff;
    if (b==0) {
      if (state) printf("%s,0",state>1? "\"": ""), state=1;
    }@+else if (b>=' ' && b<='~')
        printf("%s%c",state>1? "": state==1? ",\"": "\"",b), state=2;
    else printf("%s#%x",state>1? "\",": state==1? ",": "",b), state=1;
  }
  if (state==0) printf("0");
  else if (state>1) printf("\"");
}
@y
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
case 'N': cur_seg.h=0x80000000;@+goto passit;
case 'B': show_breaks(mem_root);@+goto passit;
case 'O': show_operating_system=true;@+goto passit;
case 'o': show_operating_system=false;@+goto passit;
passit: p++;@+break;
@y
@z

@x
@ We put pointers to the command-line strings in
M$[\.{Pool\_Segment}+8*(k+1)]_8$ for $0\le k<|argc|$;
the strings themselves are octabyte-aligned, starting at
M$[\.{Pool\_Segment}+8*(|argc|+2)]_8$. The location of the first free
octabyte in the pool segment is placed in M$[\.{Pool\_Segment}]_8$.
@:Pool_Segment}\.{Pool\_Segment@>
@^command line arguments@>
@y
@z

@x
@<Load the command line arguments@>=
x.h=0x40000000, x.l=0x8;
aux=incr(x,8*(argc+1));
for (k=0; k<argc && *cur_arg!=NULL; k++,cur_arg++) {
  store_data(8,aux,x);
  mmputchars((unsigned char *)*cur_arg,strlen(*cur_arg),aux);
  x.l+=8, aux.l+=8+(tetra)(strlen(*cur_arg)&-8);
}
if (argc>0) 
{ x.l=0;
  store_data(8,aux,x);
}
@y
@z
