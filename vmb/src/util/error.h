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

#ifndef ERROR_H
#define ERROR_H

extern char *vmb_program_name;
extern unsigned int vmb_debug_flag;
extern void vmb_fatal_error(int line,char *msg); /* no return */
extern void vmb_message(char *msg);
extern void vmb_errormsg(char *msg);
extern void vmb_debug(char *msg);
extern void vmb_debugi(char *msg,int i);
extern void vmb_debugs(char *msg, char *s);
extern void vmb_debugx(char *msg, unsigned char *s, int n); /* message wit hex info */
extern void vmb_debugm(unsigned char mtype,unsigned char msize, 
                       unsigned char mslot,unsigned char mid,
		       unsigned char maddress[8], unsigned char *mpayload);


extern void (*vmb_message_hook)(char *msg);
extern void (*vmb_debug_hook)(char *msg);


#endif
