/*
    Copyright 2008  Martin Ruckert
    
    ruckertm@acm.org

    This file is part of the Virtual Motherboard project

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

/* the files maintemplate and devicetemplate
   is a template for a simple device, containing just what is
   necessary. Everyting else was omited.
   Its a good starting pont for complex devices and a complete
   reference to the functions needed and provided by the
   vmb library.
*/


#include "vmb.h"
#include "option.h"
#include "param.h"

#define RAMSIZE 8

static unsigned char ram[RAMSIZE];

static void ram_clean(void)
/* clean the ram, done after power on and reset */
{ int i;
  for (i=0;i<RAMSIZE;i++)
    ram[i] = 0;
}


void vmb_poweron(void)
/* this function is called when the virtual power is turned on */
{  ram_clean();
}

void vmb_reset(void)
/* this function is called when the virtual reset button is pressed */
{ ram_clean();
}

static long int ms;

void vmb_put_payload(unsigned int offset,int size, unsigned char *payload)
/* this function is called if some other device on the virtual bus
   wants to write size byte to this device at the given offset.
   The new byte are contained in the payload.
   offset and size are checked to fall completely within the
   address space ocupied by this device.
*/
{ 
   memmove(ram+offset,payload,size);
   /* protect ms by mutex and set up condition variable */
   ms = chartoint(ram)<<4;
   ms = ms + chartoint(ram+4);
}

#include <sys/select.h>
void sleep_ms(long int ms)
{ /* sleep the specified number of miliseconds */
  select();
}


int main(int argc, char *argv[])
{
 param_init(argc, argv);
 size = 8;
 vmb_debugs("%s ",vmb_program_name);
 vmb_debugs("%s ", version);
 vmb_debugs("host: %s ",host);
 vmb_debugi("port: %d ",port);
 close(0);
 vmb_debugi("address hi: %x",vmb_address_hi);
 vmb_debugi("address lo: %x",vmb_address_lo);
 vmb_debugi("size: %x ",vmb_size);

 vmb_connect(host,port); 
 vmb_register(vmb_address_hi,vmb_address_lo,vmb_size,
               0, 0, vmb_program_name);
 
 while (vmb_connected)
   { if (ms == 0) 
     { /* if timer value is zero, use condition variable to
            wait until timer value non zero */
     }
     else 
     { sleep_ms(ms);
     if (ms != 0) /* ther might have been a write in between */
         vmb_raise_interrupt(interrupt);  
     }
   }

 return 0;
}
