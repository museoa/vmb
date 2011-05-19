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

#ifndef GDB_H
#define GDB_H

#ifdef WIN32
#include <windows.h>
#endif


#include "signals.h"
#include "breaks.h"

/* this is taken from tm-mmix.h in gdb/config/mmix/ it must agree with the values given there
 */

#define NUM_REGS (32 + 1 + 256)
/*   Number of machine registers */

#define REGISTER_SIZE 8
/* the number of bytes per register */


#define	PBUFSIZ ((REGISTER_SIZE * NUM_REGS * 2) + 32)

/* special register numbers see regcache */
#define RET_REGNUM 4
#define PC_REGNUM 32
/* the framepointer is in $254 and the memory stackpointer is in $rO */
#define FP_REGNUM 34  
#define SP_REGNUM 10   


/* This should be a 64 bit integer type */

#ifdef WIN32

typedef LONGLONG CORE_ADDR;

#else
typedef long long CORE_ADDR;
#endif

/*Some constants for register access*/
#define BYTE_PER_REGISTER 8
#define SPECIAL_REGISTERS 32

/* Functions from remote-utils.c */

extern int gdb_init(int port);
extern int putpkt (char *buf);
extern int getpkt (char *buf);
extern int dual_wait(int s1, int s2);
extern void single_wait(int s);

extern char *get_free_buffer(void);
extern void put_free_buffer(char *buffer);
extern char *get_gdb_command(void);

/* from mmix-sim.c */

extern bool breakpoint;
extern bool stepping; /* should we pause after the next instruction? */

extern int gdb_signal; /* signal to send to GDB */
extern int gdbport; /* port for the remot gdb to connect, with default */
extern int mmgetchars(unsigned char *buf, int size, octa addr, int stop);

/* from gdb.c */

extern int async_gdb_command(char *buffer);
extern int interact_with_gdb(int signal);
extern int handle_gdb_commands(void);

#endif
