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

extern int colors[];
extern int nleds;

option_spec options[] = {
/* description short long kind default handler */
{"the host where the bus is located", 'h',   "host",    "host",          str_arg, "localhost", {&host}},
{"the port where the bus is located",   'p', "port",    "port",          int_arg, "9002", {&port}},
{"address where the resource is located",'a', "address", "hex address",  uint64_arg, "0x8000000000000000", {&vmb_address}},
{"the x position of the window",        'x', "x",       "x position",    int_arg, "0", {&xpos}},
{"the y position of the window",        'y', "y",       "y position",    int_arg, "0", {&ypos}},
{"the color of led 7",                   '7',"color7",  "RGB value",     int_arg, "0xFF8080", {&colors[0]}},
{"the color of led 6",                   '6',"color6",  "RGB value",     int_arg, "0x8080FF", {&colors[1]}},
{"the color of led 5",                   '5',"color5",  "RGB value",     int_arg, "0xFFFF00", {&colors[2]}},
{"the color of led 4",                   '4',"color4",  "RGB value",     int_arg, "0xFF00FF", {&colors[3]}},
{"the color of led 3",                   '3',"color3",  "RGB value",     int_arg, "0x00FFFF", {&colors[4]}},
{"the color of led 2",                   '2',"color2",  "RGB value",     int_arg, "0xFF0000", {&colors[5]}},
{"the color of led 1",                   '1',"color1",  "RGB value",     int_arg, "0x00FF00", {&colors[6]}},
{"the color of led 0",                    '0',"color0",  "RGB value",     int_arg, "0x0000FF", {&colors[7]}},
{"the number of leds to display",        'n',"leds",    "int value",     int_arg, "8", {&nleds}},
{"start with a minimized window",        'm', "minimized",       "minimizedflag",    on_arg, NULL, {&minimized}},
{"to generate debug output",            'd', "debug",   "debugflag",     on_arg, NULL, {&vmb_debug_flag}},
{"make debugging verbose",   'v', "verbose",    "verbose debugging", on_arg, NULL, {&vmb_verbose_flag}},
{"set the debug mask",                  'M', "debugmask", "hide debug output",   int_arg, "0xFFF0", {&vmb_debug_mask}},
{"to define a name for conditionals",   'D', "define",  "conditional",   str_arg, NULL, {&defined}},
{"filename for a configuration file",    'c', "config", "file",          fun_arg, NULL, {parse_configfile}},
{"to print usage information",           '?', "help",   NULL,            fun_arg, NULL,{usage}},
{NULL}
};
