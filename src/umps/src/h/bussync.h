/*
  bussync.h -- (c) Copyright 2007 Martin Hauser. All Rights Reserved. 
*/
/*!
 * \file bussync.h
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief Header file providing means to sync access to the bus
 *
 *  * This file serves as workaround for the present state of bus-util.c, which is aparently not
 * threadsafe. So every access to the bus from within the simulator will be synchronized!
*/

#ifndef _BUSSYNC_H_
#define _BUSSYNC_H_
#include <pthread.h>
extern pthread_mutex_t pmtxBusAccess;

#endif /* _BUSSYNC_H_ */
