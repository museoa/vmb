#include "mspbus.h"
#include "mspcore.h"
//#include "vmb.h"
//#include <Windows.h>

device_info vmb = {0};
//HINSTANCE hInst;
//HWND hMainWnd;
//HBITMAP hBmp=NULL;
//HMENU hMenu;
//HBITMAP hon,hoff,hconnect;

static void vmb_atexit(void)
{ vmb_disconnect(&vmb);
  vmb_end();
}

// Translates the address in lower ram to extended ram,
// if translate_ram_to_upper is set
msp_word translate_address(msp_word address) {
	msp_word result;
	if (translate_ram_to_upper && (address.asWord >= 0x200) && (address.asWord <= 0x9FF))
		result.asWord = address.asWord + 0xF00;
	else
		result.asWord = address.asWord;
	return result;
}

void initVMBInterface() {
  vmb_begin();
  vmb_debug_flag = 0;
  vmb_program_name = "MSP430";
  vmb.reset=&initCore;
  vmb_connect(&vmb,host,port); 
  vmb_register(&vmb,HI32(vmb_address),LO32(vmb_address),0,-1,-1,vmb_program_name);
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
	fprintf(stderr," ON\n");
	return TRUE;
}



int vmbReadByteAt(msp_word msp_address, UINT8* readInto) {
	data_address da;
	vmb_init_data_address(&da, 1);
	da.address_hi = 0x0;
	da.address_lo = translate_address(msp_address).asWord;
	vmb_load(&vmb, &da);
	while (da.status != STATUS_VALID)
		vmb_wait_for_valid(&vmb, &da);
	*readInto = *(UINT8*)da.data;
	clocks++;
	return TRUE;
}

int vmbReadWordAt(msp_word msp_address, UINT16* readInto) {
	data_address da;
	vmb_init_data_address(&da, 2);
	da.address_hi = 0x0;
	da.address_lo = translate_address(msp_address).asWord;
	vmb_load(&vmb, &da);
	while (da.status != STATUS_VALID)
		vmb_wait_for_valid(&vmb, &da);
	*readInto = *(UINT16*)da.data;
	clocks++;
	return TRUE;
}

int vmbWriteByteAt(msp_word msp_address, UINT8* writeFrom) {
	data_address da;
	vmb_init_data_address(&da, 1);
	da.address_hi = 0x0;
	da.address_lo = translate_address(msp_address).asWord;
	da.data = (unsigned char*)writeFrom;
	vmb_store(&vmb, &da);
	clocks++;
	return TRUE;
}

int vmbWriteWordAt(msp_word msp_address, UINT16* writeFrom) {
	data_address da;
	vmb_init_data_address(&da, 2);
	da.address_hi = 0x0;
	da.address_lo = translate_address(msp_address).asWord;
	da.data = (unsigned char*)writeFrom;
	vmb_store(&vmb, &da);
	clocks++;
	return TRUE;
}


