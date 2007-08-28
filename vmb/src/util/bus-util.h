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

#ifndef BUS_UTIL_H
#define BUS_UTIL_H


/* set an interrupt, return -1 on error
*/
extern int set_interrupt(int socket, int interrupt);

/* return a socket or -1 on error 
   specifies the address range by the start address and the size in octa
   specifies an interrupt mask, with a low mask (interrupts 0 to 31)
   and a high mask (interrupts 32 to 63).
   A bit is set if the component wants to be notified .
*/

extern int bus_poweron(int socket, int blocking);
/* wait until a power on message is received.
   every other message is an error */


extern int bus_power;
/* is zero before calling bus_poweron and after receiving
   a hard  power fail  */


extern int bus_reset;
/* is set to zero by most calls and is set to 1 after receiving
   a hard reset */


/* the next four functions are for active devices that read and write data,
   and once in a while check (mostly non blocking) for interrupts */

extern int store_bus_data(int socket, 
			  unsigned char address[8], unsigned char *data, int size);
extern int load_bus_data(int socket, 
			 unsigned char address[8], unsigned char *data, int size);

extern int get_interrupt(int socket, int blocking, 
			 unsigned int *hi, unsigned int *lo);

/* the next two functions are for passive devices that wait for
   requests --- either read, write, or interrupts --- and 
   may need to return an answer to a read request. */

extern int get_request(int socket, int blocking, unsigned char *slot,
   			   unsigned char address[8], int *size, unsigned char *payload);
/* returns -1 on error, 
   returns 0 if blocking was disabled and no message was received
   returns ID_IGNORE if you can ignore this message 
   returns ID_READ if a read request was received (muliple of 8 byte)
   returns ID_READBYTE if a read request was received (1 byte)
   returns ID_READWYDE if a read request was received (2 byte)
   returns ID_READTETRA if a read request was received (4 byte)
   returns ID_WRITE if a write request was received (muliple of 8 byte)
   returns ID_WRITEBYTE if a write request was received (1 byte)
   returns ID_WRITEWYDE if a write request was received (2 byte)
   returns ID_WRITETETRA if a write request was received (4 byte)
   returns ID_READREPLY if the answer to a ID_READ message was received 
   returns ID_BYTE/WYDE/TETRA/REPLY if the answer to a ID_READBYTE/WYDE/TETRA message was received 
   returns ID_NOREPLY if there will be no answer for a READ message.
   returns ID_INTERRUPT if an interrupt was received 

   returns ID_RESET if the virtual hardware reset button was pressed.
   returns ID_POWEROFF if the virtual power goes down.
   returns ID_POWERON if the virtual power goes on
*/
 
extern int dispatch_message(int id, unsigned char a[8], int s, unsigned char slot, unsigned char *p);
/* dispatches the message described by the parameters.
   depending on the id, it calls
    read_request(), write_request(), reply_payload(), process_reset()
    process_poweron(), process_poweroff() or process_interrupt()
*/

extern unsigned char *get_payload(unsigned int offset,int size);
extern void put_payload(unsigned int offset,int size, unsigned char *payload);
extern int reply_payload(unsigned char address[8],int size, unsigned char *payload);
extern int process_interrupt(unsigned char interrupt);
extern int process_reset(void);
extern int process_poweron(void);
extern int process_poweroff(void);


extern int answer_readrequest(int socket,  unsigned char slot,
   			   unsigned char address[8], int size, unsigned char *data);

/* both passive and active devices may raise interrupts using
   the next function */

extern int set_interrupt(int socket, int interrupt);


extern int bus_fd;




#endif
