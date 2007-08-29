/*!
 * \file omain.cc
 * \author Martin Hauser <info@martin-hauser.net>
 * \author Mauro Morsiani <mmorsian@cineca.it>
 * \brief Main File for the Simulator (containing callbacks for main.c)
 *
 * This is the Main File for the MIPS R3000 Simulator (UMPS) by Mauro Morsiani
 * It has been heavily tweaked and modified by Martin Hauser to actually allow
 * it's use for the Virtual MMIX Motherboard created and Maintained by Prof. Dr. 
 * Martin Ruckert at the Munich University of Applied Sciences.
 * 
 * List of changes to Mauro Morsiani's Version:
 * - Added Pthread library
 *  -# Included pthread.h
 *  -# moved call to Forms main loop into seperate thread
 * - Changed call to main loop from blocking to non blocking
 * - Added callbacks for Simulator
 *  -# poweron and poweroff
 *  -# reset and shutdown
 *  -# param_init and init_device
 *  -# Added other needed callbacks as empty functions
 * - Added Doxygen commenting to this file.
 */

#include <h/const.h> /*!< Constants of the Simulator */
#include <h/types.h> /*!< Type Declarations of the Simulator */

#include <forms.h> /*!< Header Files for X11 Toolkit */
#include <e/xinterface.e> /*!< Declarations for the Classes used to design the Simulator UI */

#include <pthread.h> /*!< Include pthread.h to enable Simulator to become threadded */
#include <cstdlib>   /*!< Include the cstdlib to have Stuff for Extern C calls */


#include "h/omain.h"     /*!< File declaring the chitchat with C calls */
extern "C" {
#include "h/defaults.h"  /*!< Include Simulator setup defaults */
#include "../../error.h"
#include "h/guisync.h"
}





// int main(int argc, char * argv[])
// {
// 	XInterface * xint = new XInterface(&argc, argv);
// 	
// 	xint->ShowMainForm();	
// 	
// 	xint->MainLoop();
// 	
// 	// should never exit here, since XInterface should intercept all
// 	// normal exit requests from user
// 	delete xint;
// 	fl_finish();
// 	return(EXIT_FAILURE);
// }

// make these functions callable from C

  
/*!
 * \fn void param_init(int argc, char *argv[])
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief This function is called with the commandline arguments
 * \param argc The count of the arguments given
 * \param argv The array containing the Commandline arguments
 *
 * This function takes the Commandline paramters passed to it via main.c::main and then
 * parses the parameters destined for the virtual bus extensions(see defaults.c::parseArgv ). 
 * Then it passes the remaining parameters over to a new XInterface object.
 */
 
void param_init(int argc, char *argv[])
{
  //argc -= parseArgv(argc,argv);
  // xint = new XInterface(&argc, argv);
  savArgc = argc;
  savArgv = argv;
  parseArgv(savArgc,savArgv);
 
  xint = NULL; /*!< ensure that create condition is met */
}

/*! 
 * \fn int init_device(void)
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief Function called on device init - Is dummy here
 */
int init_device(void)
{
  return 0;
}

unsigned char *get_payload(unsigned int offset,int size)
{
}

int reply_payload(unsigned char address[8], int size,unsigned char *payload)
{
  return 1;
}

int put_payload(unsigned int offset,int size, unsigned char *payload)
{
  return 0;
}

/*!
 * \fn void* MainLoop(void* arg)
 * \author Martin Hauser <info@martin-hauser.net>
 * \param arg Dummy parameter
 * \brief Thread for GUI Mainloop
 * 
 * To regain controll of the Process after turning over the Controll to the User, the 
 * Main-Loop used by the GUI to dispatch events is moved into a seperate Thread.
 * \warning Be Sure not to put anything in here which actualy manipulates Data used elsewhere!
 */
void* MainLoop(void* arg)
{
  xint->ShowMainForm();
  bDoQuit = false;
  intGuiEnabled = 1;
  while(!bDoQuit)
  {
      pthread_mutex_lock(&pmtxGuiRefresh);
      fl_check_forms();
      pthread_mutex_unlock(&pmtxGuiRefresh);
  }
  intGuiEnabled = 0;
}

/*!
 * \fn int process_poweron(void)
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief Function called at poweron-message from the Motherboard
 *
 * This function is called whenever the virtual PowerON switch is pressed on the
 * Motherboard. It will do a virtual reset to the Processor and then relaunch the
 * thread used to contain the ::MainLoop.
*/

int process_poweron(void)
{
  if(xint == NULL)
  {
    xint = new XInterface(&savArgc,savArgv);
  }
  else 
      xint->Reset(false,NULL);
  
  pthread_create(&pthrUIupdater,0,MainLoop,0);
  // printf("%s(%d): process_poweron(): Received power-on from motherboard. Ready to go.\n",__FILE__,__LINE__);
  return 0;
}

/*!
 * \fn int process_poweroff(void)
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief Function called at poweron-message from the Motherboard
 *
 * This function is called whenever the virtual PowerOff switch is pressed on the
 * Motherboard. It will set the quit variable to true and then waits till the ::MainLoop
 * thread terminates. Reset of the Simulator is handled on next poweron.
*/

int process_poweroff(void)
{ 
  bDoQuit = true;
  pthread_join(pthrUIupdater,NULL);
  return 0;
}

/*!
 * \fn int process_reset(void)
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief Function called at virtual reset
 *
 * If the virtual reset switch is pressed on the Motherboard, this function will be
 * called by the messages dispatcher in main.c
*/

int process_reset(void)
{ 
  if(xint != NULL)
    xint->Reset(false,NULL);
  return 0;
}

int process_interrupt(unsigned char interrupt)
{
  return 0;
}


int process_input(unsigned char c) 
{ /* ignore input */
  return 0;
}

/*!
 * \fn int prepare_shutdown()
 * \brief Does some cleanup before the Simulator shutdowns
 * \author Martin Hauser <info@martin-hauser.net>
 * 
 * This function is called by main.c::main and is executed just before the simulator
 * shuts down. This helps cleaning up the User Interface and makes sure the simulator
 * dies gracefully.
 */  
int prepare_shutdown()
{
  delete xint;
  fl_finish();
  return 0;
}
