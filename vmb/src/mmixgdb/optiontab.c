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

#include "option.h"
#include "param.h"
#include "vmb.h"

extern int interacting;
extern int show_operating_system;


option_spec options[] = {
/* description short long kind default handler */
{"the host where the bus is located", 'h',   "host",    "host",          str_arg, "localhost", {&host}},
{"the port where the bus is located",   'p', "port",    "port",          int_arg, "9002", {&port}},
{"the x position of the window",        'x', "x",       "x position",    int_arg, "0", {&x}},
{"the y position of the window",        'y', "y",       "y position",    int_arg, "0", {&y}},
{"to generate debug output",            'd', "debug",   "debugflag",     on_arg, NULL, {&vmb_debug_flag}},
{"to run interactively with gdb",       'i', "interacting",   "interactiveflag",     off_arg, NULL, {&interacting}},
{"to show operating system code",       's', "system",   "systemflag",     on_arg, NULL, {&show_operating_system}},
{"the verbosity level (0= all, 1= less, ...)",   'v', "verbosity",    "verbosity level", int_arg, "1", {&vmb_verbose_level}},
{"to define a name for conditionals",   'D', "define",  "conditional",   str_arg, NULL, {&defined}},
{"filename for a configuration file",    'c', "config", "file",          fun_arg, NULL, {parse_configfile}},
{"to print usage information",           '?', "help",   NULL,            fun_arg, NULL,{usage}},
{NULL}
};
