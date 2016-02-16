/*!
 * \file omain.h
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief Declares C - C++ External Linking and global variables
 * 
 * (c) Copyright 2006 Martin Hauser. All Rights Reserved. 
 */
 
#ifndef _OMAIN_H_
#define _OMAIN_H_

extern "C" {
  void param_init(int argc, char *argv[]);
  int init_device(void);
  unsigned char *get_payload(unsigned int offset,int size);
  int reply_payload(unsigned char address[8], int size,unsigned char *payload);
  int put_payload(unsigned int offset,int size, unsigned char *payload);
  int process_poweron(void);
  int process_poweroff(void);
  int process_reset(void);
  int process_interrupt(unsigned char interrupt);
  int process_input(unsigned char c);
  int prepare_shutdown();
  int parseArgv(int argc, char **argv);
// 
//   // C functions used by C++
//   void fatal_error(int line,char *msg);
//   void message(char *msg);
//   void errormsg(char *msg);
//   void debug(char *msg);
//   void debugi(char *msg,int i);
//   void debugs(char *msg, char *s);
//   void debugx(char *msg, char *s, int n);
//   void inttochar(int val, unsigned char buffer[4]);
//   int load_bus_data(int socket, unsigned char address[8], unsigned char *data, int size);
}
XInterface * xint; /*!< Variable containing pointer to the GUI Classes */
pthread_t pthrUIupdater; /*!< The GUI updating Thread containing the mainloop */
bool bDoQuit; /*!< Variable which forces Mainloop thread to quit if set to 'true' */

#endif /* _OMAIN_H_ */
