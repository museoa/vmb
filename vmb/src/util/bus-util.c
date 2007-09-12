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

/* $Revision: 1.3 $ $Date: 2007-09-12 16:29:32 $ */


#include <stdlib.h>
#include <string.h>
#include "message.h"
#include "bus-arith.h"
#include "main.h"
#include "bus-util.h"




/* the simple high level interface for active components consists
   of the functions 
   bus_register
   wait_poweron   
   load/store_bus_data
   get/set_interrupt.
   bus_unregister

   It is based on the following assumptions:

   1. the first message received will be a power on message 
   2. A power fail interrupt will be received befor any additional
      power on message.
   3. for each message of TYPE_REQUEST, there will be an answer.
      either with ID_READREPLY or with ID_NOREPLY.
   4. the only messages that we will receive are answers after a request
      and interrupts.
   3. any other message we receive is an error.

   This interface is used as follows:
   1. We connect to the bus using bus_connect 
   2. We register with the bus using bus_register.
   2. We wait for the power on signal
   3. We enter a process loop, that may call store_bus_data
      or load_bus_octa, get_interrupt and set_interrupt.
      All of these functions are blocking, only get_interrupt
      has a choice of blocking or non blocking mode.
   4. If a power fail interrupt is received, we should exit the loop
      and call bus_poweron until it returns true.
   5. before the program exits it should call bus_unregister
   6. It should then call bus_disconnect to close the connection
*/


int bus_power=0;
int bus_reset=0;
int bus_fd = -1;

int bus_poweron(int socket, int blocking)
     /* wait or check for bus power */
{ unsigned char type;
  unsigned char size;
  unsigned char slot;
  unsigned char id;
  unsigned int  time;
  unsigned char address[8];
  unsigned char payload[MAXPAYLOAD];
  while (bus_connected && !bus_power)
  { int i;
    i=receive_msg(socket,blocking,&type,&size,&slot,&id,&time,address,payload);
    if (i>0) bus_power = (type&TYPE_BUS && id==ID_POWERON);
    else if (!blocking) return 0;
  }
  bus_reset = 0;
  return bus_power;
}

/* buffering of interrupts */
static unsigned int hi_interrupt=0, lo_interrupt=0;

int dispatch_message(int id, unsigned char a[8], int s, unsigned char slot, unsigned char *p)
{ switch (id)
    { case ID_IGNORE: 
        return 0;
      case ID_READ:
	return read_request(a, (s+1)*8, slot, p);
      case ID_READBYTE:
	return read_request(a, 1, slot, p);
      case ID_READWYDE:
	return read_request(a, 2, slot, p);
      case ID_READTETRA:
	return read_request(a, 4, slot, p);
      case ID_WRITE:
	return write_request(a, (s+1)*8, p);
      case ID_WRITEBYTE:
	return write_request(a, 1, p);
      case ID_WRITEWYDE:
	return write_request(a, 2, p);
      case ID_WRITETETRA:
	return write_request(a, 4, p);
      case ID_READREPLY:
	return reply_payload(a,(s+1)*8,p);
      case ID_BYTEREPLY:
	return reply_payload(a,1,p);
      case ID_WYDEREPLY:
	return reply_payload(a,2,p);
      case ID_TETRAREPLY:
	return reply_payload(a,4,p);
      case ID_NOREPLY:
	return reply_payload(a,0,p);
      case ID_RESET:
        bus_reset = 1;
	return process_reset();
      case ID_POWEROFF:
        bus_reset = 0;
        bus_power = 0;
	return process_poweroff();
      case ID_POWERON:
        bus_reset = 0;
        bus_power = 1;
	return process_poweron();
      case ID_INTERRUPT:
        if (slot<32) lo_interrupt |= BIT(slot);
        else hi_interrupt |= BIT(slot-32);
	return process_interrupt(slot);
      default:
	return 0;
      }
  return 0;
}

int store_bus_data(int socket, unsigned char address[8], unsigned char *data, int size)
{ unsigned char id;
  unsigned char p[8] ={0,};
  if (size==1)
    { id = ID_WRITEBYTE; size = 0; memcpy(p,data,1); data = p; }
  else   if (size==2)
    { id = ID_WRITEWYDE; size = 0; memcpy(p,data,2); data = p; }
  else  if (size==4)
    { id = ID_WRITETETRA; size = 0; memcpy(p,data,4); data = p; }
  else
    { id = ID_WRITE; size = (size+7)/8-1; }
  return send_msg(socket, 1,(unsigned char)(TYPE_ADDRESS|TYPE_PAYLOAD),(unsigned char)size,0,id,0,address,data);
}




int load_bus_data(int socket, unsigned char address[8], unsigned char *data, int size)
/* returns -1 on error
   1 if the the octa was successfully loaded
*/

{
  int id;
  unsigned char m;
  unsigned char payload[MAXPAYLOAD];
  if (size==1)
    { id = ID_READBYTE; m = 0; }
  else   if (size==2)
    { id = ID_READWYDE; m = 0; }
  else  if (size==4)
    { id = ID_READTETRA; m = 0; }
  else if (size > 4 && size <= MAXPAYLOAD)
    { id = ID_READ; m = (size+7)/8-1; }
  else
    return -1;
  send_msg(socket,1,TYPE_ADDRESS|TYPE_REQUEST,
           m,0,(unsigned char)id,0,address,NULL);
  do 
  { int n;
    unsigned char a[8];
    unsigned char slot;
    id = get_request(socket,1,&slot,a,&n,payload);
    if (id<0) return id;
    else if (dispatch_message(id,a,n,slot,payload))
    { if ((id == ID_READREPLY && m == n) || 
               (id == ID_NOREPLY && size == 0) ||
               (id == ID_BYTEREPLY && size == 1) ||
               (id == ID_WYDEREPLY && size == 2) ||
               (id == ID_TETRAREPLY && size == 4))
      { if (equal(a,address))
            break;
	  else
            return -1;
      }
      else
        return -1;
    }
  } while (1);
  memmove(data,payload,size);
  return 1;
}

int get_interrupt(int socket, int blocking, unsigned int *hi, unsigned int *lo)
     /* return -1 on error otherwise return 1 and or hi and lo
        with the interrupts found so far.
     */
{
  int size;
  unsigned char slot;
  int id;
  unsigned char address[8];
  unsigned char payload[MAXPAYLOAD];

  id = get_request(socket,blocking,&slot,address,&size,payload);
  if (id<0) return id;
  dispatch_message(id,address,size,slot,payload);
  *hi |=  hi_interrupt;
  *lo |=  lo_interrupt;
  hi_interrupt=lo_interrupt=0;
  return 1;
}

int reset_vmb(int socket)
{  return send_msg(socket, 1, TYPE_BUS, 0, 0, ID_RESET, 0, 0, 0);
}

int set_interrupt(int socket, int interrupt)
{ if (interrupt < 0 || interrupt >= 64)
    return -1;
  return send_msg(socket, 1, TYPE_BUS, 0, (unsigned char)interrupt, ID_INTERRUPT, 0, 0, 0);
}

int get_request(int socket, int blocking, unsigned char *slot,
   			   unsigned char address[8], int *size,
                           unsigned char *payload)
     /* return negative for an invalid request
        return ID_IGNORE == 0 for no request
        return the id otherwise
     */
{ unsigned char type;
  unsigned char s;
  unsigned char id;
  unsigned int  time;

  int i;
  i=receive_msg(socket,blocking,&type,&s,slot,&id,&time,address,payload);
  if (i<0)
    return i;
  if (i==0 && !blocking) 
    return ID_IGNORE;
  *size = s;
  if (type&TYPE_BUS)
  { if (id == ID_RESET)
      bus_reset = 1;
    else if (id == ID_POWERON) 
      bus_power=1;
    else if (id == ID_POWEROFF) 
       bus_power=0;
  }
  return id;
}


int answer_readrequest(int socket, unsigned char slot,
   			   unsigned char address[8], int size, unsigned char *data)
{ if (size == 0)
     return send_msg(socket, 1,
               TYPE_ADDRESS|TYPE_ROUTE,
               0,slot,ID_NOREPLY,
               0,address,NULL);
  else if (size == 1)
    return send_msg(socket, 1,
               TYPE_ADDRESS|TYPE_ROUTE|TYPE_PAYLOAD,
               0,slot,ID_BYTEREPLY,
               0,address,data);
  else if (size == 2)
    return send_msg(socket, 1,
               TYPE_ADDRESS|TYPE_ROUTE|TYPE_PAYLOAD,
               0,slot,ID_WYDEREPLY,
               0,address,data);
  else if (size == 4)
    return send_msg(socket, 1,
               TYPE_ADDRESS|TYPE_ROUTE|TYPE_PAYLOAD,
               0,slot,ID_TETRAREPLY,
               0,address,data);
  else if (size>4) 
    return send_msg(socket, 1,
               TYPE_ADDRESS|TYPE_ROUTE|TYPE_PAYLOAD,
               (unsigned char)((size+7)/8-1),slot,ID_READREPLY,
               0,address,data);
   else
     return send_msg(socket, 1,
               TYPE_ADDRESS|TYPE_ROUTE,
               0,slot,ID_NOREPLY,
               0,address,NULL);
}


