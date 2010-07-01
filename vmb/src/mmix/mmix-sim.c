#define panic(m) {fprintf(stderr,"Panic: %s!\n",m) ;exit(-2) ;}
#define sign_bit ((unsigned) 0x80000000)  \

#define mmo_esc 0x98
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

#define mmo_err { \
fprintf(stderr,"Bad object file! (Try running MMOtype.)\n") ; \
 \
exit(-4) ; \
} \

#define ybyte buf[2]
#define zbyte buf[3] \

#define X_BIT (1<<8) 
#define Z_BIT (1<<9) 
#define U_BIT (1<<10) 
#define O_BIT (1<<11) 
#define I_BIT (1<<12) 
#define W_BIT (1<<13) 
#define V_BIT (1<<14) 
#define D_BIT (1<<15) 
#define H_BIT (1<<16)  \

#define trace_bit (1<<3) 
#define read_bit (1<<2) 
#define write_bit (1<<1) 
#define exec_bit (1<<0)  \

#define max_sys_call Ftell \

#define Z_is_immed_bit 0x1
#define Z_is_source_bit 0x2
#define Y_is_immed_bit 0x4
#define Y_is_source_bit 0x8
#define X_is_source_bit 0x10
#define X_is_dest_bit 0x20
#define rel_addr_bit 0x40
#define push_pop_bit 0x80 \

#define VERSION 1
#define SUBVERSION 0
#define SUBSUBVERSION 1 \

#define test_store_bkpt(a) if(get_break(a) &write_bit) breakpoint= tracing= true \

#define test_load_bkpt(a) if(get_break(a) &read_bit) breakpoint= tracing= true \

#define shift_amt (z.h||z.l>=64?64:z.l)  \

#define cmp_zero store_x \

#define ROUND_OFF 1
#define ROUND_UP 2
#define ROUND_DOWN 3
#define ROUND_NEAR 4 \

#define P_BIT (1<<0) 
#define S_BIT (1<<1) 
#define B_BIT (1<<2) 
#define K_BIT (1<<3) 
#define N_BIT (1<<4) 
#define PX_BIT (1<<5) 
#define PW_BIT (1<<6) 
#define PR_BIT (1<<7)  \

#define RESUME_AGAIN 0
#define RESUME_CONT 1
#define RESUME_SET 2
#define RESUME_TRANS 3 \
 \

#define rhs &switchable_string[1] \

#define mmo_file_name *cur_arg \

#define command_buf_size 1024 \

/*124:*/
#line 2869 "mmix-sim.w"

#include <stdio.h> 
#include <stdlib.h> 
#include <ctype.h> 
#include <string.h> 
#include <signal.h> 
#include "abstime.h"
#ifdef WIN32
#include <windows.h> 
#endif
/*11:*/
#line 570 "mmix-sim.w"

#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif

/*:11*/
#line 2879 "mmix-sim.w"

/*9:*/
#line 548 "mmix-sim.w"

typedef enum{false,true}bool;

/*:9*//*10:*/
#line 560 "mmix-sim.w"

typedef unsigned int tetra;

typedef struct{tetra h,l;}octa;
typedef unsigned char byte;

/*:10*//*16:*/
#line 171 "mmix-sim.ch"

#include <time.h> 
#include "address.h"
#include "mmix-bus.h"
#include "vmb.h"
device_info vmb= {0};
#line 822 "mmix-sim.w"

/*:16*//*33:*/
#line 1273 "mmix-sim.w"

typedef enum{
TRAP,FCMP,FUN,FEQL,FADD,FIX,FSUB,FIXU,
FLOT,FLOTI,FLOTU,FLOTUI,SFLOT,SFLOTI,SFLOTU,SFLOTUI,
FMUL,FCMPE,FUNE,FEQLE,FDIV,FSQRT,FREM,FINT,
MUL,MULI,MULU,MULUI,DIV,DIVI,DIVU,DIVUI,
ADD,ADDI,ADDU,ADDUI,SUB,SUBI,SUBU,SUBUI,
IIADDU,IIADDUI,IVADDU,IVADDUI,VIIIADDU,VIIIADDUI,XVIADDU,XVIADDUI,
CMP,CMPI,CMPU,CMPUI,NEG,NEGI,NEGU,NEGUI,
SL,SLI,SLU,SLUI,SR,SRI,SRU,SRUI,
BN,BNB,BZ,BZB,BP,BPB,BOD,BODB,
BNN,BNNB,BNZ,BNZB,BNP,BNPB,BEV,BEVB,
PBN,PBNB,PBZ,PBZB,PBP,PBPB,PBOD,PBODB,
PBNN,PBNNB,PBNZ,PBNZB,PBNP,PBNPB,PBEV,PBEVB,
CSN,CSNI,CSZ,CSZI,CSP,CSPI,CSOD,CSODI,
CSNN,CSNNI,CSNZ,CSNZI,CSNP,CSNPI,CSEV,CSEVI,
ZSN,ZSNI,ZSZ,ZSZI,ZSP,ZSPI,ZSOD,ZSODI,
ZSNN,ZSNNI,ZSNZ,ZSNZI,ZSNP,ZSNPI,ZSEV,ZSEVI,
LDB,LDBI,LDBU,LDBUI,LDW,LDWI,LDWU,LDWUI,
LDT,LDTI,LDTU,LDTUI,LDO,LDOI,LDOU,LDOUI,
LDSF,LDSFI,LDHT,LDHTI,CSWAP,CSWAPI,LDUNC,LDUNCI,
LDVTS,LDVTSI,PRELD,PRELDI,PREGO,PREGOI,GO,GOI,
STB,STBI,STBU,STBUI,STW,STWI,STWU,STWUI,
STT,STTI,STTU,STTUI,STO,STOI,STOU,STOUI,
STSF,STSFI,STHT,STHTI,STCO,STCOI,STUNC,STUNCI,
SYNCD,SYNCDI,PREST,PRESTI,SYNCID,SYNCIDI,PUSHGO,PUSHGOI,
OR,ORI,ORN,ORNI,NOR,NORI,XOR,XORI,
AND,ANDI,ANDN,ANDNI,NAND,NANDI,NXOR,NXORI,
BDIF,BDIFI,WDIF,WDIFI,TDIF,TDIFI,ODIF,ODIFI,
MUX,MUXI,SADD,SADDI,MOR,MORI,MXOR,MXORI,
SETH,SETMH,SETML,SETL,INCH,INCMH,INCML,INCL,
ORH,ORMH,ORML,ORL,ANDNH,ANDNMH,ANDNML,ANDNL,
JMP,JMPB,PUSHJ,PUSHJB,GETA,GETAB,PUT,PUTI,
POP,RESUME,SAVE,UNSAVE,SYNC,SWYM,GET,TRIP}mmix_opcode;

/*:33*//*34:*/
#line 1310 "mmix-sim.w"

typedef enum{
rB,rD,rE,rH,rJ,rM,rR,rBB,
rC,rN,rO,rS,rI,rT,rTT,rK,rQ,rU,rV,rG,rL,
rA,rF,rP,rW,rX,rY,rZ,rWW,rXX,rYY,rZZ}special_reg;

/*:34*//*38:*/
#line 1349 "mmix-sim.w"

typedef enum{
Halt,Fopen,Fclose,Fread,Fgets,Fgetws,
Fwrite,Fputs,Fputws,Fseek,Ftell}sys_call;

/*:38*//*43:*/
#line 1428 "mmix-sim.w"

typedef struct{
char*name;
unsigned char flags;
unsigned char third_operand;
unsigned char mems;
unsigned char oops;
char*trace_format;
}op_info;

/*:43*//*118:*/
#line 2794 "mmix-sim.w"

typedef enum{decimal,hex,zhex,floating,handle}fmt_style;

/*:118*//*144:*/
#line 2003 "mmix-sim.ch"

extern unsigned char get_break(octa a);
extern void set_break(octa a,unsigned char b);
extern void show_breaks(void);

/*:144*/
#line 2880 "mmix-sim.w"

/*19:*/
#line 868 "mmix-sim.w"

FILE*mmo_file;
int postamble;
int byte_count;
byte buf[4];
int yzbytes;
int delta;
tetra tet;

/*:19*//*26:*/
#line 278 "mmix-sim.ch"

octa cur_loc;
octa tmp;
tetra obj_time;

/*:26*//*35:*/
#line 1316 "mmix-sim.w"

char*special_name[32]= {"rB","rD","rE","rH","rJ","rM","rR","rBB",
"rC","rN","rO","rS","rI","rT","rTT","rK","rQ","rU","rV","rG","rL",
"rA","rF","rP","rW","rX","rY","rZ","rWW","rXX","rYY","rZZ"};

/*:35*//*40:*/
#line 1382 "mmix-sim.w"

octa w,x,y,z,a,b,ma,mb;
octa*x_ptr;
octa loc;
octa inst_ptr;
tetra inst;
int old_L;
int exc;
int tracing_exceptions;
int rop;
int round_mode;
bool resuming;
bool halted;
bool breakpoint;
bool tracing;
bool stack_tracing;
#line 644 "mmix-sim.ch"
static bool interacting;
static bool show_operating_system= false;
static int busport= 9002;
static char localhost[]= "localhost";
static char*bushost= localhost;
static bool hostclock= false;
#line 1399 "mmix-sim.w"
bool interact_after_break;
bool tripping;
bool good;
tetra trace_threshold;

/*:40*//*44:*/
#line 1459 "mmix-sim.w"

op_info info[256]= {
/*45:*/
#line 1466 "mmix-sim.w"

{"TRAP",0x0a,255,0,5,"%r"},
{"FCMP",0x2a,0,0,1,"%l = %.y cmp %.z = %x"},
{"FUN",0x2a,0,0,1,"%l = [%.y(||)%.z] = %x"},
{"FEQL",0x2a,0,0,1,"%l = [%.y(==)%.z] = %x"},
{"FADD",0x2a,0,0,4,"%l = %.y %(+%) %.z = %.x"},
{"FIX",0x26,0,0,4,"%l = %(fix%) %.z = %x"},
{"FSUB",0x2a,0,0,4,"%l = %.y %(-%) %.z = %.x"},
{"FIXU",0x26,0,0,4,"%l = %(fix%) %.z = %#x"},
{"FLOT",0x26,0,0,4,"%l = %(flot%) %z = %.x"},
{"FLOTI",0x25,0,0,4,"%l = %(flot%) %z = %.x"},
{"FLOTU",0x26,0,0,4,"%l = %(flot%) %#z = %.x"},
{"FLOTUI",0x25,0,0,4,"%l = %(flot%) %z = %.x"},
{"SFLOT",0x26,0,0,4,"%l = %(sflot%) %z = %.x"},
{"SFLOTI",0x25,0,0,4,"%l = %(sflot%) %z = %.x"},
{"SFLOTU",0x26,0,0,4,"%l = %(sflot%) %#z = %.x"},
{"SFLOTUI",0x25,0,0,4,"%l = %(sflot%) %z = %.x"},
{"FMUL",0x2a,0,0,4,"%l = %.y %(*%) %.z = %.x"},
{"FCMPE",0x2a,rE,0,4,"%l = %.y cmp %.z (%.b)) = %x"},
{"FUNE",0x2a,rE,0,1,"%l = [%.y(||)%.z (%.b)] = %x"},
{"FEQLE",0x2a,rE,0,4,"%l = [%.y(==)%.z (%.b)] = %x"},
{"FDIV",0x2a,0,0,40,"%l = %.y %(/%) %.z = %.x"},
{"FSQRT",0x26,0,0,40,"%l = %(sqrt%) %.z = %.x"},
{"FREM",0x2a,0,0,4,"%l = %.y %(rem%) %.z = %.x"},
{"FINT",0x26,0,0,4,"%l = %(int%) %.z = %.x"},
{"MUL",0x2a,0,0,10,"%l = %y * %z = %x"},
{"MULI",0x29,0,0,10,"%l = %y * %z = %x"},
{"MULU",0x2a,0,0,10,"%l = %#y * %#z = %#x, rH=%#a"},
{"MULUI",0x29,0,0,10,"%l = %#y * %z = %#x, rH=%#a"},
{"DIV",0x2a,0,0,60,"%l = %y / %z = %x, rR=%a"},
{"DIVI",0x29,0,0,60,"%l = %y / %z = %x, rR=%a"},
{"DIVU",0x2a,rD,0,60,"%l = %#b%0y / %#z = %#x, rR=%#a"},
{"DIVUI",0x29,rD,0,60,"%l = %#b%0y / %z = %#x, rR=%#a"},
{"ADD",0x2a,0,0,1,"%l = %y + %z = %x"},
{"ADDI",0x29,0,0,1,"%l = %y + %z = %x"},
{"ADDU",0x2a,0,0,1,"%l = %#y + %#z = %#x"},
{"ADDUI",0x29,0,0,1,"%l = %#y + %z = %#x"},
{"SUB",0x2a,0,0,1,"%l = %y - %z = %x"},
{"SUBI",0x29,0,0,1,"%l = %y - %z = %x"},
{"SUBU",0x2a,0,0,1,"%l = %#y - %#z = %#x"},
{"SUBUI",0x29,0,0,1,"%l = %#y - %z = %#x"},
{"2ADDU",0x2a,0,0,1,"%l = %#y <<1+ %#z = %#x"},
{"2ADDUI",0x29,0,0,1,"%l = %#y <<1+ %z = %#x"},
{"4ADDU",0x2a,0,0,1,"%l = %#y <<2+ %#z = %#x"},
{"4ADDUI",0x29,0,0,1,"%l = %#y <<2+ %z = %#x"},
{"8ADDU",0x2a,0,0,1,"%l = %#y <<3+ %#z = %#x"},
{"8ADDUI",0x29,0,0,1,"%l = %#y <<3+ %z = %#x"},
{"16ADDU",0x2a,0,0,1,"%l = %#y <<4+ %#z = %#x"},
{"16ADDUI",0x29,0,0,1,"%l = %#y <<4+ %z = %#x"},
{"CMP",0x2a,0,0,1,"%l = %y cmp %z = %x"},
{"CMPI",0x29,0,0,1,"%l = %y cmp %z = %x"},
{"CMPU",0x2a,0,0,1,"%l = %#y cmp %#z = %x"},
{"CMPUI",0x29,0,0,1,"%l = %#y cmp %z = %x"},
{"NEG",0x26,0,0,1,"%l = %y - %z = %x"},
{"NEGI",0x25,0,0,1,"%l = %y - %z = %x"},
{"NEGU",0x26,0,0,1,"%l = %y - %#z = %#x"},
{"NEGUI",0x25,0,0,1,"%l = %y - %z = %#x"},
{"SL",0x2a,0,0,1,"%l = %y << %#z = %x"},
{"SLI",0x29,0,0,1,"%l = %y << %z = %x"},
{"SLU",0x2a,0,0,1,"%l = %#y << %#z = %#x"},
{"SLUI",0x29,0,0,1,"%l = %#y << %z = %#x"},
{"SR",0x2a,0,0,1,"%l = %y >> %#z = %x"},
{"SRI",0x29,0,0,1,"%l = %y >> %z = %x"},
{"SRU",0x2a,0,0,1,"%l = %#y >> %#z = %#x"},
{"SRUI",0x29,0,0,1,"%l = %#y >> %z = %#x"}

/*:45*/
#line 1461 "mmix-sim.w"
,
/*46:*/
#line 1532 "mmix-sim.w"

{"BN",0x50,0,0,1,"%b<0? %t%g"},
{"BNB",0x50,0,0,1,"%b<0? %t%g"},
{"BZ",0x50,0,0,1,"%b==0? %t%g"},
{"BZB",0x50,0,0,1,"%b==0? %t%g"},
{"BP",0x50,0,0,1,"%b>0? %t%g"},
{"BPB",0x50,0,0,1,"%b>0? %t%g"},
{"BOD",0x50,0,0,1,"%b odd? %t%g"},
{"BODB",0x50,0,0,1,"%b odd? %t%g"},
{"BNN",0x50,0,0,1,"%b>=0? %t%g"},
{"BNNB",0x50,0,0,1,"%b>=0? %t%g"},
{"BNZ",0x50,0,0,1,"%b!=0? %t%g"},
{"BNZB",0x50,0,0,1,"%b!=0? %t%g"},
{"BNP",0x50,0,0,1,"%b<=0? %t%g"},
{"BNPB",0x50,0,0,1,"%b<=0? %t%g"},
{"BEV",0x50,0,0,1,"%b even? %t%g"},
{"BEVB",0x50,0,0,1,"%b even? %t%g"},
{"PBN",0x50,0,0,1,"%b<0? %t%g"},
{"PBNB",0x50,0,0,1,"%b<0? %t%g"},
{"PBZ",0x50,0,0,1,"%b==0? %t%g"},
{"PBZB",0x50,0,0,1,"%b==0? %t%g"},
{"PBP",0x50,0,0,1,"%b>0? %t%g"},
{"PBPB",0x50,0,0,1,"%b>0? %t%g"},
{"PBOD",0x50,0,0,1,"%b odd? %t%g"},
{"PBODB",0x50,0,0,1,"%b odd? %t%g"},
{"PBNN",0x50,0,0,1,"%b>=0? %t%g"},
{"PBNNB",0x50,0,0,1,"%b>=0? %t%g"},
{"PBNZ",0x50,0,0,1,"%b!=0? %t%g"},
{"PBNZB",0x50,0,0,1,"%b!=0? %t%g"},
{"PBNP",0x50,0,0,1,"%b<=0? %t%g"},
{"PBNPB",0x50,0,0,1,"%b<=0? %t%g"},
{"PBEV",0x50,0,0,1,"%b even? %t%g"},
{"PBEVB",0x50,0,0,1,"%b even? %t%g"},
{"CSN",0x3a,0,0,1,"%l = %y<0? %z: %b = %x"},
{"CSNI",0x39,0,0,1,"%l = %y<0? %z: %b = %x"},
{"CSZ",0x3a,0,0,1,"%l = %y==0? %z: %b = %x"},
{"CSZI",0x39,0,0,1,"%l = %y==0? %z: %b = %x"},
{"CSP",0x3a,0,0,1,"%l = %y>0? %z: %b = %x"},
{"CSPI",0x39,0,0,1,"%l = %y>0? %z: %b = %x"},
{"CSOD",0x3a,0,0,1,"%l = %y odd? %z: %b = %x"},
{"CSODI",0x39,0,0,1,"%l = %y odd? %z: %b = %x"},
{"CSNN",0x3a,0,0,1,"%l = %y>=0? %z: %b = %x"},
{"CSNNI",0x39,0,0,1,"%l = %y>=0? %z: %b = %x"},
{"CSNZ",0x3a,0,0,1,"%l = %y!=0? %z: %b = %x"},
{"CSNZI",0x39,0,0,1,"%l = %y!=0? %z: %b = %x"},
{"CSNP",0x3a,0,0,1,"%l = %y<=0? %z: %b = %x"},
{"CSNPI",0x39,0,0,1,"%l = %y<=0? %z: %b = %x"},
{"CSEV",0x3a,0,0,1,"%l = %y even? %z: %b = %x"},
{"CSEVI",0x39,0,0,1,"%l = %y even? %z: %b = %x"},
{"ZSN",0x2a,0,0,1,"%l = %y<0? %z: 0 = %x"},
{"ZSNI",0x29,0,0,1,"%l = %y<0? %z: 0 = %x"},
{"ZSZ",0x2a,0,0,1,"%l = %y==0? %z: 0 = %x"},
{"ZSZI",0x29,0,0,1,"%l = %y==0? %z: 0 = %x"},
{"ZSP",0x2a,0,0,1,"%l = %y>0? %z: 0 = %x"},
{"ZSPI",0x29,0,0,1,"%l = %y>0? %z: 0 = %x"},
{"ZSOD",0x2a,0,0,1,"%l = %y odd? %z: 0 = %x"},
{"ZSODI",0x29,0,0,1,"%l = %y odd? %z: 0 = %x"},
{"ZSNN",0x2a,0,0,1,"%l = %y>=0? %z: 0 = %x"},
{"ZSNNI",0x29,0,0,1,"%l = %y>=0? %z: 0 = %x"},
{"ZSNZ",0x2a,0,0,1,"%l = %y!=0? %z: 0 = %x"},
{"ZSNZI",0x29,0,0,1,"%l = %y!=0? %z: 0 = %x"},
{"ZSNP",0x2a,0,0,1,"%l = %y<=0? %z: 0 = %x"},
{"ZSNPI",0x29,0,0,1,"%l = %y<=0? %z: 0 = %x"},
{"ZSEV",0x2a,0,0,1,"%l = %y even? %z: 0 = %x"},
{"ZSEVI",0x29,0,0,1,"%l = %y even? %z: 0 = %x"}

/*:46*/
#line 1462 "mmix-sim.w"
,
/*47:*/
#line 1598 "mmix-sim.w"

{"LDB",0x2a,0,1,1,"%l = M1[%#y+%#z] = %x"},
{"LDBI",0x29,0,1,1,"%l = M1[%#y%?+] = %x"},
{"LDBU",0x2a,0,1,1,"%l = M1[%#y+%#z] = %#x"},
{"LDBUI",0x29,0,1,1,"%l = M1[%#y%?+] = %#x"},
{"LDW",0x2a,0,1,1,"%l = M2[%#y+%#z] = %x"},
{"LDWI",0x29,0,1,1,"%l = M2[%#y%?+] = %x"},
{"LDWU",0x2a,0,1,1,"%l = M2[%#y+%#z] = %#x"},
{"LDWUI",0x29,0,1,1,"%l = M2[%#y%?+] = %#x"},
{"LDT",0x2a,0,1,1,"%l = M4[%#y+%#z] = %x"},
{"LDTI",0x29,0,1,1,"%l = M4[%#y%?+] = %x"},
{"LDTU",0x2a,0,1,1,"%l = M4[%#y+%#z] = %#x"},
{"LDTUI",0x29,0,1,1,"%l = M4[%#y%?+] = %#x"},
{"LDO",0x2a,0,1,1,"%l = M8[%#y+%#z] = %x"},
{"LDOI",0x29,0,1,1,"%l = M8[%#y%?+] = %x"},
{"LDOU",0x2a,0,1,1,"%l = M8[%#y+%#z] = %#x"},
{"LDOUI",0x29,0,1,1,"%l = M8[%#y%?+] = %#x"},
{"LDSF",0x2a,0,1,1,"%l = (M4[%#y+%#z]) = %.x"},
{"LDSFI",0x29,0,1,1,"%l = (M4[%#y%?+]) = %.x"},
{"LDHT",0x2a,0,1,1,"%l = M4[%#y+%#z]<<32 = %#x"},
{"LDHTI",0x29,0,1,1,"%l = M4[%#y%?+]<<32 = %#x"},
{"CSWAP",0x3a,0,2,2,"%l = [M8[%#y+%#z]==%a] = %x, %r"},
{"CSWAPI",0x39,0,2,2,"%l = [M8[%#y%?+]==%a] = %x, %r"},
{"LDUNC",0x2a,0,1,1,"%l = M8[%#y+%#z] = %#x"},
{"LDUNCI",0x29,0,1,1,"%l = M8[%#y%?+] = %#x"},
{"LDVTS",0x2a,0,0,1,""},
{"LDVTSI",0x29,0,0,1,""},
{"PRELD",0x0a,0,0,1,"[%#y+%#z .. %#x]"},
{"PRELDI",0x09,0,0,1,"[%#y%?+ .. %#x]"},
{"PREGO",0x0a,0,0,1,"[%#y+%#z .. %#x]"},
{"PREGOI",0x09,0,0,1,"[%#y%?+ .. %#x]"},
{"GO",0x2a,0,0,3,"%l = %#x, -> %#y+%#z"},
{"GOI",0x29,0,0,3,"%l = %#x, -> %#y%?+"},
{"STB",0x1a,0,1,1,"M1[%#y+%#z] = %b, M8[%#w]=%#a"},
{"STBI",0x19,0,1,1,"M1[%#y%?+] = %b, M8[%#w]=%#a"},
{"STBU",0x1a,0,1,1,"M1[%#y+%#z] = %#b, M8[%#w]=%#a"},
{"STBUI",0x19,0,1,1,"M1[%#y%?+] = %#b, M8[%#w]=%#a"},
{"STW",0x1a,0,1,1,"M2[%#y+%#z] = %b, M8[%#w]=%#a"},
{"STWI",0x19,0,1,1,"M2[%#y%?+] = %b, M8[%#w]=%#a"},
{"STWU",0x1a,0,1,1,"M2[%#y+%#z] = %#b, M8[%#w]=%#a"},
{"STWUI",0x19,0,1,1,"M2[%#y%?+] = %#b, M8[%#w]=%#a"},
{"STT",0x1a,0,1,1,"M4[%#y+%#z] = %b, M8[%#w]=%#a"},
{"STTI",0x19,0,1,1,"M4[%#y%?+] = %b, M8[%#w]=%#a"},
{"STTU",0x1a,0,1,1,"M4[%#y+%#z] = %#b, M8[%#w]=%#a"},
{"STTUI",0x19,0,1,1,"M4[%#y%?+] = %#b, M8[%#w]=%#a"},
{"STO",0x1a,0,1,1,"M8[%#y+%#z] = %b"},
{"STOI",0x19,0,1,1,"M8[%#y%?+] = %b"},
{"STOU",0x1a,0,1,1,"M8[%#y+%#z] = %#b"},
{"STOUI",0x19,0,1,1,"M8[%#y%?+] = %#b"},
{"STSF",0x1a,0,1,1,"%(M4[%#y+%#z]%) = %.b, M8[%#w]=%#a"},
{"STSFI",0x19,0,1,1,"%(M4[%#y%?+]%) = %.b, M8[%#w]=%#a"},
{"STHT",0x1a,0,1,1,"M4[%#y+%#z] = %#b>>32, M8[%#w]=%#a"},
{"STHTI",0x19,0,1,1,"M4[%#y%?+] = %#b>>32, M8[%#w]=%#a"},
{"STCO",0x0a,0,1,1,"M8[%#y+%#z] = %b"},
{"STCOI",0x09,0,1,1,"M8[%#y%?+] = %b"},
{"STUNC",0x1a,0,1,1,"M8[%#y+%#z] = %#b"},
{"STUNCI",0x19,0,1,1,"M8[%#y%?+] = %#b"},
{"SYNCD",0x0a,0,0,1,"[%#y+%#z .. %#x]"},
{"SYNCDI",0x09,0,0,1,"[%#y%?+ .. %#x]"},
{"PREST",0x0a,0,0,1,"[%#y+%#z .. %#x]"},
{"PRESTI",0x09,0,0,1,"[%#y%?+ .. %#x]"},
{"SYNCID",0x0a,0,0,1,"[%#y+%#z .. %#x]"},
{"SYNCIDI",0x09,0,0,1,"[%#y%?+ .. %#x]"},
{"PUSHGO",0xaa,0,0,3,"%lrO=%#b, rL=%a, rJ=%#x, -> %#y+%#z"},
{"PUSHGOI",0xa9,0,0,3,"%lrO=%#b, rL=%a, rJ=%#x, -> %#y%?+"}

/*:47*/
#line 1463 "mmix-sim.w"
,
/*48:*/
#line 1664 "mmix-sim.w"

{"OR",0x2a,0,0,1,"%l = %#y | %#z = %#x"},
{"ORI",0x29,0,0,1,"%l = %#y | %z = %#x"},
{"ORN",0x2a,0,0,1,"%l = %#y |~ %#z = %#x"},
{"ORNI",0x29,0,0,1,"%l = %#y |~ %z = %#x"},
{"NOR",0x2a,0,0,1,"%l = %#y ~| %#z = %#x"},
{"NORI",0x29,0,0,1,"%l = %#y ~| %z = %#x"},
{"XOR",0x2a,0,0,1,"%l = %#y ^ %#z = %#x"},
{"XORI",0x29,0,0,1,"%l = %#y ^ %z = %#x"},
{"AND",0x2a,0,0,1,"%l = %#y & %#z = %#x"},
{"ANDI",0x29,0,0,1,"%l = %#y & %z = %#x"},
{"ANDN",0x2a,0,0,1,"%l = %#y \\ %#z = %#x"},
{"ANDNI",0x29,0,0,1,"%l = %#y \\ %z = %#x"},
{"NAND",0x2a,0,0,1,"%l = %#y ~& %#z = %#x"},
{"NANDI",0x29,0,0,1,"%l = %#y ~& %z = %#x"},
{"NXOR",0x2a,0,0,1,"%l = %#y ~^ %#z = %#x"},
{"NXORI",0x29,0,0,1,"%l = %#y ~^ %z = %#x"},
{"BDIF",0x2a,0,0,1,"%l = %#y bdif %#z = %#x"},
{"BDIFI",0x29,0,0,1,"%l = %#y bdif %z = %#x"},
{"WDIF",0x2a,0,0,1,"%l = %#y wdif %#z = %#x"},
{"WDIFI",0x29,0,0,1,"%l = %#y wdif %z = %#x"},
{"TDIF",0x2a,0,0,1,"%l = %#y tdif %#z = %#x"},
{"TDIFI",0x29,0,0,1,"%l = %#y tdif %z = %#x"},
{"ODIF",0x2a,0,0,1,"%l = %#y odif %#z = %#x"},
{"ODIFI",0x29,0,0,1,"%l = %#y odif %z = %#x"},
{"MUX",0x2a,rM,0,1,"%l = %#b? %#y: %#z = %#x"},
{"MUXI",0x29,rM,0,1,"%l = %#b? %#y: %z = %#x"},
{"SADD",0x2a,0,0,1,"%l = nu(%#y\\%#z) = %x"},
{"SADDI",0x29,0,0,1,"%l = nu(%#y%?\\) = %x"},
{"MOR",0x2a,0,0,1,"%l = %#y mor %#z = %#x"},
{"MORI",0x29,0,0,1,"%l = %#y mor %z = %#x"},
{"MXOR",0x2a,0,0,1,"%l = %#y mxor %#z = %#x"},
{"MXORI",0x29,0,0,1,"%l = %#y mxor %z = %#x"},
{"SETH",0x20,0,0,1,"%l = %#z"},
{"SETMH",0x20,0,0,1,"%l = %#z"},
{"SETML",0x20,0,0,1,"%l = %#z"},
{"SETL",0x20,0,0,1,"%l = %#z"},
{"INCH",0x30,0,0,1,"%l = %#y + %#z = %#x"},
{"INCMH",0x30,0,0,1,"%l = %#y + %#z = %#x"},
{"INCML",0x30,0,0,1,"%l = %#y + %#z = %#x"},
{"INCL",0x30,0,0,1,"%l = %#y + %#z = %#x"},
{"ORH",0x30,0,0,1,"%l = %#y | %#z = %#x"},
{"ORMH",0x30,0,0,1,"%l = %#y | %#z = %#x"},
{"ORML",0x30,0,0,1,"%l = %#y | %#z = %#x"},
{"ORL",0x30,0,0,1,"%l = %#y | %#z = %#x"},
{"ANDNH",0x30,0,0,1,"%l = %#y \\ %#z = %#x"},
{"ANDNMH",0x30,0,0,1,"%l = %#y \\ %#z = %#x"},
{"ANDNML",0x30,0,0,1,"%l = %#y \\ %#z = %#x"},
{"ANDNL",0x30,0,0,1,"%l = %#y \\ %#z = %#x"},
{"JMP",0x40,0,0,1,"-> %#z"},
{"JMPB",0x40,0,0,1,"-> %#z"},
{"PUSHJ",0xe0,0,0,1,"%lrO=%#b, rL=%a, rJ=%#x, -> %#z"},
{"PUSHJB",0xe0,0,0,1,"%lrO=%#b, rL=%a, rJ=%#x, -> %#z"},
{"GETA",0x60,0,0,1,"%l = %#z"},
{"GETAB",0x60,0,0,1,"%l = %#z"},
{"PUT",0x02,0,0,1,"%s = %r"},
{"PUTI",0x01,0,0,1,"%s = %r"},
{"POP",0x80,rJ,0,3,"%lrL=%a, rO=%#b, -> %#y%?+"},
#line 693 "mmix-sim.ch"
{"RESUME",0x00,0,0,5,"{%#b}, $255 = %x, -> %#z"},
{"SAVE",0x20,0,20,1,"%l = %#x"},
{"UNSAVE",0x82,0,20,1,"%#z: rG=%x, ..., rL=%a"},
{"SYNC",0x01,0,0,1,"%z"},
{"SWYM",0x01,0,0,1,"%r"},
#line 1727 "mmix-sim.w"
{"GET",0x20,0,0,1,"%l = %s = %#x"},
{"TRIP",0x0a,255,0,5,"rW=%#w, rX=%#x, rY=%#y, rZ=%#z, rB=%#b, g[255]=%#a"}

/*:48*/
#line 1464 "mmix-sim.w"
};

/*:44*//*54:*/
#line 705 "mmix-sim.ch"

int G= 1,L= 0,O= 0;
#line 1779 "mmix-sim.w"

/*:54*//*55:*/
#line 1780 "mmix-sim.w"

octa g[256];
octa*l;
int lring_size;
int lring_mask;
int S;

/*:55*//*89:*/
#line 2407 "mmix-sim.w"

char arg_count[]= {1,3,1,3,3,3,3,2,2,2,1};
char*trap_format[]= {
"Halt(%z)",
#line 1310 "mmix-sim.ch"
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
#line 2421 "mmix-sim.w"

#line 1333 "mmix-sim.ch"
/*:89*//*98:*/
#line 2568 "mmix-sim.w"

char stdin_buf[256];
char*stdin_buf_start;
char*stdin_buf_end;

/*:98*//*101:*/
#line 1539 "mmix-sim.ch"

octa new_Q;

/*:101*//*112:*/
#line 2671 "mmix-sim.w"

bool showing_stats;
bool just_traced;

/*:112*//*122:*/
#line 2841 "mmix-sim.w"

char left_paren[]= {0,'[','^','_','('};
char right_paren[]= {0,']','^','_',')'};
char switchable_string[48];

char lhs[32];
int good_guesses,bad_guesses;

/*:122*//*127:*/
#line 2979 "mmix-sim.w"

char*myself;
char**cur_arg;
#line 1911 "mmix-sim.ch"
static bool interrupt= 0;
static bool profiling= 0;
#line 2984 "mmix-sim.w"
FILE*fake_stdin;
FILE*dump_file;
char*usage_help[]= {
" with these options: (<n>=decimal number, <x>=hex number)\n",
"-t<n> trace each instruction the first n times\n",
"-e<x> trace each instruction with an exception matching x\n",
"-r    trace hidden details of the register stack\n",
#line 1921 "mmix-sim.ch"
"-r    trace hidden details of the register stack\n",
"-O    trace inside the operating system\n",
"-C    use host clock for rC\n",
"-B<n> connect to Bus on port <n>\n",
"-s    show statistics after each traced instruction\n",
#line 2995 "mmix-sim.w"
"-v    be verbose: show almost everything\n",
"-q    be quiet: show only the simulated standard output\n",
"-i    run interactively (prompt for online commands)\n",
"-I    interact, but only after the program halts\n",
"-b<n> change the buffer size for source lines\n",
"-c<n> change the cyclic local register ring size\n",
"-f<filename> use given file to simulate standard input\n",
"-D<filename> dump a file for use by other simulators\n",
""};
char*interactive_help[]= {
"The interactive commands are:\n",
"<return>  trace one instruction\n",
"n         trace one instruction\n",
"c         continue until halt or breakpoint\n",
"q         quit the simulation\n",
"s         show current statistics\n",
"l<n><t>   set and/or show local register in format t\n",
"g<n><t>   set and/or show global register in format t\n",
"rA<t>     set and/or show register rA in format t\n",
"$<n><t>   set and/or show dynamic register in format t\n",
"M<x><t>   set and/or show memory octabyte in format t\n",
"+<n><t>   set and/or show n additional octabytes in format t\n",
" <t> is ! (decimal) or . (floating) or # (hex) or \" (string)\n",
"     or <empty> (previous <t>) or =<value> (change value)\n",
"@<x>      go to location x\n",
"b[rwx]<x> set or reset breakpoint at location x\n",
"t<x>      trace location x\n",
"u<x>      untrace location x\n",
#line 1934 "mmix-sim.ch"
"T         set current segment to Text_Segment\n",
"D         set current segment to Data_Segment\n",
"P         set current segment to Pool_Segment\n",
"S         set current segment to Stack_Segment\n",
"N         set current segment to Negative Addresses\n",
"O         toggle tracing inside the operating system\n",
#line 3027 "mmix-sim.w"
"B         show all current breakpoints and tracepoints\n",
"i<file>   insert commands from file\n",
"-<option> change a tracing/listing/profile option\n",
"-?        show the tracing/listing/profile options  \n",
""};

/*:127*//*134:*/
#line 3121 "mmix-sim.w"

char command_buf[command_buf_size];
FILE*incl_file;
char cur_disp_mode= 'l';
char cur_disp_type= '!';
bool cur_disp_set;
octa cur_disp_addr;
octa cur_seg;
char spec_reg_code[]= {rA,rB,rC,rD,rE,rF,rG,rH,rI,rJ,rK,rL,rM,
rN,rO,rP,rQ,rR,rS,rT,rU,rV,rW,rX,rY,rZ};
char spec_regg_code[]= {0,rBB,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,rTT,0,0,rWW,rXX,rYY,rZZ};

/*:134*/
#line 2881 "mmix-sim.w"

/*12:*/
#line 577 "mmix-sim.w"

void print_hex ARGS((octa));
void print_hex(o)
octa o;
{
if(o.h)printf("%x%08x",o.h,o.l);
else printf("%x",o.l);
}

/*:12*//*13:*/
#line 592 "mmix-sim.w"

extern octa zero_octa;
extern octa neg_one;
extern octa aux,val;
extern bool overflow;
extern int exceptions;
extern int cur_round;
extern char*next_char;
extern octa oplus ARGS((octa y,octa z));

extern octa ominus ARGS((octa y,octa z));

extern octa incr ARGS((octa y,int delta));

extern octa oand ARGS((octa y,octa z));

extern octa shift_left ARGS((octa y,int s));

extern octa shift_right ARGS((octa y,int s,int uns));

extern octa omult ARGS((octa y,octa z));

extern octa signed_omult ARGS((octa y,octa z));

extern octa odiv ARGS((octa x,octa y,octa z));

extern octa signed_odiv ARGS((octa y,octa z));

extern int count_bits ARGS((tetra z));

extern tetra byte_diff ARGS((tetra y,tetra z));

extern tetra wyde_diff ARGS((tetra y,tetra z));

extern octa bool_mult ARGS((octa y,octa z,bool xor));

extern octa load_sf ARGS((tetra z));

extern tetra store_sf ARGS((octa x));

extern octa fplus ARGS((octa y,octa z));

extern octa fmult ARGS((octa y,octa z));

extern octa fdivide ARGS((octa y,octa z));

extern octa froot ARGS((octa,int));

extern octa fremstep ARGS((octa y,octa z,int delta));

extern octa fintegerize ARGS((octa z,int mode));

extern int fcomp ARGS((octa y,octa z));

extern int fepscomp ARGS((octa y,octa z,octa eps,int sim));

extern octa floatit ARGS((octa z,int mode,int unsgnd,int shrt));

extern octa fixit ARGS((octa z,int mode));

extern void print_float ARGS((octa z));

extern int scan_const ARGS((char*buf));


/*:13*//*15:*/
#line 671 "mmix-sim.w"

void print_int ARGS((octa));
void print_int(o)
octa o;
{
register tetra hi= o.h,lo= o.l,r,t;
register int j;
char dig[20];
if(lo==0&&hi==0)printf("0");
else{
if(hi&sign_bit){
printf("-");
if(lo==0)hi= -hi;
else lo= -lo,hi= ~hi;
}
for(j= 0;hi;j++){
r= ((hi%10)<<16)+(lo>>16);
hi= hi/10;
t= ((r%10)<<16)+(lo&0xffff);
lo= ((r/10)<<16)+(t/10);
dig[j]= t%10;
}
for(;lo;j++){
dig[j]= lo%10;
lo= lo/10;
}
for(j--;j>=0;j--)printf("%c",dig[j]+'0');
}
}

#line 160 "mmix-sim.ch"
/*:15*//*20:*/
#line 888 "mmix-sim.w"

void read_tet ARGS((void));
void read_tet()
{
if(fread(buf,1,4,mmo_file)!=4)mmo_err;
yzbytes= (buf[2]<<8)+buf[3];
tet= (((buf[0]<<8)+buf[1])<<16)+yzbytes;
}

/*:20*//*21:*/
#line 897 "mmix-sim.w"

byte read_byte ARGS((void));
byte read_byte()
{
register byte b;
if(!byte_count)read_tet();
b= buf[byte_count];
byte_count= (byte_count+1)&3;
return b;
}

/*:21*//*24:*/
#line 259 "mmix-sim.ch"

void mmo_load ARGS((octa,tetra));
void mmo_load(loc,val)
octa loc;
tetra val;
{
octa x;
load_data(4,&x,loc,0);
x.h= 0;
x.l= x.l^val;
store_data(4,x,loc);
}

/*:24*//*62:*/
#line 800 "mmix-sim.ch"

void stack_store ARGS((void));
void stack_store()
{
register int k= S&lring_mask;
store_data(8,l[k],g[rS]);test_store_bkpt(g[rS]);
if(stack_tracing){
tracing= true;
printf("             M8[#%08x%08x]=l[%d]=#%08x%08x, rS+=8\n",
g[rS].h,g[rS].l,k,l[k].h,l[k].l);
}
g[rS]= incr(g[rS],8),S++;
}

/*:62*//*63:*/
#line 818 "mmix-sim.ch"

void stack_load ARGS((void));
void stack_load()
{
register int k;
S--,g[rS]= incr(g[rS],-8);
k= S&lring_mask;
load_data(8,&l[k],g[rS],0);test_load_bkpt(g[rS]);
if(stack_tracing){
tracing= true;
printf("             rS-=8, l[%d]=M8[#%08x%08x]=#%08x%08x\n",
k,g[rS].h,g[rS].l,l[k].h,l[k].l);
}
}
#line 1902 "mmix-sim.w"

/*:63*//*71:*/
#line 2066 "mmix-sim.w"

int register_truth ARGS((octa,mmix_opcode));
int register_truth(o,op)
octa o;
mmix_opcode op;
{register int b;
switch((op>>1)&0x3){
case 0:b= o.h>>31;break;
case 1:b= (o.h==0&&o.l==0);break;
case 2:b= (o.h<sign_bit&&(o.h||o.l));break;
case 3:b= o.l&0x1;break;
}
if(op&0x8)return b^1;
else return b;
}

/*:71*//*91:*/
#line 2459 "mmix-sim.w"

#line 1442 "mmix-sim.ch"
int mmgetchars(buf,size,addr,stop)
unsigned char*buf;
int size;
octa addr;
int stop;
{
register unsigned char*p;
register int m;
octa x;
octa a;
for(p= buf,m= 0,a= addr;m<size;){
if((a.l&0x7)||m+8> size)/*92:*/
#line 1459 "mmix-sim.ch"

{load_data(1,&x,a,0);
*p= x.l&0xff;
if(!*p&&stop>=0){
if(stop==0)return m;
if((a.l&0x1)&&*(p-1)=='\0')return m-1;
}
p++,m++,a= incr(a,1);
}

/*:92*/
#line 1453 "mmix-sim.ch"

else/*93:*/
#line 1469 "mmix-sim.ch"

{load_data(8,&x,a,0);
*p= x.h>>24;
if(!*p&&(stop==0||(stop> 0&&x.h<0x10000)))return m;
*(p+1)= (x.h>>16)&0xff;
if(!*(p+1)&&stop==0)return m+1;
*(p+2)= (x.h>>8)&0xff;
if(!*(p+2)&&(stop==0||(stop> 0&&(x.h&0xffff)==0)))return m+2;
*(p+3)= x.h&0xff;
if(!*(p+3)&&stop==0)return m+3;
p+= 4,m+= 4,a= incr(a,4);
*p= x.l>>24;
if(!*p&&(stop==0||(stop> 0&&x.l<0x10000)))return m;
*(p+1)= (x.l>>16)&0xff;
if(!*(p+1)&&stop==0)return m+1;
*(p+2)= (x.l>>8)&0xff;
if(!*(p+2)&&(stop==0||(stop> 0&&(x.l&0xffff)==0)))return m+2;
*(p+3)= x.l&0xff;
if(!*(p+3)&&stop==0)return m+3;
p+= 4,m+= 4,a= incr(a,4);
}

/*:93*/
#line 1454 "mmix-sim.ch"

}
return size;
}

/*:91*//*94:*/
#line 1494 "mmix-sim.ch"

void mmputchars ARGS((unsigned char*,int,octa));
void mmputchars(buf,size,addr)
unsigned char*buf;
int size;
octa addr;
{
register unsigned char*p;
register int m;
octa x;
octa a;
for(p= buf,m= 0,a= addr;m<size;){
test_store_bkpt(a);
if((a.l&0x7)||m+8> size)/*95:*/
#line 1512 "mmix-sim.ch"

{
x.l= *p;
x.h= 0;
store_data(1,x,a);
p++,m++,a= incr(a,1);
}

/*:95*/
#line 1507 "mmix-sim.ch"

else/*96:*/
#line 1520 "mmix-sim.ch"

{x.h= (*p<<24)+(*(p+1)<<16)+(*(p+2)<<8)+*(p+3);
p+= 4;
x.l= (*p<<24)+(*(p+1)<<16)+(*(p+2)<<8)+*(p+3);
p+= 4;
store_data(8,x,a);
m+= 8,a= incr(a,8);
}
#line 2537 "mmix-sim.w"

/*:96*/
#line 1508 "mmix-sim.ch"
;
}
}

/*:94*//*97:*/
#line 2549 "mmix-sim.w"

char stdin_chr ARGS((void));
char stdin_chr()
{
register char*p;
while(stdin_buf_start==stdin_buf_end){
if(interacting){
printf("StdIn> ");fflush(stdout);

}
if(!fgets(stdin_buf,256,stdin))
panic("End of file on standard input; use the -f option, not <");
stdin_buf_start= stdin_buf;
for(p= stdin_buf;p<stdin_buf+254;p++)if(*p=='\n')break;
stdin_buf_end= p+1;
}
return*stdin_buf_start++;
}

/*:97*//*120:*/
#line 2807 "mmix-sim.w"

fmt_style style;
char*stream_name[]= {"StdIn","StdOut","StdErr"};




void trace_print ARGS((octa));
void trace_print(o)
octa o;
{
switch(style){
case decimal:print_int(o);return;
case hex:fputc('#',stdout);print_hex(o);return;
case zhex:printf("%08x%08x",o.h,o.l);return;
case floating:print_float(o);return;
case handle:if(o.h==0&&o.l<3)printf(stream_name[o.l]);
else print_int(o);return;
}
}

/*:120*//*123:*/
#line 2849 "mmix-sim.w"

void show_stats ARGS((bool));
void show_stats(verbose)
bool verbose;
{
octa o;
printf("  %d instruction%s, %d mem%s, %d oop%s; %d good guess%s, %d bad\n",
g[rU].l,g[rU].l==1?"":"s",
g[rC].h,g[rC].h==1?"":"s",
g[rC].l,g[rC].l==1?"":"s",
good_guesses,good_guesses==1?"":"es",bad_guesses);
if(!verbose)return;
o= halted?incr(inst_ptr,-4):inst_ptr;
printf("  (%s at location #%08x%08x)\n",
halted?"halted":"now",o.h,o.l);
}

/*:123*//*126:*/
#line 2930 "mmix-sim.w"

void scan_option ARGS((char*,bool));
void scan_option(arg,usage)
char*arg;
bool usage;
{
register int k;
switch(*arg){
case't':if(strlen(arg)> 10)trace_threshold= 0xffffffff;
else if(sscanf(arg+1,"%d",&trace_threshold)!=1)trace_threshold= 0;
return;
case'e':if(!*(arg+1))tracing_exceptions= 0xff;
else if(sscanf(arg+1,"%x",&tracing_exceptions)!=1)tracing_exceptions= 0;
return;
case'r':stack_tracing= true;return;
case's':showing_stats= true;return;
#line 2953 "mmix-sim.w"
case'v':trace_threshold= 0xffffffff;tracing_exceptions= 0xff;
stack_tracing= true;showing_stats= true;
#line 1872 "mmix-sim.ch"
profiling= true;
#line 2957 "mmix-sim.w"
return;
case'q':trace_threshold= tracing_exceptions= 0;
#line 1879 "mmix-sim.ch"
stack_tracing= showing_stats= false;
profiling= false;
#line 2961 "mmix-sim.w"
return;
case'i':interacting= true;return;
case'I':interact_after_break= true;return;
#line 1887 "mmix-sim.ch"
case'B':
{char*p;
p= strchr(arg+1,':');
if(p==NULL)
{bushost= localhost;
busport= atoi(arg+1);
}
else
{busport= atoi(p+1);
bushost= malloc(p+1-arg+1);
if(bushost==NULL)panic("No room for hostname");
strncpy(bushost,arg+1,p-arg-1);
bushost[p-arg-1]= 0;
}
return;
}
case'O':show_operating_system= true;return;
case'C':hostclock= true;return;
#line 2965 "mmix-sim.w"
case'c':if(sscanf(arg+1,"%d",&lring_size)!=1)lring_size= 0;return;
case'f':/*128:*/
#line 3033 "mmix-sim.w"

if(fake_stdin)fclose(fake_stdin);
fake_stdin= fopen(arg+1,"r");
if(!fake_stdin)fprintf(stderr,"Sorry, I can't open file %s!\n",arg+1);

#line 1945 "mmix-sim.ch"
else fprintf(stderr,"Sorry, I can't fake stdin\n");
#line 3039 "mmix-sim.w"

/*:128*/
#line 2966 "mmix-sim.w"
;return;
case'D':/*129:*/
#line 3040 "mmix-sim.w"

dump_file= fopen(arg+1,"wb");
if(!dump_file)fprintf(stderr,"Sorry, I can't open file %s!\n",arg+1);


/*:129*/
#line 2967 "mmix-sim.w"
;return;
default:if(usage){
fprintf(stderr,
"Usage: %s <options> progfile command-line-args...\n",myself);

for(k= 0;usage_help[k][0];k++)fprintf(stderr,usage_help[k]);
exit(-1);
}else for(k= 0;usage_help[k][1]!='b';k++)printf(usage_help[k]);
return;
}
}

/*:126*//*131:*/
#line 3048 "mmix-sim.w"

void catchint ARGS((int));
void catchint(n)
int n;
{
interrupt= true;
signal(SIGINT,catchint);
}

/*:131*//*137:*/
#line 3178 "mmix-sim.w"

octa scan_hex ARGS((char*,octa));
octa scan_hex(s,offset)
char*s;
octa offset;
{
register char*p;
octa o;
o= zero_octa;
for(p= s;isxdigit(*p);p++){
o= incr(shift_left(o,4),*p-'0');
if(*p>='a')o= incr(o,'0'-'a'+10);
else if(*p>='A')o= incr(o,'0'-'A'+10);
}
next_char= p;
return oplus(o,offset);
}

/*:137*//*143:*/
#line 3285 "mmix-sim.w"

void print_string ARGS((octa));
void print_string(o)
octa o;
{
register int k,state,b;
for(k= state= 0;k<8;k++){
b= ((k<4?o.h>>(8*(3-k)):o.l>>(8*(7-k))))&0xff;
if(b==0){
if(state)printf("%s,0",state> 1?"\"":""),state= 1;
}else if(b>=' '&&b<='~')
printf("%s%c",state> 1?"":state==1?",\"":"\"",b),state= 2;
else printf("%s#%x",state> 1?"\",":state==1?",":"",b),state= 1;
}
if(state==0)printf("0");
else if(state> 1)printf("\"");
}

#line 2003 "mmix-sim.ch"
/*:143*/
#line 2882 "mmix-sim.w"


#line 1799 "mmix-sim.ch"
int main(argc,argv)
int argc;
char*argv[];
{
char**boot_cur_arg;
int boot_argc;
/*41:*/
#line 1404 "mmix-sim.w"

register mmix_opcode op;
register int xx,yy,zz,yz;
register tetra f;
register int i,j,k;
#line 1410 "mmix-sim.w"
register char*p;

#line 674 "mmix-sim.ch"
/*:41*/
#line 1805 "mmix-sim.ch"
;
/*125:*/
#line 2915 "mmix-sim.w"

myself= argv[0];
for(cur_arg= argv+1;*cur_arg&&(*cur_arg)[0]=='-';cur_arg++)
scan_option(*cur_arg+1,true);
#line 2920 "mmix-sim.w"
argc-= cur_arg-argv;

/*:125*/
#line 1806 "mmix-sim.ch"
;

if(bushost==NULL)panic("No Bus given. Use Option -B[host:]port");
init_mmix_bus(bushost,busport,"MMIX CPU");

boot_cur_arg= cur_arg;
boot_argc= argc;
boot:
argc= boot_argc;
cur_arg= boot_cur_arg;

/*14:*/
#line 660 "mmix-sim.w"

if(shift_left(neg_one,1).h!=0xffffffff)
panic("Incorrect implementation of type tetra");


/*:14*//*56:*/
#line 727 "mmix-sim.ch"

/*57:*/
#line 738 "mmix-sim.ch"

clear_all_data_vtc();
clear_all_instruction_vtc();
clear_all_data_cache();
clear_all_instruction_cache();
g[rK].h= g[rK].l= 0;
g[rN].h= (VERSION<<24)+(SUBVERSION<<16)+(SUBSUBVERSION<<8);
g[rN].l= ABSTIME;
g[rT].h= 0x80000000;g[rT].l= 0x00000000;
g[rTT].h= 0x80000000;g[rTT].l= 0x00000000;
G= g[rG].l= 255;
g[rV].h= 0x12340D00;
g[rV].l= 0x00002000;
cur_round= ROUND_NEAR;
#line 1820 "mmix-sim.w"

/*:57*//*147:*/
#line 2101 "mmix-sim.ch"

loc.h= inst_ptr.h= 0x80000000;
loc.l= inst_ptr.l= 0x00000000;
g[rJ].h= g[rJ].l= 0xFFFFFFFF;
resuming= false;
#line 3385 "mmix-sim.w"

#line 3425 "mmix-sim.w"

/*:147*/
#line 728 "mmix-sim.ch"

if(lring_size<256)lring_size= 256;
lring_mask= lring_size-1;
if(lring_size&lring_mask)
panic("The number of local registers must be a power of 2");

l= (octa*)calloc(lring_size,sizeof(octa));
if(!l)panic("No room for the local registers");


/*:56*//*130:*/
#line 3045 "mmix-sim.w"

signal(SIGINT,catchint);

/*:130*/
#line 1817 "mmix-sim.ch"
;

fprintf(stderr,"Power...");
while(!vmb.power)
{vmb_wait_for_power(&vmb);
if(!vmb.connected)goto end_simulation;
}
fprintf(stderr,"ON\n");
vmb.reset_flag= 0;

/*18:*/
#line 200 "mmix-sim.ch"

if(mmo_file_name!=NULL&&mmo_file_name[0]!=0)
{mmo_file= fopen(mmo_file_name,"rb");
if(!mmo_file){
register char*alt_name= (char*)calloc(strlen(mmo_file_name)+5,sizeof(char));
if(!alt_name)panic("Can't allocate file name buffer");
#line 855 "mmix-sim.w"

sprintf(alt_name,"%s.mmo",mmo_file_name);
mmo_file= fopen(alt_name,"rb");
if(!mmo_file){
fprintf(stderr,"Can't open the object file %s or %s!\n",

mmo_file_name,alt_name);
exit(-3);
}
free(alt_name);
}
byte_count= 0;

/*:18*//*27:*/
#line 283 "mmix-sim.ch"

cur_loc.h= cur_loc.l= 0;
postamble= 0;
/*22:*/
#line 908 "mmix-sim.w"

read_tet();
#line 211 "mmix-sim.ch"
if(buf[0]!=mmo_esc||buf[1]!=lop_pre)mmo_err;
#line 911 "mmix-sim.w"
if(ybyte!=1)mmo_err;
if(zbyte==0)obj_time= 0xffffffff;
else{
j= zbyte-1;
read_tet();obj_time= tet;
for(;j> 0;j--)read_tet();
}

/*:22*/
#line 286 "mmix-sim.ch"
;
do/*23:*/
#line 919 "mmix-sim.w"

{
read_tet();
#line 217 "mmix-sim.ch"
loop:if(buf[0]==mmo_esc)switch(buf[1]){
#line 923 "mmix-sim.w"
case lop_quote:if(yzbytes!=1)mmo_err;
read_tet();break;
/*28:*/
#line 976 "mmix-sim.w"

case lop_loc:if(zbyte==2){
j= ybyte;read_tet();cur_loc.h= (j<<24)+tet;
}else if(zbyte==1)cur_loc.h= ybyte<<24;
else mmo_err;
read_tet();cur_loc.l= tet;
continue;
case lop_skip:cur_loc= incr(cur_loc,yzbytes);continue;

/*:28*//*29:*/
#line 989 "mmix-sim.w"

#line 305 "mmix-sim.ch"
case lop_fixo:if(zbyte==2){
j= ybyte;read_tet();tmp.h= (j<<24)+tet;
}else if(zbyte==1)tmp.h= ybyte<<24;
else mmo_err;
read_tet();tmp.l= tet;
store_data(8,tmp,cur_loc);
continue;
#line 998 "mmix-sim.w"
case lop_fixr:delta= yzbytes;goto fixr;
case lop_fixrx:j= yzbytes;if(j!=16&&j!=24)mmo_err;
read_tet();delta= tet;
if(delta&0xfe000000)mmo_err;
fixr:tmp= incr(cur_loc,-(delta>=0x1000000?(delta&0xffffff)-(1<<j):delta)<<2);
mmo_load(tmp,delta);
continue;

/*:29*//*30:*/
#line 1008 "mmix-sim.w"

#line 335 "mmix-sim.ch"
case lop_file:
for(j= zbyte;j> 0;j--){
read_tet();
}
continue;
case lop_line:
continue;
#line 1028 "mmix-sim.w"

/*:30*//*31:*/
#line 1031 "mmix-sim.w"

case lop_spec:while(1){
read_tet();
#line 347 "mmix-sim.ch"
if(buf[0]==mmo_esc){
#line 1035 "mmix-sim.w"
if(buf[1]!=lop_quote||yzbytes!=1)goto loop;
read_tet();
}
}

#line 371 "mmix-sim.ch"
/*:31*/
#line 925 "mmix-sim.w"

case lop_post:postamble= 1;
if(ybyte||zbyte<32)mmo_err;
continue;
default:mmo_err;
}
/*25:*/
#line 272 "mmix-sim.ch"

{
mmo_load(cur_loc,tet);
cur_loc= incr(cur_loc,4);cur_loc.l&= -4;
}

/*:25*/
#line 931 "mmix-sim.w"
;
}

#line 256 "mmix-sim.ch"
/*:23*/
#line 287 "mmix-sim.ch"
while(!postamble);
/*32:*/
#line 380 "mmix-sim.ch"

aux.h= 0x60000000;
{octa x;
x.h= 0;x.l= argc;aux.l= 0x00;store_data(8,x,aux);
x.h= 0x40000000;x.l= 0x8;aux.l= 0x08;store_data(8,x,aux);
x.h= 0;x.l= 2;aux.l= 0x10;store_data(8,x,aux);
G= zbyte;L= 0;
aux.l= 0x18;
for(j= G;j<256;j++,aux.l+= 8)
{read_tet();x.h= tet;
read_tet(),x.l= tet;
store_data(8,x,aux);
}
g[rWW]= x;
if(interacting)set_break(x,exec_bit);
g[rXX].h= 0;g[rXX].l= 0xFB0000FF;
g[rBB]= aux= incr(aux,12*8);
x.h= G<<24;x.l= 0;
store_data(8,x,aux);
}
#line 1058 "mmix-sim.w"

#line 1269 "mmix-sim.w"

/*:32*/
#line 288 "mmix-sim.ch"
;
fclose(mmo_file);
write_all_data_cache();
clear_all_instruction_cache();
}
#line 968 "mmix-sim.w"

/*:27*/
#line 1827 "mmix-sim.ch"
;
/*146:*/
#line 2070 "mmix-sim.ch"

x.h= 0x40000000,x.l= 0x8;
aux= incr(x,8*(argc+1));
for(k= 0;k<argc&&*cur_arg!=NULL;k++,cur_arg++){
store_data(8,aux,x);
mmputchars((unsigned char*)*cur_arg,strlen(*cur_arg),aux);
x.l+= 8,aux.l+= 8+(tetra)(strlen(*cur_arg)&-8);
}
x.l= 0;store_data(8,aux,x);
#line 3369 "mmix-sim.w"

#line 2098 "mmix-sim.ch"
/*:146*/
#line 1828 "mmix-sim.ch"
;
while(1){
if(interrupt&&!breakpoint)breakpoint= interacting= true,interrupt= false;
else{
breakpoint= false;
if(interacting&&
(!(inst_ptr.h&0x80000000)||
show_operating_system||
(inst_ptr.h==0x80000000&&inst_ptr.l==0)))
/*132:*/
#line 3057 "mmix-sim.w"

{register int repeating;
#line 1951 "mmix-sim.ch"
interact:/*133:*/
#line 3093 "mmix-sim.w"

{register bool ready= false;
incl_read:while(incl_file&&!ready)
if(!fgets(command_buf,command_buf_size,incl_file)){
fclose(incl_file);
incl_file= NULL;
}else if(command_buf[0]!='\n'&&command_buf[0]!='i'&&
command_buf[0]!='%')
if(command_buf[0]==' ')printf(command_buf);
else ready= true;
while(!ready){
printf("mmix> ");fflush(stdout);

if(!fgets(command_buf,command_buf_size,stdin))command_buf[0]= 'q';
if(command_buf[0]!='i')ready= true;
else{
command_buf[strlen(command_buf)-1]= '\0';
incl_file= fopen(command_buf+1,"r");
if(incl_file)goto incl_read;
if(isspace(command_buf[1]))incl_file= fopen(command_buf+2,"r");
if(incl_file)goto incl_read;
printf("Can't open file `%s'!\n",command_buf+1);
}
}
}

/*:133*/
#line 1951 "mmix-sim.ch"
;
if(vmb_get_interrupt(&vmb,&new_Q.h,&new_Q.l)==1)
{g[rQ].h|= new_Q.h;g[rQ].l|= new_Q.l;}
#line 3060 "mmix-sim.w"
p= command_buf;
repeating= 0;
switch(*p){
case'\n':case'n':breakpoint= tracing= true;
case'c':goto resume_simulation;
case'q':goto end_simulation;
case's':show_stats(true);goto interact;
case'-':k= strlen(p);if(p[k-1]=='\n')p[k-1]= '\0';
scan_option(p+1,false);goto interact;
/*135:*/
#line 3134 "mmix-sim.w"

case'l':case'g':case'$':cur_disp_mode= *p++;
for(cur_disp_addr.l= 0;isdigit(*p);p++)
cur_disp_addr.l= 10*cur_disp_addr.l+*p-'0';
goto new_mode;
case'r':p++;cur_disp_mode= 'g';
if(*p<'A'||*p> 'Z')goto what_say;
if(*(p+1)!=*p)cur_disp_addr.l= spec_reg_code[*p-'A'],p++;
else if(spec_regg_code[*p-'A'])cur_disp_addr.l= spec_regg_code[*p-'A'],p+= 2;
else goto what_say;
goto new_mode;
case'M':cur_disp_mode= *p;
cur_disp_addr= scan_hex(p+1,cur_seg);cur_disp_addr.l&= -8;p= next_char;
new_mode:cur_disp_set= false;
repeating= 1;
goto scan_type;
case'+':if(!isdigit(*(p+1)))repeating= 1;
for(p++;isdigit(*p);p++)
repeating= 10*repeating+*p-'0';
if(repeating){
if(cur_disp_mode=='M')cur_disp_addr= incr(cur_disp_addr,8);
else cur_disp_addr.l++;
}
goto scan_type;

/*:135*/
#line 3069 "mmix-sim.w"
;
/*136:*/
#line 3159 "mmix-sim.w"

case'!':case'.':case'#':case'"':cur_disp_set= false;
repeating= 1;
set_type:cur_disp_type= *p++;break;
scan_type:if(*p=='!'||*p=='.'||*p=='#'||*p=='"')goto set_type;
if(*p!='=')break;
goto scan_eql;
case'=':repeating= 1;
scan_eql:cur_disp_set= true;
val= zero_octa;
if(*++p=='#')cur_disp_type= *p,val= scan_hex(p+1,zero_octa);
else if(*p=='"'||*p=='\'')goto scan_string;
else cur_disp_type= (scan_const(p)> 0?'.':'!');
p= next_char;
if(*p!=',')break;
val.h= 0;val.l&= 0xff;
scan_string:cur_disp_type= '"';
/*138:*/
#line 3196 "mmix-sim.w"

while(*p==','){
if(*++p=='#'){
aux= scan_hex(p+1,zero_octa),p= next_char;
val= incr(shift_left(val,8),aux.l&0xff);
}else if(isdigit(*p)){
for(k= *p++-'0';isdigit(*p);p++)k= (10*k+*p-'0')&0xff;
val= incr(shift_left(val,8),k);
}
else if(*p=='\n')goto incomplete_str;
}
if(*p=='\''&&*(p+2)==*p)*p= *(p+2)= '"';
if(*p=='"'){
for(p++;*p&&*p!='\n'&&*p!='"';p++)
val= incr(shift_left(val,8),*p);
if(*p&&*p++=='"')
if(*p==',')goto scan_string;
}

/*:138*/
#line 3176 "mmix-sim.w"
;break;

/*:136*/
#line 3070 "mmix-sim.w"
;
/*145:*/
#line 2008 "mmix-sim.ch"

case'@':inst_ptr= scan_hex(p+1,cur_seg);p= next_char;
halted= false;break;
case't':case'u':k= *p;
val= scan_hex(p+1,cur_seg);p= next_char;
if(val.h<0x20000000){
if(k=='t')set_break(val,get_break(val)|trace_bit);
else set_break(val,get_break(val)&~trace_bit);
}
break;
case'b':for(k= 0,p++;!isxdigit(*p);p++)
if(*p=='r')k|= read_bit;
else if(*p=='w')k|= write_bit;
else if(*p=='x')k|= exec_bit;
val= scan_hex(p,cur_seg);p= next_char;
set_break(val,k);
break;
#line 3324 "mmix-sim.w"
case'T':cur_seg.h= 0;goto passit;
case'D':cur_seg.h= 0x20000000;goto passit;
case'P':cur_seg.h= 0x40000000;goto passit;
case'S':cur_seg.h= 0x60000000;goto passit;
#line 2030 "mmix-sim.ch"
case'N':cur_seg.h= 0x80000000;goto passit;
case'B':show_breaks();
case'O':show_operating_system= !show_operating_system;goto passit;
#line 3329 "mmix-sim.w"
passit:p++;break;

#line 3349 "mmix-sim.w"

/*:145*/
#line 3071 "mmix-sim.w"
;
default:what_say:k= strlen(command_buf);
if(k<10&&command_buf[k-1]=='\n')command_buf[k-1]= '\0';
else strcpy(command_buf+9,"...");
printf("Eh? Sorry, I don't understand `%s'. (Type h for help)\n",
command_buf);
goto interact;
case'h':for(k= 0;interactive_help[k][0];k++)printf(interactive_help[k]);
goto interact;
}
check_syntax:if(*p!='\n'){
if(!*p)incomplete_str:printf("Syntax error: Incomplete command!\n");
else{
p[strlen(p)-1]= '\0';
printf("Syntax error; I'm ignoring `%s'!\n",p);
}
}
while(repeating)/*139:*/
#line 3215 "mmix-sim.w"

{
if(cur_disp_set)/*140:*/
#line 3226 "mmix-sim.w"

switch(cur_disp_mode){
case'l':l[cur_disp_addr.l&lring_mask]= val;break;
case'$':k= cur_disp_addr.l&0xff;
if(k<L)l[(O+k)&lring_mask]= val;else if(k>=G)g[k]= val;
break;
case'g':k= cur_disp_addr.l&0xff;
if(k<32)/*141:*/
#line 3244 "mmix-sim.w"

if(k>=9&&k!=rI){
if(k<=19)break;
if(k==rA){
if(val.h!=0||val.l>=0x40000)break;
cur_round= (val.l>=0x10000?val.l>>16:ROUND_NEAR);
}else if(k==rG){
if(val.h!=0||val.l> 255||val.l<L||val.l<32)break;
for(j= val.l;j<G;j++)g[j]= zero_octa;
G= val.l;
}else if(k==rL){
if(val.h==0&&val.l<L)L= val.l;
else break;
}
}

/*:141*/
#line 3233 "mmix-sim.w"
;
g[k]= val;break;
#line 1963 "mmix-sim.ch"
case'M':
store_data(8,val,cur_disp_addr);
break;
#line 3239 "mmix-sim.w"
}

/*:140*/
#line 3217 "mmix-sim.w"
;
/*142:*/
#line 3260 "mmix-sim.w"

switch(cur_disp_mode){
case'l':k= cur_disp_addr.l&lring_mask;
printf("l[%d]=",k);aux= l[k];break;
case'$':k= cur_disp_addr.l&0xff;
if(k<L)printf("$%d=l[%d]=",k,(O+k)&lring_mask),aux= l[(O+k)&lring_mask];
else if(k>=G)printf("$%d=g[%d]=",k,k),aux= g[k];
else printf("$%d=",k),aux= zero_octa;
break;
case'g':k= cur_disp_addr.l&0xff;
printf("g[%d]=",k);aux= g[k];break;
#line 1975 "mmix-sim.ch"
case'M':
load_data(8,&aux,cur_disp_addr,0);
#line 3276 "mmix-sim.w"
printf("M8[#");print_hex(cur_disp_addr);printf("]=");break;
}
switch(cur_disp_type){
case'!':print_int(aux);break;
case'.':print_float(aux);break;
case'#':fputc('#',stdout);print_hex(aux);break;
case'"':print_string(aux);break;
}

/*:142*/
#line 3218 "mmix-sim.w"
;
fputc('\n',stdout);
repeating--;
if(!repeating)break;
if(cur_disp_mode=='M')cur_disp_addr= incr(cur_disp_addr,8);
else cur_disp_addr.l++;
}

/*:139*/
#line 3088 "mmix-sim.w"
;
goto interact;
resume_simulation:;
}

/*:132*/
#line 1837 "mmix-sim.ch"
;
}
if(halted)break;
do/*39:*/
#line 1357 "mmix-sim.w"

{
#line 622 "mmix-sim.ch"
if(resuming)loc= incr(inst_ptr,-4),inst= g[zz?rXX:rX].l;
else/*42:*/
#line 674 "mmix-sim.ch"

{unsigned char b;
loc= inst_ptr;
load_instruction(&inst,loc);
b= get_break(loc);
if(b&exec_bit)breakpoint= true;
tracing= breakpoint||(b&trace_bit);
inst_ptr= incr(inst_ptr,4);
}
#line 1424 "mmix-sim.w"

/*:42*/
#line 623 "mmix-sim.ch"
;
op= inst>>24;xx= (inst>>16)&0xff;yy= (inst>>8)&0xff;zz= inst&0xff;
/*103:*/
#line 1568 "mmix-sim.ch"

{
if(loc.h&sign_bit)
{g[rQ].h|= P_BIT;
new_Q.h|= P_BIT;
if(g[rK].h&P_BIT)
goto security_inst;
}
else
{g[rQ].h&= ~P_BIT;
new_Q.h&= ~P_BIT;
if((g[rK].h&0xff)!=0xff)
{g[rQ].h|= S_BIT;
new_Q.h|= S_BIT;
g[rK].h|= S_BIT;
goto security_inst;
}
}
}


/*:103*/
#line 625 "mmix-sim.ch"

#line 1362 "mmix-sim.w"
f= info[op].flags;yz= inst&0xffff;
x= y= z= a= b= zero_octa;exc= 0;old_L= L;
if(f&rel_addr_bit)/*49:*/
#line 1730 "mmix-sim.w"

{
if((op&0xfe)==JMP)yz= inst&0xffffff;
if(op&1)yz-= (op==JMPB?0x1000000:0x10000);
y= inst_ptr;z= incr(loc,yz<<2);
}

/*:49*/
#line 1364 "mmix-sim.w"
;
/*50:*/
#line 1737 "mmix-sim.w"

if(resuming&&rop!=RESUME_AGAIN)
/*109:*/
#line 1697 "mmix-sim.ch"

if(zz==0)
{if(rop==RESUME_SET){
op= ORI;
y= g[rZ];
z= zero_octa;
exc= g[rX].h&0xff00;
f= X_is_dest_bit;
}else if(rop==RESUME_CONT){
y= g[rY];
z= g[rZ];
}
}
else
{if(rop==RESUME_SET){
op= ORI;
y= g[rZZ];
z= zero_octa;
exc= g[rXX].h&0xff00;
f= X_is_dest_bit;
}else if(rop==RESUME_TRANS)
{if((b.l>>24)==SWYM)
store_exec_translation(&g[rYY],&g[rZZ]);
else
store_data_translation(&g[rYY],&g[rZZ]);
}else if(rop==RESUME_CONT){
y= g[rYY];
z= g[rZZ];
}
}
#line 2642 "mmix-sim.w"

/*:109*/
#line 1739 "mmix-sim.w"

else{
if(f&0x10)/*53:*/
#line 1771 "mmix-sim.w"

{
if(xx>=G)b= g[xx];
else if(xx<L)b= l[(O+xx)&lring_mask];
}

#line 705 "mmix-sim.ch"
/*:53*/
#line 1741 "mmix-sim.w"
;
if(info[op].third_operand)/*59:*/
#line 1837 "mmix-sim.w"

b= g[info[op].third_operand];

/*:59*/
#line 1742 "mmix-sim.w"
;
if(f&0x1)z.l= zz;
else if(f&0x2)/*51:*/
#line 1759 "mmix-sim.w"

{
if(zz>=G)z= g[zz];
else if(zz<L)z= l[(O+zz)&lring_mask];
}

/*:51*/
#line 1744 "mmix-sim.w"

else if((op&0xf0)==SETH)/*58:*/
#line 1826 "mmix-sim.w"

{
switch(op&3){
case 0:z.h= yz<<16;break;
case 1:z.h= yz;break;
case 2:z.l= yz<<16;break;
case 3:z.l= yz;break;
}
y= b;
}

/*:58*/
#line 1745 "mmix-sim.w"
;
if(f&0x4)y.l= yy;
else if(f&0x8)/*52:*/
#line 1765 "mmix-sim.w"

{
if(yy>=G)y= g[yy];
else if(yy<L)y= l[(O+yy)&lring_mask];
}

/*:52*/
#line 1747 "mmix-sim.w"
;
}

/*:50*/
#line 1365 "mmix-sim.w"
;
if(f&X_is_dest_bit)/*60:*/
#line 1840 "mmix-sim.w"

if(xx>=G){
sprintf(lhs,"$%d=g[%d]",xx,xx);
x_ptr= &g[xx];
}else{
while(xx>=L)/*61:*/
#line 1850 "mmix-sim.w"

{
l[(O+L)&lring_mask]= zero_octa;
L= g[rL].l= L+1;
if(((S-O-L)&lring_mask)==0)stack_store();
}

/*:61*/
#line 1845 "mmix-sim.w"
;
sprintf(lhs,"$%d=l[%d]",xx,(O+xx)&lring_mask);
x_ptr= &l[(O+xx)&lring_mask];
}

/*:60*/
#line 1367 "mmix-sim.w"
;
w= oplus(y,z);
#line 1370 "mmix-sim.w"
switch(op){
/*64:*/
#line 1919 "mmix-sim.w"

case ADD:case ADDI:x= w;
if(((y.h^z.h)&sign_bit)==0&&((y.h^x.h)&sign_bit)!=0)exc|= V_BIT;
store_x:*x_ptr= x;break;

/*:64*//*65:*/
#line 1928 "mmix-sim.w"

case SUB:case SUBI:case NEG:case NEGI:x= ominus(y,z);
if(((x.h^z.h)&sign_bit)==0&&((x.h^y.h)&sign_bit)!=0)exc|= V_BIT;
goto store_x;
case ADDU:case ADDUI:case INCH:case INCMH:case INCML:case INCL:
x= w;goto store_x;
case SUBU:case SUBUI:case NEGU:case NEGUI:x= ominus(y,z);goto store_x;
case IIADDU:case IIADDUI:case IVADDU:case IVADDUI:
case VIIIADDU:case VIIIADDUI:case XVIADDU:case XVIADDUI:
x= oplus(shift_left(y,((op&0xf)>>1)-3),z);goto store_x;
case SETH:case SETMH:case SETML:case SETL:case GETA:case GETAB:
x= z;goto store_x;

/*:65*//*66:*/
#line 1943 "mmix-sim.w"

case OR:case ORI:case ORH:case ORMH:case ORML:case ORL:
x.h= y.h|z.h;x.l= y.l|z.l;goto store_x;
case ORN:case ORNI:
x.h= y.h|~z.h;x.l= y.l|~z.l;goto store_x;
case NOR:case NORI:
x.h= ~(y.h|z.h);x.l= ~(y.l|z.l);goto store_x;
case XOR:case XORI:
x.h= y.h^z.h;x.l= y.l^z.l;goto store_x;
case AND:case ANDI:
x.h= y.h&z.h;x.l= y.l&z.l;goto store_x;
case ANDN:case ANDNI:case ANDNH:case ANDNMH:case ANDNML:case ANDNL:
x.h= y.h&~z.h;x.l= y.l&~z.l;goto store_x;
case NAND:case NANDI:
x.h= ~(y.h&z.h);x.l= ~(y.l&z.l);goto store_x;
case NXOR:case NXORI:
x.h= ~(y.h^z.h);x.l= ~(y.l^z.l);goto store_x;

/*:66*//*67:*/
#line 1968 "mmix-sim.w"

case SL:case SLI:x= shift_left(y,shift_amt);
a= shift_right(x,shift_amt,0);
if(a.h!=y.h||a.l!=y.l)exc|= V_BIT;
goto store_x;
case SLU:case SLUI:x= shift_left(y,shift_amt);goto store_x;
case SR:case SRI:case SRU:case SRUI:
x= shift_right(y,shift_amt,op&0x2);goto store_x;
case MUX:case MUXI:
x.h= (y.h&b.h)|(z.h&~b.h);x.l= (y.l&b.l)|(z.l&~b.l);
goto store_x;
case SADD:case SADDI:
x.l= count_bits(y.h&~z.h)+count_bits(y.l&~z.l);goto store_x;
case MOR:case MORI:
x= bool_mult(y,z,false);goto store_x;
case MXOR:case MXORI:
x= bool_mult(y,z,true);goto store_x;
case BDIF:case BDIFI:
x.h= byte_diff(y.h,z.h);x.l= byte_diff(y.l,z.l);goto store_x;
case WDIF:case WDIFI:
x.h= wyde_diff(y.h,z.h);x.l= wyde_diff(y.l,z.l);goto store_x;
case TDIF:case TDIFI:
if(y.h> z.h)x.h= y.h-z.h;
tdif_l:if(y.l> z.l)x.l= y.l-z.l;goto store_x;
case ODIF:case ODIFI:if(y.h> z.h)x= ominus(y,z);
else if(y.h==z.h)goto tdif_l;
goto store_x;

/*:67*//*68:*/
#line 1999 "mmix-sim.w"

case MUL:case MULI:x= signed_omult(y,z);
test_overflow:if(overflow)exc|= V_BIT;
goto store_x;
case MULU:case MULUI:x= omult(y,z);a= g[rH]= aux;goto store_x;
case DIV:case DIVI:if(!z.l&&!z.h)aux= y,exc|= D_BIT;
else x= signed_odiv(y,z);
a= g[rR]= aux;goto test_overflow;
case DIVU:case DIVUI:x= odiv(b,y,z);a= g[rR]= aux;goto store_x;

/*:68*//*69:*/
#line 2015 "mmix-sim.w"

case FADD:x= fplus(y,z);
fin_float:round_mode= cur_round;
store_fx:exc|= exceptions;goto store_x;
case FSUB:a= z;if(fcomp(a,zero_octa)!=2)a.h^= sign_bit;
x= fplus(y,a);goto fin_float;
case FMUL:x= fmult(y,z);goto fin_float;
case FDIV:x= fdivide(y,z);goto fin_float;
case FREM:x= fremstep(y,z,2500);goto fin_float;
case FSQRT:x= froot(z,y.l);
fin_unifloat:if(y.h||y.l> 4)goto illegal_inst;
round_mode= (y.l?y.l:cur_round);goto store_fx;
case FINT:x= fintegerize(z,y.l);goto fin_unifloat;
case FIX:x= fixit(z,y.l);goto fin_unifloat;
case FIXU:x= fixit(z,y.l);exceptions&= ~W_BIT;goto fin_unifloat;
case FLOT:case FLOTI:case FLOTU:case FLOTUI:
case SFLOT:case SFLOTI:case SFLOTU:case SFLOTUI:
x= floatit(z,y.l,op&0x2,op&0x4);goto fin_unifloat;

/*:69*//*70:*/
#line 2039 "mmix-sim.w"

case CMP:case CMPI:if((y.h&sign_bit)> (z.h&sign_bit))goto cmp_neg;
if((y.h&sign_bit)<(z.h&sign_bit))goto cmp_pos;
case CMPU:case CMPUI:if(y.h<z.h)goto cmp_neg;
if(y.h> z.h)goto cmp_pos;
if(y.l<z.l)goto cmp_neg;
if(y.l==z.l)goto cmp_zero;
cmp_pos:x.l= 1;goto store_x;
cmp_neg:x= neg_one;goto store_x;
case FCMPE:k= fepscomp(y,z,b,true);
if(k)goto cmp_zero_or_invalid;
case FCMP:k= fcomp(y,z);
if(k<0)goto cmp_neg;
cmp_fin:if(k==1)goto cmp_pos;
cmp_zero_or_invalid:if(k==2)exc|= I_BIT;
goto cmp_zero;
case FUN:if(fcomp(y,z)==2)goto cmp_pos;else goto cmp_zero;
case FEQL:if(fcomp(y,z)==0)goto cmp_pos;else goto cmp_zero;
case FEQLE:k= fepscomp(y,z,b,false);
goto cmp_fin;
case FUNE:if(fepscomp(y,z,b,true)==2)goto cmp_pos;else goto cmp_zero;

/*:70*//*72:*/
#line 2085 "mmix-sim.w"

case CSN:case CSNI:case CSZ:case CSZI:
case CSP:case CSPI:case CSOD:case CSODI:
case CSNN:case CSNNI:case CSNZ:case CSNZI:
case CSNP:case CSNPI:case CSEV:case CSEVI:
case ZSN:case ZSNI:case ZSZ:case ZSZI:
case ZSP:case ZSPI:case ZSOD:case ZSODI:
case ZSNN:case ZSNNI:case ZSNZ:case ZSNZI:
case ZSNP:case ZSNPI:case ZSEV:case ZSEVI:
x= register_truth(y,op)?z:b;goto store_x;

/*:72*//*73:*/
#line 2099 "mmix-sim.w"

case BN:case BNB:case BZ:case BZB:
case BP:case BPB:case BOD:case BODB:
case BNN:case BNNB:case BNZ:case BNZB:
case BNP:case BNPB:case BEV:case BEVB:
case PBN:case PBNB:case PBZ:case PBZB:
case PBP:case PBPB:case PBOD:case PBODB:
case PBNN:case PBNNB:case PBNZ:case PBNZB:
case PBNP:case PBNPB:case PBEV:case PBEVB:
x.l= register_truth(b,op);
if(x.l){
inst_ptr= z;
good= (op>=PBN);
}else good= (op<PBN);
if(good)good_guesses++;
else bad_guesses++,g[rC].l+= 2;
break;

/*:73*//*74:*/
#line 2120 "mmix-sim.w"

#line 855 "mmix-sim.ch"
case LDB:case LDBI:
if((w.h&sign_bit)&&!(loc.h&sign_bit))goto translation_bypassed_inst;
if(!load_data(1,&x,w,1))goto page_fault;
goto check_ld;
case LDBU:case LDBUI:
if((w.h&sign_bit)&&!(loc.h&sign_bit))goto translation_bypassed_inst;
if(!load_data(1,&x,w,0))goto page_fault;
goto check_ld;
case LDW:case LDWI:
if((w.h&sign_bit)&&!(loc.h&sign_bit))goto translation_bypassed_inst;
if(!load_data(2,&x,w,1))goto page_fault;
goto check_ld;
case LDWU:case LDWUI:
if((w.h&sign_bit)&&!(loc.h&sign_bit))goto translation_bypassed_inst;
if(!load_data(2,&x,w,0))goto page_fault;
goto check_ld;
case LDT:case LDTI:
if((w.h&sign_bit)&&!(loc.h&sign_bit))goto translation_bypassed_inst;
if(!load_data(4,&x,w,1))goto page_fault;
goto check_ld;
case LDTU:case LDTUI:
if((w.h&sign_bit)&&!(loc.h&sign_bit))goto translation_bypassed_inst;
if(!load_data(4,&x,w,0))goto page_fault;
goto check_ld;
case LDHT:case LDHTI:
if((w.h&sign_bit)&&!(loc.h&sign_bit))goto translation_bypassed_inst;
if(!load_data(4,&x,w,0))goto page_fault;
x.h= x.l;
x.l= 0;
goto check_ld;
case LDO:case LDOI:
case LDOU:case LDOUI:
case LDUNC:case LDUNCI:
if((w.h&sign_bit)&&!(loc.h&sign_bit))goto translation_bypassed_inst;
if(!load_data(8,&x,w,0))goto page_fault;
goto check_ld;
case LDSF:case LDSFI:
if((w.h&sign_bit)&&!(loc.h&sign_bit))goto translation_bypassed_inst;
if(!load_data(4,&x,w,0))goto page_fault;
x= load_sf(x.l);
check_ld:
test_load_bkpt(w);
goto store_x;
page_fault:
if((g[rK].h&g[rQ].h)!=0||(g[rK].l&g[rQ].l)!=0)
{x.h= 0,x.l= inst;
/*106:*/
#line 1603 "mmix-sim.ch"

g[rWW]= inst_ptr;
g[rXX]= x;
g[rYY]= y;
g[rZZ]= z;
z.h= 0,z.l= zz;
g[rK].h= g[rK].l= 0;
g[rBB]= g[255];
g[255]= g[rJ];

/*:106*/
#line 901 "mmix-sim.ch"

inst_ptr= y= g[rTT];
}
break;
#line 2140 "mmix-sim.w"

#line 938 "mmix-sim.ch"
/*:74*//*75:*/
#line 938 "mmix-sim.ch"

case STB:case STBI:case STBU:case STBUI:
i= 56;j= 1;goto fin_pst;
case STW:case STWI:case STWU:case STWUI:
i= 48;j= 2;goto fin_pst;
case STT:case STTI:case STTU:case STTUI:
i= 32;j= 4;goto fin_pst;
fin_pst:
if((op&0x2)==0){
a= shift_right(shift_left(b,i),i,0);
if(a.h!=b.h||a.l!=b.l)exc|= V_BIT;
}
fin_st:if((w.h&sign_bit)&&!(loc.h&sign_bit))goto translation_bypassed_inst;
if(!store_data(j,b,w))goto page_fault;
test_store_bkpt(w);
break;
case STSF:case STSFI:
b.l= store_sf(b);exc= exceptions;
j= 4;
goto fin_st;
case STHT:case STHTI:
b.l= b.h;
j= 4;
goto fin_st;
case STCO:case STCOI:b.h= 0;b.l= xx;
case STO:case STOI:case STOU:case STOUI:
case STUNC:case STUNCI:
j= 8;
goto fin_st;
#line 2170 "mmix-sim.w"

#line 990 "mmix-sim.ch"
/*:75*//*76:*/
#line 996 "mmix-sim.ch"

case CSWAP:case CSWAPI:
if((w.h&sign_bit)&&!(loc.h&sign_bit))goto translation_bypassed_inst;
if(!load_data(8,&a,w,0))goto page_fault;
if(g[rP].h==a.h&&g[rP].l==a.l){
x.h= 0,x.l= 1;
test_store_bkpt(w);
if(!store_data(8,b,w))goto page_fault;
strcpy(rhs,"M8[%#w]=%#b");
}else{
b= a;
a= g[rP];
g[rP]= b;
x.h= 0,x.l= 0;
strcpy(rhs,"rP=%#b");
}
goto check_ld;
#line 2190 "mmix-sim.w"

/*:76*//*77:*/
#line 2193 "mmix-sim.w"

#line 1031 "mmix-sim.ch"
case GET:if(yy!=0||zz>=32)goto illegal_inst;
if(zz==rC&&hostclock)
{x.l= clock();
x.h= 0;
}
else
x= g[zz];
if(zz==rQ)new_Q.h= new_Q.l= 0;
goto store_x;
case PUT:case PUTI:if(yy!=0||xx>=32)goto illegal_inst;
strcpy(rhs,"%z = %#z");
if(xx>=8){
if(xx<=9)goto illegal_inst;
if(xx<=18&&!(loc.h&sign_bit))goto privileged_inst;
if(xx==rA)/*80:*/
#line 2227 "mmix-sim.w"

{
if(z.h!=0||z.l>=0x40000)goto illegal_inst;
cur_round= (z.l>=0x10000?z.l>>16:ROUND_NEAR);
}

/*:80*/
#line 1045 "mmix-sim.ch"

else if(xx==rL)/*78:*/
#line 2208 "mmix-sim.w"

{
x= z;strcpy(rhs,z.h?"min(rL,%#x) = %z":"min(rL,%x) = %z");
if(z.l> L||z.h)z.h= 0,z.l= L;
else old_L= L= z.l;
}

/*:78*/
#line 1046 "mmix-sim.ch"

else if(xx==rG)/*79:*/
#line 2215 "mmix-sim.w"

{
if(z.h!=0||z.l> 255||z.l<L||z.l<32)goto illegal_inst;
for(j= z.l;j<G;j++)g[j]= zero_octa;
G= z.l;
}

/*:79*/
#line 1047 "mmix-sim.ch"

else if(xx==rQ)
{new_Q.h|= z.h&~g[rQ].h;
new_Q.l|= z.l&~g[rQ].l;
z.l|= new_Q.l;
z.h|= new_Q.h;
}
}
g[xx]= z;zz= xx;break;
#line 2207 "mmix-sim.w"

/*:77*//*81:*/
#line 2236 "mmix-sim.w"

#line 1062 "mmix-sim.ch"
case PUSHGO:case PUSHGOI:
if((w.h&sign_bit)&&!(loc.h&sign_bit))
{new_Q.h|= P_BIT;g[rQ].h|= P_BIT;goto security_inst;}
inst_ptr= w;goto push;
case PUSHJ:case PUSHJB:
if((z.h&sign_bit)&&!(loc.h&sign_bit))
{new_Q.h|= P_BIT;g[rQ].h|= P_BIT;goto security_inst;}
inst_ptr= z;
#line 2239 "mmix-sim.w"
push:if(xx>=G){
xx= L++;
if(((S-O-L)&lring_mask)==0)stack_store();
}
x.l= xx;l[(O+xx)&lring_mask]= x;
sprintf(lhs,"l[%d]=%d, ",(O+xx)&lring_mask,xx);
x= g[rJ]= incr(loc,4);
L-= xx+1;O+= xx+1;
b= g[rO]= incr(g[rO],(xx+1)<<3);
sync_L:a.l= g[rL].l= L;break;
case POP:if(xx!=0&&xx<=L)y= l[(O+xx-1)&lring_mask];
if(g[rS].l==g[rO].l)stack_load();
k= l[(O-1)&lring_mask].l&0xff;
while((tetra)(O-S)<=(tetra)k)stack_load();
L= k+(xx<=L?xx:L+1);
if(L> G)L= G;
if(L> k){
l[(O-1)&lring_mask]= y;
if(y.h)sprintf(lhs,"l[%d]=#%x%08x, ",(O-1)&lring_mask,y.h,y.l);
else sprintf(lhs,"l[%d]=#%x, ",(O-1)&lring_mask,y.l);
}else lhs[0]= '\0';
y= g[rJ];z.l= yz<<2;inst_ptr= oplus(y,z);
O-= k+1;b= g[rO]= incr(g[rO],-((k+1)<<3));
goto sync_L;

/*:81*//*82:*/
#line 2267 "mmix-sim.w"

#line 1077 "mmix-sim.ch"
case SAVE:if(xx<G||yy!=0||zz!=0)goto illegal_inst;
l[(O+L)&lring_mask].l= L,L++;
#line 2270 "mmix-sim.w"
if(((S-O-L)&lring_mask)==0)stack_store();
O+= L;g[rO]= incr(g[rO],L<<3);
L= g[rL].l= 0;
while(g[rO].l!=g[rS].l)stack_store();
for(k= G;;){
/*83:*/
#line 1093 "mmix-sim.ch"

if(k==rZ+1)x.h= G<<24,x.l= g[rA].l;
else x= g[k];
if(!store_data(8,x,g[rS]))goto page_fault;
test_store_bkpt(g[rS]);
if(stack_tracing){
tracing= true;
#line 2297 "mmix-sim.w"
if(k>=32)printf("             M8[#%08x%08x]=g[%d]=#%08x%08x, rS+=8\n",
g[rS].h,g[rS].l,k,x.h,x.l);
else printf("             M8[#%08x%08x]=%s=#%08x%08x, rS+=8\n",
g[rS].h,g[rS].l,k==rZ+1?"(rG,rA)":special_name[k],x.h,x.l);
}
S++,g[rS]= incr(g[rS],8);

/*:83*/
#line 2275 "mmix-sim.w"
;
if(k==255)k= rB;
else if(k==rR)k= rP;
else if(k==rZ+1)break;
else k++;
}
O= S,g[rO]= g[rS];
x= incr(g[rO],-8);goto store_x;

/*:82*//*84:*/
#line 2304 "mmix-sim.w"

case UNSAVE:if(xx!=0||yy!=0)goto illegal_inst;
z.l&= -8;g[rS]= incr(z,8);
for(k= rZ+1;;){
/*85:*/
#line 1120 "mmix-sim.ch"

g[rS]= incr(g[rS],-8);
test_load_bkpt(g[rS]);
if(k==rZ+1)
{if(!load_data(8,&a,g[rS],0))goto page_fault;
x.l= G= g[rG].l= a.h>>24;
a.l= g[rA].l= a.l&0x3ffff;
}
else
if(!load_data(8,&(g[k]),g[rS],0))goto page_fault;
if(stack_tracing){
tracing= true;
if(k>=32)printf("             rS-=8, g[%d]=M8[#%08x%08x]=#%08x%08x\n",
k,g[rS].h,g[rS].l,g[k].h,g[k].l);
else if(k==rZ+1)printf("             (rG,rA)=M8[#%08x%08x]=#%08x%08x\n",
g[rS].h,g[rS].l,g[k].h,g[k].l);
else printf("             rS-=8, %s=M8[#%08x%08x]=#%08x%08x\n",
special_name[k],g[rS].h,g[rS].l,g[k].h,g[k].l);
}
#line 2339 "mmix-sim.w"

#line 1152 "mmix-sim.ch"
/*:85*/
#line 2308 "mmix-sim.w"
;
if(k==rP)k= rR;
else if(k==rB)k= 255;
else if(k==G)break;
else k--;
}
S= g[rS].l>>3;
stack_load();
k= l[S&lring_mask].l&0xff;
for(j= 0;j<k;j++)stack_load();
O= S;g[rO]= g[rS];
L= k> G?G:k;
g[rL].l= L;a= g[rL];
g[rG].l= G;break;

#line 1120 "mmix-sim.ch"
/*:84*//*86:*/
#line 1154 "mmix-sim.ch"

case SYNCID:case SYNCIDI:
delete_instruction(w,xx+1);
if(loc.h&sign_bit)
delete_data(w,xx+1);
else
write_data(w,xx+1);
break;
case PREST:case PRESTI:x= incr(w,xx);break;
case SYNCD:case SYNCDI:
write_data(w,xx+1);
if(loc.h&sign_bit)
delete_data(w,xx+1);
break;
case PREGO:case PREGOI:
prego_instruction(w,xx+1);
break;
case PRELD:case PRELDI:
preload_data_cache(w,xx+1);
x= incr(w,xx);break;
#line 2349 "mmix-sim.w"

/*:86*//*87:*/
#line 2353 "mmix-sim.w"

case GO:case GOI:x= inst_ptr;inst_ptr= w;goto store_x;
case JMP:case JMPB:inst_ptr= z;break;
#line 1186 "mmix-sim.ch"
case SYNC:if(xx!=0||yy!=0||zz> 7)goto illegal_inst;

else if(zz==4)
vmb_wait_for_event(&vmb);
else if(zz==5)
write_all_data_cache();
else if(zz==6)
{clear_all_data_vtc();
clear_all_instruction_vtc();
}
else if(zz==7)
{clear_all_data_cache();
clear_all_instruction_cache();
}
break;
case LDVTS:case LDVTSI:
{if(!(loc.h&sign_bit))goto privileged_inst;
if(w.h&sign_bit)goto illegal_inst;
x= update_vtc(w);
goto store_x;
}
break;
case SWYM:
if((inst&0xFFFFFF)!=0)
{char buf[256];
int n;
strcpy(rhs,"$%x,%z");
z.h= 0,z.l= yz;
x.h= 0,x.l= xx;
tracing= interacting;
breakpoint= true;
interrupt= false;
if(loc.h&sign_bit)show_operating_system= true;
/*53:*/
#line 1771 "mmix-sim.w"

{
if(xx>=G)b= g[xx];
else if(xx<L)b= l[(O+xx)&lring_mask];
}

#line 705 "mmix-sim.ch"
/*:53*/
#line 1219 "mmix-sim.ch"
;
n= mmgetchars(buf,256,b,0);
if(strncmp(buf,"DEBUG ",6)==0)printf("\n\t%s!\n\n",buf+6);
}
else
strcpy(rhs,"");
break;
translation_bypassed_inst:strcpy(lhs,"!virtual translation bypassed");
g[rQ].h|= N_BIT;new_Q.h|= N_BIT;
goto break_inst;
privileged_inst:strcpy(lhs,"!privileged");
g[rQ].h|= K_BIT;new_Q.h|= K_BIT;
goto break_inst;
illegal_inst:strcpy(lhs,"!illegal");
g[rQ].h|= B_BIT;new_Q.h|= B_BIT;
goto break_inst;
security_inst:strcpy(lhs,"!security violation");
break_inst:breakpoint= tracing= true;
if(!interacting&&!interact_after_break)halted= true;
break;
#line 2364 "mmix-sim.w"

/*:87*//*88:*/
#line 1279 "mmix-sim.ch"

case TRIP:exc|= H_BIT;break;
case TRAP:if(xx==0&&yy<=max_sys_call)
{strcpy(rhs,trap_format[yy]);
a= incr(b,8);
/*90:*/
#line 1333 "mmix-sim.ch"

if(arg_count[yy]==3){
load_data(8,&mb,b,0);
load_data(8,&ma,a,0);
}
#line 2429 "mmix-sim.w"

#line 2449 "mmix-sim.w"

/*:90*/
#line 1284 "mmix-sim.ch"
;
}
else strcpy(rhs,"%#x -> %#y");
if(inst==0)
{if(interacting)
tracing= breakpoint= true,interrupt= false;
}
x.h= sign_bit,x.l= inst;
/*106:*/
#line 1603 "mmix-sim.ch"

g[rWW]= inst_ptr;
g[rXX]= x;
g[rYY]= y;
g[rZZ]= z;
z.h= 0,z.l= zz;
g[rK].h= g[rK].l= 0;
g[rBB]= g[255];
g[255]= g[rJ];

/*:106*/
#line 1292 "mmix-sim.ch"

inst_ptr= y= g[rT];
break;
#line 2406 "mmix-sim.w"

/*:88*//*107:*/
#line 2603 "mmix-sim.w"

#line 1623 "mmix-sim.ch"
case RESUME:if(xx||yy)goto illegal_inst;
if(zz==0)
{inst_ptr= z= g[rW];
b= g[rX];
}
else if(zz==1)
{
if(!(loc.h&sign_bit))goto privileged_inst;
inst_ptr= z= g[rWW];
b= g[rXX];
g[rK]= g[255];
x= g[255]= g[rBB];
}
else goto illegal_inst;
if(!(b.h&sign_bit))/*108:*/
#line 1681 "mmix-sim.ch"

{
rop= b.h>>24;
switch(rop){
case RESUME_CONT:if((1<<(b.l>>28))&0x8f30)goto illegal_inst;
case RESUME_SET:k= (b.l>>16)&0xff;
if(k>=L&&k<G)goto illegal_inst;
case RESUME_AGAIN:if((b.l>>24)==RESUME)goto illegal_inst;
break;
case RESUME_TRANS:if(zz==0)goto illegal_inst;
break;
default:goto illegal_inst;
}
resuming= true;
}

/*:108*/
#line 1637 "mmix-sim.ch"
;
break;
#line 2609 "mmix-sim.w"

/*:107*/
#line 1371 "mmix-sim.w"
;
}
/*99:*/
#line 2577 "mmix-sim.w"

if((exc&(U_BIT+X_BIT))==U_BIT&&!(g[rA].l&U_BIT))exc&= ~U_BIT;
if(exc){
if(exc&tracing_exceptions)tracing= true;
j= exc&(g[rA].l|H_BIT);
if(j)/*100:*/
#line 2586 "mmix-sim.w"

{
tripping= true;
for(k= 0;!(j&H_BIT);j<<= 1,k++);
exc&= ~(H_BIT>>k);
g[rW]= inst_ptr;
inst_ptr.h= 0,inst_ptr.l= k<<4;
g[rX].h= sign_bit,g[rX].l= inst;
if((op&0xe0)==STB)g[rY]= w,g[rZ]= b;
else g[rY]= y,g[rZ]= z;
g[rB]= g[255];
g[255]= g[rJ];
if(op==TRIP)w= g[rW],x= g[rX],a= g[255];
}

#line 1533 "mmix-sim.ch"
/*:100*/
#line 2582 "mmix-sim.w"
;
g[rA].l|= exc>>8;
}

/*:99*/
#line 1373 "mmix-sim.w"
;
/*110:*/
#line 2645 "mmix-sim.w"

if(g[rU].l||g[rU].h||!resuming){
g[rC].h+= info[op].mems;
g[rC]= incr(g[rC],info[op].oops);
g[rU]= incr(g[rU],1);
g[rI]= incr(g[rI],-1);
if(g[rI].l==0&&g[rI].h==0)tracing= breakpoint= true;
}

/*:110*/
#line 1374 "mmix-sim.w"
;
#line 637 "mmix-sim.ch"
/*111:*/
#line 1743 "mmix-sim.ch"

if(tracing&&(!(loc.h&0x80000000)||show_operating_system)){
/*113:*/
#line 2675 "mmix-sim.w"

if(resuming&&op!=RESUME){
switch(rop){
case RESUME_AGAIN:printf("           (%08x%08x: %08x (%s)) ",
loc.h,loc.l,inst,info[op].name);break;
case RESUME_CONT:printf("           (%08x%08x: %04xrYrZ (%s)) ",
loc.h,loc.l,inst>>16,info[op].name);break;
case RESUME_SET:printf("           (%08x%08x: ..%02x..rZ (SET)) ",
loc.h,loc.l,(inst>>16)&0xff);break;
}
}else{
#line 1759 "mmix-sim.ch"
printf("%08x%08x: %08x (%s) ",loc.h,loc.l,inst,info[op].name);
#line 2688 "mmix-sim.w"
}

/*:113*/
#line 1745 "mmix-sim.ch"
;
/*114:*/
#line 1766 "mmix-sim.ch"

if(lhs[0]=='!'){printf("%s instruction!\n",lhs+1);
lhs[0]= '\0';
}
#line 2701 "mmix-sim.w"
else{
/*115:*/
#line 2714 "mmix-sim.w"

if(L!=old_L&&!(f&push_pop_bit))printf("rL=%d, ",L);

/*:115*/
#line 2702 "mmix-sim.w"
;
if(z.l==0&&(op==ADDUI||op==ORI))p= "%l = %y = %#x";
else p= info[op].trace_format;
for(;*p;p++)/*116:*/
#line 2765 "mmix-sim.w"

{
if(*p!='%')fputc(*p,stdout);
else{
style= decimal;
char_switch:switch(*++p){
/*117:*/
#line 2788 "mmix-sim.w"

case'#':style= hex;goto char_switch;
case'0':style= zhex;goto char_switch;
case'.':style= floating;goto char_switch;
case'!':style= handle;goto char_switch;

/*:117*//*119:*/
#line 2797 "mmix-sim.w"

case'a':trace_print(a);break;
case'b':trace_print(b);break;
case'p':trace_print(ma);break;
case'q':trace_print(mb);break;
case'w':trace_print(w);break;
case'x':trace_print(x);break;
case'y':trace_print(y);break;
case'z':trace_print(z);break;

/*:119*//*121:*/
#line 2828 "mmix-sim.w"

case'(':fputc(left_paren[round_mode],stdout);break;
case')':fputc(right_paren[round_mode],stdout);break;
case't':if(x.l)printf(" Yes, -> #"),print_hex(inst_ptr);
else printf(" No");break;
case'g':if(!good)printf(" (bad guess)");break;
case's':printf(special_name[zz]);break;
case'?':p++;if(z.l)printf("%c%d",*p,z.l);break;
case'l':printf(lhs);break;
case'r':p= switchable_string;break;

/*:121*/
#line 2771 "mmix-sim.w"
;
default:printf("BUG!!");
}
}
}

/*:116*/
#line 2705 "mmix-sim.w"
;
if(exc)printf(", rA=#%05x",g[rA].l);
if(tripping)tripping= false,printf(", -> #%02x",inst_ptr.l);
printf("\n");
}

/*:114*/
#line 1746 "mmix-sim.ch"
;
if(showing_stats||breakpoint)show_stats(breakpoint);
just_traced= true;
}else if(just_traced){
printf(" ...............................................\n");
just_traced= false;
}
#line 2670 "mmix-sim.w"

/*:111*/
#line 637 "mmix-sim.ch"
;
/*102:*/
#line 1544 "mmix-sim.ch"

if(!resuming)
{if(vmb_get_interrupt(&vmb,&new_Q.h,&new_Q.l)==1)
{g[rQ].h|= new_Q.h;g[rQ].l|= new_Q.l;}
if(!vmb.connected)goto end_simulation;
if(!vmb.power||vmb.reset_flag){breakpoint= true;vmb.reset_flag= 0;goto boot;}
if((g[rK].h&g[rQ].h)!=0||(g[rK].l&g[rQ].l)!=0)
{
x.h= sign_bit,x.l= inst;
/*106:*/
#line 1603 "mmix-sim.ch"

g[rWW]= inst_ptr;
g[rXX]= x;
g[rYY]= y;
g[rZZ]= z;
z.h= 0,z.l= zz;
g[rK].h= g[rK].l= 0;
g[rBB]= g[255];
g[255]= g[rJ];

/*:106*/
#line 1553 "mmix-sim.ch"

inst_ptr= y= g[rTT];
}
}

/*:102*/
#line 638 "mmix-sim.ch"
;
#line 1376 "mmix-sim.w"
if(resuming&&op!=RESUME)resuming= false;
}

/*:39*/
#line 1840 "mmix-sim.ch"

while((!interrupt&&!breakpoint)||resuming);
if(interact_after_break)interacting= true,interact_after_break= false;
if(!vmb.power)goto boot;
}
end_simulation:
if(interacting||profiling||showing_stats)show_stats(true);
return g[255].l;
}
#line 2909 "mmix-sim.w"

/*:124*/
