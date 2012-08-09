#include "mspbus.h"
#include "mspcore.h"
#include "vmb.h"

device_info vmb = {0};

void initVMBInterface() {
	/* the vmb library uses debuging output, that we need to switch on if we
	want to see it. It will use the variable vmb_program_name to mark the output
	as comming from this program.*/
  vmb_debug_on();
  vmb_program_name = "MSP430";

  //init_device(&vmb);
	
  /* establish a connection to the virtual bus on the localhost
     on port 9002. This port is the default port for the virtual bus. */
  vmb_connect(&vmb,"localhost",9002); 

  // WD: MCU will not receive any interrupts from other devices
  // RAM and ROM will answer to requests synchonous
  vmb_register(&vmb,0x00000000,0x00000000, /* start address, hi 32bit, lo 32 bit
                                         where the device is mapped. */
               0xF, /* the size of the mapped area 
					(meets the size of the special function registers @0x0) */
               0x00000000,0x00000000, /* the mask for the 64 interupt lines
                                         only interrups with the corresponding
                                         bit set will reach this device. 
										 (No interrupts will be accepted) */
               "MSP430");         /* the name of this device */

  
  // WD: Wait for motherboard to power on
  vmb_wait_for_power(&vmb);
}



void wait_for_disconnect() {
	vmb_wait_for_disconnect(&vmb);
}

void init_device(device_info *vmb)
{  
	vmb->poweron=device_poweron;
	vmb->poweroff=vmb_poweroff; /* use default */
	vmb->disconnected=vmb_disconnected;  /* use default */
	vmb->reset=&initCore;
	vmb->terminate=vmb_terminate; /* use default */
	vmb->get_payload=device_get_payload;
	vmb->put_payload=device_put_payload;
}

int vmbReadByteAt(UINT16 msp_address, UINT8* readInto) {
	data_address da;
	vmb_init_data_address(&da, 1);
	da.address_hi = 0x0;
	da.address_lo = msp_address;
	vmb_load(&vmb, &da);
	while (da.status != STATUS_VALID)
		vmb_wait_for_valid(&vmb, &da);
	*readInto = *(UINT8*)da.data;
	return TRUE;
}

int vmbReadWordAt(UINT16 msp_address, UINT16* readInto) {
	data_address da;
	vmb_init_data_address(&da, 2);
	da.address_hi = 0x0;
	da.address_lo = msp_address;
	vmb_load(&vmb, &da);
	while (da.status != STATUS_VALID)
		vmb_wait_for_valid(&vmb, &da);
	*readInto = *(UINT16*)da.data;
	return TRUE;
}

int vmbWriteByteAt(UINT16 msp_address, UINT8* writeFrom) {
	data_address da;
	vmb_init_data_address(&da, 1);
	da.address_hi = 0x0;
	da.address_lo = msp_address;
	da.data = (unsigned char*)writeFrom;
	vmb_store(&vmb, &da);
	return TRUE;
}

int vmbWriteWordAt(UINT16 msp_address, UINT16* writeFrom) {
	data_address da;
	vmb_init_data_address(&da, 2);
	da.address_hi = 0x0;
	da.address_lo = msp_address;
	da.data = (unsigned char*)writeFrom;
	vmb_store(&vmb, &da);
	return TRUE;
}

/* the next funtions are required callback functions
   for the vmb interface. They are called from threads
   distinct from the main thread. If these callbacks
   share resources with the main thread, it might be necessary
   to use a mutex to synchronize access to the resources.
   In this template, the main thread does nothing with
   the ram. Hence no synchronization is needed.
*/

void device_poweron(void)
/* this function is called when the virtual power is turned on */
{  
	initCore();
}


void device_reset(void)
/* this function is called when the virtual reset button is pressed */
{ 
	initCore();
}


unsigned char *device_get_payload(unsigned int offset,int size)
/* this function is called if some other device on the virtual bus
   wants to read size byte from this device at the given offset.
   offset and size are checked to fall completely within the
   address space ocupied by this device
   The function must return a pointer to the requested bytes.
*/
{ 
	// Returns a zero map of size bytes
	unsigned char *result = (unsigned char*)malloc(size);
	return result;
}

void device_put_payload(unsigned int offset,int size, unsigned char *payload)
/* this function is called if some other device on the virtual bus
   wants to write size byte to this device at the given offset.
   The new byte are contained in the payload.
   offset and size are checked to fall completely within the
   address space ocupied by this device.
*/
{  
	//memmove(ram+offset,payload,size);
	// do nothing
	return;
}