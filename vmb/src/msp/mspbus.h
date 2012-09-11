/*
	mspbus functions provide the connectivity to virtual
	msp430 peripherals such as RAM, ROM, DMA controller, multiplier, watchdog
	and other timers.
	msp430 address space is 16 bit, ranging from 0h
	to 0FFFFh (20 bit implementations are not yet supported).
	
	Layout

	0x0000 - 0x0007: interrupt control registers
	0x0008 - 0x00FF: 8-bit peripheral (accessed by byte instructions)
	0x0100 - 0x01FF: 16-bit peripherals (accessed by wyde operations)
	0x0200 - 0x09FF: Up to 2K RAM are mapped here
	0x0C00 - 0x0FFF: bootstrap loader flash ROM (1K)
	0x1000 - 0x10FF: data flash ROM (256 bytes)
	0x1100 - 0x38FF: extended RAM (or a copy of lower RAM address space, 2K, upper border varies)
	0x1100 - 0xFFFF: Up to 60KB programm ROM, smaller ROMs start at higher addresses
	0xFFE0 - 0xFFFF: Interrupt vector table (16 words, might be 32 words)

	Depending on the simulated msp430 device and the peripherals must be configured
	accordingly. The read and write requests will be issued over the vmb bus.
	Address decoding must be provided by the peripheral devices themself.
*/

#ifndef MSP_BUS
#define MSP_BUS
#include "vmb.h"		// Datatypes

extern device_info vmb;

// Read and write access functions provide access to RAM, ROM and device memory
// The function block the execution till the byte/word was delivered by bus
extern int vmbReadByteAt(UINT16 msp_address, UINT8* readInto);
extern int vmbReadWordAt(UINT16 msp_address, UINT16* readInto);
extern int vmbWriteByteAt(UINT16 msp_address, UINT8* writeFrom);
extern int vmbWriteWordAt(UINT16 msp_address, UINT16* writeFrom);

extern void initVMBInterface();
extern void wait_for_disconnect();
extern void init_device(device_info *vmb);
extern void device_poweron(void);
extern void device_reset(void);
extern unsigned char *device_get_payload(unsigned int offset,int size);
extern void device_put_payload(unsigned int offset,int size, unsigned char *payload);

extern int wait_for_power(void);
#endif
