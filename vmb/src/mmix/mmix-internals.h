/*
    Copyright 2005 Martin Ruckert
    
    ruckertm@acm.org

    This file is part of the MMIX Motherboard project

    This file is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this software; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef INTERNALS_H
#define INTERNALS_H

typedef enum{false=0,true}bool;
typedef unsigned int tetra;
typedef struct{tetra h,l;} octa;

extern octa g[256]; /* global registers */
extern int G,L; /* accessible copies of key registers */
extern int O,S; /* accessible copies of key registers divided by 8*/
extern octa new_Q;
extern octa inst_ptr; /*pointer to next instruction*/
extern octa loc; /*instruction pointer */
extern octa *l; /* local registers */
extern int lring_size; /* the number of local registers (a power of 2) */
extern int lring_mask; /* one less than |lring_size| */
extern octa new_Q;

typedef enum{
rB,rD,rE,rH,rJ,rM,rR,rBB,
rC,rN,rO,rS,rI,rT,rTT,rK,rQ,rU,rV,rG,rL,
rA,rF,rP,rW,rX,rY,rZ,rWW,rXX,rYY,rZZ} special_reg;

#define trace_bit ((unsigned char)(1<<3))
#define read_bit  ((unsigned char)(1<<2))
#define write_bit ((unsigned char)(1<<1))
#define exec_bit  ((unsigned char)(1<<0))  



extern void mmputchars(unsigned char* buf, int size, octa dest);

extern int mmgetchars(unsigned char *buf, int size, octa addr, int stop);

extern octa shift_right(octa y,int s,int uns);

extern octa oplus(octa x, octa y);

/* Here are the bit codes that affect traps. The first eight
   cases apply to the upper half of~rQ. (program bits) 
*/

#define P_BIT (1<<0) /* instruction in privileged location */
#define S_BIT (1<<1) /* security violation */
#define B_BIT (1<<2) /* instruction breaks the rules */
#define K_BIT (1<<3) /* instruction for kernel only */
#define N_BIT (1<<4) /* virtual translation bypassed */
#define PX_BIT (1<<5) /* permission lacking to execute from page */
#define PW_BIT (1<<6) /* permission lacking to write on page */
#define PR_BIT (1<<7) /* permission lacking to read from page */

/* The next eight cases apply to the lower half of~rQ. (machine bits) 
*/
#define PF_BIT (1<<0) /* power fail */
#define MP_BIT (1<<1) /* memory parity error */
#define NM_BIT (1<<2) /* non existent memory */
#define YY_BIT (1<<3) /* unassigned */
#define RE_BIT (1<<4) /* rebooting */
#define CP_BIT (1<<5) /* continuation page used */
#define PT_BIT (1<<6) /* page table error */
#define IN_BIT (1<<7) /* interval counter rI reaches zero */

#endif
