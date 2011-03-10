#define mm 0x98 \

#define lop_quote 0x0
#define lop_loc 0x1
#define lop_skip 0x2
#define lop_fixo 0x3
#define lop_fixr 0x4
#define lop_fixrx 0x5
#define lop_file 0x6
#define lop_line 0x7
#define lop_spec 0x8
#define lop_pre 0x9
#define lop_post 0xa
#define lop_stab 0xb
#define lop_end 0xc \

#define err(m) {report_error(m) ;if(m[0]!='*') goto bypass;}
#define derr(m,p) {sprintf(err_buf,m,p) ; \
report_error(err_buf) ;if(err_buf[0]!='*') goto bypass;}
#define dderr(m,p,q) {sprintf(err_buf,m,p,q) ; \
report_error(err_buf) ;if(err_buf[0]!='*') goto bypass;}
#define panic(m) {sprintf(err_buf,"!%s",m) ;report_error(err_buf) ;}
#define dpanic(m,p) {err_buf[0]= '!';sprintf(err_buf+1,m,p) ; \
report_error(err_buf) ;} \

#define mmo_write(buf) if(fwrite(buf,1,4,obj_file) !=4)  \
dpanic("Can't write on %s",obj_file_name)  \
 \

#define tex_nl "\\\\ "
#define tex_tab " & "
#define tex_startlinenr ""
#define tex_endlinenr " & "
#define tex_nolabel " & "
#define tex_labelstart " "
#define tex_labelend " & "
#define tex_opcodestart ""
#define tex_opcodeend " & "
#define tex_startop ""
#define tex_endop ""
#define tex_startlcomment " & \\relax "
#define tex_startcomment " \\comment{\\relax "
#define tex_endcomment "}\\\\ "
#define tex_line_wo_comment "\\hidewidth & "
#define tex_startstring "{\\tt \\symbol{34}"
#define tex_endstring "\\symbol{34}}"
#define tex_blank "\\texttt{\\symbol{32}}"
#define tex_braceopen "{\\tt\\symbol{123}}"
#define tex_braceclose "{\\tt\\symbol{125}}"
#define tex_pipe "\\(|\\)"
#define tex_hat "\\^{ }"
#define tex_ll "\\(\\ll\\)"
#define tex_gg "\\(\\gg\\)"
#define TEX_MAXSYM_LEN 511 \

#define isletter(c) (isalpha(c) ||c=='_'||c==':'||c> 126)  \

#define DEFINED (sym_node*) 1
#define REGISTER (sym_node*) 2
#define PREDEFINED (sym_node*) 3
#define fix_o 0
#define fix_yz 1
#define fix_xyz 2 \

#define recycle_fixup(pp) pp->link= sym_avail,sym_avail= pp \

#define rel_addr_bit 0x1
#define immed_bit 0x2
#define zar_bit 0x4
#define zr_bit 0x8
#define yar_bit 0x10
#define yr_bit 0x20
#define xar_bit 0x40
#define xr_bit 0x80
#define yzar_bit 0x100
#define yzr_bit 0x200
#define xyzar_bit 0x400
#define xyzr_bit 0x800
#define one_arg_bit 0x1000
#define two_arg_bit 0x2000
#define three_arg_bit 0x4000
#define many_arg_bit 0x8000
#define align_bits 0x30000
#define no_label_bit 0x40000
#define mem_bit 0x80000
#define spec_bit 0x100000 \

#define top_op op_stack[op_ptr-1]
#define top_val val_stack[val_ptr-1]
#define next_val val_stack[val_ptr-2] \

#define unary_check(verb) if(top_val.status!=pure)  \
derr("can %s pure values only",verb)  \

#define binary_check(verb)  \
if(top_val.status!=pure||next_val.status!=pure)  \
derr("can %s pure values only",verb)  \

#define SETH 0xe0
#define ORH 0xe8
#define ORL 0xeb \

/*139:*/
#line 3137 "./mmixal.w"

#include <stdio.h> 
#include <stdlib.h> 
#include <ctype.h> 
#include <string.h> 
#include <time.h> 

/*31:*/
#line 1008 "./mmixal.w"

#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif

/*:31*//*39:*/
#line 1105 "./mmixal.w"

#ifndef FILENAME_MAX
#define FILENAME_MAX 256
#endif

/*:39*/
#line 3144 "./mmixal.w"

/*26:*/
#line 943 "./mmixal.w"

typedef unsigned int tetra;

typedef struct{tetra h,l;}octa;
typedef enum{false,true}bool;

/*:26*//*30:*/
#line 999 "./mmixal.w"

typedef unsigned char Char;

/*:30*//*57:*/
#line 1427 "./mmixal.w"

typedef struct ternary_trie_struct{
unsigned short ch;
struct ternary_trie_struct*left,*mid,*right;

struct sym_tab_struct*sym;
}trie_node;

/*:57*//*61:*/
#line 1531 "./mmixal.w"

#line 112 "./mmixal.ch"
typedef struct sym_tab_struct{
int serial;
struct sym_tab_struct*link;
octa equiv;
int linenr;
}sym_node;
#line 1537 "./mmixal.w"

/*:61*//*65:*/
#line 1612 "./mmixal.w"

typedef struct{
Char*name;
short code;
int bits;
}op_spec;

typedef enum{
SET= 0x100,IS,LOC,PREFIX,BSPEC,ESPEC,GREG,LOCAL,
BYTE,WYDE,TETRA,OCTA}pseudo_op;

/*:65*//*71:*/
#line 1976 "./mmixal.w"

typedef struct{
Char*name;
tetra h,l;
}predef_spec;

/*:71*//*85:*/
#line 2281 "./mmixal.w"

typedef enum{negate,serialize,complement,registerize,inner_lp,
plus,minus,times,over,frac,mod,shl,shr,and,or,xor,
outer_lp,outer_rp,inner_rp}stack_op;
typedef enum{zero,weak,strong,unary}prec;
typedef enum{pure,reg_val,undefined}stat;
typedef struct{
octa equiv;
trie_node*link;
stat status;
}val_node;

/*:85*/
#line 3145 "./mmixal.w"

/*27:*/
#line 949 "./mmixal.w"

extern octa zero_octa;
extern octa neg_one;
extern octa aux;
extern bool overflow;

/*:27*//*33:*/
#line 1031 "./mmixal.w"

Char*buffer;
Char*buf_ptr;
Char*lab_field;
Char*op_field;
Char*operand_list;
Char*err_buf;

/*:33*//*36:*/
#line 1062 "./mmixal.w"

int cur_file;
int line_no;
bool line_listed;
bool long_warning_given;

/*:36*//*37:*/
#line 1072 "./mmixal.w"

Char*filename[257];

int filename_count;

/*:37*//*43:*/
#line 1156 "./mmixal.w"

octa cur_loc;
octa listing_loc;
unsigned char hold_buf[4];
unsigned char held_bits;
unsigned char listing_bits;
bool spec_mode;
tetra spec_mode_loc;

/*:43*//*46:*/
#line 1240 "./mmixal.w"

int err_count;

/*:46*//*51:*/
#line 1370 "./mmixal.w"

octa mmo_cur_loc;
int mmo_line_no;
int mmo_cur_file;
char filename_passed[256];

/*:51*//*55:*/
#line 37 "./mmixal.ch"

Char tex_res[TEX_MAXSYM_LEN];
Char*start_comment= NULL;
int string_mode= 0;

/*:55*//*59:*/
#line 1452 "./mmixal.w"

trie_node*trie_root;
trie_node*op_root;
trie_node*next_trie_node,*last_trie_node;
trie_node*cur_prefix;

/*:59*//*63:*/
#line 1566 "./mmixal.w"

int serial_number;
sym_node*sym_root;
sym_node*next_sym_node,*last_sym_node;
sym_node*sym_avail;

/*:63*//*66:*/
#line 1623 "./mmixal.w"

op_spec op_init_table[]= {
{"TRAP",0x00,0x27554},

{"FCMP",0x01,0x240a8},

{"FUN",0x02,0x240a8},

{"FEQL",0x03,0x240a8},

{"FADD",0x04,0x240a8},

{"FIX",0x05,0x26288},

{"FSUB",0x06,0x240a8},

{"FIXU",0x07,0x26288},

{"FLOT",0x08,0x26282},

{"FLOTU",0x0a,0x26282},

{"SFLOT",0x0c,0x26282},

{"SFLOTU",0x0e,0x26282},

{"FMUL",0x10,0x240a8},

{"FCMPE",0x11,0x240a8},

{"FUNE",0x12,0x240a8},

{"FEQLE",0x13,0x240a8},

{"FDIV",0x14,0x240a8},

{"FSQRT",0x15,0x26288},

{"FREM",0x16,0x240a8},

{"FINT",0x17,0x26288},

{"MUL",0x18,0x240a2},

{"MULU",0x1a,0x240a2},

{"DIV",0x1c,0x240a2},

{"DIVU",0x1e,0x240a2},

{"ADD",0x20,0x240a2},

{"ADDU",0x22,0x240a2},

{"SUB",0x24,0x240a2},

{"SUBU",0x26,0x240a2},

{"2ADDU",0x28,0x240a2},

{"4ADDU",0x2a,0x240a2},

{"8ADDU",0x2c,0x240a2},

{"16ADDU",0x2e,0x240a2},

{"CMP",0x30,0x240a2},

{"CMPU",0x32,0x240a2},

{"NEG",0x34,0x26082},

{"NEGU",0x36,0x26082},

{"SL",0x38,0x240a2},

{"SLU",0x3a,0x240a2},

{"SR",0x3c,0x240a2},

{"SRU",0x3e,0x240a2},

{"BN",0x40,0x22081},

{"BZ",0x42,0x22081},

{"BP",0x44,0x22081},

{"BOD",0x46,0x22081},

{"BNN",0x48,0x22081},

{"BNZ",0x4a,0x22081},

{"BNP",0x4c,0x22081},

{"BEV",0x4e,0x22081},

{"PBN",0x50,0x22081},

{"PBZ",0x52,0x22081},

{"PBP",0x54,0x22081},

{"PBOD",0x56,0x22081},

{"PBNN",0x58,0x22081},

{"PBNZ",0x5a,0x22081},

{"PBNP",0x5c,0x22081},

{"PBEV",0x5e,0x22081},

{"CSN",0x60,0x240a2},

{"CSZ",0x62,0x240a2},

{"CSP",0x64,0x240a2},

{"CSOD",0x66,0x240a2},

{"CSNN",0x68,0x240a2},

{"CSNZ",0x6a,0x240a2},

{"CSNP",0x6c,0x240a2},

{"CSEV",0x6e,0x240a2},

{"ZSN",0x70,0x240a2},

{"ZSZ",0x72,0x240a2},

{"ZSP",0x74,0x240a2},

{"ZSOD",0x76,0x240a2},

{"ZSNN",0x78,0x240a2},

{"ZSNZ",0x7a,0x240a2},

{"ZSNP",0x7c,0x240a2},

{"ZSEV",0x7e,0x240a2},

{"LDB",0x80,0xa60a2},

{"LDBU",0x82,0xa60a2},

{"LDW",0x84,0xa60a2},

{"LDWU",0x86,0xa60a2},

{"LDT",0x88,0xa60a2},

{"LDTU",0x8a,0xa60a2},

{"LDO",0x8c,0xa60a2},

{"LDOU",0x8e,0xa60a2},

{"LDSF",0x90,0xa60a2},

{"LDHT",0x92,0xa60a2},

{"CSWAP",0x94,0xa60a2},

{"LDUNC",0x96,0xa60a2},

{"LDVTS",0x98,0xa60a2},

{"PRELD",0x9a,0xa6022},

{"PREGO",0x9c,0xa6022},

{"GO",0x9e,0xa60a2},

{"STB",0xa0,0xa60a2},

{"STBU",0xa2,0xa60a2},

{"STW",0xa4,0xa60a2},

{"STWU",0xa6,0xa60a2},

{"STT",0xa8,0xa60a2},

{"STTU",0xaa,0xa60a2},

{"STO",0xac,0xa60a2},

{"STOU",0xae,0xa60a2},

{"STSF",0xb0,0xa60a2},

{"STHT",0xb2,0xa60a2},

{"STCO",0xb4,0xa6022},

{"STUNC",0xb6,0xa60a2},

{"SYNCD",0xb8,0xa6022},

{"PREST",0xba,0xa6022},

{"SYNCID",0xbc,0xa6022},

{"PUSHGO",0xbe,0xa6062},

{"OR",0xc0,0x240a2},

{"ORN",0xc2,0x240a2},

{"NOR",0xc4,0x240a2},

{"XOR",0xc6,0x240a2},

{"AND",0xc8,0x240a2},

{"ANDN",0xca,0x240a2},

{"NAND",0xcc,0x240a2},

{"NXOR",0xce,0x240a2},

{"BDIF",0xd0,0x240a2},

{"WDIF",0xd2,0x240a2},

{"TDIF",0xd4,0x240a2},

{"ODIF",0xd6,0x240a2},

{"MUX",0xd8,0x240a2},

{"SADD",0xda,0x240a2},

{"MOR",0xdc,0x240a2},

{"MXOR",0xde,0x240a2},

{"SETH",0xe0,0x22080},

{"SETMH",0xe1,0x22080},

{"SETML",0xe2,0x22080},

{"SETL",0xe3,0x22080},

{"INCH",0xe4,0x22080},

{"INCMH",0xe5,0x22080},

{"INCML",0xe6,0x22080},

{"INCL",0xe7,0x22080},

{"ORH",0xe8,0x22080},

{"ORMH",0xe9,0x22080},

{"ORML",0xea,0x22080},

{"ORL",0xeb,0x22080},

{"ANDNH",0xec,0x22080},

{"ANDNMH",0xed,0x22080},

{"ANDNML",0xee,0x22080},

{"ANDNL",0xef,0x22080},

{"JMP",0xf0,0x21001},

{"PUSHJ",0xf2,0x22041},

{"GETA",0xf4,0x22081},

{"PUT",0xf6,0x22002},

{"POP",0xf8,0x23000},

{"RESUME",0xf9,0x21000},

{"SAVE",0xfa,0x22080},

{"UNSAVE",0xfb,0x23a00},

{"SYNC",0xfc,0x21000},

{"SWYM",0xfd,0x27554},

{"GET",0xfe,0x22080},

{"TRIP",0xff,0x27554},

{"SET",SET,0x22180},

{"LDA",0x22,0xa60a2},

{"IS",IS,0x101400},

{"LOC",LOC,0x1400},

{"PREFIX",PREFIX,0x141000},

{"BYTE",BYTE,0x10f000},

{"WYDE",WYDE,0x11f000},

{"TETRA",TETRA,0x12f000},

{"OCTA",OCTA,0x13f000},

{"BSPEC",BSPEC,0x41400},

{"ESPEC",ESPEC,0x141000},

{"GREG",GREG,0x101000},

{"LOCAL",LOCAL,0x141800}};

int op_init_size;

/*:66*//*70:*/
#line 1970 "./mmixal.w"

Char*special_name[32]= {"rB","rD","rE","rH","rJ","rM","rR","rBB",
"rC","rN","rO","rS","rI","rT","rTT","rK","rQ","rU","rV","rG","rL",
"rA","rF","rP","rW","rX","rY","rZ","rWW","rXX","rYY","rZZ"};


/*:70*//*72:*/
#line 1982 "./mmixal.w"

predef_spec predefs[]= {
{"ROUND_CURRENT",0,0},

{"ROUND_OFF",0,1},

{"ROUND_UP",0,2},

{"ROUND_DOWN",0,3},

{"ROUND_NEAR",0,4},

{"Inf",0x7ff00000,0},

{"Data_Segment",0x20000000,0},

{"Pool_Segment",0x40000000,0},

{"Stack_Segment",0x60000000,0},

{"D_BIT",0,0x80},

{"V_BIT",0,0x40},

{"W_BIT",0,0x20},

{"I_BIT",0,0x10},

{"O_BIT",0,0x08},

{"U_BIT",0,0x04},

{"Z_BIT",0,0x02},

{"X_BIT",0,0x01},

{"D_Handler",0,0x10},

{"V_Handler",0,0x20},

{"W_Handler",0,0x30},

{"I_Handler",0,0x40},

{"O_Handler",0,0x50},

{"U_Handler",0,0x60},

{"Z_Handler",0,0x70},

{"X_Handler",0,0x80},

{"StdIn",0,0},

{"StdOut",0,1},

{"StdErr",0,2},

{"TextRead",0,0},

{"TextWrite",0,1},

{"BinaryRead",0,2},

{"BinaryWrite",0,3},

{"BinaryReadWrite",0,4},

{"Halt",0,0},

{"Fopen",0,1},

{"Fclose",0,2},

{"Fread",0,3},

{"Fgets",0,4},

{"Fgetws",0,5},

{"Fwrite",0,6},

{"Fputs",0,7},

{"Fputws",0,8},

{"Fseek",0,9},

{"Ftell",0,10}};

int predef_size;


/*:72*//*80:*/
#line 2219 "./mmixal.w"

Char sym_buf[1000];
Char*sym_ptr;

/*:80*//*86:*/
#line 2297 "./mmixal.w"

stack_op*op_stack;
int op_ptr;
val_node*val_stack;
int val_ptr;
prec precedence[]= {unary,unary,unary,unary,zero,
weak,weak,strong,strong,strong,strong,strong,strong,strong,weak,weak,
zero,zero,zero};
stack_op rt_op;
octa acc;

/*:86*//*93:*/
#line 2386 "./mmixal.w"

trie_node forward_local_host[10],backward_local_host[10];
sym_node forward_local[10],backward_local[10];

/*:93*//*108:*/
#line 2634 "./mmixal.w"

tetra opcode;
tetra op_bits;

/*:108*//*123:*/
#line 2867 "./mmixal.w"

tetra z,y,x,yz,xyz;
int future_bits;

/*:123*//*136:*/
#line 3100 "./mmixal.w"

octa greg_val[256];

/*:136*//*142:*/
#line 3207 "./mmixal.w"

char*src_file_name;
char obj_file_name[FILENAME_MAX+1];
#line 257 "./mmixal.ch"
char listing_name[FILENAME_MAX+1];
char tex_file_name[FILENAME_MAX+1];
FILE*src_file,*obj_file,*listing_file,*tex_file;
#line 3212 "./mmixal.w"
int expanding;
int buf_size;

/*:142*//*146:*/
#line 3239 "./mmixal.w"

int greg= 255;
int cur_greg;
int lreg= 32;

/*:146*/
#line 3146 "./mmixal.w"

/*28:*/
#line 961 "./mmixal.w"

extern octa oplus ARGS((octa y,octa z));

extern octa ominus ARGS((octa y,octa z));

extern octa incr ARGS((octa y,int delta));

extern octa oand ARGS((octa y,octa z));

extern octa shift_left ARGS((octa y,int s));

extern octa shift_right ARGS((octa y,int s,int uns));

extern octa omult ARGS((octa y,octa z));

extern octa odiv ARGS((octa x,octa y,octa z));


/*:28*//*41:*/
#line 1127 "./mmixal.w"

void flush_listing_line ARGS((char*));
void flush_listing_line(s)
char*s;
{
if(line_listed)fprintf(listing_file,"\n");
else{
fprintf(listing_file,"%s%s\n",s,buffer);
line_listed= true;
}
}

/*:41*//*42:*/
#line 1143 "./mmixal.w"

void update_listing_loc ARGS((int));
void update_listing_loc(k)
int k;
{
if(cur_loc.h!=listing_loc.h||((cur_loc.l^listing_loc.l)&0xfffff000)){
fprintf(listing_file,"%08x%08x:",cur_loc.h,(cur_loc.l&-4)|k);
flush_listing_line("  ");
}
listing_loc.h= cur_loc.h;
listing_loc.l= (cur_loc.l&-4)|k;
}

/*:42*//*44:*/
#line 1177 "./mmixal.w"

void listing_clear ARGS((void));
void listing_clear()
{
register int j,k;
for(k= 0;k<4;k++)if(listing_bits&(1<<k))break;
if(spec_mode)fprintf(listing_file,"         ");
else{
update_listing_loc(k);
fprintf(listing_file," ...%03x: ",(listing_loc.l&0xffc)|k);
}
for(j= 0;j<4;j++)
if(listing_bits&(0x10<<j))fprintf(listing_file,"xx");
else if(listing_bits&(1<<j))fprintf(listing_file,"%02x",hold_buf[j]);
else fprintf(listing_file,"  ");
flush_listing_line("  ");
listing_bits= 0;
}

/*:44*//*45:*/
#line 1211 "./mmixal.w"

void report_error ARGS((char*));
void report_error(message)
char*message;
{
if(!filename[cur_file])filename[cur_file]= "(nofile)";
if(message[0]=='*')
fprintf(stderr,"\"%s\", line %d warning: %s\n",
filename[cur_file],line_no,message+1);
else if(message[0]=='!')
fprintf(stderr,"\"%s\", line %d fatal error: %s\n",
filename[cur_file],line_no,message+1);
else{
fprintf(stderr,"\"%s\", line %d: %s!\n",
filename[cur_file],line_no,message);
err_count++;
}
if(listing_file){
if(!line_listed)flush_listing_line("****************** ");
if(message[0]=='*')fprintf(listing_file,
"************ warning: %s\n",message+1);
else if(message[0]=='!')fprintf(listing_file,
"******** fatal error: %s!\n",message+1);
else fprintf(listing_file,
"********** error: %s!\n",message);
}
if(message[0]=='!')exit(-2);
}

/*:45*//*47:*/
#line 1254 "./mmixal.w"

void mmo_clear ARGS((void));
void mmo_out ARGS((void));
unsigned char lop_quote_command[4]= {mm,lop_quote,0,1};
void mmo_clear()
{
if(hold_buf[0]==mm)mmo_write(lop_quote_command);
mmo_write(hold_buf);
if(listing_file&&listing_bits)listing_clear();
held_bits= 0;
hold_buf[0]= hold_buf[1]= hold_buf[2]= hold_buf[3]= 0;
mmo_cur_loc= incr(mmo_cur_loc,4);mmo_cur_loc.l&= -4;
if(mmo_line_no)mmo_line_no++;
}

unsigned char mmo_buf[4];
int mmo_ptr;
void mmo_out()
{
if(held_bits)mmo_clear();
mmo_write(mmo_buf);
}

/*:47*//*48:*/
#line 1277 "./mmixal.w"

void mmo_tetra ARGS((tetra));
void mmo_byte ARGS((unsigned char));
void mmo_lop ARGS((char,unsigned char,unsigned char));
void mmo_lopp ARGS((char,unsigned short));
void mmo_tetra(t)
tetra t;
{
mmo_buf[0]= t>>24;mmo_buf[1]= (t>>16)&0xff;
mmo_buf[2]= (t>>8)&0xff;mmo_buf[3]= t&0xff;
mmo_out();
}

void mmo_byte(b)
unsigned char b;
{
mmo_buf[(mmo_ptr++)&3]= b;
if(!(mmo_ptr&3))mmo_out();
}

void mmo_lop(x,y,z)
char x;
unsigned char y,z;
{
mmo_buf[0]= mm;mmo_buf[1]= x;mmo_buf[2]= y;mmo_buf[3]= z;
mmo_out();
}

void mmo_lopp(x,yz)
char x;
unsigned short yz;
{
mmo_buf[0]= mm;mmo_buf[1]= x;
mmo_buf[2]= yz>>8;mmo_buf[3]= yz&0xff;
mmo_out();
}

/*:48*//*49:*/
#line 1317 "./mmixal.w"

void mmo_loc ARGS((void));
void mmo_loc()
{
octa o;
if(held_bits)mmo_clear();
o= ominus(cur_loc,mmo_cur_loc);
if(o.h==0&&o.l<0x10000){
if(o.l)mmo_lopp(lop_skip,o.l);
}else{
if(cur_loc.h&0xffffff){
mmo_lop(lop_loc,0,2);
mmo_tetra(cur_loc.h);
}else mmo_lop(lop_loc,cur_loc.h>>24,1);
mmo_tetra(cur_loc.l);
}
mmo_cur_loc= cur_loc;
}

/*:49*//*50:*/
#line 1339 "./mmixal.w"

void mmo_sync ARGS((void));
void mmo_sync()
{
register int j;register unsigned char*p;
if(cur_file!=mmo_cur_file){
if(filename_passed[cur_file])mmo_lop(lop_file,cur_file,0);
else{
mmo_lop(lop_file,cur_file,(strlen(filename[cur_file])+3)>>2);
for(j= 0,p= filename[cur_file];*p;p++,j= (j+1)&3){
mmo_buf[j]= *p;
if(j==3)mmo_out();
}
if(j){
for(;j<4;j++)mmo_buf[j]= 0;
mmo_out();
}
filename_passed[cur_file]= 1;
}
mmo_cur_file= cur_file;
mmo_line_no= 0;
}
if(line_no!=mmo_line_no){
if(line_no>=0x10000)
panic("I can't deal with line numbers exceeding 65535");

mmo_lopp(lop_line,line_no);
mmo_line_no= line_no;
}
}

/*:50*//*52:*/
#line 1381 "./mmixal.w"

void assemble ARGS((char,tetra,unsigned char));
void assemble(k,dat,x_bits)
char k;
tetra dat;
unsigned char x_bits;
{
register int j,jj,l;
if(spec_mode)l= spec_mode_loc;
else{
l= cur_loc.l;
/*53:*/
#line 1409 "./mmixal.w"

if(cur_loc.h!=mmo_cur_loc.h||((cur_loc.l^mmo_cur_loc.l)&0xfffffffc))
#line 4 "./mmixal.ch"
mmo_loc();

/*:53*/
#line 1392 "./mmixal.w"
;
if(!held_bits&&!(cur_loc.h&0xe0000000))mmo_sync();
}
for(j= 0;j<k;j++){
jj= (l+j)&3;
hold_buf[jj]= (dat>>(8*(k-1-j)))&0xff;
held_bits|= 1<<jj;
listing_bits|= 1<<jj;
}
listing_bits|= x_bits;
if(((l+k)&3)==0){
if(listing_file)listing_clear();
mmo_clear();
}
if(spec_mode)spec_mode_loc+= k;else cur_loc= incr(cur_loc,k);
}

/*:52*//*56:*/
#line 42 "./mmixal.ch"

Char*toTex(Char*symbol)
{int i,j;
i= j= 0;
while(j<TEX_MAXSYM_LEN-1&&symbol[i]){
tex_res[j]= '\0';
switch(symbol[i]){
case'&':
case'#':
case'%':
tex_res[j++]= '\\';break;
case'{':if(strlen(tex_res)<TEX_MAXSYM_LEN-strlen(tex_braceopen))
strcat(tex_res,tex_braceopen);
i++,j+= (int)strlen(tex_braceopen);
continue;
case'}':if(strlen(tex_res)<TEX_MAXSYM_LEN-strlen(tex_braceclose))
strcat(tex_res,tex_braceclose);
i++,j+= (int)strlen(tex_braceclose);
continue;
case'|':if(strlen(tex_res)<TEX_MAXSYM_LEN-strlen(tex_pipe))
strcat(tex_res,tex_pipe);
i++,j+= (int)strlen(tex_pipe);
continue;
case'^':if(strlen(tex_res)<TEX_MAXSYM_LEN-strlen(tex_hat))
strcat(tex_res,tex_hat);
i++,j+= (int)strlen(tex_hat);
continue;
case'>':if(symbol[i+1]=='>'&&strlen(tex_res)<TEX_MAXSYM_LEN-strlen(tex_gg))
strcat(tex_res,tex_gg);
i+= 2,j+= (int)strlen(tex_gg);
continue;
case'<':if(symbol[i+1]=='<'&&strlen(tex_res)<TEX_MAXSYM_LEN-strlen(tex_ll))
strcat(tex_res,tex_ll);
i+= 2,j+= (int)strlen(tex_ll);
continue;
case' ':if(string_mode&&j+strlen(tex_blank)<TEX_MAXSYM_LEN){
tex_res[j]= '\0';
strcat(tex_res,tex_blank);
j+= (int)strlen(tex_blank);}


else break;

i++;
continue;
case'\"':
tex_res[j]= '\0';
if(string_mode){
if(j+strlen(tex_endstring)<TEX_MAXSYM_LEN)
strcat(tex_res,tex_endstring),j+= strlen(tex_endstring);}
else{if(j+strlen(tex_startstring)<TEX_MAXSYM_LEN)
strcat(tex_res,tex_startstring),j+= strlen(tex_startstring);}
string_mode^= 1;
i++;
continue;
}
tex_res[j++]= symbol[i++];
}
tex_res[j]= '\0';
return tex_res;
}
#line 1412 "./mmixal.w"

/*:56*//*58:*/
#line 1437 "./mmixal.w"

trie_node*new_trie_node ARGS((void));
trie_node*new_trie_node()
{
register trie_node*t= next_trie_node;
if(t==last_trie_node){
t= (trie_node*)calloc(1000,sizeof(trie_node));
if(!t)panic("Capacity exceeded: Out of trie memory");

last_trie_node= t+1000;
}
next_trie_node= t+1;
return t;
}

/*:58*//*60:*/
#line 1465 "./mmixal.w"

trie_node*trie_search ARGS((trie_node*,Char*));
Char*terminator;
trie_node*trie_search(t,s)
trie_node*t;
Char*s;
{
register trie_node*tt= t;
register Char*p= s;
while(1){
if(!isletter(*p)&&!isdigit(*p)){
terminator= p;return tt;
}
if(tt->mid){
tt= tt->mid;
while(*p!=tt->ch){
if(*p<tt->ch){
if(tt->left)tt= tt->left;
else{
tt->left= new_trie_node();tt= tt->left;goto store_new_char;
}
}else{
if(tt->right)tt= tt->right;
else{
tt->right= new_trie_node();tt= tt->right;goto store_new_char;
}
}
}
p++;
}else{
tt->mid= new_trie_node();tt= tt->mid;
store_new_char:tt->ch= *p++;
}
}
}

/*:60*//*62:*/
#line 1544 "./mmixal.w"

sym_node*new_sym_node ARGS((bool));
sym_node*new_sym_node(serialize)
bool serialize;
{
register sym_node*p= sym_avail;
if(p){
sym_avail= p->link;p->link= NULL;p->serial= 0;p->equiv= zero_octa;
}else{
p= next_sym_node;
if(p==last_sym_node){
p= (sym_node*)calloc(1000,sizeof(sym_node));
if(!p)panic("Capacity exceeded: Out of symbol memory");

last_sym_node= p+1000;
}
next_sym_node= p+1;
}
if(serialize)p->serial= ++serial_number;
return p;
}

/*:62*//*76:*/
#line 2130 "./mmixal.w"

trie_node*prune ARGS((trie_node*));
trie_node*prune(t)
trie_node*t;
{
register int useful= 0;
if(t->sym){
if(t->sym->serial)useful= 1;
else t->sym= NULL;
}
if(t->left){
t->left= prune(t->left);
if(t->left)useful= 1;
}
if(t->mid){
t->mid= prune(t->mid);
if(t->mid)useful= 1;
}
if(t->right){
t->right= prune(t->right);
if(t->right)useful= 1;
}
if(useful)return t;
else return NULL;
}

/*:76*//*77:*/
#line 2158 "./mmixal.w"

void out_stab ARGS((trie_node*));
void out_stab(t)
trie_node*t;
{
register int m= 0,j;
register sym_node*pp;
if(t->ch> 0xff)m+= 0x80;
if(t->left)m+= 0x40;
if(t->mid)m+= 0x20;
if(t->right)m+= 0x10;
if(t->sym){
if(t->sym->link==REGISTER)m+= 0xf;
else if(t->sym->link==DEFINED)
/*79:*/
#line 2206 "./mmixal.w"

{register tetra x;
if((t->sym->equiv.h&0xffff0000)==0x20000000)
m+= 8,x= t->sym->equiv.h-0x20000000;
else x= t->sym->equiv.h;
if(x)m+= 4;else x= t->sym->equiv.l;
for(j= 1;j<4;j++)if(x<(1<<(8*j)))break;
m+= j;
}

/*:79*/
#line 2172 "./mmixal.w"

else if(t->sym->link||t->sym->serial==1)/*82:*/
#line 2242 "./mmixal.w"

{
*sym_ptr= (m&0x80?'?':t->ch);
*(sym_ptr+1)= '\0';
fprintf(stderr,"undefined symbol: %s\n",sym_buf+1);

err_count++;
m+= 2;
}

/*:82*/
#line 2173 "./mmixal.w"
;
}
mmo_byte(m);
if(t->left)out_stab(t->left);
if(m&0x2f)/*78:*/
#line 2186 "./mmixal.w"

{
if(m&0x80)mmo_byte(t->ch>>8);
mmo_byte(t->ch&0xff);
*sym_ptr++= (m&0x80?'?':t->ch);
m&= 0xf;if(m&&t->sym->link){
if(listing_file)/*81:*/
#line 2229 "./mmixal.w"

{
*sym_ptr= '\0';
fprintf(listing_file," %s = ",sym_buf+1);
pp= t->sym;
if(pp->link==DEFINED)
fprintf(listing_file,"#%08x%08x",pp->equiv.h,pp->equiv.l);
else if(pp->link==REGISTER)
fprintf(listing_file,"$%03d",pp->equiv.l);
else fprintf(listing_file,"?");
#line 123 "./mmixal.ch"
fprintf(listing_file," (%d)\n",pp->serial);
if(tex_file)fprintf(tex_file,"%s%s%s: %s%d%s %s\n",tex_startop,toTex(sym_buf+1),
tex_endop,tex_startlinenr,
pp->linenr,tex_endlinenr,tex_nl);
#line 2240 "./mmixal.w"
}

/*:81*/
#line 2192 "./mmixal.w"
;
if(m==15)m= 1;
else if(m> 8)m-= 8;
for(;m> 0;m--)
if(m> 4)mmo_byte((t->sym->equiv.h>>(8*(m-5)))&0xff);
else mmo_byte((t->sym->equiv.l>>(8*(m-1)))&0xff);
for(m= 0;m<4;m++)if(t->sym->serial<(1<<(7*(m+1))))break;
for(;m>=0;m--)
mmo_byte(((t->sym->serial>>(7*m))&0x7f)+(m?0:0x80));
}
if(t->mid)out_stab(t->mid);
sym_ptr--;
}

/*:78*/
#line 2177 "./mmixal.w"
;
if(t->right)out_stab(t->right);
}

/*:77*/
#line 3147 "./mmixal.w"


int main(argc,argv)
int argc;
char*argv[];
{
register int j,k;
/*40:*/
#line 1110 "./mmixal.w"

register Char*p,*q;

/*:40*//*68:*/
#line 1958 "./mmixal.w"

register trie_node*tt;
register sym_node*pp,*qq;

/*:68*/
#line 3154 "./mmixal.w"
;
/*140:*/
#line 3174 "./mmixal.w"

for(j= 1;j<argc-1&&argv[j][0]=='-';j++)if(!argv[j][2]){
if(argv[j][1]=='x')expanding= 1;
#line 233 "./mmixal.ch"
else if(argv[j][1]=='o')j++,strcpy(obj_file_name,argv[j]);
else if(argv[j][1]=='t')j++,strcpy(tex_file_name,argv[j]);
#line 3178 "./mmixal.w"
else if(argv[j][1]=='l')j++,strcpy(listing_name,argv[j]);
else if(argv[j][1]=='b'&&sscanf(argv[j+1],"%d",&buf_size)==1)j++;
else break;
}else if(argv[j][1]!='b'||sscanf(argv[j]+1,"%d",&buf_size)!=1)break;
if(j!=argc-1){
fprintf(stderr,"Usage: %s %s sourcefilename\n",

#line 240 "./mmixal.ch"
argv[0],"[-x] [-l listingname] [-t TeXfilename] [-b buffersize] [-o objectfilename]");
#line 3186 "./mmixal.w"
exit(-1);
}
src_file_name= argv[j];

/*:140*/
#line 3155 "./mmixal.w"
;
/*29:*/
#line 981 "./mmixal.w"

acc= shift_left(neg_one,1);
if(acc.h!=0xffffffff)panic("Type tetra is not implemented correctly");


/*:29*//*32:*/
#line 1020 "./mmixal.w"

if(buf_size<72)buf_size= 72;
buffer= (Char*)calloc(buf_size+1,sizeof(Char));
lab_field= (Char*)calloc(buf_size+1,sizeof(Char));
op_field= (Char*)calloc(buf_size,sizeof(Char));
operand_list= (Char*)calloc(buf_size,sizeof(Char));
err_buf= (Char*)calloc(buf_size+60,sizeof(Char));
if(!buffer||!lab_field||!op_field||!operand_list||!err_buf)
panic("No room for the buffers");


/*:32*//*64:*/
#line 1577 "./mmixal.w"

trie_root= new_trie_node();
cur_prefix= trie_root;
op_root= new_trie_node();
trie_root->mid= op_root;
trie_root->ch= ':';
op_root->ch= '^';
/*67:*/
#line 1949 "./mmixal.w"

op_init_size= (sizeof op_init_table)/sizeof(op_spec);
for(j= 0;j<op_init_size;j++){
tt= trie_search(op_root,op_init_table[j].name);
pp= tt->sym= new_sym_node(false);
pp->link= PREDEFINED;
pp->equiv.h= op_init_table[j].code,pp->equiv.l= op_init_table[j].bits;
}

/*:67*/
#line 1584 "./mmixal.w"
;
/*69:*/
#line 1962 "./mmixal.w"

for(j= 0;j<32;j++){
tt= trie_search(trie_root,special_name[j]);
pp= tt->sym= new_sym_node(false);
pp->link= PREDEFINED;
pp->equiv.l= j;
}

/*:69*/
#line 1585 "./mmixal.w"
;
/*73:*/
#line 2075 "./mmixal.w"

predef_size= (sizeof predefs)/sizeof(predef_spec);
for(j= 0;j<predef_size;j++){
tt= trie_search(trie_root,predefs[j].name);
pp= tt->sym= new_sym_node(false);
pp->link= PREDEFINED;
pp->equiv.h= predefs[j].h,pp->equiv.l= predefs[j].l;
}

/*:73*/
#line 1586 "./mmixal.w"
;

/*:64*//*74:*/
#line 2089 "./mmixal.w"

trie_search(trie_root,"Main")->sym= new_sym_node(true);

/*:74*//*87:*/
#line 2308 "./mmixal.w"

op_stack= (stack_op*)calloc(buf_size,sizeof(stack_op));
val_stack= (val_node*)calloc(buf_size,sizeof(val_node));
if(!op_stack||!val_stack)panic("No room for the stacks");


/*:87*//*94:*/
#line 2392 "./mmixal.w"

for(j= 0;j<10;j++){
forward_local_host[j].sym= &forward_local[j];
backward_local_host[j].sym= &backward_local[j];
backward_local[j].link= DEFINED;
}

/*:94*//*143:*/
#line 3215 "./mmixal.w"

/*141:*/
#line 3190 "./mmixal.w"

src_file= fopen(src_file_name,"r");
if(!src_file)dpanic("Can't open the source file %s",src_file_name);

if(!obj_file_name[0]){
j= strlen(src_file_name);
if(src_file_name[j-1]=='s'){
strcpy(obj_file_name,src_file_name);obj_file_name[j-1]= 'o';
}else sprintf(obj_file_name,"%s.mmo",src_file_name);
}
obj_file= fopen(obj_file_name,"wb");
if(!obj_file)dpanic("Can't open the object file %s",obj_file_name);
#line 246 "./mmixal.ch"
if(tex_file_name[0]){
tex_file= fopen(tex_file_name,"w");
if(!tex_file)dpanic("Can't open the TeX file %s",tex_file_name);
}
if(listing_name[0]){
#line 3203 "./mmixal.w"
listing_file= fopen(listing_name,"w");
if(!listing_file)dpanic("Can't open the listing file %s",listing_name);
}

/*:141*/
#line 3216 "./mmixal.w"
;
filename[0]= src_file_name;
filename_count= 1;
/*144:*/
#line 3221 "./mmixal.w"

mmo_lop(lop_pre,1,1);
mmo_tetra(time(NULL));
mmo_cur_file= -1;

/*:144*/
#line 3219 "./mmixal.w"
;

/*:143*/
#line 3156 "./mmixal.w"
;
while(1){
#line 214 "./mmixal.ch"
/*34:*/
#line 1039 "./mmixal.w"

if(!fgets(buffer,buf_size+1,src_file))break;
line_no++;
line_listed= false;
j= strlen(buffer);
if(buffer[j-1]=='\n')buffer[j-1]= '\0';
else if((j= fgetc(src_file))!=EOF)
/*35:*/
#line 1050 "./mmixal.w"

{
while(j!='\n'&&j!=EOF)j= fgetc(src_file);
if(!long_warning_given){
long_warning_given= true;
err("*trailing characters of long input line have been dropped");

fprintf(stderr,
"(say `-b <number>' to increase the length of my input buffer)\n");
}else err("*trailing characters dropped");
}

/*:35*/
#line 1046 "./mmixal.w"
;
if(buffer[0]=='#')/*38:*/
#line 1080 "./mmixal.w"

{
for(p= buffer+1;isspace(*p);p++);
for(j= *p++-'0';isdigit(*p);p++)j= 10*j+*p-'0';
for(;isspace(*p);p++);
if(*p=='\"'){
if(!filename[filename_count]){
filename[filename_count]= (Char*)calloc(FILENAME_MAX+1,sizeof(Char));
if(!filename[filename_count])
panic("Capacity exceeded: Out of filename memory");

}
for(p++,q= filename[filename_count];*p&&*p!='\"';p++,q++)*q= *p;
if(*p=='\"'&&*(p-1)!='\"'){
*q= '\0';
for(k= 0;strcmp(filename[k],filename[filename_count])!=0;k++);
if(k==filename_count)filename_count++;
cur_file= k;
line_no= j-1;
}
}
}

/*:38*/
#line 1047 "./mmixal.w"
;
buf_ptr= buffer;

/*:34*/
#line 214 "./mmixal.ch"
;
if(tex_file)
fprintf(tex_file,"%s%d %s",tex_startlinenr,line_no,tex_endlinenr);
#line 3159 "./mmixal.w"
while(1){
/*105:*/
#line 2578 "./mmixal.w"

p= buf_ptr;buf_ptr= "";
/*106:*/
#line 2599 "./mmixal.w"

#line 132 "./mmixal.ch"
if(!*p){if(tex_file)fprintf(tex_file,"%s%s",tex_nolabel,tex_nl);goto bypass;}
#line 2601 "./mmixal.w"
q= lab_field;
if(!isspace(*p)){
#line 138 "./mmixal.ch"
if(!isdigit(*p)&&!isletter(*p)){

if(tex_file&&*p)
if(*p=='#')
fprintf(tex_file,"\\relax %s%s%s\\hidewidth%s",
tex_startop,toTex(p),tex_endop,tex_nl);
else
fprintf(tex_file,"%s%s%s\n",tex_startcomment,p,tex_endcomment);
goto bypass;
}
#line 2604 "./mmixal.w"
for(*q++= *p++;isdigit(*p)||isletter(*p);p++,q++)*q= *p;
if(*p&&!isspace(*p))derr("label syntax error at `%c'",*p);

}
*q= '\0';
if(isdigit(lab_field[0])&&(lab_field[1]!='H'||lab_field[2]))
derr("improper local label `%s'",lab_field);

#line 153 "./mmixal.ch"
if(tex_file)
fprintf(tex_file,"%s %s %s",tex_labelstart,toTex(lab_field),tex_labelend);
for(p++;isspace(*p);p++);
#line 2613 "./mmixal.w"

/*:106*/
#line 2580 "./mmixal.w"
;
/*107:*/
#line 2617 "./mmixal.w"

q= op_field;
while(isletter(*p)||isdigit(*p))*q++= *p++;
*q= '\0';
#line 161 "./mmixal.ch"
if(!isspace(*p)&&*p&&op_field[0]){
if(tex_file&&op_field)fprintf(tex_file,"%s%s%s%s",op_field,tex_tab,p,tex_nl);
derr("opcode syntax error at `%c'",*p);
}
#line 2622 "./mmixal.w"

pp= trie_search(op_root,op_field)->sym;
#line 170 "./mmixal.ch"
if(!pp){
if(tex_file)fprintf(tex_file,tex_nl);
#line 2625 "./mmixal.w"
if(op_field[0])derr("unknown operation code `%s'",op_field);

if(lab_field[0])derr("*no opcode; label `%s' will be ignored",lab_field);

goto bypass;
}
#line 177 "./mmixal.ch"
opcode= pp->equiv.h,op_bits= pp->equiv.l;
if(tex_file)fprintf(tex_file,"%s %s %s",tex_opcodestart,toTex(op_field),tex_opcodeend);
#line 2632 "./mmixal.w"
while(isspace(*p))p++;

/*:107*/
#line 2581 "./mmixal.w"
;
/*109:*/
#line 2641 "./mmixal.w"

q= operand_list;
while(*p){
if(*p==';')break;
if(*p=='\''){
*q++= *p++;
if(!*p)err("incomplete character constant");

*q++= *p++;
if(*p!='\'')err("illegal character constant");

}else if(*p=='\"'){
for(*q++= *p++;*p&&*p!='\"';p++,q++)*q= *p;
if(!*p)err("incomplete string constant");
}
*q++= *p++;
if(isspace(*p))break;
}
while(isspace(*p))p++;
if(*p==';')p++;
#line 184 "./mmixal.ch"
else start_comment= p,p= "";
#line 2662 "./mmixal.w"
if(q==operand_list)*q++= '0';
#line 190 "./mmixal.ch"
*q= '\0';
if(tex_file)fprintf(tex_file,"%s%s%s",tex_startop,toTex(operand_list),tex_endop);
#line 2664 "./mmixal.w"

/*:109*/
#line 2582 "./mmixal.w"
;
buf_ptr= p;
if(spec_mode&&!(op_bits&spec_bit))
derr("cannot use `%s' in special mode",op_field);

if((op_bits&no_label_bit)&&lab_field[0]){
derr("*label field of `%s' instruction is ignored",op_field);
lab_field[0]= '\0';
}

if(op_bits&align_bits)/*110:*/
#line 2668 "./mmixal.w"

{
j= (op_bits&align_bits)>>16;
acc.h= -1,acc.l= -(1<<j);
cur_loc= oand(incr(cur_loc,(1<<j)-1),acc);
}

/*:110*/
#line 2592 "./mmixal.w"
;
/*88:*/
#line 2317 "./mmixal.w"

p= operand_list;
val_ptr= 0;
op_stack[0]= outer_lp,op_ptr= 1;

while(1){
/*89:*/
#line 2333 "./mmixal.w"

scan_open:if(isletter(*p))/*90:*/
#line 2359 "./mmixal.w"

{
if(*p==':')tt= trie_search(trie_root,p+1);
else tt= trie_search(cur_prefix,p);
p= terminator;
symbol_found:val_ptr++;
pp= tt->sym;
if(!pp)pp= tt->sym= new_sym_node(true);
top_val.link= tt,top_val.equiv= pp->equiv;
if(pp->link==PREDEFINED)pp->link= DEFINED;
top_val.status= (pp->link==DEFINED?pure:pp->link==REGISTER?reg_val:
undefined);
}

/*:90*/
#line 2334 "./mmixal.w"

else if(isdigit(*p)){
if(*(p+1)=='F')/*91:*/
#line 2373 "./mmixal.w"

{
tt= &forward_local_host[*p-'0'];p+= 2;goto symbol_found;
}

/*:91*/
#line 2336 "./mmixal.w"

else if(*(p+1)=='B')/*92:*/
#line 2378 "./mmixal.w"

{
tt= &backward_local_host[*p-'0'];p+= 2;goto symbol_found;
}

/*:92*/
#line 2337 "./mmixal.w"

else/*97:*/
#line 2415 "./mmixal.w"

acc.h= 0,acc.l= *p-'0';
for(p++;isdigit(*p);p++){
acc= oplus(acc,shift_left(acc,2));
acc= incr(shift_left(acc,1),*p-'0');
}
constant_found:val_ptr++;
top_val.link= NULL;
top_val.equiv= acc;
top_val.status= pure;

/*:97*/
#line 2338 "./mmixal.w"
;
}else switch(*p++){
case'#':/*98:*/
#line 2426 "./mmixal.w"

if(!isxdigit(*p))err("illegal hexadecimal constant");

acc.h= acc.l= 0;
for(;isxdigit(*p);p++){
acc= incr(shift_left(acc,4),*p-'0');
if(*p>='a')acc= incr(acc,'0'-'a'+10);
else if(*p>='A')acc= incr(acc,'0'-'A'+10);
}
goto constant_found;

/*:98*/
#line 2340 "./mmixal.w"
;break;
case'\'':/*95:*/
#line 2401 "./mmixal.w"

acc.h= 0,acc.l= *p;
p+= 2;
goto constant_found;

/*:95*/
#line 2341 "./mmixal.w"
;break;
case'\"':/*96:*/
#line 2406 "./mmixal.w"

acc.h= 0,acc.l= *p;
if(*p=='\"'){
p++;acc.l= 0;err("*null string is treated as zero");

}else if(*(p+1)=='\"')p+= 2;
else*p= '\"',*--p= ',';
goto constant_found;

/*:96*/
#line 2342 "./mmixal.w"
;break;
case'@':/*99:*/
#line 2437 "./mmixal.w"

acc= cur_loc;
goto constant_found;

/*:99*/
#line 2343 "./mmixal.w"
;break;
case'-':op_stack[op_ptr++]= negate;
case'+':goto scan_open;
case'&':op_stack[op_ptr++]= serialize;goto scan_open;
case'~':op_stack[op_ptr++]= complement;goto scan_open;
case'$':op_stack[op_ptr++]= registerize;goto scan_open;
case'(':op_stack[op_ptr++]= inner_lp;goto scan_open;
default:if(p==operand_list+1){
operand_list[0]= '0',operand_list[1]= '\0',p= operand_list;
goto scan_open;
}
if(*(p-1))derr("syntax error at character `%c'",*(p-1));
derr("syntax error after character `%c'",*(p-2));

}

/*:89*/
#line 2323 "./mmixal.w"
;
scan_close:/*100:*/
#line 2441 "./mmixal.w"

switch(*p++){
case'+':rt_op= plus;break;
case'-':rt_op= minus;break;
case'*':rt_op= times;break;
case'/':if(*p!='/')rt_op= over;
else p++,rt_op= frac;break;
case'%':rt_op= mod;break;
case'<':rt_op= shl;goto sh_check;
case'>':rt_op= shr;
sh_check:if(*p++==*(p-1))break;
derr("syntax error at `%c'",*(p-2));

case'&':rt_op= and;break;
case'|':rt_op= or;break;
case'^':rt_op= xor;break;
case')':rt_op= inner_rp;break;
case'\0':case',':rt_op= outer_rp;break;
default:derr("syntax error at `%c'",*(p-1));
}

/*:100*/
#line 2324 "./mmixal.w"
;
while(precedence[top_op]>=precedence[rt_op])
/*101:*/
#line 2462 "./mmixal.w"

switch(op_stack[--op_ptr]){
case inner_lp:if(rt_op==inner_rp)goto scan_close;
err("*missing right parenthesis");break;

case outer_lp:if(rt_op==outer_rp){
if(top_val.status==reg_val&&(top_val.equiv.l> 0xff||top_val.equiv.h)){
err("*register number too large, will be reduced mod 256");

top_val.equiv.h= 0,top_val.equiv.l&= 0xff;
}
if(!*(p-1))goto operands_done;
else rt_op= outer_lp;goto hold_op;
}else{
op_ptr++;
err("*missing left parenthesis");

goto scan_close;
}
/*103:*/
#line 2508 "./mmixal.w"

case negate:unary_check("negate");

top_val.equiv= ominus(zero_octa,top_val.equiv);goto delink;
case complement:unary_check("complement");

top_val.equiv.h= ~top_val.equiv.h,top_val.equiv.l= ~top_val.equiv.l;
goto delink;
case registerize:unary_check("registerize");

top_val.status= reg_val;goto delink;
case serialize:if(!top_val.link)
err("can take serial number of symbol only");

top_val.equiv.h= 0,top_val.equiv.l= top_val.link->sym->serial;
top_val.status= pure;goto delink;

/*:103*/
#line 2481 "./mmixal.w"

/*102:*/
#line 2492 "./mmixal.w"

case plus:if(top_val.status==undefined)
err("cannot add an undefined quantity");

if(next_val.status==undefined)
err("cannot add to an undefined quantity");
if(top_val.status==reg_val&&next_val.status==reg_val)
err("cannot add two register numbers");
next_val.equiv= oplus(next_val.equiv,top_val.equiv);
fin_bin:next_val.status= (top_val.status==next_val.status?pure:reg_val);
val_ptr--;
delink:top_val.link= NULL;break;

/*:102*//*104:*/
#line 2529 "./mmixal.w"

case minus:if(top_val.status==undefined)
err("cannot subtract an undefined quantity");

if(next_val.status==undefined)
err("cannot subtract from an undefined quantity");
if(top_val.status==reg_val&&next_val.status!=reg_val)
err("cannot subtract register number from pure value");
next_val.equiv= ominus(next_val.equiv,top_val.equiv);goto fin_bin;
case times:binary_check("multiply");

next_val.equiv= omult(next_val.equiv,top_val.equiv);goto fin_bin;
case over:case mod:binary_check("divide");

if(top_val.equiv.l==0&&top_val.equiv.h==0)
err("*division by zero");

next_val.equiv= odiv(zero_octa,next_val.equiv,top_val.equiv);
if(op_stack[op_ptr]==mod)next_val.equiv= aux;
goto fin_bin;
case frac:binary_check("compute a ratio of");

if(next_val.equiv.h>=top_val.equiv.h&&
(next_val.equiv.l>=top_val.equiv.l||next_val.equiv.h> top_val.equiv.h))
err("*illegal fraction");

next_val.equiv= odiv(next_val.equiv,zero_octa,top_val.equiv);goto fin_bin;
case shl:case shr:binary_check("compute a bitwise shift of");
if(top_val.equiv.h||top_val.equiv.l> 63)next_val.equiv= zero_octa;
else if(op_stack[op_ptr]==shl)
next_val.equiv= shift_left(next_val.equiv,top_val.equiv.l);
else next_val.equiv= shift_right(next_val.equiv,top_val.equiv.l,true);
goto fin_bin;
case and:binary_check("compute bitwise and of");
next_val.equiv.h&= top_val.equiv.h,next_val.equiv.l&= top_val.equiv.l;
goto fin_bin;
case or:binary_check("compute bitwise or of");
next_val.equiv.h|= top_val.equiv.h,next_val.equiv.l|= top_val.equiv.l;
goto fin_bin;
case xor:binary_check("compute bitwise xor of");
next_val.equiv.h^= top_val.equiv.h,next_val.equiv.l^= top_val.equiv.l;
goto fin_bin;

/*:104*/
#line 2482 "./mmixal.w"

}

/*:101*/
#line 2326 "./mmixal.w"
;
hold_op:op_stack[op_ptr++]= rt_op;
}
operands_done:

/*:88*/
#line 2593 "./mmixal.w"
;
if(opcode==GREG)/*111:*/
#line 2675 "./mmixal.w"

{
if(val_stack[0].equiv.l||val_stack[0].equiv.h){
for(j= greg;j<255;j++)
if(greg_val[j].l==val_stack[0].equiv.l&&
greg_val[j].h==val_stack[0].equiv.h){
cur_greg= j;goto got_greg;
}
}
if(greg==32)err("too many global registers");

greg--;
greg_val[greg]= val_stack[0].equiv;cur_greg= greg;
got_greg:;
}

/*:111*/
#line 2594 "./mmixal.w"
;
if(lab_field[0])/*112:*/
#line 2702 "./mmixal.w"

{
sym_node*new_link= DEFINED;
acc= cur_loc;
if(opcode==IS){
cur_loc= val_stack[0].equiv;
if(val_stack[0].status==reg_val)new_link= REGISTER;
}else if(opcode==GREG)cur_loc.h= 0,cur_loc.l= cur_greg,new_link= REGISTER;
/*114:*/
#line 2741 "./mmixal.w"

if(isdigit(lab_field[0]))pp= &forward_local[lab_field[0]-'0'];
else{
if(lab_field[0]==':')tt= trie_search(trie_root,lab_field+1);
else tt= trie_search(cur_prefix,lab_field);
pp= tt->sym;
if(!pp)pp= tt->sym= new_sym_node(true);
}

/*:114*/
#line 2710 "./mmixal.w"
;
if(pp->link==DEFINED||pp->link==REGISTER){
if(pp->equiv.l!=cur_loc.l||pp->equiv.h!=cur_loc.h||pp->link!=new_link){
if(pp->serial)derr("symbol `%s' is already defined",lab_field);

pp->serial= ++serial_number;
derr("*redefinition of predefined symbol `%s'",lab_field);

}
}else if(pp->link==PREDEFINED)pp->serial= ++serial_number;
else if(pp->link){
if(new_link==REGISTER)err("future reference cannot be to a register");

do/*115:*/
#line 2750 "./mmixal.w"

{
qq= pp->link;
pp->link= qq->link;
mmo_loc();
if(qq->serial==fix_o)/*116:*/
#line 2760 "./mmixal.w"

{
if(qq->equiv.h&0xffffff){
mmo_lop(lop_fixo,0,2);
mmo_tetra(qq->equiv.h);
}else mmo_lop(lop_fixo,qq->equiv.h>>24,1);
mmo_tetra(qq->equiv.l);
}

/*:116*/
#line 2755 "./mmixal.w"

else/*117:*/
#line 2769 "./mmixal.w"

{
octa o;
o= ominus(cur_loc,qq->equiv);
if(o.l&3)
dderr("*relative address in location #%08x%08x not divisible by 4",

qq->equiv.h,qq->equiv.l);
o= shift_right(o,2,0);
k= 0;
if(o.h==0)
if(o.l<0x10000)mmo_lopp(lop_fixr,o.l);
else if(qq->serial==fix_xyz&&o.l<0x1000000){
mmo_lop(lop_fixrx,0,24);mmo_tetra(o.l);
}else k= 1;
else if(o.h==0xffffffff)
if(qq->serial==fix_xyz&&o.l>=0xff000000){
mmo_lop(lop_fixrx,0,24);mmo_tetra(o.l&0x1ffffff);
}else if(qq->serial==fix_yz&&o.l>=0xffff0000){
mmo_lop(lop_fixrx,0,16);mmo_tetra(o.l&0x100ffff);
}else k= 1;
else k= 1;
if(k)dderr("relative address in location #%08x%08x is too far away",
qq->equiv.h,qq->equiv.l);
}

/*:117*/
#line 2756 "./mmixal.w"
;
recycle_fixup(qq);
}

/*:115*/
#line 2723 "./mmixal.w"
while(pp->link);
}
if(isdigit(lab_field[0]))pp= &backward_local[lab_field[0]-'0'];
#line 197 "./mmixal.ch"
pp->equiv= cur_loc;pp->link= new_link;pp->linenr= line_no;
#line 2727 "./mmixal.w"
/*113:*/
#line 2733 "./mmixal.w"

if(!isdigit(lab_field[0]))
for(j= 0;j<val_ptr;j++)
if(val_stack[j].status==undefined&&val_stack[j].link->sym==pp){
val_stack[j].status= (new_link==REGISTER?reg_val:pure);
val_stack[j].equiv= cur_loc;
}

/*:113*/
#line 2727 "./mmixal.w"
;
if(listing_file&&(opcode==IS||opcode==LOC))
/*118:*/
#line 2795 "./mmixal.w"

if(new_link==DEFINED){
fprintf(listing_file,"(%08x%08x)",cur_loc.h,cur_loc.l);
flush_listing_line(" ");
}else{
fprintf(listing_file,"($%03d)",cur_loc.l&0xff);
flush_listing_line("             ");
}

/*:118*/
#line 2729 "./mmixal.w"
;
cur_loc= acc;
}

/*:112*/
#line 2595 "./mmixal.w"
;
/*119:*/
#line 2804 "./mmixal.w"

future_bits= 0;
if(op_bits&many_arg_bit)/*120:*/
#line 2826 "./mmixal.w"

for(j= 0;j<val_ptr;j++){
/*121:*/
#line 2842 "./mmixal.w"

if(val_stack[j].status==reg_val)
err("*register number used as a constant")

else if(val_stack[j].status==undefined){
if(opcode!=OCTA)err("undefined constant");

pp= val_stack[j].link->sym;
qq= new_sym_node(false);
qq->link= pp->link;
pp->link= qq;
qq->serial= fix_o;
qq->equiv= cur_loc;
}

/*:121*/
#line 2828 "./mmixal.w"
;
k= 1<<(opcode-BYTE);
if((val_stack[j].equiv.h&&opcode<OCTA)||
(val_stack[j].equiv.l> 0xffff&&opcode<TETRA)||
(val_stack[j].equiv.l> 0xff&&opcode<WYDE))
if(k==1)err("*constant doesn't fit in one byte")

else derr("*constant doesn't fit in %d bytes",k);
if(k<8)assemble(k,val_stack[j].equiv.l,0);
else if(val_stack[j].status==undefined)
assemble(4,0,0xf0),assemble(4,0,0xf0);
else assemble(4,val_stack[j].equiv.h,0),assemble(4,val_stack[j].equiv.l,0);
}

/*:120*/
#line 2806 "./mmixal.w"

else switch(val_ptr){
case 1:if(!(op_bits&one_arg_bit))
derr("opcode `%s' needs more than one operand",op_field);

/*132:*/
#line 3018 "./mmixal.w"

if(val_stack[0].status==undefined){
if(op_bits&rel_addr_bit)
/*133:*/
#line 3041 "./mmixal.w"

{
pp= val_stack[0].link->sym;
qq= new_sym_node(false);
qq->link= pp->link;
pp->link= qq;
qq->serial= fix_xyz;
qq->equiv= cur_loc;
xyz= 0;
future_bits= 0xe0;
goto assemble_inst;
}

/*:133*/
#line 3021 "./mmixal.w"

else if(opcode!=PREFIX)err("the operand is undefined");

}else if(val_stack[0].status==reg_val){
if(!(op_bits&(xyzr_bit+xyzar_bit)))
derr("*operand of `%s' should not be a register number",op_field);

}else{
if(op_bits&xyzr_bit)
derr("*operand of `%s' should be a register number",op_field);
if(op_bits&rel_addr_bit)
/*134:*/
#line 3054 "./mmixal.w"

{
octa source,dest;
if(val_stack[0].equiv.l&3)
err("*relative address is not divisible by 4");

source= shift_right(cur_loc,2,0);
dest= shift_right(val_stack[0].equiv,2,0);
acc= ominus(dest,source);
if(!(acc.h&0x80000000)){
if(acc.l> 0xffffff||acc.h)
err("relative address is more than #ffffff tetrabytes forward");
}else{
acc= incr(acc,0x1000000);
opcode++;
if(acc.l> 0xffffff||acc.h)
err("relative address is more than #1000000 tetrabytes backward");
}
xyz= acc.l;
goto assemble_inst;
}

/*:134*/
#line 3032 "./mmixal.w"
;
}
if(opcode> 0xff)/*135:*/
#line 3076 "./mmixal.w"

switch(opcode){
case LOC:cur_loc= val_stack[0].equiv;
case IS:goto bypass;
case PREFIX:if(!val_stack[0].link)err("not a valid prefix");

cur_prefix= val_stack[0].link;goto bypass;
case GREG:if(listing_file)/*137:*/
#line 3103 "./mmixal.w"

if(val_stack[0].equiv.l||val_stack[0].equiv.h){
fprintf(listing_file,"($%03d=#%08x",cur_greg,val_stack[0].equiv.h);
flush_listing_line("    ");
fprintf(listing_file,"         %08x)",val_stack[0].equiv.l);
flush_listing_line(" ");
}else{
fprintf(listing_file,"($%03d)",cur_greg);
flush_listing_line("             ");
}

/*:137*/
#line 3083 "./mmixal.w"
;
goto bypass;
case LOCAL:if(val_stack[0].equiv.l> lreg)lreg= val_stack[0].equiv.l;
if(listing_file){
fprintf(listing_file,"($%03d)",val_stack[0].equiv.l);
flush_listing_line("             ");
}
goto bypass;
case BSPEC:if(val_stack[0].equiv.l> 0xffff||val_stack[0].equiv.h)
err("*operand of `BSPEC' doesn't fit in two bytes");

mmo_loc();mmo_sync();
mmo_lopp(lop_spec,val_stack[0].equiv.l);
spec_mode= true;spec_mode_loc= 0;goto bypass;
case ESPEC:spec_mode= false;goto bypass;
}

/*:135*/
#line 3034 "./mmixal.w"
;
if(val_stack[0].equiv.h||val_stack[0].equiv.l> 0xffffff)
err("*XYZ field doesn't fit in three bytes");

xyz= val_stack[0].equiv.l&0xffffff;
goto assemble_inst;

/*:132*/
#line 2811 "./mmixal.w"
;
case 2:if(!(op_bits&two_arg_bit))
if(op_bits&one_arg_bit)
derr("opcode `%s' must not have two operands",op_field)
else derr("opcode `%s' must have more than two operands",op_field);
/*127:*/
#line 2916 "./mmixal.w"

if(val_stack[1].status==undefined){
if(op_bits&rel_addr_bit)
/*128:*/
#line 2946 "./mmixal.w"

{
pp= val_stack[1].link->sym;
qq= new_sym_node(false);
qq->link= pp->link;
pp->link= qq;
qq->serial= fix_yz;
qq->equiv= cur_loc;
yz= 0;
future_bits= 0xc0;
goto assemble_X;
}

/*:128*/
#line 2919 "./mmixal.w"

else err("YZ field is undefined");

}else if(val_stack[1].status==reg_val){
if(!(op_bits&(immed_bit+yzr_bit+yzar_bit)))
derr("*YZ field of `%s' should not be a register number",op_field);

if(opcode==SET)val_stack[1].equiv.l<<= 8,opcode= 0xc1;
else if(op_bits&mem_bit)
val_stack[1].equiv.l<<= 8,opcode++;
}else{
if(op_bits&mem_bit)
/*130:*/
#line 2981 "./mmixal.w"

{
octa o;
o= val_stack[1].equiv,k= 0;
for(j= greg;j<255;j++)if(greg_val[j].h||greg_val[j].l){
acc= ominus(val_stack[1].equiv,greg_val[j]);
if(acc.h<=o.h&&(acc.l<=o.l||acc.h<o.h))o= acc,k= j;
}
if(o.l<=0xff&&!o.h&&k)yz= (k<<8)+o.l,opcode++;
else if(!expanding)err("no base address is close enough to the address A")

else/*131:*/
#line 3000 "./mmixal.w"

{
for(j= SETH;j<=ORL;j++){
switch(j&3){
case 0:yz= o.h>>16;break;
case 1:yz= o.h&0xffff;break;
case 2:yz= o.l>>16;break;
case 3:yz= o.l&0xffff;break;
}
if(yz){
assemble(4,(j<<24)+(255<<16)+yz,0);
j|= ORH;
}
}
if(k)yz= (k<<8)+255;
else yz= 255<<8,opcode++;
}

/*:131*/
#line 2992 "./mmixal.w"
;
goto assemble_X;
}

/*:130*/
#line 2931 "./mmixal.w"
;
if(opcode==SET)opcode= 0xe3;
else if(op_bits&immed_bit)opcode++;
else if(op_bits&yzr_bit){
derr("*YZ field of `%s' should be a register number",op_field);
}
if(op_bits&rel_addr_bit)
/*129:*/
#line 2959 "./mmixal.w"

{
octa source,dest;
if(val_stack[1].equiv.l&3)
err("*relative address is not divisible by 4");

source= shift_right(cur_loc,2,0);
dest= shift_right(val_stack[1].equiv,2,0);
acc= ominus(dest,source);
if(!(acc.h&0x80000000)){
if(acc.l> 0xffff||acc.h)
err("relative address is more than #ffff tetrabytes forward");
}else{
acc= incr(acc,0x10000);
opcode++;
if(acc.l> 0xffff||acc.h)
err("relative address is more than #10000 tetrabytes backward");
}
yz= acc.l;
goto assemble_X;
}

/*:129*/
#line 2938 "./mmixal.w"
;
}
if(val_stack[1].equiv.h||val_stack[1].equiv.l> 0xffff)
err("*YZ field doesn't fit in two bytes");

yz= val_stack[1].equiv.l&0xffff;
goto assemble_X;

/*:127*/
#line 2816 "./mmixal.w"
;
case 3:if(!(op_bits&three_arg_bit))
derr("opcode `%s' must not have three operands",op_field);
/*122:*/
#line 2857 "./mmixal.w"

/*124:*/
#line 2871 "./mmixal.w"

if(val_stack[2].status==undefined)err("Z field is undefined");

if(val_stack[2].status==reg_val){
if(!(op_bits&(immed_bit+zr_bit+zar_bit)))
derr("*Z field of `%s' should not be a register number",op_field);

}else if(op_bits&immed_bit)opcode++;
else if(op_bits&zr_bit)
derr("*Z field of `%s' should be a register number",op_field);
if(val_stack[2].equiv.h||val_stack[2].equiv.l> 0xff)
err("*Z field doesn't fit in one byte");

z= val_stack[2].equiv.l&0xff;

/*:124*/
#line 2858 "./mmixal.w"
;
/*125:*/
#line 2886 "./mmixal.w"

if(val_stack[1].status==undefined)err("Y field is undefined");

if(val_stack[1].status==reg_val){
if(!(op_bits&(yr_bit+yar_bit)))
derr("*Y field of `%s' should not be a register number",op_field);

}else if(op_bits&yr_bit)
derr("*Y field of `%s' should be a register number",op_field);
if(val_stack[1].equiv.h||val_stack[1].equiv.l> 0xff)
err("*Y field doesn't fit in one byte");

y= val_stack[1].equiv.l&0xff;
yz= (y<<8)+z;

/*:125*/
#line 2859 "./mmixal.w"
;
assemble_X:/*126:*/
#line 2901 "./mmixal.w"

if(val_stack[0].status==undefined)err("X field is undefined");

if(val_stack[0].status==reg_val){
if(!(op_bits&(xr_bit+xar_bit)))
derr("*X field of `%s' should not be a register number",op_field);

}else if(op_bits&xr_bit)
derr("*X field of `%s' should be a register number",op_field);
if(val_stack[0].equiv.h||val_stack[0].equiv.l> 0xff)
err("*X field doesn't fit in one byte");

x= val_stack[0].equiv.l&0xff;
xyz= (x<<16)+yz;

/*:126*/
#line 2860 "./mmixal.w"
;
assemble_inst:assemble(4,(opcode<<24)+xyz,future_bits);
break;

/*:122*/
#line 2819 "./mmixal.w"
;
default:derr("too many operands for opcode `%s'",op_field);

}

/*:119*/
#line 2596 "./mmixal.w"
;
bypass:

/*:105*/
#line 3160 "./mmixal.w"
;
if(!*buf_ptr)break;
}
#line 222 "./mmixal.ch"
if(tex_file&&start_comment){
if(*start_comment)
fprintf(tex_file,"%s%s%s\n",tex_startlcomment,start_comment,tex_nl);
else fprintf(tex_file,"%s%s\n",tex_line_wo_comment,tex_nl);
start_comment= NULL;}
if(listing_file){
#line 3164 "./mmixal.w"
if(listing_bits)listing_clear();
else if(!line_listed)flush_listing_line("                   ");
}
}
/*145:*/
#line 3226 "./mmixal.w"

if(lreg>=greg)
dpanic("Danger: Must reduce the number of GREGs by %d",lreg-greg+1);

/*147:*/
#line 3244 "./mmixal.w"

mmo_lop(lop_post,0,greg);
greg_val[255]= trie_search(trie_root,"Main")->sym->equiv;
for(j= greg;j<256;j++){
mmo_tetra(greg_val[j].h);
mmo_tetra(greg_val[j].l);
}

/*:147*/
#line 3230 "./mmixal.w"
;
/*83:*/
#line 2252 "./mmixal.w"

op_root->mid= NULL;
prune(trie_root);
sym_ptr= sym_buf;
if(listing_file)fprintf(listing_file,"\nSymbol table:\n");
mmo_lop(lop_stab,0,0);
out_stab(trie_root);
while(mmo_ptr&3)mmo_byte(0);
mmo_lopp(lop_end,mmo_ptr>>2);

/*:83*/
#line 3231 "./mmixal.w"
;
/*148:*/
#line 3252 "./mmixal.w"

for(j= 0;j<10;j++)if(forward_local[j].link)
err_count++,fprintf(stderr,"undefined local symbol %dF\n",j);


/*:148*/
#line 3232 "./mmixal.w"
;
if(err_count){
if(err_count> 1)fprintf(stderr,"(%d errors were found.)\n",err_count);
else fprintf(stderr,"(One error was found.)\n");
}
exit(err_count);

/*:145*/
#line 3168 "./mmixal.w"
;
}

/*:139*/
