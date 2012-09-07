#include "mspbus.h"
#include "mspcore.h"
//#include "vmb.h"

device_info vmb = {0};

static void vmb_atexit(void)
{ vmb_disconnect(&vmb);
  vmb_end();
}

void initVMBInterface() {
	/* the vmb library uses debuging output, that we need to switch on if we
	want to see it. It will use the variable vmb_program_name to mark the output
	as comming from this program.*/
  vmb_begin();
  vmb_debug_flag = 0;
  vmb_program_name = "MSP430";
	vmb.reset=&initCore;

  vmb_connect(&vmb,"localhost",9002); 
  /* vmb_connect(&vmb,host, port); */
  vmb_register(&vmb,0,0,0,-1,-1,vmb_program_name);
  atexit(vmb_atexit);
  vmb_debug_on();
  
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


