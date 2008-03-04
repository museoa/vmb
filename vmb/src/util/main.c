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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include "message.h"
#include "bus-arith.h"
#include "bus-util.h"
#include "option.h"
#include "param.h"
#include "error.h"
#include "main.h"


void clean_up(void)
{ 
 if (bus_fd>=0)
  { bus_unregister(bus_fd);
    bus_disconnect(bus_fd); 
	bus_fd = INVALID_SOCKET;
  }
}


  

int wait_event(int blocking)
{ fd_set readfs;    /* file descriptor set for read */
  FD_ZERO(&readfs);
  FD_SET(bus_fd, &readfs);  /* set testing for source */
  FD_SET(0, &readfs);  /* set testing for stdin */
  vmb_debug("Waiting for event");
  if (blocking)
    select(bus_fd+1, &readfs, NULL, NULL, NULL);
  else
    { struct timeval timeout;
      timeout.tv_sec = 0;
      timeout.tv_usec = 0;
      if (select(bus_fd+1, &readfs, NULL, NULL, &timeout)<=0)
       return INVALID_SOCKET;
    }
  if (FD_ISSET(bus_fd, &readfs)) return bus_fd;
  else if (FD_ISSET(0, &readfs)) return 0;
  else return  INVALID_SOCKET;
}

void handle_event(int fd)
{ int i;
  unsigned char a[8], slot;
  unsigned char p[MAXPAYLOAD];
  int s;
  if (fd == bus_fd) 
  {
    vmb_debug("Reading request");
    i = get_request(bus_fd,0,&slot,a,&s,p);
    if (i < 0) vmb_errormsg(strerror(errno));
    else dispatch_message(i,a,s,slot,p);
  }
  else if (fd == 0)
  { unsigned char c;
    int i;
    vmb_debug("reading character:");
    i = read(0,&c,1);
    if (i == 0) return;
    if (i < 0) vmb_errormsg("Read Error");
    vmb_debugi("got %02X",c&0xFF);
    process_input(c);
  }
}





void process_bus()
/* find out whats going on and service requests.
*/

{ int fd;
  fd = wait_event(1);
  if (valid_socket(fd)) 
    handle_event(fd);
}

int write_request(unsigned char a[8], int s, unsigned char p[])
{ unsigned int offset;
  offset = get_offset(address,a);
  if (hi_offset || overflow_offset || offset + s > size)
  { char hex[17]={0};
    chartohex(a,hex,8);
    vmb_debugs("Write request out of range %s",hex);
    vmb_debugx("Address: %s",address,8);
    vmb_debugi("Size:    %d",size);
    vmb_debugi("Offset:  %ud", offset);
    vmb_debug("raising interrupt");
    set_interrupt(bus_fd, INT_NOMEM);
    return 0;
  }
  vmb_debug("Writing");
  put_payload(offset,s,p);
  return 0;
}

/* global variables to contain additional context
   if get_payload is called */
int get_payload_slot = 0;
int get_payload_fd = 0;

int read_request( unsigned char a[8], int s, unsigned char slot, unsigned char p[])
{ unsigned int offset;
  unsigned char *data;
  int i;
  offset = get_offset(address,a);
  if (hi_offset || overflow_offset || offset + s > size)
  { char hex[17]={0};
    chartohex(a,hex,8);
    vmb_debugs("Read request out of range %s",hex);
    vmb_debug("Sending empty answer");
    i = answer_readrequest(bus_fd,slot, a,0,NULL);
    if (i < 0) vmb_errormsg("Write Error");
    vmb_debug("raising interrupt");
    set_interrupt(bus_fd, INT_NOMEM);
    return 0;
  }
  vmb_debug("sending answer");
  get_payload_slot = slot;
  get_payload_fd = bus_fd;
  data = get_payload(offset,s);
  if (data != NULL)
  { i = answer_readrequest(bus_fd,slot,a,s,data);
    if (i < 0) vmb_errormsg("Write Error");
  }
  return 0;
}



/* The main loop  */
void process_loop(void)
{ 
  bus_fd= bus_connect(host,port);
  if (bus_fd<0) vmb_fatal_error(__LINE__,"Unable to connect to motherboard");
  if (bus_register(bus_fd,address,limit,0,0,defined)<0)
    vmb_fatal_error(__LINE__,"Unable to register with motherboard");
  atexit(clean_up);
  while (bus_connected)
      process_bus();
  bus_fd=-1;
}


int main(int argc, char *argv[])
{
 param_init(argc, argv);
 vmb_debugs("%s ",vmb_program_name);
 vmb_debugs("%s ", version);
 vmb_debugs("host: %s ",host);
 vmb_debugi("port: %d ",port);
 init_device();
 hextochar(hexaddress,address,8);
 add_offset(address,size,limit);
 vmb_debugs("address: %s ",hexaddress);
 vmb_debugi("size: %x ",size);
 process_loop();
 return 0;
}
