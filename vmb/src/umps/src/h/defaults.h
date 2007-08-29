/*!
 * \file defaults.h
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief header file for defaults (including declarations)
 *
 * (c) Copyright 2006 Martin Hauser. All Rights Reserved. 
 */
 
#ifndef _DEFAULTS_H_
#define _DEFAULTS_H_

#include <stdlib.h>
int parseArgv(int argc, char **argv);
void clean_argv();

extern int savArgc; /*!< Save argument counter (ugly but needed) */
extern char** savArgv; /*!< Save Argument Vector (ugly but needed) */
extern unsigned long interrupts;
extern unsigned int enableCache;
extern unsigned int enableRomFork;
extern unsigned long ulRamSize;
extern int debugflag;

#endif /* _DEFAULTS_H_ */

