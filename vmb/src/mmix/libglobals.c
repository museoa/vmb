#include <stdio.h>
#include "mmix-internals.h"

/* global exported varaibles of mmix-sim */

tetra priority= 314159265;
mem_node*mem_root;
mem_node*last_mem;
octa sclock;
FILE*mmo_file;
int postamble;
int byte_count;
byte buf[4];
int yzbytes;
int delta;
tetra tet;
octa cur_loc;
int cur_file= -1;
int cur_line;
octa tmp;
tetra obj_time;
file_node file_info[256];
int ybyte2file_no[256];
int buf_size;
Char*buffer;
FILE*src_file;
int shown_file= -1;
int shown_line;
int gap;
bool line_shown;
bool showing_source;
int profile_gap;
bool profile_showing_source;
octa implied_loc;
bool profile_started;
char*special_name[32]= {"rB","rD","rE","rH","rJ","rM","rR","rBB",
"rC","rN","rO","rS","rI","rT","rTT","rK","rQ","rU","rV","rG","rL",
"rA","rF","rP","rW","rX","rY","rZ","rWW","rXX","rYY","rZZ"};
octa w,x,y,z,a,b,ma,mb;
octa*x_ptr;
octa loc;
octa inst_ptr;
tetra inst;
int old_L;
int exc;
unsigned int tracing_exceptions;
int rop;
int rzz;
int round_mode;
bool resuming;
bool halted;
bool breakpoint;
bool tracing;
bool trace_once;
bool stack_tracing;
bool interacting;
bool show_operating_system= false;
octa rOlimit={-1,-1};
#ifdef MMIXLIB
extern int port; /* on which port to connect to the bus */
extern char *host; /* on which host to connect to the bus */
#else
char localhost[]="localhost";
int port=9002; /* on which port to connect to the bus */
char *host=localhost; /* on which host to connect to the bus */
#endif
bool interact_after_break;
bool tripping;
bool good;
tetra trace_threshold;
mmix_opcode op;
tetra f;
int xx,yy,zz,yz;
op_info info[256]= {
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
{"SRUI",0x29,0,0,1,"%l = %#y >> %z = %#x"},
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
{"ZSEVI",0x29,0,0,1,"%l = %y even? %z: 0 = %x"},
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
{"PUSHGOI",0xa9,0,0,3,"%lrO=%#b, rL=%a, rJ=%#x, -> %#y%?+"},
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
{"RESUME",0x00,0,0,5,"{%#b}, $255 = %x, -> %#z"},
{"SAVE",0x20,0,20,1,"%l = %#x"},
{"UNSAVE",0x82,0,20,1,"%#z: rG=%x, ..., rL=%a"},
{"SYNC",0x01,0,0,1,"%z"},
{"SWYM",0x01,0,0,1,"%r"},
{"GET",0x20,0,0,1,"%l = %s = %#x"},
{"TRIP",0x0a,255,0,5,"rW=%#w, rX=%#x, rY=%#y, rZ=%#z, rB=%#b, g[255]=%#a"}
};
int G= 255,L= 0,O= 0;
octa g[256];
octa*l;
int lring_size;
int lring_mask;
int S;
char arg_count[]= {1,3,1,3,3,3,3,2,2,2,1};
char*trap_format[]= {
"Halt(%z)",
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
char stdin_buf[256];
char*stdin_buf_start;
char*stdin_buf_end;
octa new_Q;
bool showing_stats;
bool just_traced;
char left_paren[]= {0,'[','^','_','('};
char right_paren[]= {0,']','^','_',')'};
char switchable_string[300]= {0};
char lhs[32];
int good_guesses,bad_guesses;
char*myself;
char**cur_arg;
bool interrupt= 0;
bool profiling= 0;
FILE*fake_stdin;
FILE*dump_file;
char*usage_help[]= {
" with these options: (<n>=decimal number, <x>=hex number)\n",
"-t<n> trace each instruction the first n times\n",
"-e<x> trace each instruction with an exception matching x\n",
"-r    trace hidden details of the register stack\n",
"-O    trace inside the operating system\n",
"-o    disable trace inside the operating system\n",
"-B<n> connect to Bus on port <n>\n",
"-s    show statistics after each traced instruction\n",
"-P    print a profile when simulation ends\n",
"-L<n> list source lines with the profile\n",
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
"T         set current segment to Text_Segment\n",
"D         set current segment to Data_Segment\n",
"P         set current segment to Pool_Segment\n",
"S         set current segment to Stack_Segment\n",
"N         set current segment to Negative Addresses\n",
"O         enable tracing inside the operating system\n",
"o         disable tracing inside the operating system\n",
"B         show all current breakpoints and tracepoints\n",
"i<file>   insert commands from file\n",
"-<option> change a tracing/listing/profile option\n",
"-?        show the tracing/listing/profile options  \n",
""};
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
