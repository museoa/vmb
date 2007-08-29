#define panic(m) {fprintf(stderr,"Panic: %s!\n",m) ;exit(-2) ;}
#define sign_bit ((unsigned) 0x80000000)  \
 #define mm 0x98
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
 #define mmo_load(loc,val) ll= mem_find(loc) ,ll->tet^= val \
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
 #define test_store_bkpt(ll) if((ll) ->bkpt&write_bit) breakpoint= tracing= true \
 #define test_load_bkpt(ll) if((ll) ->bkpt&read_bit) breakpoint= tracing= true \
 #define shift_amt (z.h||z.l>=64?64:z.l)  \
 #define cmp_zero store_x \
 #define ROUND_OFF 1
#define ROUND_UP 2
#define ROUND_DOWN 3
#define ROUND_NEAR 4 \
 #define RESUME_AGAIN 0
#define RESUME_CONT 1
#define RESUME_SET 2 \
 #define rhs &switchable_string[1] \
 #define mmo_file_name *cur_arg \
 #define command_buf_size 1024 \
 #define _REENTERANT  \
 #define window_size_x 640
#define window_size_y 480
#define window_pos_x 100
#define window_pos_y 100 \
 \
 #define C2bits(c,sp) (((((c) &0xff) *sp.cch_scale) >>8) *sp.cch_factor)
#define RGB2Pixel(r,g,b) (C2bits(r,vga.red) |C2bits(g,vga.green) |C2bits(b,vga.blue) )  \
 #define do_pixel(x,y,r,g,b) XPutPixel(vga.image,x,y,RGB2Pixel(r,g,b) )  \
 \
    #include <stdio.h>                                               /* 141 */
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include "abstime.h"
#include <pthread.h>                                                 /* 173 */
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#ifdef __STDC__                                                      /*  11 */
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif
#include <sys/types.h>                                               /*  43 */
#include <sys/stat.h>
#ifndef SEEK_SET                                                     /*  46 */
#define SEEK_SET 0
#endif
typedef enum
{ false, true } bool;                                                /*   9 */
typedef unsigned int tetra;                                          /*  10 */
typedef struct
{
  tetra h, l;
} octa;
typedef unsigned char byte;
typedef struct
{                                                                    /*  16 */
  tetra tet;
  tetra freq;
  unsigned char bkpt;
  unsigned char file_no;
  unsigned short line_no;
} mem_tetra;
typedef struct mem_node_struct
{
  octa loc;
  tetra stamp;
  struct mem_node_struct *left, *right;
  mem_tetra dat[512];
} mem_node;
typedef struct
{                                                                    /*  38 */
  char *name;
  int line_count;
  long *map;
} file_node;
typedef char Char;                                                   /*  39 */
typedef enum
{                                                                    /*  54 */
  TRAP, FCMP, FUN, FEQL, FADD, FIX, FSUB, FIXU,
  FLOT, FLOTI, FLOTU, FLOTUI, SFLOT, SFLOTI, SFLOTU, SFLOTUI,
  FMUL, FCMPE, FUNE, FEQLE, FDIV, FSQRT, FREM, FINT,
  MUL, MULI, MULU, MULUI, DIV, DIVI, DIVU, DIVUI,
  ADD, ADDI, ADDU, ADDUI, SUB, SUBI, SUBU, SUBUI,
  IIADDU, IIADDUI, IVADDU, IVADDUI, VIIIADDU, VIIIADDUI, XVIADDU, XVIADDUI,
  CMP, CMPI, CMPU, CMPUI, NEG, NEGI, NEGU, NEGUI,
  SL, SLI, SLU, SLUI, SR, SRI, SRU, SRUI,
  BN, BNB, BZ, BZB, BP, BPB, BOD, BODB,
  BNN, BNNB, BNZ, BNZB, BNP, BNPB, BEV, BEVB,
  PBN, PBNB, PBZ, PBZB, PBP, PBPB, PBOD, PBODB,
  PBNN, PBNNB, PBNZ, PBNZB, PBNP, PBNPB, PBEV, PBEVB,
  CSN, CSNI, CSZ, CSZI, CSP, CSPI, CSOD, CSODI,
  CSNN, CSNNI, CSNZ, CSNZI, CSNP, CSNPI, CSEV, CSEVI,
  ZSN, ZSNI, ZSZ, ZSZI, ZSP, ZSPI, ZSOD, ZSODI,
  ZSNN, ZSNNI, ZSNZ, ZSNZI, ZSNP, ZSNPI, ZSEV, ZSEVI,
  LDB, LDBI, LDBU, LDBUI, LDW, LDWI, LDWU, LDWUI,
  LDT, LDTI, LDTU, LDTUI, LDO, LDOI, LDOU, LDOUI,
  LDSF, LDSFI, LDHT, LDHTI, CSWAP, CSWAPI, LDUNC, LDUNCI,
  LDVTS, LDVTSI, PRELD, PRELDI, PREGO, PREGOI, GO, GOI,
  STB, STBI, STBU, STBUI, STW, STWI, STWU, STWUI,
  STT, STTI, STTU, STTUI, STO, STOI, STOU, STOUI,
  STSF, STSFI, STHT, STHTI, STCO, STCOI, STUNC, STUNCI,
  SYNCD, SYNCDI, PREST, PRESTI, SYNCID, SYNCIDI, PUSHGO, PUSHGOI,
  OR, ORI, ORN, ORNI, NOR, NORI, XOR, XORI,
  AND, ANDI, ANDN, ANDNI, NAND, NANDI, NXOR, NXORI,
  BDIF, BDIFI, WDIF, WDIFI, TDIF, TDIFI, ODIF, ODIFI,
  MUX, MUXI, SADD, SADDI, MOR, MORI, MXOR, MXORI,
  SETH, SETMH, SETML, SETL, INCH, INCMH, INCML, INCL,
  ORH, ORMH, ORML, ORL, ANDNH, ANDNMH, ANDNML, ANDNL,
  JMP, JMPB, PUSHJ, PUSHJB, GETA, GETAB, PUT, PUTI,
  POP, RESUME, SAVE, UNSAVE, SYNC, SWYM, GET, TRIP
} mmix_opcode;
typedef enum
{                                                                    /*  55 */
  rB, rD, rE, rH, rJ, rM, rR, rBB,
  rC, rN, rO, rS, rI, rT, rTT, rK, rQ, rU, rV, rG, rL,
  rA, rF, rP, rW, rX, rY, rZ, rWW, rXX, rYY, rZZ
} special_reg;
typedef enum
{                                                                    /*  59 */
  Halt, Fopen, Fclose, Fread, Fgets, Fgetws,
  Fwrite, Fputs, Fputws, Fseek, Ftell
} sys_call;
typedef struct
{                                                                    /*  64 */
  char *name;
  unsigned char flags;
  unsigned char third_operand;
  unsigned char mems;
  unsigned char oops;
  char *trace_format;
} op_info;
typedef enum
{ decimal, hex, zhex, floating, handle } fmt_style;                  /* 135 */
typedef struct
{                                                                    /* 175 */
  unsigned long cch_scale, cch_factor;
} ColorChannel;
typedef struct
{                                                                    /* 178 */
  Display *display;
  GC gc;
  Window win;
  XImage *image;
  ColorChannel red, green, blue;
  XExposeEvent expose;
  int argc;
  char **argv;
} VGA;
typedef struct
{                                                                    /* 207 */
  int x, y;
  int r, g, b;
} PutPixel;
enum
{                                                                    /* 212 */
  vgaHalt,                                                           /* 203 */
  vgaPutPixel,                                                       /* 205 */
  vgaNop,                                                            /* 210 */
};
typedef union
{
  PutPixel pp;                                                       /* 206 */
} Cmd;
typedef struct
{
  pthread_mutex_t lock;
  pthread_cond_t out_c;
  volatile int status;
  int cmd;
  Cmd data;
} CommandPort;
tetra priority = 314159265;                                          /*  19 */
mem_node *mem_root;
mem_node *last_mem;
FILE *mmo_file;                                                      /*  25 */
int postamble;
int byte_count;
byte buf[4];
int yzbytes;
int delta;
tetra tet;
octa cur_loc;                                                        /*  31 */
int cur_file = -1;
int cur_line;
octa tmp;
tetra obj_time;
file_node file_info[256];                                            /*  40 */
int buf_size;
Char *buffer;
FILE *src_file;                                                      /*  48 */
int shown_file = -1;
int shown_line;
int gap;
bool line_shown;
bool showing_source;
int profile_gap;
bool profile_showing_source;
octa implied_loc;                                                    /*  52 */
bool profile_started;
char *special_name[32] = { "rB", "rD", "rE", "rH", "rJ", "rM", "rR", "rBB",     /*  56 */
  "rC", "rN", "rO", "rS", "rI", "rT", "rTT", "rK", "rQ", "rU", "rV", "rG", "rL",
  "rA", "rF", "rP", "rW", "rX", "rY", "rZ", "rWW", "rXX", "rYY", "rZZ"
};
octa w, x, y, z, a, b, ma, mb;                                       /*  61 */
octa *x_ptr;
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
bool interacting;
bool interact_after_break;
bool tripping;
bool good;
tetra trace_threshold;
op_info info[256] = {                                                /*  65 */
  {"TRAP", 0x0a, 255, 0, 5, "%r"},                                   /*  66 */
  {"FCMP", 0x2a, 0, 0, 1, "%l = %.y cmp %.z = %x"},
  {"FUN", 0x2a, 0, 0, 1, "%l = [%.y(||)%.z] = %x"},
  {"FEQL", 0x2a, 0, 0, 1, "%l = [%.y(==)%.z] = %x"},
  {"FADD", 0x2a, 0, 0, 4, "%l = %.y %(+%) %.z = %.x"},
  {"FIX", 0x26, 0, 0, 4, "%l = %(fix%) %.z = %x"},
  {"FSUB", 0x2a, 0, 0, 4, "%l = %.y %(-%) %.z = %.x"},
  {"FIXU", 0x26, 0, 0, 4, "%l = %(fix%) %.z = %#x"},
  {"FLOT", 0x26, 0, 0, 4, "%l = %(flot%) %z = %.x"},
  {"FLOTI", 0x25, 0, 0, 4, "%l = %(flot%) %z = %.x"},
  {"FLOTU", 0x26, 0, 0, 4, "%l = %(flot%) %#z = %.x"},
  {"FLOTUI", 0x25, 0, 0, 4, "%l = %(flot%) %z = %.x"},
  {"SFLOT", 0x26, 0, 0, 4, "%l = %(sflot%) %z = %.x"},
  {"SFLOTI", 0x25, 0, 0, 4, "%l = %(sflot%) %z = %.x"},
  {"SFLOTU", 0x26, 0, 0, 4, "%l = %(sflot%) %#z = %.x"},
  {"SFLOTUI", 0x25, 0, 0, 4, "%l = %(sflot%) %z = %.x"},
  {"FMUL", 0x2a, 0, 0, 4, "%l = %.y %(*%) %.z = %.x"},
  {"FCMPE", 0x2a, rE, 0, 4, "%l = %.y cmp %.z (%.b)) = %x"},
  {"FUNE", 0x2a, rE, 0, 1, "%l = [%.y(||)%.z (%.b)] = %x"},
  {"FEQLE", 0x2a, rE, 0, 4, "%l = [%.y(==)%.z (%.b)] = %x"},
  {"FDIV", 0x2a, 0, 0, 40, "%l = %.y %(/%) %.z = %.x"},
  {"FSQRT", 0x26, 0, 0, 40, "%l = %(sqrt%) %.z = %.x"},
  {"FREM", 0x2a, 0, 0, 4, "%l = %.y %(rem%) %.z = %.x"},
  {"FINT", 0x26, 0, 0, 4, "%l = %(int%) %.z = %.x"},
  {"MUL", 0x2a, 0, 0, 10, "%l = %y * %z = %x"},
  {"MULI", 0x29, 0, 0, 10, "%l = %y * %z = %x"},
  {"MULU", 0x2a, 0, 0, 10, "%l = %#y * %#z = %#x, rH=%#a"},
  {"MULUI", 0x29, 0, 0, 10, "%l = %#y * %z = %#x, rH=%#a"},
  {"DIV", 0x2a, 0, 0, 60, "%l = %y / %z = %x, rR=%a"},
  {"DIVI", 0x29, 0, 0, 60, "%l = %y / %z = %x, rR=%a"},
  {"DIVU", 0x2a, rD, 0, 60, "%l = %#b%0y / %#z = %#x, rR=%#a"},
  {"DIVUI", 0x29, rD, 0, 60, "%l = %#b%0y / %z = %#x, rR=%#a"},
  {"ADD", 0x2a, 0, 0, 1, "%l = %y + %z = %x"},
  {"ADDI", 0x29, 0, 0, 1, "%l = %y + %z = %x"},
  {"ADDU", 0x2a, 0, 0, 1, "%l = %#y + %#z = %#x"},
  {"ADDUI", 0x29, 0, 0, 1, "%l = %#y + %z = %#x"},
  {"SUB", 0x2a, 0, 0, 1, "%l = %y - %z = %x"},
  {"SUBI", 0x29, 0, 0, 1, "%l = %y - %z = %x"},
  {"SUBU", 0x2a, 0, 0, 1, "%l = %#y - %#z = %#x"},
  {"SUBUI", 0x29, 0, 0, 1, "%l = %#y - %z = %#x"},
  {"2ADDU", 0x2a, 0, 0, 1, "%l = %#y <<1+ %#z = %#x"},
  {"2ADDUI", 0x29, 0, 0, 1, "%l = %#y <<1+ %z = %#x"},
  {"4ADDU", 0x2a, 0, 0, 1, "%l = %#y <<2+ %#z = %#x"},
  {"4ADDUI", 0x29, 0, 0, 1, "%l = %#y <<2+ %z = %#x"},
  {"8ADDU", 0x2a, 0, 0, 1, "%l = %#y <<3+ %#z = %#x"},
  {"8ADDUI", 0x29, 0, 0, 1, "%l = %#y <<3+ %z = %#x"},
  {"16ADDU", 0x2a, 0, 0, 1, "%l = %#y <<4+ %#z = %#x"},
  {"16ADDUI", 0x29, 0, 0, 1, "%l = %#y <<4+ %z = %#x"},
  {"CMP", 0x2a, 0, 0, 1, "%l = %y cmp %z = %x"},
  {"CMPI", 0x29, 0, 0, 1, "%l = %y cmp %z = %x"},
  {"CMPU", 0x2a, 0, 0, 1, "%l = %#y cmp %#z = %x"},
  {"CMPUI", 0x29, 0, 0, 1, "%l = %#y cmp %z = %x"},
  {"NEG", 0x26, 0, 0, 1, "%l = %y - %z = %x"},
  {"NEGI", 0x25, 0, 0, 1, "%l = %y - %z = %x"},
  {"NEGU", 0x26, 0, 0, 1, "%l = %y - %#z = %#x"},
  {"NEGUI", 0x25, 0, 0, 1, "%l = %y - %z = %#x"},
  {"SL", 0x2a, 0, 0, 1, "%l = %y << %#z = %x"},
  {"SLI", 0x29, 0, 0, 1, "%l = %y << %z = %x"},
  {"SLU", 0x2a, 0, 0, 1, "%l = %#y << %#z = %#x"},
  {"SLUI", 0x29, 0, 0, 1, "%l = %#y << %z = %#x"},
  {"SR", 0x2a, 0, 0, 1, "%l = %y >> %#z = %x"},
  {"SRI", 0x29, 0, 0, 1, "%l = %y >> %z = %x"},
  {"SRU", 0x2a, 0, 0, 1, "%l = %#y >> %#z = %#x"},
  {"SRUI", 0x29, 0, 0, 1, "%l = %#y >> %z = %#x"}
  ,
  {"BN", 0x50, 0, 0, 1, "%b<0? %t%g"},                               /*  67 */
  {"BNB", 0x50, 0, 0, 1, "%b<0? %t%g"},
  {"BZ", 0x50, 0, 0, 1, "%b==0? %t%g"},
  {"BZB", 0x50, 0, 0, 1, "%b==0? %t%g"},
  {"BP", 0x50, 0, 0, 1, "%b>0? %t%g"},
  {"BPB", 0x50, 0, 0, 1, "%b>0? %t%g"},
  {"BOD", 0x50, 0, 0, 1, "%b odd? %t%g"},
  {"BODB", 0x50, 0, 0, 1, "%b odd? %t%g"},
  {"BNN", 0x50, 0, 0, 1, "%b>=0? %t%g"},
  {"BNNB", 0x50, 0, 0, 1, "%b>=0? %t%g"},
  {"BNZ", 0x50, 0, 0, 1, "%b!=0? %t%g"},
  {"BNZB", 0x50, 0, 0, 1, "%b!=0? %t%g"},
  {"BNP", 0x50, 0, 0, 1, "%b<=0? %t%g"},
  {"BNPB", 0x50, 0, 0, 1, "%b<=0? %t%g"},
  {"BEV", 0x50, 0, 0, 1, "%b even? %t%g"},
  {"BEVB", 0x50, 0, 0, 1, "%b even? %t%g"},
  {"PBN", 0x50, 0, 0, 1, "%b<0? %t%g"},
  {"PBNB", 0x50, 0, 0, 1, "%b<0? %t%g"},
  {"PBZ", 0x50, 0, 0, 1, "%b==0? %t%g"},
  {"PBZB", 0x50, 0, 0, 1, "%b==0? %t%g"},
  {"PBP", 0x50, 0, 0, 1, "%b>0? %t%g"},
  {"PBPB", 0x50, 0, 0, 1, "%b>0? %t%g"},
  {"PBOD", 0x50, 0, 0, 1, "%b odd? %t%g"},
  {"PBODB", 0x50, 0, 0, 1, "%b odd? %t%g"},
  {"PBNN", 0x50, 0, 0, 1, "%b>=0? %t%g"},
  {"PBNNB", 0x50, 0, 0, 1, "%b>=0? %t%g"},
  {"PBNZ", 0x50, 0, 0, 1, "%b!=0? %t%g"},
  {"PBNZB", 0x50, 0, 0, 1, "%b!=0? %t%g"},
  {"PBNP", 0x50, 0, 0, 1, "%b<=0? %t%g"},
  {"PBNPB", 0x50, 0, 0, 1, "%b<=0? %t%g"},
  {"PBEV", 0x50, 0, 0, 1, "%b even? %t%g"},
  {"PBEVB", 0x50, 0, 0, 1, "%b even? %t%g"},
  {"CSN", 0x3a, 0, 0, 1, "%l = %y<0? %z: %b = %x"},
  {"CSNI", 0x39, 0, 0, 1, "%l = %y<0? %z: %b = %x"},
  {"CSZ", 0x3a, 0, 0, 1, "%l = %y==0? %z: %b = %x"},
  {"CSZI", 0x39, 0, 0, 1, "%l = %y==0? %z: %b = %x"},
  {"CSP", 0x3a, 0, 0, 1, "%l = %y>0? %z: %b = %x"},
  {"CSPI", 0x39, 0, 0, 1, "%l = %y>0? %z: %b = %x"},
  {"CSOD", 0x3a, 0, 0, 1, "%l = %y odd? %z: %b = %x"},
  {"CSODI", 0x39, 0, 0, 1, "%l = %y odd? %z: %b = %x"},
  {"CSNN", 0x3a, 0, 0, 1, "%l = %y>=0? %z: %b = %x"},
  {"CSNNI", 0x39, 0, 0, 1, "%l = %y>=0? %z: %b = %x"},
  {"CSNZ", 0x3a, 0, 0, 1, "%l = %y!=0? %z: %b = %x"},
  {"CSNZI", 0x39, 0, 0, 1, "%l = %y!=0? %z: %b = %x"},
  {"CSNP", 0x3a, 0, 0, 1, "%l = %y<=0? %z: %b = %x"},
  {"CSNPI", 0x39, 0, 0, 1, "%l = %y<=0? %z: %b = %x"},
  {"CSEV", 0x3a, 0, 0, 1, "%l = %y even? %z: %b = %x"},
  {"CSEVI", 0x39, 0, 0, 1, "%l = %y even? %z: %b = %x"},
  {"ZSN", 0x2a, 0, 0, 1, "%l = %y<0? %z: 0 = %x"},
  {"ZSNI", 0x29, 0, 0, 1, "%l = %y<0? %z: 0 = %x"},
  {"ZSZ", 0x2a, 0, 0, 1, "%l = %y==0? %z: 0 = %x"},
  {"ZSZI", 0x29, 0, 0, 1, "%l = %y==0? %z: 0 = %x"},
  {"ZSP", 0x2a, 0, 0, 1, "%l = %y>0? %z: 0 = %x"},
  {"ZSPI", 0x29, 0, 0, 1, "%l = %y>0? %z: 0 = %x"},
  {"ZSOD", 0x2a, 0, 0, 1, "%l = %y odd? %z: 0 = %x"},
  {"ZSODI", 0x29, 0, 0, 1, "%l = %y odd? %z: 0 = %x"},
  {"ZSNN", 0x2a, 0, 0, 1, "%l = %y>=0? %z: 0 = %x"},
  {"ZSNNI", 0x29, 0, 0, 1, "%l = %y>=0? %z: 0 = %x"},
  {"ZSNZ", 0x2a, 0, 0, 1, "%l = %y!=0? %z: 0 = %x"},
  {"ZSNZI", 0x29, 0, 0, 1, "%l = %y!=0? %z: 0 = %x"},
  {"ZSNP", 0x2a, 0, 0, 1, "%l = %y<=0? %z: 0 = %x"},
  {"ZSNPI", 0x29, 0, 0, 1, "%l = %y<=0? %z: 0 = %x"},
  {"ZSEV", 0x2a, 0, 0, 1, "%l = %y even? %z: 0 = %x"},
  {"ZSEVI", 0x29, 0, 0, 1, "%l = %y even? %z: 0 = %x"}
  ,
  {"LDB", 0x2a, 0, 1, 1, "%l = M1[%#y+%#z] = %x"},                   /*  68 */
  {"LDBI", 0x29, 0, 1, 1, "%l = M1[%#y%?+] = %x"},
  {"LDBU", 0x2a, 0, 1, 1, "%l = M1[%#y+%#z] = %#x"},
  {"LDBUI", 0x29, 0, 1, 1, "%l = M1[%#y%?+] = %#x"},
  {"LDW", 0x2a, 0, 1, 1, "%l = M2[%#y+%#z] = %x"},
  {"LDWI", 0x29, 0, 1, 1, "%l = M2[%#y%?+] = %x"},
  {"LDWU", 0x2a, 0, 1, 1, "%l = M2[%#y+%#z] = %#x"},
  {"LDWUI", 0x29, 0, 1, 1, "%l = M2[%#y%?+] = %#x"},
  {"LDT", 0x2a, 0, 1, 1, "%l = M4[%#y+%#z] = %x"},
  {"LDTI", 0x29, 0, 1, 1, "%l = M4[%#y%?+] = %x"},
  {"LDTU", 0x2a, 0, 1, 1, "%l = M4[%#y+%#z] = %#x"},
  {"LDTUI", 0x29, 0, 1, 1, "%l = M4[%#y%?+] = %#x"},
  {"LDO", 0x2a, 0, 1, 1, "%l = M8[%#y+%#z] = %x"},
  {"LDOI", 0x29, 0, 1, 1, "%l = M8[%#y%?+] = %x"},
  {"LDOU", 0x2a, 0, 1, 1, "%l = M8[%#y+%#z] = %#x"},
  {"LDOUI", 0x29, 0, 1, 1, "%l = M8[%#y%?+] = %#x"},
  {"LDSF", 0x2a, 0, 1, 1, "%l = (M4[%#y+%#z]) = %.x"},
  {"LDSFI", 0x29, 0, 1, 1, "%l = (M4[%#y%?+]) = %.x"},
  {"LDHT", 0x2a, 0, 1, 1, "%l = M4[%#y+%#z]<<32 = %#x"},
  {"LDHTI", 0x29, 0, 1, 1, "%l = M4[%#y%?+]<<32 = %#x"},
  {"CSWAP", 0x3a, 0, 2, 2, "%l = [M8[%#y+%#z]==%a] = %x, %r"},
  {"CSWAPI", 0x39, 0, 2, 2, "%l = [M8[%#y%?+]==%a] = %x, %r"},
  {"LDUNC", 0x2a, 0, 1, 1, "%l = M8[%#y+%#z] = %#x"},
  {"LDUNCI", 0x29, 0, 1, 1, "%l = M8[%#y%?+] = %#x"},
  {"LDVTS", 0x2a, 0, 0, 1, ""},
  {"LDVTSI", 0x29, 0, 0, 1, ""},
  {"PRELD", 0x0a, 0, 0, 1, "[%#y+%#z .. %#x]"},
  {"PRELDI", 0x09, 0, 0, 1, "[%#y%?+ .. %#x]"},
  {"PREGO", 0x0a, 0, 0, 1, "[%#y+%#z .. %#x]"},
  {"PREGOI", 0x09, 0, 0, 1, "[%#y%?+ .. %#x]"},
  {"GO", 0x2a, 0, 0, 3, "%l = %#x, -> %#y+%#z"},
  {"GOI", 0x29, 0, 0, 3, "%l = %#x, -> %#y%?+"},
  {"STB", 0x1a, 0, 1, 1, "M1[%#y+%#z] = %b, M8[%#w]=%#a"},
  {"STBI", 0x19, 0, 1, 1, "M1[%#y%?+] = %b, M8[%#w]=%#a"},
  {"STBU", 0x1a, 0, 1, 1, "M1[%#y+%#z] = %#b, M8[%#w]=%#a"},
  {"STBUI", 0x19, 0, 1, 1, "M1[%#y%?+] = %#b, M8[%#w]=%#a"},
  {"STW", 0x1a, 0, 1, 1, "M2[%#y+%#z] = %b, M8[%#w]=%#a"},
  {"STWI", 0x19, 0, 1, 1, "M2[%#y%?+] = %b, M8[%#w]=%#a"},
  {"STWU", 0x1a, 0, 1, 1, "M2[%#y+%#z] = %#b, M8[%#w]=%#a"},
  {"STWUI", 0x19, 0, 1, 1, "M2[%#y%?+] = %#b, M8[%#w]=%#a"},
  {"STT", 0x1a, 0, 1, 1, "M4[%#y+%#z] = %b, M8[%#w]=%#a"},
  {"STTI", 0x19, 0, 1, 1, "M4[%#y%?+] = %b, M8[%#w]=%#a"},
  {"STTU", 0x1a, 0, 1, 1, "M4[%#y+%#z] = %#b, M8[%#w]=%#a"},
  {"STTUI", 0x19, 0, 1, 1, "M4[%#y%?+] = %#b, M8[%#w]=%#a"},
  {"STO", 0x1a, 0, 1, 1, "M8[%#y+%#z] = %b"},
  {"STOI", 0x19, 0, 1, 1, "M8[%#y%?+] = %b"},
  {"STOU", 0x1a, 0, 1, 1, "M8[%#y+%#z] = %#b"},
  {"STOUI", 0x19, 0, 1, 1, "M8[%#y%?+] = %#b"},
  {"STSF", 0x1a, 0, 1, 1, "%(M4[%#y+%#z]%) = %.b, M8[%#w]=%#a"},
  {"STSFI", 0x19, 0, 1, 1, "%(M4[%#y%?+]%) = %.b, M8[%#w]=%#a"},
  {"STHT", 0x1a, 0, 1, 1, "M4[%#y+%#z] = %#b>>32, M8[%#w]=%#a"},
  {"STHTI", 0x19, 0, 1, 1, "M4[%#y%?+] = %#b>>32, M8[%#w]=%#a"},
  {"STCO", 0x0a, 0, 1, 1, "M8[%#y+%#z] = %b"},
  {"STCOI", 0x09, 0, 1, 1, "M8[%#y%?+] = %b"},
  {"STUNC", 0x1a, 0, 1, 1, "M8[%#y+%#z] = %#b"},
  {"STUNCI", 0x19, 0, 1, 1, "M8[%#y%?+] = %#b"},
  {"SYNCD", 0x0a, 0, 0, 1, "[%#y+%#z .. %#x]"},
  {"SYNCDI", 0x09, 0, 0, 1, "[%#y%?+ .. %#x]"},
  {"PREST", 0x0a, 0, 0, 1, "[%#y+%#z .. %#x]"},
  {"PRESTI", 0x09, 0, 0, 1, "[%#y%?+ .. %#x]"},
  {"SYNCID", 0x0a, 0, 0, 1, "[%#y+%#z .. %#x]"},
  {"SYNCIDI", 0x09, 0, 0, 1, "[%#y%?+ .. %#x]"},
  {"PUSHGO", 0xaa, 0, 0, 3, "%lrO=%#b, rL=%a, rJ=%#x, -> %#y+%#z"},
  {"PUSHGOI", 0xa9, 0, 0, 3, "%lrO=%#b, rL=%a, rJ=%#x, -> %#y%?+"}
  ,
  {"OR", 0x2a, 0, 0, 1, "%l = %#y | %#z = %#x"},                     /*  69 */
  {"ORI", 0x29, 0, 0, 1, "%l = %#y | %z = %#x"},
  {"ORN", 0x2a, 0, 0, 1, "%l = %#y |~ %#z = %#x"},
  {"ORNI", 0x29, 0, 0, 1, "%l = %#y |~ %z = %#x"},
  {"NOR", 0x2a, 0, 0, 1, "%l = %#y ~| %#z = %#x"},
  {"NORI", 0x29, 0, 0, 1, "%l = %#y ~| %z = %#x"},
  {"XOR", 0x2a, 0, 0, 1, "%l = %#y ^ %#z = %#x"},
  {"XORI", 0x29, 0, 0, 1, "%l = %#y ^ %z = %#x"},
  {"AND", 0x2a, 0, 0, 1, "%l = %#y & %#z = %#x"},
  {"ANDI", 0x29, 0, 0, 1, "%l = %#y & %z = %#x"},
  {"ANDN", 0x2a, 0, 0, 1, "%l = %#y \\ %#z = %#x"},
  {"ANDNI", 0x29, 0, 0, 1, "%l = %#y \\ %z = %#x"},
  {"NAND", 0x2a, 0, 0, 1, "%l = %#y ~& %#z = %#x"},
  {"NANDI", 0x29, 0, 0, 1, "%l = %#y ~& %z = %#x"},
  {"NXOR", 0x2a, 0, 0, 1, "%l = %#y ~^ %#z = %#x"},
  {"NXORI", 0x29, 0, 0, 1, "%l = %#y ~^ %z = %#x"},
  {"BDIF", 0x2a, 0, 0, 1, "%l = %#y bdif %#z = %#x"},
  {"BDIFI", 0x29, 0, 0, 1, "%l = %#y bdif %z = %#x"},
  {"WDIF", 0x2a, 0, 0, 1, "%l = %#y wdif %#z = %#x"},
  {"WDIFI", 0x29, 0, 0, 1, "%l = %#y wdif %z = %#x"},
  {"TDIF", 0x2a, 0, 0, 1, "%l = %#y tdif %#z = %#x"},
  {"TDIFI", 0x29, 0, 0, 1, "%l = %#y tdif %z = %#x"},
  {"ODIF", 0x2a, 0, 0, 1, "%l = %#y odif %#z = %#x"},
  {"ODIFI", 0x29, 0, 0, 1, "%l = %#y odif %z = %#x"},
  {"MUX", 0x2a, rM, 0, 1, "%l = %#b? %#y: %#z = %#x"},
  {"MUXI", 0x29, rM, 0, 1, "%l = %#b? %#y: %z = %#x"},
  {"SADD", 0x2a, 0, 0, 1, "%l = nu(%#y\\%#z) = %x"},
  {"SADDI", 0x29, 0, 0, 1, "%l = nu(%#y%?\\) = %x"},
  {"MOR", 0x2a, 0, 0, 1, "%l = %#y mor %#z = %#x"},
  {"MORI", 0x29, 0, 0, 1, "%l = %#y mor %z = %#x"},
  {"MXOR", 0x2a, 0, 0, 1, "%l = %#y mxor %#z = %#x"},
  {"MXORI", 0x29, 0, 0, 1, "%l = %#y mxor %z = %#x"},
  {"SETH", 0x20, 0, 0, 1, "%l = %#z"},
  {"SETMH", 0x20, 0, 0, 1, "%l = %#z"},
  {"SETML", 0x20, 0, 0, 1, "%l = %#z"},
  {"SETL", 0x20, 0, 0, 1, "%l = %#z"},
  {"INCH", 0x30, 0, 0, 1, "%l = %#y + %#z = %#x"},
  {"INCMH", 0x30, 0, 0, 1, "%l = %#y + %#z = %#x"},
  {"INCML", 0x30, 0, 0, 1, "%l = %#y + %#z = %#x"},
  {"INCL", 0x30, 0, 0, 1, "%l = %#y + %#z = %#x"},
  {"ORH", 0x30, 0, 0, 1, "%l = %#y | %#z = %#x"},
  {"ORMH", 0x30, 0, 0, 1, "%l = %#y | %#z = %#x"},
  {"ORML", 0x30, 0, 0, 1, "%l = %#y | %#z = %#x"},
  {"ORL", 0x30, 0, 0, 1, "%l = %#y | %#z = %#x"},
  {"ANDNH", 0x30, 0, 0, 1, "%l = %#y \\ %#z = %#x"},
  {"ANDNMH", 0x30, 0, 0, 1, "%l = %#y \\ %#z = %#x"},
  {"ANDNML", 0x30, 0, 0, 1, "%l = %#y \\ %#z = %#x"},
  {"ANDNL", 0x30, 0, 0, 1, "%l = %#y \\ %#z = %#x"},
  {"JMP", 0x40, 0, 0, 1, "-> %#z"},
  {"JMPB", 0x40, 0, 0, 1, "-> %#z"},
  {"PUSHJ", 0xe0, 0, 0, 1, "%lrO=%#b, rL=%a, rJ=%#x, -> %#z"},
  {"PUSHJB", 0xe0, 0, 0, 1, "%lrO=%#b, rL=%a, rJ=%#x, -> %#z"},
  {"GETA", 0x60, 0, 0, 1, "%l = %#z"},
  {"GETAB", 0x60, 0, 0, 1, "%l = %#z"},
  {"PUT", 0x02, 0, 0, 1, "%s = %r"},
  {"PUTI", 0x01, 0, 0, 1, "%s = %r"},
  {"POP", 0x80, rJ, 0, 3, "%lrL=%a, rO=%#b, -> %#y%?+"},
  {"RESUME", 0x00, 0, 0, 5, "{%#b} -> %#z"},
  {"SAVE", 0x20, 0, 20, 1, "%l = %#x"},
  {"UNSAVE", 0x82, 0, 20, 1, "%#z: rG=%x, ..., rL=%a"},
  {"SYNC", 0x01, 0, 0, 1, ""},
  {"SWYM", 0x00, 0, 0, 1, ""},
  {"GET", 0x20, 0, 0, 1, "%l = %s = %#x"},
  {"TRIP", 0x0a, 255, 0, 5, "rW=%#w, rX=%#x, rY=%#y, rZ=%#z, rB=%#b, g[255]=%#a"}
};
octa g[256];                                                         /*  76 */
octa *l;
int lring_size;
int lring_mask;
int S;
char arg_count[] = { 1, 3, 1, 3, 3, 3, 3, 2, 2, 2, 1 };              /* 110 */
char *trap_format[] = {
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
  "$255 = Ftell(%!z) = %x"
};
extern void mmix_io_init ARGS ((void));                              /* 113 */
extern octa mmix_fopen ARGS ((unsigned char, octa, octa));
extern octa mmix_fclose ARGS ((unsigned char));
extern octa mmix_fread ARGS ((unsigned char, octa, octa));
extern octa mmix_fgets ARGS ((unsigned char, octa, octa));
extern octa mmix_fgetws ARGS ((unsigned char, octa, octa));
extern octa mmix_fwrite ARGS ((unsigned char, octa, octa));
extern octa mmix_fputs ARGS ((unsigned char, octa));
extern octa mmix_fputws ARGS ((unsigned char, octa));
extern octa mmix_fseek ARGS ((unsigned char, octa));
extern octa mmix_ftell ARGS ((unsigned char));
extern void print_trip_warning ARGS ((int, octa));
extern void mmix_fake_stdin ARGS ((FILE *));
char stdin_buf[256];                                                 /* 121 */
char *stdin_buf_start;
char *stdin_buf_end;
bool showing_stats;                                                  /* 129 */
bool just_traced;
char left_paren[] = { 0, '[', '^', '_', '(' };                       /* 139 */
char right_paren[] = { 0, ']', '^', '_', ')' };
char switchable_string[48];
char lhs[32];
int good_guesses, bad_guesses;
char *myself;                                                        /* 144 */
char **cur_arg;
bool interrupt;
bool profiling;
FILE *fake_stdin;
FILE *dump_file;
char *usage_help[] = {
  " with these options: (<n>=decimal number, <x>=hex number)\n",
  "-t<n> trace each instruction the first n times\n",
  "-e<x> trace each instruction with an exception matching x\n",
  "-r    trace hidden details of the register stack\n",
  "-l<n> list source lines when tracing, filling gaps <= n\n",
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
  ""
};
char *interactive_help[] = {
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
  "B         show all current breakpoints and tracepoints\n",
  "i<file>   insert commands from file\n",
  "-<option> change a tracing/listing/profile option\n",
  "-?        show the tracing/listing/profile options  \n",
  ""
};
char command_buf[command_buf_size];                                  /* 151 */
FILE *incl_file;
char cur_disp_mode = 'l';
char cur_disp_type = '!';
bool cur_disp_set;
octa cur_disp_addr;
octa cur_seg;
char spec_reg_code[] = { rA, rB, rC, rD, rE, rF, rG, rH, rI, rJ, rK, rL, rM,
  rN, rO, rP, rQ, rR, rS, rT, rU, rV, rW, rX, rY, rZ
};
char spec_regg_code[] = { 0, rBB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, rTT, 0, 0, rWW, rXX, rYY, rZZ
};
tetra vram_addr;                                                     /* 170 */
mem_tetra vram_tetra[2];                                             /* 171 */
VGA vga;                                                             /* 179 */
CommandPort cp;                                                      /* 213 */
void print_hex ARGS ((octa));                                        /*  12 */
void
print_hex (o)
     octa o;
{
  if (o.h)
    printf ("%x%08x", o.h, o.l);
  else
    printf ("%x", o.l);
}
extern octa zero_octa;                                               /*  13 */
extern octa neg_one;
extern octa aux, val;
extern bool overflow;
extern int exceptions;
extern int cur_round;
extern char *next_char;
extern octa oplus ARGS ((octa y, octa z));
extern octa ominus ARGS ((octa y, octa z));
extern octa incr ARGS ((octa y, int delta));
extern octa oand ARGS ((octa y, octa z));
extern octa shift_left ARGS ((octa y, int s));
extern octa shift_right ARGS ((octa y, int s, int uns));
extern octa omult ARGS ((octa y, octa z));
extern octa signed_omult ARGS ((octa y, octa z));
extern octa odiv ARGS ((octa x, octa y, octa z));
extern octa signed_odiv ARGS ((octa y, octa z));
extern int count_bits ARGS ((tetra z));
extern tetra byte_diff ARGS ((tetra y, tetra z));
extern tetra wyde_diff ARGS ((tetra y, tetra z));
extern octa bool_mult ARGS ((octa y, octa z, bool xor));
extern octa load_sf ARGS ((tetra z));
extern tetra store_sf ARGS ((octa x));
extern octa fplus ARGS ((octa y, octa z));
extern octa fmult ARGS ((octa y, octa z));
extern octa fdivide ARGS ((octa y, octa z));
extern octa froot ARGS ((octa, int));
extern octa fremstep ARGS ((octa y, octa z, int delta));
extern octa fintegerize ARGS ((octa z, int mode));
extern int fcomp ARGS ((octa y, octa z));
extern int fepscomp ARGS ((octa y, octa z, octa eps, int sim));
extern octa floatit ARGS ((octa z, int mode, int unsgnd, int shrt));
extern octa fixit ARGS ((octa z, int mode));
extern void print_float ARGS ((octa z));
extern int scan_const ARGS ((char *buf));
void print_int ARGS ((octa));                                        /*  15 */
void
print_int (o)
     octa o;
{
  register tetra hi = o.h, lo = o.l, r, t;
  register int j;
  char dig[20];

  if (lo == 0 && hi == 0)
    printf ("0");
  else {
    if (hi & sign_bit) {
      printf ("-");
      if (lo == 0)
        hi = -hi;
      else
        lo = -lo, hi = ~hi;
    }
    for (j = 0; hi; j++) {
      r = ((hi % 10) << 16) + (lo >> 16);
      hi = hi / 10;
      t = ((r % 10) << 16) + (lo & 0xffff);
      lo = ((r / 10) << 16) + (t / 10);
      dig[j] = t % 10;
    }
    for (; lo; j++) {
      dig[j] = lo % 10;
      lo = lo / 10;
    }
    for (j--; j >= 0; j--)
      printf ("%c", dig[j] + '0');
  }
}
mem_node *new_mem ARGS ((void));                                     /*  17 */
mem_node *
new_mem ()
{
  register mem_node *p;

  p = (mem_node *) calloc (1, sizeof (mem_node));
  if (!p)
    panic ("Can't allocate any more memory");
  p->stamp = priority;
  priority += 0x9e3779b9;
  return p;
}

mem_tetra *mem_find ARGS ((octa));                                   /*  20 */
mem_tetra *
mem_find (addr)
     octa addr;
{
  octa key;
  register int offset;
  register mem_node *p = last_mem;

  if (addr.h == 0x50000000) {
    vram_addr = addr.l;
    return vram_tetra;
  }
  key.h = addr.h;
  key.l = addr.l & 0xfffff800;
  offset = addr.l & 0x7fc;
  if (p->loc.l != key.l || p->loc.h != key.h) {
    register mem_node **q;                                           /*  21 */

    for (p = mem_root; p;) {
      if (key.l == p->loc.l && key.h == p->loc.h)
        goto found;
      if ((key.l < p->loc.l && key.h <= p->loc.h) || key.h < p->loc.h)
        p = p->left;
      else
        p = p->right;
    }
    for (p = mem_root, q = &mem_root; p && p->stamp < priority; p = *q) {
      if ((key.l < p->loc.l && key.h <= p->loc.h) || key.h < p->loc.h)
        q = &p->left;
      else
        q = &p->right;
    }
    *q = new_mem ();
    (*q)->loc = key;
    {                                                                /*  22 */
      register mem_node **l = &(*q)->left, **r = &(*q)->right;

      while (p) {
        if ((key.l < p->loc.l && key.h <= p->loc.h) || key.h < p->loc.h)
          *r = p, r = &p->left, p = *r;
        else
          *l = p, l = &p->right, p = *l;
      }
      *l = *r = NULL;
    }
    ;
    p = *q;
  found:last_mem = p;
  }
  ;
  return &p->dat[offset >> 2];
}
void read_tet ARGS ((void));                                         /*  26 */
void
read_tet ()
{
  if (fread (buf, 1, 4, mmo_file) != 4)
    mmo_err;
  yzbytes = (buf[2] << 8) + buf[3];
  tet = (((buf[0] << 8) + buf[1]) << 16) + yzbytes;
}
byte read_byte ARGS ((void));                                        /*  27 */
byte
read_byte ()
{
  register byte b;

  if (!byte_count)
    read_tet ();
  b = buf[byte_count];
  byte_count = (byte_count + 1) & 3;
  return b;
}
void make_map ARGS ((void));                                         /*  42 */
void
make_map ()
{
  long map[65536];
  register int k, l;
  register long *p;

  {                                                                  /*  44 */
    struct stat stat_buf;

    if (stat (file_info[cur_file].name, &stat_buf) >= 0)
      if ((tetra) stat_buf.st_mtime > obj_time)
        fprintf (stderr,
                 "Warning: File %s was modified; it may not match the program!\n",
                 file_info[cur_file].name);
  }
  ;
  for (l = 1; l < 65536 && !feof (src_file); l++) {
    map[l] = ftell (src_file);
  loop:if (!fgets (buffer, buf_size, src_file))
      break;
    if (buffer[strlen (buffer) - 1] != '\n')
      goto loop;
  }
  file_info[cur_file].line_count = l;
  file_info[cur_file].map = p = (long *) calloc (l, sizeof (long));
  if (!p)
    panic ("No room for a source-line map");
  for (k = 1; k < l; k++)
    p[k] = map[k];
}
void print_line ARGS ((int));                                        /*  45 */
void
print_line (k)
     int k;
{
  char buf[11];

  if (k >= file_info[cur_file].line_count)
    return;
  if (fseek (src_file, file_info[cur_file].map[k], SEEK_SET) != 0)
    return;
  if (!fgets (buffer, buf_size, src_file))
    return;
  sprintf (buf, "%d:    ", k);
  printf ("line %.6s %s", buf, buffer);
  if (buffer[strlen (buffer) - 1] != '\n')
    printf ("\n");
  line_shown = true;
}
void show_line ARGS ((void));                                        /*  47 */
void
show_line ()
{
  register int k;

  if (shown_file != cur_file) {                                      /*  49 */
    if (!src_file)
      src_file = fopen (file_info[cur_file].name, "r");
    else
      freopen (file_info[cur_file].name, "r", src_file);
    if (!src_file) {
      fprintf (stderr,
               "Warning: I can't open file %s; source listing omitted.\n",
               file_info[cur_file].name);
      showing_source = false;
      return;
    }
    printf ("\"%s\"\n", file_info[cur_file].name);
    shown_file = cur_file;
    shown_line = 0;
    if (!file_info[cur_file].map)
      make_map ();
  }
  else if (shown_line == cur_line)
    return;
  if (cur_line > shown_line + gap + 1 || cur_line < shown_line) {
    if (shown_line > 0)
      if (cur_line < shown_line)
        printf ("--------\n");
      else
        printf ("     ...\n");
    print_line (cur_line);
  }
  else
    for (k = shown_line + 1; k <= cur_line; k++)
      print_line (k);
  shown_line = cur_line;
}
void print_freqs ARGS ((mem_node *));                                /*  50 */
void
print_freqs (p)
     mem_node *p;
{
  register int j;
  octa cur_loc;

  if (p->left)
    print_freqs (p->left);
  for (j = 0; j < 512; j++)
    if (p->dat[j].freq) {                                            /*  51 */
      cur_loc = incr (p->loc, 4 * j);
      if (showing_source && p->dat[j].line_no) {
        cur_file = p->dat[j].file_no, cur_line = p->dat[j].line_no;
        line_shown = false;
        show_line ();
        if (line_shown)
          goto loc_implied;
      }
      if (cur_loc.l != implied_loc.l || cur_loc.h != implied_loc.h)
        if (profile_started)
          printf ("         0.        ...\n");
    loc_implied:printf ("%10d. %08x%08x: %08x (%s)\n",
              p->dat[j].freq, cur_loc.h, cur_loc.l, p->dat[j].tet,
              info[p->dat[j].tet >> 24].name);
      implied_loc = incr (cur_loc, 4);
      profile_started = true;
    }
  ;
  if (p->right)
    print_freqs (p->right);
}
void stack_store ARGS ((void));                                      /*  82 */
void
stack_store ()
{
  register mem_tetra *ll = mem_find (g[rS]);
  register int k = S & lring_mask;

  ll->tet = l[k].h;
  test_store_bkpt (ll);
  (ll + 1)->tet = l[k].l;
  test_store_bkpt (ll + 1);
  if (stack_tracing) {
    tracing = true;
    if (cur_line)
      show_line ();
    printf ("             M8[#%08x%08x]=l[%d]=#%08x%08x, rS+=8\n",
            g[rS].h, g[rS].l, k, l[k].h, l[k].l);
  }
  g[rS] = incr (g[rS], 8), S++;
}
void stack_load ARGS ((void));                                       /*  83 */
void
stack_load ()
{
  register mem_tetra *ll;
  register int k;

  S--, g[rS] = incr (g[rS], -8);
  ll = mem_find (g[rS]);
  k = S & lring_mask;
  l[k].h = ll->tet;
  test_load_bkpt (ll);
  l[k].l = (ll + 1)->tet;
  test_load_bkpt (ll + 1);
  if (stack_tracing) {
    tracing = true;
    if (cur_line)
      show_line ();
    printf ("             rS-=8, l[%d]=M8[#%08x%08x]=#%08x%08x\n",
            k, g[rS].h, g[rS].l, l[k].h, l[k].l);
  }
}
int register_truth ARGS ((octa, mmix_opcode));                       /*  91 */
int
register_truth (o, op)
     octa o;
     mmix_opcode op;
{
  register int b;

  switch ((op >> 1) & 0x3) {
  case 0:
    b = o.h >> 31;
    break;
  case 1:
    b = (o.h == 0 && o.l == 0);
    break;
  case 2:
    b = (o.h < sign_bit && (o.h || o.l));
    break;
  case 3:
    b = o.l & 0x1;
    break;
  }
  if (op & 0x8)
    return b ^ 1;
  else
    return b;
}
int mmgetchars ARGS ((char *, int, octa, int));                      /* 114 */
int
mmgetchars (buf, size, addr, stop)
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

  for (p = buf, m = 0, a = addr; m < size;) {
    ll = mem_find (a);
    test_load_bkpt (ll);
    x = ll->tet;
    if ((a.l & 0x3) || m > size - 4) {                               /* 115 */
      *p = (x >> (8 * ((~a.l) & 0x3))) & 0xff;
      if (!*p && stop >= 0) {
        if (stop == 0)
          return m;
        if ((a.l & 0x1) && *(p - 1) == '\0')
          return m - 1;
      }
      p++, m++, a = incr (a, 1);
    }
    else {                                                           /* 116 */
      *p = x >> 24;
      if (!*p && (stop == 0 || (stop > 0 && x < 0x10000)))
        return m;
      *(p + 1) = (x >> 16) & 0xff;
      if (!*(p + 1) && stop == 0)
        return m + 1;
      *(p + 2) = (x >> 8) & 0xff;
      if (!*(p + 2) && (stop == 0 || (stop > 0 && (x & 0xffff) == 0)))
        return m + 2;
      *(p + 3) = x & 0xff;
      if (!*(p + 3) && stop == 0)
        return m + 3;
      p += 4, m += 4, a = incr (a, 4);
    }
  }
  return size;
}
void mmputchars ARGS ((unsigned char *, int, octa));                 /* 117 */
void
mmputchars (buf, size, addr)
     unsigned char *buf;
     int size;
     octa addr;
{
  register unsigned char *p;
  register int m;
  register mem_tetra *ll;
  octa a;

  for (p = buf, m = 0, a = addr; m < size;) {
    ll = mem_find (a);
    test_store_bkpt (ll);
    if ((a.l & 0x3) || m > size - 4) {                               /* 118 */
      register int s = 8 * ((~a.l) & 0x3);

      ll->tet ^= (((ll->tet >> s) ^ *p) & 0xff) << s;
      p++, m++, a = incr (a, 1);
    }
    else {                                                           /* 119 */
      ll->tet = (*p << 24) + (*(p + 1) << 16) + (*(p + 2) << 8) + *(p + 3);
      p += 4, m += 4, a = incr (a, 4);
    }
    ;
  }
}
char stdin_chr ARGS ((void));                                        /* 120 */
char
stdin_chr ()
{
  register char *p;

  while (stdin_buf_start == stdin_buf_end) {
    if (interacting) {
      printf ("StdIn> ");
      fflush (stdout);
    }
    if (!fgets (stdin_buf, 256, stdin))
      panic ("End of file on standard input; use the -f option, not <");
    stdin_buf_start = stdin_buf;
    for (p = stdin_buf; p < stdin_buf + 254; p++)
      if (*p == '\n')
        break;
    stdin_buf_end = p + 1;
  }
  return *stdin_buf_start++;
}

fmt_style style;                                                     /* 137 */
char *stream_name[] = { "StdIn", "StdOut", "StdErr" };
void trace_print ARGS ((octa));
void
trace_print (o)
     octa o;
{
  switch (style) {
  case decimal:
    print_int (o);
    return;
  case hex:
    fputc ('#', stdout);
    print_hex (o);
    return;
  case zhex:
    printf ("%08x%08x", o.h, o.l);
    return;
  case floating:
    print_float (o);
    return;
  case handle:
    if (o.h == 0 && o.l < 3)
      printf (stream_name[o.l]);
    else
      print_int (o);
    return;
  }
}
void show_stats ARGS ((bool));                                       /* 140 */
void
show_stats (verbose)
     bool verbose;
{
  octa o;

  printf ("  %d instruction%s, %d mem%s, %d oop%s; %d good guess%s, %d bad\n",
          g[rU].l, g[rU].l == 1 ? "" : "s",
          g[rC].h, g[rC].h == 1 ? "" : "s",
          g[rC].l, g[rC].l == 1 ? "" : "s",
          good_guesses, good_guesses == 1 ? "" : "es", bad_guesses);
  if (!verbose)
    return;
  o = halted ? incr (inst_ptr, -4) : inst_ptr;
  printf ("  (%s at location #%08x%08x)\n",
          halted ? "halted" : "now", o.h, o.l);
}
void scan_option ARGS ((char *, bool));                              /* 143 */
void
scan_option (arg, usage)
     char *arg;
     bool usage;
{
  register int k;

  switch (*arg) {
  case 't':
    if (strlen (arg) > 10)
      trace_threshold = 0xffffffff;
    else if (sscanf (arg + 1, "%d", &trace_threshold) != 1)
      trace_threshold = 0;
    return;
  case 'e':
    if (!*(arg + 1))
      tracing_exceptions = 0xff;
    else if (sscanf (arg + 1, "%x", &tracing_exceptions) != 1)
      tracing_exceptions = 0;
    return;
  case 'r':
    stack_tracing = true;
    return;
  case 's':
    showing_stats = true;
    return;
  case 'l':
    if (!*(arg + 1))
      gap = 3;
    else if (sscanf (arg + 1, "%d", &gap) != 1)
      gap = 0;
    showing_source = true;
    return;
  case 'L':
    if (!*(arg + 1))
      profile_gap = 3;
    else if (sscanf (arg + 1, "%d", &profile_gap) != 1)
      profile_gap = 0;
    profile_showing_source = true;
  case 'P':
    profiling = true;
    return;
  case 'v':
    trace_threshold = 0xffffffff;
    tracing_exceptions = 0xff;
    stack_tracing = true;
    showing_stats = true;
    gap = 10, showing_source = true;
    profile_gap = 10, profile_showing_source = true, profiling = true;
    return;
  case 'q':
    trace_threshold = tracing_exceptions = 0;
    stack_tracing = showing_stats = showing_source = false;
    profiling = profile_showing_source = false;
    return;
  case 'i':
    interacting = true;
    return;
  case 'I':
    interact_after_break = true;
    return;
  case 'b':
    if (sscanf (arg + 1, "%d", &buf_size) != 1)
      buf_size = 0;
    return;
  case 'c':
    if (sscanf (arg + 1, "%d", &lring_size) != 1)
      lring_size = 0;
    return;
  case 'f':
    if (fake_stdin)
      fclose (fake_stdin);                                           /* 145 */
    fake_stdin = fopen (arg + 1, "r");
    if (!fake_stdin)
      fprintf (stderr, "Sorry, I can't open file %s!\n", arg + 1);
    else
      mmix_fake_stdin (fake_stdin);
    ;
    return;
  case 'D':
    dump_file = fopen (arg + 1, "wb");                               /* 146 */
    if (!dump_file)
      fprintf (stderr, "Sorry, I can't open file %s!\n", arg + 1);
    ;
    return;
  default:
    if (usage) {
      fprintf (stderr,
               "Usage: %s <options> progfile command-line-args...\n", myself);
      for (k = 0; usage_help[k][0]; k++)
        fprintf (stderr, usage_help[k]);
      exit (-1);
    }
    else
      for (k = 0; usage_help[k][1] != 'b'; k++)
        printf (usage_help[k]);
    return;
  }
}
void catchint ARGS ((int));                                          /* 148 */
void
catchint (n)
     int n;
{
  interrupt = true;
  signal (SIGINT, catchint);
}
octa scan_hex ARGS ((char *, octa));                                 /* 154 */
octa
scan_hex (s, offset)
     char *s;
     octa offset;
{
  register char *p;
  octa o;

  o = zero_octa;
  for (p = s; isxdigit (*p); p++) {
    o = incr (shift_left (o, 4), *p - '0');
    if (*p >= 'a')
      o = incr (o, '0' - 'a' + 10);
    else if (*p >= 'A')
      o = incr (o, '0' - 'A' + 10);
  }
  next_char = p;
  return oplus (o, offset);
}
void print_string ARGS ((octa));                                     /* 160 */
void
print_string (o)
     octa o;
{
  register int k, state, b;

  for (k = state = 0; k < 8; k++) {
    b = ((k < 4 ? o.h >> (8 * (3 - k)) : o.l >> (8 * (7 - k)))) & 0xff;
    if (b == 0) {
      if (state)
        printf ("%s,0", state > 1 ? "\"" : ""), state = 1;
    }
    else if (b >= ' ' && b <= '~')
      printf ("%s%c", state > 1 ? "" : state == 1 ? ",\"" : "\"", b), state = 2;
    else
      printf ("%s#%x", state > 1 ? "\"," : state == 1 ? "," : "", b), state = 1;
  }
  if (state == 0)
    printf ("0");
  else if (state > 1)
    printf ("\"");
}
void show_breaks ARGS ((mem_node *));                                /* 162 */
void
show_breaks (p)
     mem_node *p;
{
  register int j;
  octa cur_loc;

  if (p->left)
    show_breaks (p->left);
  for (j = 0; j < 512; j++)
    if (p->dat[j].bkpt) {
      cur_loc = incr (p->loc, 4 * j);
      printf ("  %08x%08x %c%c%c%c\n", cur_loc.h, cur_loc.l,
              p->dat[j].bkpt & trace_bit ? 't' : '-',
              p->dat[j].bkpt & read_bit ? 'r' : '-',
              p->dat[j].bkpt & write_bit ? 'w' : '-',
              p->dat[j].bkpt & exec_bit ? 'x' : '-');
    }
  if (p->right)
    show_breaks (p->right);
}
void dump ARGS ((mem_node *));                                       /* 165 */
void dump_tet ARGS ((tetra));
void
dump (p)
     mem_node *p;
{
  register int j;
  octa cur_loc;

  if (p->left)
    dump (p->left);
  for (j = 0; j < 512; j += 2)
    if (p->dat[j].tet || p->dat[j + 1].tet) {
      cur_loc = incr (p->loc, 4 * j);
      if (cur_loc.l != x.l || cur_loc.h != x.h) {
        if (x.l != 1)
          dump_tet (0), dump_tet (0);
        dump_tet (cur_loc.h);
        dump_tet (cur_loc.l);
        x = cur_loc;
      }
      dump_tet (p->dat[j].tet);
      dump_tet (p->dat[j + 1].tet);
      x = incr (x, 8);
    }
  if (p->right)
    dump (p->right);
}

void
dump_tet (t)                                                         /* 166 */
     tetra t;
{
  fputc (t >> 24, dump_file);
  fputc ((t >> 16) & 0xff, dump_file);
  fputc ((t >> 8) & 0xff, dump_file);
  fputc (t & 0xff, dump_file);
}
ColorChannel mask2channel ARGS ((unsigned long));                    /* 176 */
ColorChannel
mask2channel (mask)
     unsigned long mask;
{
  unsigned long f;
  ColorChannel ch;

  if (mask == 0)
    panic ("mask2channel(0)");
  for (f = 1; !(mask & 1); f <<= 1)
    mask >>= 1;
  ch.cch_factor = f;
  ch.cch_scale = mask + 1;
  while (mask & 1)
    mask >>= 1;
  if (mask != 0)
    panic ("mask2channel(): scattered bits in the mask");
  return ch;
}
void init_monitor ARGS ((int, char *[]));                            /* 198 */
void
init_monitor (argc, argv)
     int argc;
     char *argv[];
{
  int screen_num;                                                    /* 182 */
  Window root_win;
  XVisualInfo vi_temp;                                               /* 184 */
  XVisualInfo *visual_info;
  int visual_count;
  int best_depth;
  int best_match;
  Visual *visual;
  int i;
  Colormap colormap;                                                 /* 186 */
  XSetWindowAttributes attrib;                                       /* 188 */
  XClassHint *class_hints;                                           /* 190 */
  XSizeHints *size_hints;
  XWMHints *wm_hints;
  XTextProperty windowName;
  XGCValues gc_values;                                               /* 196 */

  if (!XInitThreads ())                                              /* 181 */
    panic ("Can't enable thread support for X11");
  vga.display = XOpenDisplay (NULL);
  if (vga.display == NULL)
    panic ("X connection failed");
  screen_num = DefaultScreen (vga.display);
  root_win = RootWindow (vga.display, screen_num);
  vi_temp.screen = screen_num;                                       /* 183 */
  vi_temp.class = TrueColor;
  visual_info = XGetVisualInfo (vga.display, VisualClassMask | VisualScreenMask,
                                &vi_temp, &visual_count);
  if ((visual_info == NULL) || visual_count == 0)
    panic ("No TrueColor visual found");
  best_match = 0;
  best_depth = visual_info[0].depth;
  visual = visual_info[0].visual;
  for (i = 1; i < visual_count; i++) {
    if (visual_info[i].depth > best_depth) {
      best_match = i;
      best_depth = visual_info[i].depth;
      visual = visual_info[i].visual;
    }
  }
  vga.red = mask2channel (visual_info[best_match].red_mask);         /* 180 */
  vga.green = mask2channel (visual_info[best_match].green_mask);
  vga.blue = mask2channel (visual_info[best_match].blue_mask);
  vga.image = XCreateImage (vga.display, visual, best_depth, ZPixmap,   /* 194 */
                            0, NULL, window_size_x, window_size_y, 32, 0);
  if (!vga.image)
    panic ("image allocation error");
  vga.image->data = malloc (vga.image->height * vga.image->bytes_per_line);
  if (!vga.image->data) {
    panic ("image data allocation error");
  }
  else {
    int x, y;
    unsigned long p = RGB2Pixel (0, 0, 0);

    for (y = 0; y < window_size_y; y++)
      for (x = 0; x < window_size_x; x++)
        XPutPixel (vga.image, x, y, p);
  }
  colormap = XCreateColormap (vga.display, root_win, visual, AllocNone);        /* 185 */
  attrib.colormap = colormap;                                        /* 187 */
  attrib.event_mask = ExposureMask;
  attrib.background_pixel = attrib.border_pixel = RGB2Pixel (0, 0, 0);
  vga.win = XCreateWindow (vga.display, root_win,
                           window_pos_x, window_pos_y,
                           window_size_x, window_size_y, 0, best_depth,
                           InputOutput, visual,
                           CWEventMask | CWColormap |
                           CWBackPixel | CWBorderPixel, &attrib);
  if (!(size_hints = XAllocSizeHints ()) ||                          /* 189 */
      !(wm_hints = XAllocWMHints ()) || !(class_hints = XAllocClassHint ()))
    panic ("Hint allocation failed");
  size_hints->flags = PMinSize | PMaxSize;                           /* 191 */
  size_hints->min_width = window_size_x;
  size_hints->min_height = window_size_y;
  size_hints->max_width = window_size_x;
  size_hints->max_height = window_size_y;
  wm_hints->flags = StateHint | InputHint;                           /* 192 */
  wm_hints->input = True;
  wm_hints->initial_state = NormalState;
  class_hints->res_name = "mmix";                                    /* 193 */
  class_hints->res_class = "MMIX";
  if (!XStringListToTextProperty (&class_hints->res_name, 1, &windowName))
    panic ("property allocation failed");
  XSetWMProperties (vga.display, vga.win, &windowName, NULL,
                    vga.argv, vga.argc, size_hints, wm_hints, class_hints);
  vga.gc = XCreateGC (vga.display, vga.win, 0, &gc_values);          /* 195 */
  XMapWindow (vga.display, vga.win);
  XFlush (vga.display);
  vga.expose.type = Expose;                                          /* 197 */
  vga.expose.display = vga.display;
  vga.expose.window = vga.win;
  vga.expose.count = 0;
  pthread_mutex_lock (&cp.lock);
  if (cp.status != 1)
    panic ("cp.status should be one now!");
  cp.status = 0;
  pthread_cond_signal (&cp.out_c);
  pthread_mutex_unlock (&cp.lock);
}

void *server ARGS ((void *));                                        /* 200 */
void *
server (_ignore)
     void *_ignore;
{
  int done;

  init_monitor (vga.argc, vga.argv);
  for (done = 0; !done;) {
    XEvent event;

    XNextEvent (vga.display, &event);
    switch (event.type) {
    case Expose:{                                                   /* 202 */
        Cmd data;
        int cmd;

        switch ((cmd = get_cmd (&data))) {
        case vgaHalt:                                               /* 204 */
          done = 1;
          break;
        case vgaPutPixel:                                           /* 209 */
          do_pixel (data.pp.x, data.pp.y, data.pp.r, data.pp.g, data.pp.b);
          break;
        case vgaNop:                                                /* 211 */
          break;
        default:{
            char buffer[128];

            sprintf (buffer, "server: Unknown command %d\n", cmd);
            panic (buffer);
          }
        }
        XPutImage (vga.display, vga.win, vga.gc, vga.image,
                   event.xexpose.x, event.xexpose.y,
                   event.xexpose.x, event.xexpose.y,
                   event.xexpose.width, event.xexpose.height);
      } break;
    default:
      break;
    }
  }
  XFreeGC (vga.display, vga.gc);                                     /* 199 */
  XUnmapWindow (vga.display, vga.win);
  XDestroyWindow (vga.display, vga.win);
  XDestroyImage (vga.image);
  XCloseDisplay (vga.display);
  return 0;
}
void send_cmd ARGS ((int, Cmd *));                                   /* 214 */
void
send_cmd (cmd, data)
     int cmd;
     Cmd *data;
{
  pthread_mutex_lock (&cp.lock);
  while (cp.status != 0)
    pthread_cond_wait (&cp.out_c, &cp.lock);
  cp.status = 1;
  cp.cmd = cmd;
  cp.data = *data;
  pthread_mutex_unlock (&cp.lock);
  XSendEvent (vga.display, vga.win, False, 0, (XEvent *) & vga.expose);
  XFlush (vga.display);
}
int get_cmd ARGS ((Cmd *));                                          /* 215 */
int
get_cmd (data)
     Cmd *data;
{
  int cmd;

  pthread_mutex_lock (&cp.lock);
  if (cp.status == 0) {
    cmd = vgaNop;
  }
  else {
    cmd = cp.cmd;
    *data = cp.data;
    cp.status = 0;
    pthread_cond_signal (&cp.out_c);
  }
  pthread_mutex_unlock (&cp.lock);
  return cmd;
}
void vram_init ARGS ((int, char *[]));                               /* 216 */
void
vram_init (argc, argv)
     int argc;
     char *argv[];
{
  pthread_attr_t attr;
  pthread_t thread;

  pthread_mutex_init (&cp.lock, NULL);
  pthread_cond_init (&cp.out_c, NULL);
  cp.status = 1;
  cp.cmd = vgaNop;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  vga.argc = argc;
  vga.argv = argv;
  if (pthread_create (&thread, &attr, server, NULL) != 0) {
    perror ("server");
    exit (1);
  }
}
void vram_fini ARGS ((void));                                        /* 217 */
void
vram_fini ()
{
  Cmd cmd;

  vga.expose.x = 0;
  vga.expose.y = 0;
  vga.expose.width = 1;
  vga.expose.height = 1;
  send_cmd (vgaHalt, &cmd);
}
void vram_write ARGS ((tetra, tetra));                               /* 218 */
void
vram_write (addr, val)
     tetra addr;
     tetra val;
{
  int x, y;
  int r, g, b;
  Cmd cmd;

  x = (addr >> 2) % window_size_x;
  y = (addr >> 2) / window_size_x;
  r = 0xff & (val >> 16);
  g = 0xff & (val >> 8);
  b = 0xff & val;
  if ((x < 0) || (x >= window_size_x) || (y < 0) || (y >= window_size_y))
    return;
  cmd.pp.x = x;
  cmd.pp.y = y;
  cmd.pp.r = r;
  cmd.pp.g = g;
  cmd.pp.b = b;
  vga.expose.x = x;
  vga.expose.y = y;
  vga.expose.width = 1;
  vga.expose.height = 1;
  send_cmd (vgaPutPixel, &cmd);
}

int
main (argc, argv)
     int argc;
     char *argv[];
{
  register mmix_opcode op;                                           /*  62 */
  register int xx, yy, zz, yz;
  register tetra f;
  register int i, j, k;
  register mem_tetra *ll;
  register char *p;
  register int G, L, O;                                              /*  75 */

  ;
  mmix_io_init ();
  myself = argv[0];                                                  /* 142 */
  for (cur_arg = argv + 1; *cur_arg && (*cur_arg)[0] == '-'; cur_arg++)
    scan_option (*cur_arg + 1, true);
  if (!*cur_arg)
    scan_option ("?", true);
  argc -= cur_arg - argv;
  ;
  if (shift_left (neg_one, 1).h != 0xffffffff)                       /*  14 */
    panic ("Incorrect implementation of type tetra");
  mem_root = new_mem ();                                             /*  18 */
  mem_root->loc.h = 0x40000000;
  last_mem = mem_root;
  mmo_file = fopen (mmo_file_name, "rb");                            /*  24 */
  if (!mmo_file) {
    register char *alt_name =
      (char *) calloc (strlen (mmo_file_name) + 5, sizeof (char));
    if (!alt_name)
      panic ("Can't allocate file name buffer");
    sprintf (alt_name, "%s.mmo", mmo_file_name);
    mmo_file = fopen (alt_name, "rb");
    if (!mmo_file) {
      fprintf (stderr, "Can't open the object file %s or %s!\n",
               mmo_file_name, alt_name);
      exit (-3);
    }
    free (alt_name);
  }
  byte_count = 0;
  cur_loc.h = cur_loc.l = 0;                                         /*  32 */
  cur_file = -1;
  cur_line = 0;
  read_tet ();                                                       /*  28 */
  if (buf[0] != mm || buf[1] != lop_pre)
    mmo_err;
  if (ybyte != 1)
    mmo_err;
  if (zbyte == 0)
    obj_time = 0xffffffff;
  else {
    j = zbyte - 1;
    read_tet ();
    obj_time = tet;
    for (; j > 0; j--)
      read_tet ();
  }
  ;
  do {                                                               /*  29 */
    read_tet ();
  loop:if (buf[0] == mm)
      switch (buf[1]) {
      case lop_quote:
        if (yzbytes != 1)
          mmo_err;
        read_tet ();
        break;
      case lop_loc:
        if (zbyte == 2) {                                            /*  33 */
          j = ybyte;
          read_tet ();
          cur_loc.h = (j << 24) + tet;
        }
        else if (zbyte == 1)
          cur_loc.h = ybyte << 24;
        else
          mmo_err;
        read_tet ();
        cur_loc.l = tet;
        continue;
      case lop_skip:
        cur_loc = incr (cur_loc, yzbytes);
        continue;
      case lop_fixo:
        if (zbyte == 2) {                                            /*  34 */
          j = ybyte;
          read_tet ();
          tmp.h = (j << 24) + tet;
        }
        else if (zbyte == 1)
          tmp.h = ybyte << 24;
        else
          mmo_err;
        read_tet ();
        tmp.l = tet;
        mmo_load (tmp, cur_loc.h);
        mmo_load (incr (tmp, 4), cur_loc.l);
        continue;
      case lop_fixr:
        delta = yzbytes;
        goto fixr;
      case lop_fixrx:
        j = yzbytes;
        if (j != 16 && j != 24)
          mmo_err;
        read_tet ();
        delta = tet;
        if (delta & 0xfe000000)
          mmo_err;
      fixr:tmp =
          incr (cur_loc,
                -(delta >=
                  0x1000000 ? (delta & 0xffffff) - (1 << j) : delta) << 2);
        mmo_load (tmp, delta);
        continue;
      case lop_file:
        if (file_info[ybyte].name) {                                 /*  35 */
          if (zbyte)
            mmo_err;
          cur_file = ybyte;
        }
        else {
          if (!zbyte)
            mmo_err;
          file_info[ybyte].name = (char *) calloc (4 * zbyte + 1, 1);
          if (!file_info[ybyte].name) {
            fprintf (stderr, "No room to store the file name!\n");
            exit (-5);
          }
          cur_file = ybyte;
          for (j = zbyte, p = file_info[ybyte].name; j > 0; j--, p += 4) {
            read_tet ();
            *p = buf[0];
            *(p + 1) = buf[1];
            *(p + 2) = buf[2];
            *(p + 3) = buf[3];
          }
        }
        cur_line = 0;
        continue;
      case lop_line:
        if (cur_file < 0)
          mmo_err;
        cur_line = yzbytes;
        continue;
      case lop_spec:
        while (1) {                                                  /*  36 */
          read_tet ();
          if (buf[0] == mm) {
            if (buf[1] != lop_quote || yzbytes != 1)
              goto loop;
            read_tet ();
          }
        }
      case lop_post:
        postamble = 1;
        if (ybyte || zbyte < 32)
          mmo_err;
        continue;
      default:
        mmo_err;
      }
    {                                                                /*  30 */
      mmo_load (cur_loc, tet);
      if (cur_line) {
        ll->file_no = cur_file;
        ll->line_no = cur_line;
        cur_line++;
      }
      cur_loc = incr (cur_loc, 4);
      cur_loc.l &= -4;
    }
    ;
  }
  while (!postamble);
  aux.h = 0x60000000;
  aux.l = 0x18;                                                      /*  37 */
  ll = mem_find (aux);
  (ll - 1)->tet = 2;
  (ll - 5)->tet = argc;
  (ll - 4)->tet = 0x40000000;
  (ll - 3)->tet = 0x8;
  G = zbyte;
  L = 0;
  for (j = G + G; j < 256 + 256; j++, ll++, aux.l += 4)
    read_tet (), ll->tet = tet;
  inst_ptr.h = (ll - 2)->tet, inst_ptr.l = (ll - 1)->tet;
  (ll + 2 * 12)->tet = G << 24;
  g[255] = incr (aux, 12 * 8);
  ;
  fclose (mmo_file);
  cur_line = 0;
  if (buf_size < 72)
    buf_size = 72;                                                   /*  41 */
  buffer = (Char *) calloc (buf_size + 1, sizeof (Char));
  if (!buffer)
    panic ("Can't allocate source line buffer");
  g[rK] = neg_one;                                                   /*  77 */
  g[rN].h = (VERSION << 24) + (SUBVERSION << 16) + (SUBSUBVERSION << 8);
  g[rN].l = ABSTIME;
  g[rT].h = 0x80000005;
  g[rTT].h = 0x80000006;
  g[rV].h = 0x369c2004;
  if (lring_size < 256)
    lring_size = 256;
  lring_mask = lring_size - 1;
  if (lring_size & lring_mask)
    panic ("The number of local registers must be a power of 2");
  l = (octa *) calloc (lring_size, sizeof (octa));
  if (!l)
    panic ("No room for the local registers");
  cur_round = ROUND_NEAR;
  signal (SIGINT, catchint);                                         /* 147 */
  vram_init (argc, argv);                                            /* 167 */
  vram_tetra[0].tet = vram_tetra[1].tet = 0;                         /* 172 */
  vram_tetra[0].freq = vram_tetra[1].freq = 0;
  vram_tetra[0].bkpt = vram_tetra[1].bkpt = 0;
  vram_tetra[0].file_no = vram_tetra[1].file_no = -1;
  vram_tetra[0].line_no = vram_tetra[1].line_no = -1;
  ;
  x.h = 0x40000000, x.l = 0x8;                                       /* 163 */
  loc = incr (x, 8 * (argc + 1));
  for (k = 0; k < argc; k++, cur_arg++) {
    ll = mem_find (x);
    ll->tet = loc.h, (ll + 1)->tet = loc.l;
    ll = mem_find (loc);
    mmputchars ((unsigned char *) *cur_arg, strlen (*cur_arg), loc);
    x.l += 8, loc.l += 8 + (strlen (*cur_arg) & -8);
  }
  x.l = 0;
  ll = mem_find (x);
  ll->tet = loc.h, (ll + 1)->tet = loc.l;
  ;
  x.h = 0, x.l = 0x90;                                               /* 164 */
  ll = mem_find (x);
  if (ll->tet)
    inst_ptr = x;
  resuming = true;
  rop = RESUME_AGAIN;
  g[rX].l = ((tetra) UNSAVE << 24) + 255;
  if (dump_file) {
    x.l = 1;
    dump (mem_root);
    dump_tet (0), dump_tet (0);
    exit (0);
  }
  ;
  while (1) {
    if (interrupt && !breakpoint)
      breakpoint = interacting = true, interrupt = false;
    else {
      breakpoint = false;
      if (interacting) {
        register int repeating;                                      /* 149 */

      interact:{
          register bool ready = false;                               /* 150 */

        incl_read:while (incl_file && !ready)
            if (!fgets (command_buf, command_buf_size, incl_file)) {
              fclose (incl_file);
              incl_file = NULL;
            }
            else if (command_buf[0] != '\n' && command_buf[0] != 'i' &&
                     command_buf[0] != '%')
              if (command_buf[0] == ' ')
                printf (command_buf);
              else
                ready = true;
          while (!ready) {
            printf ("mmix> ");
            fflush (stdout);
            if (!fgets (command_buf, command_buf_size, stdin))
              command_buf[0] = 'q';
            if (command_buf[0] != 'i')
              ready = true;
            else {
              command_buf[strlen (command_buf) - 1] = '\0';
              incl_file = fopen (command_buf + 1, "r");
              if (incl_file)
                goto incl_read;
              if (isspace (command_buf[1]))
                incl_file = fopen (command_buf + 2, "r");
              if (incl_file)
                goto incl_read;
              printf ("Can't open file `%s'!\n", command_buf + 1);
            }
          }
        }
        ;
        p = command_buf;
        repeating = 0;
        switch (*p) {
        case '\n':
        case 'n':
          breakpoint = tracing = true;
        case 'c':
          goto resume_simulation;
        case 'q':
          goto end_simulation;
        case 's':
          show_stats (true);
          goto interact;
        case '-':
          k = strlen (p);
          if (p[k - 1] == '\n')
            p[k - 1] = '\0';
          scan_option (p + 1, false);
          goto interact;
        case 'l':
        case 'g':
        case '$':
          cur_disp_mode = *p++;                                      /* 152 */
          for (cur_disp_addr.l = 0; isdigit (*p); p++)
            cur_disp_addr.l = 10 * cur_disp_addr.l + *p - '0';
          goto new_mode;
        case 'r':
          p++;
          cur_disp_mode = 'g';
          if (*p < 'A' || *p > 'Z')
            goto what_say;
          if (*(p + 1) != *p)
            cur_disp_addr.l = spec_reg_code[*p - 'A'], p++;
          else if (spec_regg_code[*p - 'A'])
            cur_disp_addr.l = spec_regg_code[*p - 'A'], p += 2;
          else
            goto what_say;
          goto new_mode;
        case 'M':
          cur_disp_mode = *p;
          cur_disp_addr = scan_hex (p + 1, cur_seg);
          cur_disp_addr.l &= -8;
          p = next_char;
        new_mode:cur_disp_set = false;
          repeating = 1;
          goto scan_type;
        case '+':
          if (!isdigit (*(p + 1)))
            repeating = 1;
          for (p++; isdigit (*p); p++)
            repeating = 10 * repeating + *p - '0';
          if (repeating) {
            if (cur_disp_mode == 'M')
              cur_disp_addr = incr (cur_disp_addr, 8);
            else
              cur_disp_addr.l++;
          }
          goto scan_type;
          ;
        case '!':
        case '.':
        case '#':
        case '"':
          cur_disp_set = false;                                      /* 153 */
          repeating = 1;
        set_type:cur_disp_type = *p++;
          break;
        scan_type:if (*p == '!' || *p == '.' || *p == '#'
              || *p == '"')
            goto set_type;
          if (*p != '=')
            break;
          goto scan_eql;
        case '=':
          repeating = 1;
        scan_eql:cur_disp_set = true;
          val = zero_octa;
          if (*++p == '#')
            cur_disp_type = *p, val = scan_hex (p + 1, zero_octa);
          else if (*p == '"' || *p == '\'')
            goto scan_string;
          else
            cur_disp_type = (scan_const (p) > 0 ? '.' : '!');
          p = next_char;
          if (*p != ',')
            break;
          val.h = 0;
          val.l &= 0xff;
        scan_string:cur_disp_type = '"';
          while (*p == ',') {                                        /* 155 */
            if (*++p == '#') {
              aux = scan_hex (p + 1, zero_octa), p = next_char;
              val = incr (shift_left (val, 8), aux.l & 0xff);
            }
            else if (isdigit (*p)) {
              for (k = *p++ - '0'; isdigit (*p); p++)
                k = (10 * k + *p - '0') & 0xff;
              val = incr (shift_left (val, 8), k);
            }
            else if (*p == '\n')
              goto incomplete_str;
          }
          if (*p == '\'' && *(p + 2) == *p)
            *p = *(p + 2) = '"';
          if (*p == '"') {
            for (p++; *p && *p != '\n' && *p != '"'; p++)
              val = incr (shift_left (val, 8), *p);
            if (*p && *p++ == '"')
              if (*p == ',')
                goto scan_string;
          }
          ;
          break;
          ;
        case '@':
          inst_ptr = scan_hex (p + 1, cur_seg);
          p = next_char;                                             /* 161 */
          halted = false;
          break;
        case 't':
        case 'u':
          k = *p;
          val = scan_hex (p + 1, cur_seg);
          p = next_char;
          if (val.h < 0x20000000) {
            ll = mem_find (val);
            if (k == 't')
              ll->bkpt |= trace_bit;
            else
              ll->bkpt &= ~trace_bit;
          }
          break;
        case 'b':
          for (k = 0, p++; !isxdigit (*p); p++)
            if (*p == 'r')
              k |= read_bit;
            else if (*p == 'w')
              k |= write_bit;
            else if (*p == 'x')
              k |= exec_bit;
          val = scan_hex (p, cur_seg);
          p = next_char;
          if (!(val.h & sign_bit) && (val.h != 0x50000000)) {
            ll = mem_find (val);
            ll->bkpt = (ll->bkpt & -8) | k;
          }
          break;
        case 'T':
          cur_seg.h = 0;
          goto passit;
        case 'D':
          cur_seg.h = 0x20000000;
          goto passit;
        case 'P':
          cur_seg.h = 0x40000000;
          goto passit;
        case 'S':
          cur_seg.h = 0x60000000;
          goto passit;
        case 'B':
          show_breaks (mem_root);
        passit:p++;
          break;
          ;
        default:
        what_say:k = strlen (command_buf);
          if (k < 10 && command_buf[k - 1] == '\n')
            command_buf[k - 1] = '\0';
          else
            strcpy (command_buf + 9, "...");
          printf ("Eh? Sorry, I don't understand `%s'. (Type h for help)\n",
                  command_buf);
          goto interact;
        case 'h':
          for (k = 0; interactive_help[k][0]; k++)
            printf (interactive_help[k]);
          goto interact;
        }
      check_syntax:if (*p != '\n') {
          if (!*p)
          incomplete_str:printf
              ("Syntax error: Incomplete command!\n");
          else {
            p[strlen (p) - 1] = '\0';
            printf ("Syntax error; I'm ignoring `%s'!\n", p);
          }
        }
        while (repeating) {                                          /* 156 */
          if (cur_disp_set)
            switch (cur_disp_mode) {                                 /* 157 */
            case 'l':
              l[cur_disp_addr.l & lring_mask] = val;
              break;
            case '$':
              k = cur_disp_addr.l & 0xff;
              if (k < L)
                l[(O + k) & lring_mask] = val;
              else if (k >= G)
                g[k] = val;
              break;
            case 'g':
              k = cur_disp_addr.l & 0xff;
              if (k < 32)
                if (k >= 9 && k != rI) {                             /* 158 */
                  if (k <= 19)
                    break;
                  if (k == rA) {
                    if (val.h != 0 || val.l >= 0x40000)
                      break;
                    cur_round = (val.l >= 0x10000 ? val.l >> 16 : ROUND_NEAR);
                  }
                  else if (k == rG) {
                    if (val.h != 0 || val.l > 255 || val.l < L || val.l < 32)
                      break;
                    for (j = val.l; j < G; j++)
                      g[j] = zero_octa;
                    G = val.l;
                  }
                  else if (k == rL) {
                    if (val.h == 0 && val.l < L)
                      L = val.l;
                    else
                      break;
                  }
                }
              ;
              g[k] = val;
              break;
            case 'M':
              if (!(cur_disp_addr.h & sign_bit)
                  && (cur_disp_addr.h != 0x50000000)) {
                ll = mem_find (cur_disp_addr);
                ll->tet = val.h;
                (ll + 1)->tet = val.l;
              }
              break;
            }
          ;
          switch (cur_disp_mode) {                                   /* 159 */
          case 'l':
            k = cur_disp_addr.l & lring_mask;
            printf ("l[%d]=", k);
            aux = l[k];
            break;
          case '$':
            k = cur_disp_addr.l & 0xff;
            if (k < L)
              printf ("$%d=l[%d]=", k, (O + k) & lring_mask), aux =
                l[(O + k) & lring_mask];
            else if (k >= G)
              printf ("$%d=g[%d]=", k, k), aux = g[k];
            else
              printf ("$%d=", k), aux = zero_octa;
            break;
          case 'g':
            k = cur_disp_addr.l & 0xff;
            printf ("g[%d]=", k);
            aux = g[k];
            break;
          case 'M':
            if (cur_disp_addr.h & sign_bit)
              aux = zero_octa;
            else {
              ll = mem_find (cur_disp_addr);
              aux.h = ll->tet;
              aux.l = (ll + 1)->tet;
            }
            printf ("M8[#");
            print_hex (cur_disp_addr);
            printf ("]=");
            break;
          }
          switch (cur_disp_type) {
          case '!':
            print_int (aux);
            break;
          case '.':
            print_float (aux);
            break;
          case '#':
            fputc ('#', stdout);
            print_hex (aux);
            break;
          case '"':
            print_string (aux);
            break;
          }
          ;
          fputc ('\n', stdout);
          repeating--;
          if (!repeating)
            break;
          if (cur_disp_mode == 'M')
            cur_disp_addr = incr (cur_disp_addr, 8);
          else
            cur_disp_addr.l++;
        }
        ;
        goto interact;
      resume_simulation:;
      }
      ;
    }
    if (halted)
      break;
    do {                                                             /*  60 */
      if (resuming)
        loc = incr (inst_ptr, -4), inst = g[rX].l;
      else {                                                         /*  63 */
        loc = inst_ptr;
        ll = mem_find (loc);
        inst = ll->tet;
        cur_file = ll->file_no;
        cur_line = ll->line_no;
        ll->freq++;
        if (ll->bkpt & exec_bit)
          breakpoint = true;
        tracing = breakpoint || (ll->bkpt & trace_bit)
          || (ll->freq <= trace_threshold);
        inst_ptr = incr (inst_ptr, 4);
      }
      ;
      op = inst >> 24;
      xx = (inst >> 16) & 0xff;
      yy = (inst >> 8) & 0xff;
      zz = inst & 0xff;
      f = info[op].flags;
      yz = inst & 0xffff;
      x = y = z = a = b = zero_octa;
      exc = 0;
      old_L = L;
      if (f & rel_addr_bit) {                                        /*  70 */
        if ((op & 0xfe) == JMP)
          yz = inst & 0xffffff;
        if (op & 1)
          yz -= (op == JMPB ? 0x1000000 : 0x10000);
        y = inst_ptr;
        z = incr (loc, yz << 2);
      }
      ;
      if (resuming && rop != RESUME_AGAIN)                           /*  71 */
        if (rop == RESUME_SET) {                                     /* 126 */
          op = ORI;
          y = g[rZ];
          z = zero_octa;
          exc = g[rX].h & 0xff00;
          f = X_is_dest_bit;
        }
        else {
          y = g[rY];
          z = g[rZ];
        }
      else {
        if (f & 0x10) {                                              /*  74 */
          if (xx >= G)
            b = g[xx];
          else if (xx < L)
            b = l[(O + xx) & lring_mask];
        }
        ;
        if (info[op].third_operand)
          b = g[info[op].third_operand];                             /*  79 */
        ;
        if (f & 0x1)
          z.l = zz;
        else if (f & 0x2) {                                          /*  72 */
          if (zz >= G)
            z = g[zz];
          else if (zz < L)
            z = l[(O + zz) & lring_mask];
        }
        else if ((op & 0xf0) == SETH) {                              /*  78 */
          switch (op & 3) {
          case 0:
            z.h = yz << 16;
            break;
          case 1:
            z.h = yz;
            break;
          case 2:
            z.l = yz << 16;
            break;
          case 3:
            z.l = yz;
            break;
          }
          y = b;
        }
        ;
        if (f & 0x4)
          y.l = yy;
        else if (f & 0x8) {                                          /*  73 */
          if (yy >= G)
            y = g[yy];
          else if (yy < L)
            y = l[(O + yy) & lring_mask];
        }
        ;
      }
      ;
      if (f & X_is_dest_bit)
        if (xx >= G) {                                               /*  80 */
          sprintf (lhs, "$%d=g[%d]", xx, xx);
          x_ptr = &g[xx];
        }
        else {
          while (xx >= L) {                                          /*  81 */
            l[(O + L) & lring_mask] = zero_octa;
            L = g[rL].l = L + 1;
            if (((S - O - L) & lring_mask) == 0)
              stack_store ();
          }
          ;
          sprintf (lhs, "$%d=l[%d]", xx, (O + xx) & lring_mask);
          x_ptr = &l[(O + xx) & lring_mask];
        }
      ;
      w = oplus (y, z);
      if (loc.h >= 0x20000000)
        goto privileged_inst;
      switch (op) {
      case ADD:
      case ADDI:
        x = w;                                                       /*  84 */
        if (((y.h ^ z.h) & sign_bit) == 0 && ((y.h ^ x.h) & sign_bit) != 0)
          exc |= V_BIT;
      store_x:*x_ptr = x;
        break;
      case SUB:
      case SUBI:
      case NEG:
      case NEGI:
        x = ominus (y, z);                                           /*  85 */
        if (((x.h ^ z.h) & sign_bit) == 0 && ((x.h ^ y.h) & sign_bit) != 0)
          exc |= V_BIT;
        goto store_x;
      case ADDU:
      case ADDUI:
      case INCH:
      case INCMH:
      case INCML:
      case INCL:
        x = w;
        goto store_x;
      case SUBU:
      case SUBUI:
      case NEGU:
      case NEGUI:
        x = ominus (y, z);
        goto store_x;
      case IIADDU:
      case IIADDUI:
      case IVADDU:
      case IVADDUI:
      case VIIIADDU:
      case VIIIADDUI:
      case XVIADDU:
      case XVIADDUI:
        x = oplus (shift_left (y, ((op & 0xf) >> 1) - 3), z);
        goto store_x;
      case SETH:
      case SETMH:
      case SETML:
      case SETL:
      case GETA:
      case GETAB:
        x = z;
        goto store_x;
      case OR:
      case ORI:
      case ORH:
      case ORMH:
      case ORML:
      case ORL:                                                     /*  86 */
        x.h = y.h | z.h;
        x.l = y.l | z.l;
        goto store_x;
      case ORN:
      case ORNI:
        x.h = y.h | ~z.h;
        x.l = y.l | ~z.l;
        goto store_x;
      case NOR:
      case NORI:
        x.h = ~(y.h | z.h);
        x.l = ~(y.l | z.l);
        goto store_x;
      case XOR:
      case XORI:
        x.h = y.h ^ z.h;
        x.l = y.l ^ z.l;
        goto store_x;
      case AND:
      case ANDI:
        x.h = y.h & z.h;
        x.l = y.l & z.l;
        goto store_x;
      case ANDN:
      case ANDNI:
      case ANDNH:
      case ANDNMH:
      case ANDNML:
      case ANDNL:
        x.h = y.h & ~z.h;
        x.l = y.l & ~z.l;
        goto store_x;
      case NAND:
      case NANDI:
        x.h = ~(y.h & z.h);
        x.l = ~(y.l & z.l);
        goto store_x;
      case NXOR:
      case NXORI:
        x.h = ~(y.h ^ z.h);
        x.l = ~(y.l ^ z.l);
        goto store_x;
      case SL:
      case SLI:
        x = shift_left (y, shift_amt);                               /*  87 */
        a = shift_right (x, shift_amt, 0);
        if (a.h != y.h || a.l != y.l)
          exc |= V_BIT;
        goto store_x;
      case SLU:
      case SLUI:
        x = shift_left (y, shift_amt);
        goto store_x;
      case SR:
      case SRI:
      case SRU:
      case SRUI:
        x = shift_right (y, shift_amt, op & 0x2);
        goto store_x;
      case MUX:
      case MUXI:
        x.h = (y.h & b.h) | (z.h & ~b.h);
        x.l = (y.l & b.l) | (z.l & ~b.l);
        goto store_x;
      case SADD:
      case SADDI:
        x.l = count_bits (y.h & ~z.h) + count_bits (y.l & ~z.l);
        goto store_x;
      case MOR:
      case MORI:
        x = bool_mult (y, z, false);
        goto store_x;
      case MXOR:
      case MXORI:
        x = bool_mult (y, z, true);
        goto store_x;
      case BDIF:
      case BDIFI:
        x.h = byte_diff (y.h, z.h);
        x.l = byte_diff (y.l, z.l);
        goto store_x;
      case WDIF:
      case WDIFI:
        x.h = wyde_diff (y.h, z.h);
        x.l = wyde_diff (y.l, z.l);
        goto store_x;
      case TDIF:
      case TDIFI:
        if (y.h > z.h)
          x.h = y.h - z.h;
      tdif_l:if (y.l > z.l)
          x.l = y.l - z.l;
        goto store_x;
      case ODIF:
      case ODIFI:
        if (y.h > z.h)
          x = ominus (y, z);
        else if (y.h == z.h)
          goto tdif_l;
        goto store_x;
      case MUL:
      case MULI:
        x = signed_omult (y, z);                                     /*  88 */
      test_overflow:if (overflow)
          exc |= V_BIT;
        goto store_x;
      case MULU:
      case MULUI:
        x = omult (y, z);
        a = g[rH] = aux;
        goto store_x;
      case DIV:
      case DIVI:
        if (!z.l && !z.h)
          aux = y, exc |= D_BIT;
        else
          x = signed_odiv (y, z);
        a = g[rR] = aux;
        goto test_overflow;
      case DIVU:
      case DIVUI:
        x = odiv (b, y, z);
        a = g[rR] = aux;
        goto store_x;
      case FADD:
        x = fplus (y, z);                                            /*  89 */
      fin_float:round_mode = cur_round;
      store_fx:exc |= exceptions;
        goto store_x;
      case FSUB:
        a = z;
        if (fcomp (a, zero_octa) != 2)
          a.h ^= sign_bit;
        x = fplus (y, a);
        goto fin_float;
      case FMUL:
        x = fmult (y, z);
        goto fin_float;
      case FDIV:
        x = fdivide (y, z);
        goto fin_float;
      case FREM:
        x = fremstep (y, z, 2500);
        goto fin_float;
      case FSQRT:
        x = froot (z, y.l);
      fin_unifloat:if (y.h || y.l > 4)
          goto illegal_inst;
        round_mode = (y.l ? y.l : cur_round);
        goto store_fx;
      case FINT:
        x = fintegerize (z, y.l);
        goto fin_unifloat;
      case FIX:
        x = fixit (z, y.l);
        goto fin_unifloat;
      case FIXU:
        x = fixit (z, y.l);
        exceptions &= ~W_BIT;
        goto fin_unifloat;
      case FLOT:
      case FLOTI:
      case FLOTU:
      case FLOTUI:
      case SFLOT:
      case SFLOTI:
      case SFLOTU:
      case SFLOTUI:
        x = floatit (z, y.l, op & 0x2, op & 0x4);
        goto fin_unifloat;
      case CMP:
      case CMPI:
        if ((y.h & sign_bit) > (z.h & sign_bit))
          goto cmp_neg;                                              /*  90 */
        if ((y.h & sign_bit) < (z.h & sign_bit))
          goto cmp_pos;
      case CMPU:
      case CMPUI:
        if (y.h < z.h)
          goto cmp_neg;
        if (y.h > z.h)
          goto cmp_pos;
        if (y.l < z.l)
          goto cmp_neg;
        if (y.l == z.l)
          goto cmp_zero;
      cmp_pos:x.l = 1;
        goto store_x;
      cmp_neg:x = neg_one;
        goto store_x;
      case FCMPE:
        k = fepscomp (y, z, b, true);
        if (k)
          goto cmp_zero_or_invalid;
      case FCMP:
        k = fcomp (y, z);
        if (k < 0)
          goto cmp_neg;
      cmp_fin:if (k == 1)
          goto cmp_pos;
      cmp_zero_or_invalid:if (k == 2)
          exc |= I_BIT;
        goto cmp_zero;
      case FUN:
        if (fcomp (y, z) == 2)
          goto cmp_pos;
        else
          goto cmp_zero;
      case FEQL:
        if (fcomp (y, z) == 0)
          goto cmp_pos;
        else
          goto cmp_zero;
      case FEQLE:
        k = fepscomp (y, z, b, false);
        goto cmp_fin;
      case FUNE:
        if (fepscomp (y, z, b, true) == 2)
          goto cmp_pos;
        else
          goto cmp_zero;
      case CSN:
      case CSNI:
      case CSZ:
      case CSZI:                                                    /*  92 */
      case CSP:
      case CSPI:
      case CSOD:
      case CSODI:
      case CSNN:
      case CSNNI:
      case CSNZ:
      case CSNZI:
      case CSNP:
      case CSNPI:
      case CSEV:
      case CSEVI:
      case ZSN:
      case ZSNI:
      case ZSZ:
      case ZSZI:
      case ZSP:
      case ZSPI:
      case ZSOD:
      case ZSODI:
      case ZSNN:
      case ZSNNI:
      case ZSNZ:
      case ZSNZI:
      case ZSNP:
      case ZSNPI:
      case ZSEV:
      case ZSEVI:
        x = register_truth (y, op) ? z : b;
        goto store_x;
      case BN:
      case BNB:
      case BZ:
      case BZB:                                                     /*  93 */
      case BP:
      case BPB:
      case BOD:
      case BODB:
      case BNN:
      case BNNB:
      case BNZ:
      case BNZB:
      case BNP:
      case BNPB:
      case BEV:
      case BEVB:
      case PBN:
      case PBNB:
      case PBZ:
      case PBZB:
      case PBP:
      case PBPB:
      case PBOD:
      case PBODB:
      case PBNN:
      case PBNNB:
      case PBNZ:
      case PBNZB:
      case PBNP:
      case PBNPB:
      case PBEV:
      case PBEVB:
        x.l = register_truth (b, op);
        if (x.l) {
          inst_ptr = z;
          good = (op >= PBN);
        }
        else
          good = (op < PBN);
        if (good)
          good_guesses++;
        else
          bad_guesses++, g[rC].l += 2;
        break;
      case LDB:
      case LDBI:
      case LDBU:
      case LDBUI:                                                   /*  94 */
        i = 56;
        j = (w.l & 0x3) << 3;
        goto fin_ld;
      case LDW:
      case LDWI:
      case LDWU:
      case LDWUI:
        i = 48;
        j = (w.l & 0x2) << 3;
        goto fin_ld;
      case LDT:
      case LDTI:
      case LDTU:
      case LDTUI:
        i = 32;
        j = 0;
        goto fin_ld;
      case LDHT:
      case LDHTI:
        i = j = 0;
      fin_ld:ll = mem_find (w);
        test_load_bkpt (ll);
        x.h = ll->tet;
        x = shift_right (shift_left (x, j), i, op & 0x2);
      check_ld:if (w.h & sign_bit)
          goto privileged_inst;
        goto store_x;
      case LDO:
      case LDOI:
      case LDOU:
      case LDOUI:
      case LDUNC:
      case LDUNCI:
        w.l &= -8;
        ll = mem_find (w);
        test_load_bkpt (ll);
        test_load_bkpt (ll + 1);
        x.h = ll->tet;
        x.l = (ll + 1)->tet;
        goto check_ld;
      case LDSF:
      case LDSFI:
        ll = mem_find (w);
        test_load_bkpt (ll);
        x = load_sf (ll->tet);
        goto check_ld;
      case STB:
      case STBI:
      case STBU:
      case STBUI:                                                   /*  95 */
        i = 56;
        j = (w.l & 0x3) << 3;
        goto fin_pst;
      case STW:
      case STWI:
      case STWU:
      case STWUI:
        i = 48;
        j = (w.l & 0x2) << 3;
        goto fin_pst;
      case STT:
      case STTI:
      case STTU:
      case STTUI:
        i = 32;
        j = 0;
      fin_pst:ll = mem_find (w);
        if ((op & 0x2) == 0) {
          a = shift_right (shift_left (b, i), i, 0);
          if (a.h != b.h || a.l != b.l)
            exc |= V_BIT;
        }
        if (ll != vram_tetra)
          ll->tet ^=
            (ll->
             tet ^ (b.l << (i - 32 - j))) & ((((tetra) - 1) << (i - 32)) >> j);
        else if (i == 32)
          vram_write (vram_addr, b.l);
        goto fin_st;
      case STSF:
      case STSFI:
        ll = mem_find (w);
        if (ll != vram_tetra)
          ll->tet = store_sf (b);
        else
          vram_write (vram_addr, store_sf (b));
        exc = exceptions;
        goto fin_st;
      case STHT:
      case STHTI:
        ll = mem_find (w);
        if (ll != vram_tetra)
          ll->tet = b.h;
        else
          vram_write (vram_addr, b.h);
      fin_st:test_store_bkpt (ll);
        w.l &= -8;
        ll = mem_find (w);
        a.h = ll->tet;
        a.l = (ll + 1)->tet;
        goto check_st;
      case STCO:
      case STCOI:
        b.l = xx;
      case STO:
      case STOI:
      case STOU:
      case STOUI:
      case STUNC:
      case STUNCI:
        w.l &= -8;
        ll = mem_find (w);
        test_store_bkpt (ll);
        test_store_bkpt (ll + 1);
        if (ll != vram_tetra)
          ll->tet = b.h, (ll + 1)->tet = b.l;
        else {
          vram_write (vram_addr, b.h);
          vram_write (vram_addr + 4, b.l);
        }
      check_st:if (w.h & sign_bit)
          goto privileged_inst;
        break;
      case CSWAP:
      case CSWAPI:
        w.l &= -8;
        ll = mem_find (w);                                           /*  96 */
        test_load_bkpt (ll);
        test_load_bkpt (ll + 1);
        a = g[rP];
        if (ll->tet == a.h && (ll + 1)->tet == a.l) {
          x.h = 0, x.l = 1;
          test_store_bkpt (ll);
          test_store_bkpt (ll + 1);
          if (ll != vram_tetra)
            ll->tet = b.h, (ll + 1)->tet = b.l;
          else {
            vram_write (vram_addr, b.h);
            vram_write (vram_addr + 4, b.l);
          }
          strcpy (rhs, "M8[%#w]=%#b");
        }
        else {
          b.h = ll->tet, b.l = (ll + 1)->tet;
          g[rP] = b;
          strcpy (rhs, "rP=%#b");
        }
        goto check_ld;
      case GET:
        if (yy != 0 || zz >= 32)
          goto illegal_inst;                                         /*  97 */
        x = g[zz];
        goto store_x;
      case PUT:
      case PUTI:
        if (yy != 0 || xx >= 32)
          goto illegal_inst;
        strcpy (rhs, "%z = %#z");
        if (xx >= 8) {
          if (xx <= 11)
            goto illegal_inst;
          if (xx <= 18)
            goto privileged_inst;
          if (xx == rA) {                                            /* 100 */
            if (z.h != 0 || z.l >= 0x40000)
              goto illegal_inst;
            cur_round = (z.l >= 0x10000 ? z.l >> 16 : ROUND_NEAR);
          }
          else if (xx == rL) {                                       /*  98 */
            x = z;
            strcpy (rhs, z.h ? "min(rL,%#x) = %z" : "min(rL,%x) = %z");
            if (z.l > L || z.h)
              z.h = 0, z.l = L;
            else
              old_L = L = z.l;
          }
          else if (xx == rG) {                                       /*  99 */
            if (z.h != 0 || z.l > 255 || z.l < L || z.l < 32)
              goto illegal_inst;
            for (j = z.l; j < G; j++)
              g[j] = zero_octa;
            G = z.l;
          }
          ;
        }
        g[xx] = z;
        zz = xx;
        break;
      case PUSHGO:
      case PUSHGOI:
        inst_ptr = w;
        goto push;                                                   /* 101 */
      case PUSHJ:
      case PUSHJB:
        inst_ptr = z;
      push:if (xx >= G) {
          xx = L++;
          if (((S - O - L) & lring_mask) == 0)
            stack_store ();
        }
        x.l = xx;
        l[(O + xx) & lring_mask] = x;
        sprintf (lhs, "l[%d]=%d, ", (O + xx) & lring_mask, xx);
        x = g[rJ] = incr (loc, 4);
        L -= xx + 1;
        O += xx + 1;
        b = g[rO] = incr (g[rO], (xx + 1) << 3);
      sync_L:a.l = g[rL].l = L;
        break;
      case POP:
        if (xx != 0 && xx <= L)
          y = l[(O + xx - 1) & lring_mask];
        if (g[rS].l == g[rO].l)
          stack_load ();
        k = l[(O - 1) & lring_mask].l & 0xff;
        while ((tetra) (O - S) <= (tetra) k)
          stack_load ();
        L = k + (xx <= L ? xx : L + 1);
        if (L > G)
          L = G;
        if (L > k) {
          l[(O - 1) & lring_mask] = y;
          if (y.h)
            sprintf (lhs, "l[%d]=#%x%08x, ", (O - 1) & lring_mask, y.h, y.l);
          else
            sprintf (lhs, "l[%d]=#%x, ", (O - 1) & lring_mask, y.l);
        }
        else
          lhs[0] = '\0';
        y = g[rJ];
        z.l = yz << 2;
        inst_ptr = oplus (y, z);
        O -= k + 1;
        b = g[rO] = incr (g[rO], -((k + 1) << 3));
        goto sync_L;
      case SAVE:
        if (xx < G || yy != 0 || zz != 0)
          goto illegal_inst;                                         /* 102 */
        l[(O + L) & lring_mask].l = L++;
        if (((S - O - L) & lring_mask) == 0)
          stack_store ();
        O += L;
        g[rO] = incr (g[rO], L << 3);
        L = g[rL].l = 0;
        while (g[rO].l != g[rS].l)
          stack_store ();
        for (k = G;;) {
          ll = mem_find (g[rS]);                                     /* 103 */
          if (k == rZ + 1)
            x.h = G << 24, x.l = g[rA].l;
          else
            x = g[k];
          ll->tet = x.h;
          test_store_bkpt (ll);
          (ll + 1)->tet = x.l;
          test_store_bkpt (ll + 1);
          if (stack_tracing) {
            tracing = true;
            if (cur_line)
              show_line ();
            if (k >= 32)
              printf ("             M8[#%08x%08x]=g[%d]=#%08x%08x, rS+=8\n",
                      g[rS].h, g[rS].l, k, x.h, x.l);
            else
              printf ("             M8[#%08x%08x]=%s=#%08x%08x, rS+=8\n",
                      g[rS].h, g[rS].l,
                      k == rZ + 1 ? "(rG,rA)" : special_name[k], x.h, x.l);
          }
          S++, g[rS] = incr (g[rS], 8);
          ;
          if (k == 255)
            k = rB;
          else if (k == rR)
            k = rP;
          else if (k == rZ + 1)
            break;
          else
            k++;
        }
        O = S, g[rO] = g[rS];
        x = incr (g[rO], -8);
        goto store_x;
      case UNSAVE:
        if (xx != 0 || yy != 0)
          goto illegal_inst;                                         /* 104 */
        z.l &= -8;
        g[rS] = incr (z, 8);
        for (k = rZ + 1;;) {
          g[rS] = incr (g[rS], -8);                                  /* 105 */
          ll = mem_find (g[rS]);
          test_load_bkpt (ll);
          test_load_bkpt (ll + 1);
          if (k == rZ + 1)
            x.l = G = g[rG].l = ll->tet >> 24, a.l = g[rA].l =
              (ll + 1)->tet & 0x3ffff;
          else
            g[k].h = ll->tet, g[k].l = (ll + 1)->tet;
          if (stack_tracing) {
            tracing = true;
            if (cur_line)
              show_line ();
            if (k >= 32)
              printf ("             rS-=8, g[%d]=M8[#%08x%08x]=#%08x%08x\n",
                      k, g[rS].h, g[rS].l, ll->tet, (ll + 1)->tet);
            else if (k == rZ + 1)
              printf ("             (rG,rA)=M8[#%08x%08x]=#%08x%08x\n",
                      g[rS].h, g[rS].l, ll->tet, (ll + 1)->tet);
            else
              printf ("             rS-=8, %s=M8[#%08x%08x]=#%08x%08x\n",
                      special_name[k], g[rS].h, g[rS].l, ll->tet,
                      (ll + 1)->tet);
          }
          ;
          if (k == rP)
            k = rR;
          else if (k == rB)
            k = 255;
          else if (k == G)
            break;
          else
            k--;
        }
        S = g[rS].l >> 3;
        stack_load ();
        k = l[S & lring_mask].l & 0xff;
        for (j = 0; j < k; j++)
          stack_load ();
        O = S;
        g[rO] = g[rS];
        L = k > G ? G : k;
        g[rL].l = L;
        a = g[rL];
        g[rG].l = G;
        break;
      case SYNCID:
      case SYNCIDI:
      case PREST:
      case PRESTI:                                                  /* 106 */
      case SYNCD:
      case SYNCDI:
      case PREGO:
      case PREGOI:
      case PRELD:
      case PRELDI:
        x = incr (w, xx);
        break;
      case GO:
      case GOI:
        x = inst_ptr;
        inst_ptr = w;
        goto store_x;                                                /* 107 */
      case JMP:
      case JMPB:
        inst_ptr = z;
        break;
      case SYNC:
        if (xx != 0 || yy != 0 || zz > 7)
          goto illegal_inst;
        if (zz <= 3)
          break;
      case LDVTS:
      case LDVTSI:
      privileged_inst:strcpy (lhs, "!privileged");
        goto break_inst;
      illegal_inst:strcpy (lhs, "!illegal");
      break_inst:breakpoint = tracing = true;
        if (!interacting && !interact_after_break)
          halted = true;
        break;
      case TRIP:
        exc |= H_BIT;
        break;                                                       /* 108 */
      case TRAP:
        if (xx != 0 || yy > max_sys_call)
          goto privileged_inst;
        strcpy (rhs, trap_format[yy]);
        g[rWW] = inst_ptr;
        g[rXX].h = sign_bit, g[rXX].l = inst;
        g[rYY] = y, g[rZZ] = z;
        z.h = 0, z.l = zz;
        a = incr (b, 8);
        if (arg_count[yy] == 3) {                                    /* 111 */
          ll = mem_find (b);
          test_load_bkpt (ll);
          test_load_bkpt (ll + 1);
          mb.h = ll->tet, mb.l = (ll + 1)->tet;
          ll = mem_find (a);
          test_load_bkpt (ll);
          test_load_bkpt (ll + 1);
          ma.h = ll->tet, ma.l = (ll + 1)->tet;
        }
        ;
        switch (yy) {
        case Halt:
          if (!zz)
            halted = breakpoint = true;                              /* 109 */
          else if (zz == 1) {
            if (loc.h || loc.l >= 0x90)
              goto privileged_inst;
            print_trip_warning (loc.l >> 4, incr (g[rW], -4));
          }
          else
            goto privileged_inst;
          ;
          g[rBB] = g[255];
          break;
        case Fopen:
          g[rBB] = mmix_fopen ((unsigned char) zz, mb, ma);
          break;
        case Fclose:
          g[rBB] = mmix_fclose ((unsigned char) zz);
          break;
        case Fread:
          g[rBB] = mmix_fread ((unsigned char) zz, mb, ma);
          break;
        case Fgets:
          g[rBB] = mmix_fgets ((unsigned char) zz, mb, ma);
          break;
        case Fgetws:
          g[rBB] = mmix_fgetws ((unsigned char) zz, mb, ma);
          break;
        case Fwrite:
          g[rBB] = mmix_fwrite ((unsigned char) zz, mb, ma);
          break;
        case Fputs:
          g[rBB] = mmix_fputs ((unsigned char) zz, b);
          break;
        case Fputws:
          g[rBB] = mmix_fputws ((unsigned char) zz, b);
          break;
        case Fseek:
          g[rBB] = mmix_fseek ((unsigned char) zz, b);
          break;
        case Ftell:
          g[rBB] = mmix_ftell ((unsigned char) zz);
          break;
        }
        x = g[255] = g[rBB];
        break;
      case RESUME:
        if (xx || yy || zz)
          goto illegal_inst;                                         /* 124 */
        inst_ptr = z = g[rW];
        b = g[rX];
        if (!(b.h & sign_bit)) {                                     /* 125 */
          rop = b.h >> 24;
          switch (rop) {
          case RESUME_CONT:
            if ((1 << (b.l >> 28)) & 0x8f30)
              goto illegal_inst;
          case RESUME_SET:
            k = (b.l >> 16) & 0xff;
            if (k >= L && k < G)
              goto illegal_inst;
          case RESUME_AGAIN:
            if ((b.l >> 24) == RESUME)
              goto illegal_inst;
            break;
          default:
            goto illegal_inst;
          }
          resuming = true;
        }
        ;
        break;
        ;
      }
      if ((exc & (U_BIT + X_BIT)) == U_BIT && !(g[rA].l & U_BIT))
        exc &= ~U_BIT;                                               /* 122 */
      if (exc) {
        if (exc & tracing_exceptions)
          tracing = true;
        j = exc & (g[rA].l | H_BIT);
        if (j) {                                                     /* 123 */
          tripping = true;
          for (k = 0; !(j & H_BIT); j <<= 1, k++);
          exc &= ~(H_BIT >> k);
          g[rW] = inst_ptr;
          inst_ptr.h = 0, inst_ptr.l = k << 4;
          g[rX].h = sign_bit, g[rX].l = inst;
          if ((op & 0xe0) == STB)
            g[rY] = w, g[rZ] = b;
          else
            g[rY] = y, g[rZ] = z;
          g[rB] = g[255];
          g[255] = g[rJ];
          if (op == TRIP)
            w = g[rW], x = g[rX], a = g[255];
        }
        ;
        g[rA].l |= exc >> 8;
      }
      ;
      if (g[rU].l || g[rU].h || !resuming) {                         /* 127 */
        g[rC].h += info[op].mems;
        g[rC] = incr (g[rC], info[op].oops);
        g[rU] = incr (g[rU], 1);
        g[rI] = incr (g[rI], -1);
        if (g[rI].l == 0 && g[rI].h == 0)
          tracing = breakpoint = true;
      }
      ;
      if (tracing) {                                                 /* 128 */
        if (showing_source && cur_line)
          show_line ();
        if (resuming && op != RESUME) {                              /* 130 */
          switch (rop) {
          case RESUME_AGAIN:
            printf ("           (%08x%08x: %08x (%s)) ",
                    loc.h, loc.l, inst, info[op].name);
            break;
          case RESUME_CONT:
            printf ("           (%08x%08x: %04xrYrZ (%s)) ",
                    loc.h, loc.l, inst >> 16, info[op].name);
            break;
          case RESUME_SET:
            printf ("           (%08x%08x: ..%02x..rZ (SET)) ",
                    loc.h, loc.l, (inst >> 16) & 0xff);
            break;
          }
        }
        else {
          ll = mem_find (loc);
          printf ("%10d. %08x%08x: %08x (%s) ", ll->freq, loc.h, loc.l, inst,
                  info[op].name);
        }
        ;
        if (lhs[0] == '!')
          printf ("%s instruction!\n", lhs + 1);                     /* 131 */
        else {
          if (L != old_L && !(f & push_pop_bit))
            printf ("rL=%d, ", L);                                   /* 132 */
          ;
          if (z.l == 0 && (op == ADDUI || op == ORI))
            p = "%l = %y = %#x";
          else
            p = info[op].trace_format;
          for (; *p; p++) {                                          /* 133 */
            if (*p != '%')
              fputc (*p, stdout);
            else {
              style = decimal;
            char_switch:switch (*++p) {
              case '#':
                style = hex;
                goto char_switch;                                    /* 134 */
              case '0':
                style = zhex;
                goto char_switch;
              case '.':
                style = floating;
                goto char_switch;
              case '!':
                style = handle;
                goto char_switch;
              case 'a':
                trace_print (a);
                break;                                               /* 136 */
              case 'b':
                trace_print (b);
                break;
              case 'p':
                trace_print (ma);
                break;
              case 'q':
                trace_print (mb);
                break;
              case 'w':
                trace_print (w);
                break;
              case 'x':
                trace_print (x);
                break;
              case 'y':
                trace_print (y);
                break;
              case 'z':
                trace_print (z);
                break;
              case '(':
                fputc (left_paren[round_mode], stdout);
                break;                                               /* 138 */
              case ')':
                fputc (right_paren[round_mode], stdout);
                break;
              case 't':
                if (x.l)
                  printf (" Yes, -> #"), print_hex (inst_ptr);
                else
                  printf (" No");
                break;
              case 'g':
                if (!good)
                  printf (" (bad guess)");
                break;
              case 's':
                printf (special_name[zz]);
                break;
              case '?':
                p++;
                if (z.l)
                  printf ("%c%d", *p, z.l);
                break;
              case 'l':
                printf (lhs);
                break;
              case 'r':
                p = switchable_string;
                break;
                ;
              default:
                printf ("BUG!!");
              }
            }
          }
          ;
          if (exc)
            printf (", rA=#%05x", g[rA].l);
          if (tripping)
            tripping = false, printf (", -> #%02x", inst_ptr.l);
          printf ("\n");
        }
        ;
        if (showing_stats || breakpoint)
          show_stats (breakpoint);
        just_traced = true;
      }
      else if (just_traced) {
        printf (" ...............................................\n");
        just_traced = false;
        shown_line = -gap - 1;
      }
      ;
      if (resuming && op != RESUME)
        resuming = false;
    }
    while ((!interrupt && !breakpoint) || resuming);
    if (interact_after_break)
      interacting = true, interact_after_break = false;
  }
end_simulation:if (profiling) {                                     /*  53 */
    printf ("\nProgram profile:\n");
    shown_file = cur_file = -1;
    shown_line = cur_line = 0;
    gap = profile_gap;
    showing_source = profile_showing_source;
    implied_loc = neg_one;
    print_freqs (mem_root);
  }
  ;
  if (interacting || profiling || showing_stats)
    show_stats (true);
  vram_fini ();                                                      /* 168 */
  ;
  return g[255].l;
}
