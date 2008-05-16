/*
    Copyright 2007 Martin Ruckert
    
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

#ifndef VMB_H
#define VMB_H

/* Functions and Varaibles called by the CPU thread */
extern unsigned int vmb_connected;
extern unsigned int vmb_power;
extern unsigned int vmb_reset_flag;

extern void vmb_connect(char *host, int port);
extern void vmb_register(unsigned int adress_hi, unsigned int address_lo,
                         unsigned int size,
                         unsigned int lo_mask, unsigned int hi_mask,
                         char *name);
extern void vmb_wait_for_disconnect(void);
extern void vmb_disconnect(void);
extern int vmb_get_interrupt(unsigned int *hi, unsigned int *lo);
extern void vmb_raise_interrupt(unsigned char interrupt);
extern void vmb_raise_reset(void);
extern void vmb_wait_for_event(void);
extern void vmb_cancel_wait_for_event(void);
extern void vmb_wait_for_power(void);


typedef struct {
  unsigned char *data;
  int address_hi; 
  int address_lo;
  int size;
  int status;  
} data_address;

#define STATUS_VALID 0
#define STATUS_INVALID 1
#define STATUS_WRITING 2
#define STATUS_READING 4

extern void vmb_init_data_address(data_address *da, int size);

extern void vmb_load(data_address *da);
extern void vmb_wait_for_valid(data_address *da);
extern void vmb_cancel_all_loads(void);
extern void vmb_store(data_address *da);

#include "cache.h"

/* Functions called by the Bus-Read thread */
extern void vmb_poweron(void);
extern void vmb_poweroff(void);
extern void vmb_reset(void);
extern void vmb_terminate(void);
extern void vmb_disconnected(void);

extern void vmb_interrupt(unsigned char interrupt);
extern unsigned char *vmb_get_payload(unsigned int offset,int size);
extern void vmb_put_payload(unsigned int offset,int size, unsigned char *payload);
extern void vmb_unknown(unsigned char type,
                        unsigned char size,
                        unsigned char slot,
                        unsigned char id,
                        unsigned int offset,
			unsigned char *payload);

/* Functions called by the Bus Write thread */

#include "error.h"

#endif
