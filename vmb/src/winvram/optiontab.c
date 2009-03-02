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

int width,height,framewidth,frameheight,zoom;

option_spec options[] = {
/* description short long kind default handler */
{"the host where the bus is located", 'h',   "host",    "host",          str_arg, "localhost", {&host}},
{"the port where the bus is located",   'p', "port",    "port",          int_arg, "9002", {&port}},
{"to generate debug output",            'd', "debug",   "debugflag",     on_arg, NULL, {&vmb_debug_flag}},
{"the verbosity level (0= all, 1= less, ...)",   'v', "verbosity",    "verbosity level", int_arg, "1", {&vmb_verbose_level}},
{"to define a name for conditionals",   'D', "define",  "conditional",   str_arg, NULL, {&defined}},
{"address whre the resource is located",'a', "address", "start address",   uint64_arg, "8000000000000000", {&vmb_address}},
{"size of address range in octas",      's', "size",    "size in octas", int_arg, "1", {&vmb_size}},
{"interrupt send by device",            'i', "interrupt", "interrupt number", int_arg, "8", {&interrupt}},
{"filename for input file",             'f', "file",    "file name",     str_arg, NULL, {&filename}},
{"the visible width",                   'w', "width",    "visible width",int_arg, "640", {&width}},
{"the visible height",                  'h', "height",    "visible height",int_arg, "480", {&height}},
{"the frame width",                     'q', "fwidth",    "frame width",int_arg, "640", {&framewidth}},
{"the frame height",                    'r', "fheight",    "frame height",int_arg, "480", {&frameheight}},
{"the zoom factor",                      'z', "zoom",    "zoom factor",int_arg, "1", {&zoom}},
{"command to execute",                   'x', "exec",    "command line",     fun_arg, NULL, {store_command}},
{"filename for a configuration file",    'c', "config", "file",          fun_arg, NULL, {parse_configfile}},
{"to print usage information",           '?', "help",   NULL,            fun_arg, NULL,{usage}},
{NULL}
};
