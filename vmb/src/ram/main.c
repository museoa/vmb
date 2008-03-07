/*
    Copyright 2005 Alexander Ukas, Martin Ruckert
    
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "param.h"
#include "bus-arith.h"
#include "vmb.h"
#include "option.h"
#include "param.h"


int main(int argc, char *argv[])
{
 param_init(argc, argv);
 vmb_debugs("%s ",vmb_program_name);
 vmb_debugs("%s ", version);
 vmb_debugs("host: %s ",host);
 vmb_debugi("port: %d ",port);
 close(0); /* stdin */
 ram_clean();
 hextochar(hexaddress,address,8);
 add_offset(address,size,limit);
 vmb_debugs("address: %s ",hexaddress);
 vmb_debugi("size: %x ",size);
 
 vmb_connect(host,port); 

 vmb_register(chartoint(address),chartoint(address+4),
              size, 0, 0, vmb_program_name);
 vmb_wait_for_disconnect();
 return 0;
}
