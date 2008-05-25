/*!
 * \file main.c
 * \author Martin Ruckert <ruckertm@acm.org>
 * \author Martin Hauser  <info@martin-hauser.net>
 * \copyright Copyright 2005 Martin Ruckert, changes Copyright 2007 Martin Hauser. 
 * \brief main file for uMPS Simulator
 *
 * This file is a modified version of the main.c file provided with the MMIX Motherboard
 * Project by Martin Ruckert. Changes to it include the addition of mutexes for some 
 * operations and the change of some variables.
 *
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

pthread_mutex_t pmtxBusAccess;  //!< mutex to control bus access
pthread_mutex_t pmtxGuiRefresh; //!< mutex to control gui refreshment
int bus_fd;

/*!
 * \fn void clean_up(void)
 * \author Martin Hauser <info@martin-hauser.net>
 * 
 * \brief does cleanup for main.c
 *
 * This function is called on shutdown of the Simulator main program and cleans up
 * remaining Motherboard connections. The modified version also calls ::prepare_shutdown
 * to enable the simulator client to disconnect.
 */

void clean_up(void)
{ 
    if (bus_fd>=0)
    {
        bus_unregister(bus_fd);
        bus_disconnect(bus_fd); 
	    bus_fd = INVALID_SOCKET;
    }
    prepare_shutdown();
    clean_argv();           //!< cleanup argv stuff
}


  
/*!
 * \fn void process_bus()
 * \author Martin Hauser <info@martin-hauser.net>
 * 
 * \brief does bus processing, called from ::process_loop
 *
 * This function reads from the bus file-descriptor and processes 
 * the returned data using get_request. Some parts are protected by
 * the ::pmtxBusAccess Mutex. 
 */

void process_bus()
{
    int i;
    unsigned char a[8], slot;
    unsigned char p[256*8];
    int s;
    fd_set readfs;            /* file descriptor set for read */
    FD_ZERO(&readfs);
    FD_SET(bus_fd, &readfs);  /* set testing for source */
    FD_SET(0, &readfs);       /* set testing for stdin */
    debug("Waiting for event");
  
    //! make sure this is the only thread waiting for bus access
    pthread_mutex_lock(&pmtxBusAccess); 
    pthread_mutex_unlock(&pmtxBusAccess);
  
    select(bus_fd+1, &readfs, NULL, NULL, NULL);
    if (FD_ISSET(bus_fd, &readfs))//!< reading from bus
    {
        debug("Reading request");
        
        //! lock all bus access
        pthread_mutex_lock(&pmtxBusAccess); 
        i = get_request(bus_fd,0,&slot,a,&s,p);
        pthread_mutex_unlock(&pmtxBusAccess); //!< free bus access

        if (i < 0)
            errormsg(strerror(errno));
        else 
            dispatch_message(i,a,s,slot,p);
    }
    else if (FD_ISSET(0, &readfs)) //!< reading from stdin
    { 
        unsigned char c;
        int i;
        debug("reading character:");
        i = read(0,&c,1);
        
        if (i == 0)
            return;
            
        if (i < 0)
            errormsg("Read Error");
        
        debugi("got %02X",c&0xFF);
        process_input(c);
    }
}

/*!
 * \fn int write_request(unsigned char *a, int s, unsigned char *p)
 * \author Martin Hauser <info@martin-hauser.net>
 * 
 * \brief dummy function
 */

int write_request(unsigned char *a, int s, unsigned char *p)
{ /* dummy */
  return 0;
}



/*!
 * \fn int read_request( unsign ed char *a, int s, unsigned char slot, unsigned char *p)
 * \author Martin Hauser <info@martin-hauser.net>
 * 
 * \brief dummy function
 *
 */

int read_request( unsign ed char *a, int s, unsigned char slot, unsigned char *p)
{ /* dummy */
  return 0;
}



/*!
 * \fn void process_loop(void)
 * \author Martin Hauser <info@martin-hauser.net>
 * 
 * \brief bus-processing main-loop
 *
 * This function loops as long as the simulator is connected to the bus. It reads and writes
 * data as necersarry with the help of ::process_bus. It also calls fl_check_forms(), 
 * protected by a mutex.
 */

void process_loop(void)
{ 
  bus_fd= bus_connect(host,port);
  if (bus_fd<0) fatal_error(__LINE__,"Unable to connect to motherboard");
  if (bus_register(bus_fd,0,0,0,interrupts,programname)<0)
    fatal_error(__LINE__,"Unable to register with motherboard");
  atexit(clean_up); //!< register ::clean_up as exit function
  while (bus_connected) //!< loop through bus connection
  {
      if(intGuiEnabled == 1) //!< only do gui updating if gui is enabled
      {
          pthread_mutex_lock(&pmtxGuiRefresh);
          fl_check_forms();
          pthread_mutex_unlock(&pmtxGuiRefresh);
      }
      process_bus();
  }
  bus_fd=-1; //!< reset bus_fd
}


/*!
 * \fn int main(int argc, char *argv[])
 * \author Martin Hauser <info@martin-hauser.net>
 * 
 * \brief simulator main function
 *
 * This main function just does initialisation and turns control over to ::process_loop.
 */

int main(int argc, char *argv[])
{
  pthread_mutex_init(&pmtxBusAccess, NULL); // init mutex  
  pthread_mutex_init(&pmtxGuiRefresh, NULL); // init mutex  
  param_init(argc, argv); //!< init parameters
  init_device();          //!< call device setup function
  process_loop();         //!< turn controll over to setup function

  return 0;
}
