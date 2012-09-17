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

#include <string.h>
#include "mspio.h"

		
// Memory-mapped registers	
static unsigned char regs[MEMSIZE];
static int memsize = MEMSIZE;

char version[] = "1.0";
char howto[] = "Simulator for MSP430 digital i/o port\n";

// The address of the output device
uint64_t output_address;

// vmb interface
static device_info vmb = {0};

static void mem_clean(void)
/* set the initial conditions */
{ int i;
  for (i=0;i<MEMSIZE;i++)
    regs[i] = 0;
}

void device_poweron(void)
{  mem_clean();
}


void device_reset(void)
{ 
	// Send off to all leds
	regs[REG_OUT] = (-1) & 0xFF;
	sendChanges(0);
	// Set to reset conditions
	mem_clean();
}


unsigned char *device_get_payload(unsigned int offset,int size)
/* this function is called if some other device on the virtual bus
   wants to read size byte from this device at the given offset.
   offset and size are checked to fall completely within the
   address space ocupied by this device
   The function must return a pointer to the requested bytes.
*/
{ return regs+offset;
}

/*
	Sends signals on state change
*/
void sendChanges(unsigned char payload) {
	unsigned char mask, sendTo, signal;
	int i;
	sendTo = payload^regs[REG_OUT];
	// If the state (level) of any output ports is changing, 
	// a signal will be sent to the corresponding device
	if (sendTo) {
		regs[REG_OUT] = payload;
		mask = 1;
		for (i=0;i<8;i++) {
			// Check if the channel is switched to out and
			// there is a change in the output level
			if ((sendTo & mask) & (regs[REG_DIR] & mask)) {
				data_address da;
				signal = ((regs[REG_OUT] & mask) != 0);
				vmb_init_data_address(&da, 1);
				da.address_hi = HI32(output_address+i);
				da.address_lo = LO32(output_address+i);
				da.data = &signal;
				vmb_store(&vmb, &da);
			}
			mask <<= 1;
		}
	}
}

void device_put_payload(unsigned int offset,int size, unsigned char *payload)
{  
	// Check if the write access is bytewise
	// and the read-only register is not accessed
	if (size == 1 && offset != 0) {
		if (offset == REG_OUT) {
			sendChanges(*payload);
		} else {
			memmove(regs+offset,payload,size);
		}
	}
}

void initVMBInterface() {
	/* the vmb library uses debuging output, that we need to switch on if we
	want to see it. It will use the variable vmb_program_name to mark the output
	as comming from this program.*/
	vmb_begin();
	vmb_debug_flag = 0;
	vmb_program_name = "Digital I/O for MSP430";
	vmb.poweron=device_poweron;
	vmb.poweroff=vmb_poweroff; /* use default */
	vmb.reset=device_reset;
	vmb.get_payload=device_get_payload;
	vmb.put_payload=device_put_payload;
	vmb.terminate=vmb_terminate; /* use default */

	vmb_connect(&vmb,host,port); 
	vmb_register(&vmb,HI32(vmb_address),LO32(vmb_address),memsize,0,0,vmb_program_name);
	atexit(vmb_atexit);
	vmb_debug_on();
}

static void vmb_atexit(void)
{ 
	vmb_disconnect(&vmb);
	vmb_end();
}

int wait_for_power(void)
{
	fprintf(stderr,"Power...");
	while(!vmb.power)
	{
		vmb_wait_for_power(&vmb);
		if (!vmb.connected) return FALSE;
	}
	fprintf(stderr,"ON\n");
	return TRUE;
}

int main(int argc, char *argv[])
{	
	// Init core and start execution
	initVMBInterface();
 boot:
	if (!wait_for_power())
		goto end_simulation;
	device_reset();

	
	while (vmb.connected) {
		if (!vmb.power || vmb.reset_flag)
		{  /* breakpoint ?*/
		  vmb.reset_flag = 0;
		  goto boot;
		}
		vmb_wait_for_disconnect(&vmb);
	}
end_simulation:
	return 0;
}

