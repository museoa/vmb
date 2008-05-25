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


#define trace_bit ((unsigned char)(1<<3))
#define read_bit  ((unsigned char)(1<<2))
#define write_bit ((unsigned char)(1<<1))
#define exec_bit  ((unsigned char)(1<<0))  

extern unsigned char get_break(octa a);
/* returns a byte with break information for the given address */

extern void set_break(octa a, unsigned char b);
/* sets the break information for a to b. */

extern void show_breaks(void);
/* display the breakpoints */