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
#include <pthread.h>
#include "../../message.h"
#include "../../bus-arith.h"
#include "../../bus-util.h"
#include "../../option.h"
#include "../../param.h"
#include "../../error.h"
#include "../../main.h"

#include "h/guisync.h"
#include "h/bussync.h"
#include "forms.h"
#include "h/defaults.h"

pthread_mutex_t pmtxBusAccess;
pthread_mutex_t pmtxGuiRefresh;

void clean_up(void)
{ 
 if (bus_fd>=0)
  { bus_unregister(bus_fd);
    bus_disconnect(bus_fd); 
	bus_fd = INVALID_SOCKET;
  }
}


  
void process_bus()
/* find out whats going on and service requests.
*/
{ int i;
  unsigned char a[8], slot;
  unsigned char p[256*8];
  int s;
  fd_set readfs;    /* file descriptor set for read */
  FD_ZERO(&readfs);
  FD_SET(bus_fd, &readfs);  /* set testing for source */
  FD_SET(0, &readfs);  /* set testing for stdin */
  debug("Waiting for event");
  
  /* check whether we are allowed to pass the barrier */
  pthread_mutex_lock(&pmtxBusAccess); 
  pthread_mutex_unlock(&pmtxBusAccess);
  
  select(bus_fd+1, &readfs, NULL, NULL, NULL);
  if (FD_ISSET(bus_fd, &readfs)) 
  {
    debug("Reading request");
    /* check whether we are allowed to pass the barrier */
    pthread_mutex_lock(&pmtxBusAccess); 
    i = get_request(bus_fd,0,&slot,a,&s,p);
    pthread_mutex_unlock(&pmtxBusAccess);
    
    if (i < 0) errormsg(strerror(errno));
    else dispatch_message(i,a,s,slot,p);
  }
  else if (FD_ISSET(0, &readfs))
  { unsigned char c;
    int i;
    debug("reading character:");
    i = read(0,&c,1);
    if (i == 0) return;
    if (i < 0) errormsg("Read Error");
    debugi("got %02X",c&0xFF);
    process_input(c);
  }
}

int write_request(unsigned char *a, int s, unsigned char *p)
{ /* dummy */
  return 0;
}



int read_request( unsigned char *a, int s, unsigned char slot, unsigned char *p)
{ /* dummy */
  return 0;
}



/* The main loop  */
void process_loop(void)
{ 
  bus_fd= bus_connect(host,port);
  if (bus_fd<0) fatal_error(__LINE__,"Unable to connect to motherboard");
  if (bus_register(bus_fd,0,0,0,interrupts,programname)<0)
    fatal_error(__LINE__,"Unable to register with motherboard");
  atexit(clean_up);
  while (bus_connected)
  {
      if(intGuiEnabled == 1)
      {
          pthread_mutex_lock(&pmtxGuiRefresh);
          fl_check_forms();
          pthread_mutex_unlock(&pmtxGuiRefresh);
      }
      process_bus();
  }
  bus_fd=-1;
}


int main(int argc, char *argv[])
{
  pthread_mutex_init(&pmtxBusAccess, NULL); // init mutex  
  pthread_mutex_init(&pmtxGuiRefresh, NULL); // init mutex  
  param_init(argc, argv);
  debugs("%s ",programname);
  debugs("%s ", version);
  debugs("host: %s ",host);
  debugi("port: %d ",port);
  init_device();
  process_loop();
  prepare_shutdown();
  clean_argv();
  return 0;
}
