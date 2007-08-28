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

/* param.h
 */

extern char *host;
extern int port;
extern int debugflag;
extern char *hexaddress;
extern unsigned char address[8];
extern unsigned char limit[8];
extern int interrupt;
extern char *filename;
extern unsigned int size;
#define MAX_EXEC 256
extern char *commands[MAX_EXEC];
extern void do_commands(void);
#ifdef WIN32
extern void param_init(void);
#else
extern void param_init(int argc, char *argv[]);
#endif
extern void load_configfile(char *name);

