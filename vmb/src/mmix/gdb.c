/*
    Copyright 2005 lothar Kaiser, Martin Ruckert
    
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

#undef DEBUG 


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <signal.h>
#include "vmb.h"
#include "mmix-internals.h"
#include "mmix-bus.h"
#include "gdb.h"
#include "bus-arith.h"
#include "breaks.h"
#include "message.h"
#include "address.h"


extern device_info vmb;
int gdbport = 2331;

static int answer_expected = 1;
static int break_level = 0;
int async_gdb_command(char *gdb_command)
/* somme commands are handled by the read loop
   independent of the simulator. In this case this functions
   takes necessary actions and returns true, else it returns false.
*/  
{ if (*gdb_command == 3 ) /* break */
  { breakpoint = true;
    break_level++;
    vmb_cancel_wait_for_event(&vmb);
    if (break_level>1)
      vmb_cancel_all_loads();
    answer_expected=1;
#ifdef DEBUG
    fprintf(stderr, "Async Break Level %d received\n",break_level);
#endif
    return 1;
  }
 return 0;
}


/* now functions for the simulator to interact with gdb */

static void
octatohex (octa *from, char *to, int n)
{
    while (n--)
    {
	inttohex(from->h, to);
	to += 8;

	inttohex(from->l, to);
	to += 8;

	from++;
    }
}


static char *hextoocta(char *from, octa *to, int n)
     /* puts the value of the hex string into *to.
        use at most 16 hex digits
        repeates for n octas.
        returns a pointer past the end of the hex string.
     */
{ 
  while (n--)
    { 
      int k=16;
      to->l=to->h=0;
      while(k-->0 && isxdigit(*from)){
	int i = fromhex(*from++);
	to->h = to->h << 4;
	to->h |= (to->l >> (sizeof(to->l)*8-4)) & 0xF;
	to->l = to->l << 4;
	to->l |= i & 0xF;
      }
      to++;
    }
  return from;
}




/* messages to return to gdb */

static void OK_msg(char *buffer)
{
  buffer[0]='O';
  buffer[1]='K';
  buffer[2]= 0;
}


int gdb_signal =  TARGET_SIGNAL_TRAP;

#define rBB 7

static void exit_msg(char *buffer)
{ buffer[0]='W';
  buffer[1]= tohex((g[rBB].l>>4)&0x0F);
  buffer[2]= tohex(g[rBB].l&0x0F);
  buffer[3]=0;  
}

void gdb_exit(char *buffer)
{ if (buffer==NULL) buffer=get_free_buffer();
  if (buffer==NULL) 
  { perror("Unable to get free gdb buffer");
    return;
  }
  exit_msg(buffer);
  putpkt (buffer);
}

static void termination_msg(char *buffer)
{ char *p;
  unsigned char n;
  unsigned char sig;
  if (gdb_signal>=0)
    sig=gdb_signal; 
  else 
    { exit_msg(buffer);
      return;
    }
  buffer[0]='T'; /* terminated */
  chartohex(&sig,buffer+1,1);
  p=buffer+3;
  n =  PC_REGNUM;
  chartohex(&n,p,1);
  p[2]=':';
  octatohex(&inst_ptr,p+3, 1);
  p[3+16]=';';
  p=p+3+16+1;
  n =  RET_REGNUM;
  chartohex(&n,p,1);
  p[2]=':';
  octatohex(&g[RET_REGNUM],p+3, 1);
  p[3+16]=';';
  p=p+3+16+1;
  n =  SP_REGNUM;
  chartohex(&n,p,1);
  p[2]=':';
  octatohex(&g[ SP_REGNUM],p+3, 1);
  p[3+16]=';';
  p=p+3+16+1;
  /*
  n =  FP_REGNUM;
  chartohex(&n,p,1);
  p[2]=':';
  octatohex(&g[254],p+3, 1);
  p[3+16]=';';
  p=p+3+16+1;
  */
  p[0]=0;
}

/* handling requests from gdb */


static void general_query(char *buffer)
{
  char *query = buffer+1;

  if (strcmp(query,"C")==0){
  /* Return the current thread id.

     Reply:

     QCpid

        Where pid is a HEX encoded 16 bit process id.
     *

     Any other reply implies the old pid.
     */
     OK_msg(buffer);
     return;
  }
  else if (strcmp(query,"Offsets")==0){
  /* query sect offs

     Get section offsets that the target used when re-locating the
     downloaded image. Note: while a Bss offset is included in the
     response, gdb ignores this and instead applies the Data offset to
     the Bss section.

     Reply:

     Text=xxx;Data=yyy;Bss=zzz
     */
    buffer[0]=0;
    return;
    }
  buffer[0]=0;
  return;
}





static void getRegisters(char *buffer)
{
	/*
	The structure of the m-Packet payload looks as follows:

	[ special-registers | program counter | global-registers | local-registers ]

	*/


	int rCount;
	unsigned int currentPosInBuffer = 0;


	/*the special registers*/
	octatohex(g, buffer+currentPosInBuffer, SPECIAL_REGISTERS);
	currentPosInBuffer = 2*BYTE_PER_REGISTER * SPECIAL_REGISTERS;

	/*the instruction pointer*/
	octatohex(&inst_ptr, buffer+currentPosInBuffer, 1);
	currentPosInBuffer += 2*BYTE_PER_REGISTER;


	for(rCount = 255; rCount >= G; rCount--)
	{
		octatohex(&(g[rCount]), buffer+currentPosInBuffer, 1);
		currentPosInBuffer += 2*BYTE_PER_REGISTER;
	}
        /* for now we send also marginal registers */
	for(; rCount >= L; rCount--)
	{
		memset(buffer+currentPosInBuffer,'0',2*BYTE_PER_REGISTER);
		currentPosInBuffer += 2*BYTE_PER_REGISTER;
	}

	for(; rCount >= 0; rCount--)
	{
		octatohex(&l[(O+rCount)&lring_mask], buffer+currentPosInBuffer, 1);
		currentPosInBuffer += 2*BYTE_PER_REGISTER;
	}
}

static void setRegisters(char *buffer)
{
	/*
	The structure of the M-Packet payload looks as follows:

	[ special-registers | program counter | global-registers | local-registers ]

	*/


	int rCount;
	unsigned int currentPosInBuffer = 1; //the position 0 contains the 'G' character

	/*the special registers*/
	hextoocta(buffer+currentPosInBuffer, g, SPECIAL_REGISTERS);
	currentPosInBuffer += 2*BYTE_PER_REGISTER * SPECIAL_REGISTERS;
	L=g[rL].l;
	G=g[rG].l;
	O=g[rO].l>>3;
        S=g[rS].l>>3;

	/*the program counter*/
	hextoocta(buffer+currentPosInBuffer, &inst_ptr, 1);
	currentPosInBuffer += 2*BYTE_PER_REGISTER;

	for(rCount = 255; rCount >= G; rCount--)
	{
		hextoocta(buffer+currentPosInBuffer, (octa*)&g[rCount], 1);
		currentPosInBuffer += 2*BYTE_PER_REGISTER;
	}
	/* skip marginal registers */
	for(; rCount >= L-1; rCount--)
 	  currentPosInBuffer += 2*BYTE_PER_REGISTER;

	for(; rCount >= 0; rCount--)
	{
		hextoocta(buffer+currentPosInBuffer,&l[(O+rCount)&lring_mask], 1);
		currentPosInBuffer += 2*BYTE_PER_REGISTER;
	}
	OK_msg(buffer);
}

static void setSingleRegister(char *buffer)
{

	int reg = 0;
	char *buffPtr = buffer;
	buffPtr++;


	buffPtr = hextoint(buffPtr, &reg);
	buffPtr++;

	if(reg<32){
		hextoocta (buffPtr, (octa*)&g[reg], 1);
		L=g[rL].l;
		G=g[rG].l;
		O=g[rO].l>>3;
		S=g[rS].l>>3;
	}
	if (reg == 32){
		hextoocta(buffPtr, (octa*)&inst_ptr, 1);
	}
	if (reg > 32){
		reg = 255-(reg-SPECIAL_REGISTERS-1);
		if(reg >= G){
			hextoocta (buffPtr, (octa*)&g[reg], 1);
		}
		if(reg < L){
			hextoocta(buffPtr,&l[(O+reg)&lring_mask], 1);
		}
		if(reg < G  && reg >= L){
			printf("Setting of marginal register %d<=%d<%d not yet supported!\n",
L,reg,G);
		}
	}
	OK_msg(buffer);
}


static void getSingleRegister(char *buffer)
{
	int reg = 0;
	char *buffPtr = buffer;
	octa emptyOcta;
	buffPtr++;

	buffPtr = hextoint(buffPtr, &reg);
	buffPtr++;

	if(reg<32){
		octatohex ((octa*)&g[reg], buffer, 1);
	}
	if (reg == 32){
		octatohex (&inst_ptr, buffer, 1);
	}
	if (reg > 32){
		reg = 255-(reg-SPECIAL_REGISTERS-1);
		if(reg >= G){
			octatohex ((octa*)&g[reg], buffer, 1);
		}
		if(reg < L){
			octatohex (&l[(O+reg)&lring_mask], buffer, 1);
		}
		if(reg < G  && reg >= L){
			emptyOcta.l = 0;
			emptyOcta.h = 0;
			octatohex (&emptyOcta, buffer, 1);
		}
	}
	buffer[2*BYTE_PER_REGISTER] = 0; //Terminate the buffer by a trailing \0
}

extern octa ominus(octa x, octa y);
extern octa incr(octa x, int d);
extern int lring_mask;

static int ocmp(octa x, octa y)
{ octa d;
  d = ominus(x,y);
  if (d.h&0x80000000) return -1;
  else if (d.h == 0 && d.l == 0) return 0;
  else return 1;
}
/* auxiliar function to read and write memory */



static int write_to_text = 0;
bool stepping = false;


/* gdb commands to read and write memory */

static void readMemory(char *buffer)
{     /* format maaaaa,nn  address aaaaa, bytes to read nnn */
	int bytesToRead = 0;
	unsigned char tmpBuffer[PBUFSIZ/2];
	octa srcAddr;

	hextoint(hextoocta(buffer+1, &srcAddr,1)+1,&bytesToRead);

        /* bytes in the range rS <= srcAddr < rO
           are actually in the register Stack, but we pretend
           they are in memory */

        if (ocmp(g[rS],srcAddr)<=0 && ocmp(incr(srcAddr,bytesToRead),g[rO])<0)
	  { int d = ominus(g[rO],srcAddr).l;
	    int dreg = (d+7)/8;
            int u = ominus(g[rO],incr(srcAddr,bytesToRead)).l;
	    int ureg = u/8;

            int i;
	    for (i=-dreg; i< -ureg; i++)
	    { inttochar(l[(O+i)&lring_mask].h, tmpBuffer+(dreg+i)*8);
              inttochar(l[(O+i)&lring_mask].l, tmpBuffer+(dreg+i)*8+4);
            }
            chartohex(tmpBuffer+(255*8-d)%8,buffer,bytesToRead);
	  }
        else if (ocmp(g[rO],srcAddr)<=0 || ocmp(incr(srcAddr,bytesToRead),g[rS])<0)
          { mmgetchars(tmpBuffer, bytesToRead, srcAddr, -1);
            chartohex(tmpBuffer,buffer,bytesToRead);
	  }
        else if (ocmp(g[rS],srcAddr)<=0)
          { int d = ominus(g[rO],srcAddr).l;
	    int dreg = (d+7)/8;
            int i;
            mmgetchars(tmpBuffer, bytesToRead, srcAddr, -1);
	    for (i=-dreg; i< 0; i++)
	    { inttochar(l[(O+i)&lring_mask].h, tmpBuffer+(dreg+i)*8);
              inttochar(l[(O+i)&lring_mask].l, tmpBuffer+(dreg+i)*8+4);
            }
            chartohex(tmpBuffer,buffer,bytesToRead);
	  }
        else if (ocmp(incr(srcAddr,bytesToRead),g[rO])<0)
          { int d = ominus(g[rO],g[rS]).l;
	    int dreg = (d+7)/8;
            int u = ominus(g[rO],incr(srcAddr,bytesToRead)).l;
	    int ureg = u/8;
            int i;
            mmgetchars(tmpBuffer, bytesToRead, srcAddr, -1);
	    for (i=-dreg; i< -ureg; i++)
	    { inttochar(l[(O+i)&lring_mask].h, tmpBuffer+(dreg+i)*8);
              inttochar(l[(O+i)&lring_mask].l, tmpBuffer+(dreg+i)*8+4);
            }
            chartohex(tmpBuffer,buffer,bytesToRead);
	  }
        else
          { int d = ominus(g[rO],g[rS]).l;
	    int dreg = (d+7)/8;
            int i;
            mmgetchars(tmpBuffer, bytesToRead, srcAddr, -1);
	    for (i=-dreg; i< 0; i++)
	    { inttochar(l[(O+i)&lring_mask].h, tmpBuffer+(dreg+i)*8);
              inttochar(l[(O+i)&lring_mask].l, tmpBuffer+(dreg+i)*8+4);
            }
            chartohex(tmpBuffer,buffer,bytesToRead);
	  }
}


static void writeMemory(char *buffer)
{   /* format Maaaaa,nn  address aaaaa, bytes to read nnn */
	int bytesToWrite = 0;
	unsigned char tmpBuffer[PBUFSIZ/2];
	octa dstAddr;
	char *buffPtr = buffer;

	buffPtr=hextoint(hextoocta(buffer+1, &dstAddr,1)+1,&bytesToWrite);
        hextochar(buffPtr+1,tmpBuffer,bytesToWrite);

        if ((dstAddr.h&0xe0000000) == 0) write_to_text = 1;
	mmputchars(tmpBuffer, bytesToWrite, dstAddr);
	OK_msg(buffer);
}

static void remove_escape(unsigned char * from, int size)
{ unsigned char * to = from;
  while (size-- > 0)
  { if (*from != 0x7D)
        *to++ = *from++;
    else
    {    *to++ = *(from+1) ^ 0x20;
         from = from+2; 
    }
  }
}

static void write_binary_memory(char *buffer)
	/*
	  Xaddr,length:XX\x{2026} -- write mem (binary)

	  addr is address, length is number of bytes, XX\x{2026} is
	  binary data. The characters $, #, and 0x7d are escaped using
	  0x7d.

	  Reply:

	  OK

	  for success
	  ENN

	  for an error
	*/
{
	int bytesToWrite = 0;
	octa dstAddr;
	char *buffPtr;
  
        buffPtr = buffer;

	buffPtr=hextoint(hextoocta(buffer+1, &dstAddr,1)+1,&bytesToWrite);
        remove_escape((unsigned char *)buffPtr+1, bytesToWrite);
        if ((dstAddr.h&0xe0000000) == 0) write_to_text = 1;
	mmputchars((unsigned char *)buffPtr+1, bytesToWrite, dstAddr);
	OK_msg(buffer);
}

static void removeBreakPoint(char *buffer)
{
	char sep = ',';
	char *buffPtr = buffer;
	octa dstAddr;


	do{
		buffPtr++;
	}while(*buffPtr != sep);
	buffPtr++; //now we point at the address to remove the breakpoint

	//buffPtr first points on the destination address
	buffPtr = hextoocta(buffPtr, &dstAddr,1);
	set_break(dstAddr, 0);
	OK_msg(buffer);
}


static void setBreakPoint(char *buffer)
{
	char sep = ',';
	char *buffPtr = buffer;
	octa dstAddr;
	
	do{
		buffPtr++;
	}while(*buffPtr != sep);
	buffPtr++; //now we point at the address to set the breakpoint

	//buffPtr first points on the destination address
	buffPtr = hextoocta(buffPtr, &dstAddr,1);

	switch(buffer[1]){
		case '2':
        	set_break(dstAddr, write_bit);
		break;
		case '3':
        	set_break(dstAddr, read_bit);
		break;
      		default:
           	set_break(dstAddr, exec_bit);
		break;
	}
	OK_msg(buffer);
}

octa save_rQ, save_newrQ;

int handle_gdb_commands(void)
/* the command is in the buffer,
   the command is handled and the answer is supplied 
   to gdb. If the simulator should continue,
   the function exits.
*/    
{ char * buffer = NULL;
  while (1)
  { buffer = get_gdb_command();
    switch(buffer[0]) {
    case '+':
       put_free_buffer(buffer);
#ifdef DEBUG
       fprintf(stderr, "Ignoring + command\n");
#endif
       continue;
    case '-':
       put_free_buffer(buffer);
#ifdef DEBUG
       fprintf(stderr, "Ignoring - command\n");
#endif
       continue;
     case 'B':
	/* Baddr,mode -- set breakpoint (deprecated)
 	   Set (mode is S) or clear (mode is C) a breakpoint at addr.
	*/
       break;
     case 'C':
	  /* continue with signal, addr  (signal, addr ignored)*/
     case 'c':
          /* continue with the simulator addr ignored */
       put_free_buffer(buffer);
       answer_expected = 1;
       return 1;
     case 's':
       breakpoint = true;
       put_free_buffer(buffer);
       answer_expected = 1;
       return 1;
     case 'g':
       getRegisters(buffer);
       break;
     case 'G':
       setRegisters(buffer);
       break;
     case 'm':
       readMemory(buffer);
       break;
     case 'M':
       writeMemory(buffer);
       break;
     case 'k':
       breakpoint = true;
       put_free_buffer(buffer);
       answer_expected = 1;
       return 0;
     case '?':
         termination_msg(buffer);
       break;
#if 1
       /* disabling p and P command will force gdb to use the more efficient 
          (at least for multiple registers) g and G command 
       */
     case 'p':
	/* pn... -- read reg (reserved)
 	  Reply:
	  r....
	  The hex encoded value of the register in target byte order.
	*/
        getSingleRegister(buffer);
	break;
     case 'P':
	/* Pn\x{2026}=r\x{2026} -- write register
	  Write register n\x{2026} with value r\x{2026}, which
	  contains two hex digits for each byte in the register
	  (target byte order).
	  Reply:
	  OK
	  for success
	  ENN
	  for an error
	*/
       setSingleRegister(buffer);
       break;
#endif
     case 'H':
	/* Hct
           Set thread for subsequent operations (m, M, g, G,
	   et.al.). c depends on the operation to be performed: it
	   should be c for step and continue operations, g for other
	   operations. The thread designator t... may be -1, meaning
	   all the threads, a thread number, or zero which means pick
	   any thread.
	   Reply: OK   for success
	          ENN  for an error */
       OK_msg(buffer);
       break;
     case 'q':
       general_query(buffer);
       break;
     case 'X':
       write_binary_memory(buffer);
       OK_msg(buffer);
       break;
	/*
	  Xaddr,length:XX\x{2026} -- write mem (binary)

	  addr is address, length is number of bytes, XX\x{2026} is
	  binary data. The characters $, #, and 0x7d are escaped using
	  0x7d.

	  Reply:

	  OK

	  for success
	  ENN

	  for an error
	*/
     case 'z':
       removeBreakPoint(buffer);
       break;
     case 'Z':
       setBreakPoint(buffer);
       break;
     default:
       buffer[0]=0;
       break;
   }
   putpkt (buffer);
   put_free_buffer(buffer);
 }
 return 1;
}


int interact_with_gdb(int signal)
  /* this function should be called when the simulator stops */
{ int r;
  break_level=0;
  if (answer_expected)
  { char buffer[PBUFSIZ];
    gdb_signal = signal;
    termination_msg(buffer);
    putpkt (buffer);
    answer_expected = 0;
  }
  save_rQ = g[rQ];   /* debuger actions by default do not change rQ */
  save_newrQ = new_Q;
  write_to_text = 0;
  r = handle_gdb_commands();
  g[rQ] = save_rQ;
  new_Q = save_newrQ;
  if (write_to_text) {
    write_all_data_cache();
    clear_all_instruction_cache();
  }
  return r;
}
