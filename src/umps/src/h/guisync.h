/*
    guisync.h -- (c) Copyright 2007 Martin Hauser. All Rights Reserved. 
*/
/*!
 * \file guisync.h
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief Header file to enable sync of the gui between omain.cc and main.c
 *
 * This file exists for the sole purpose of providing the declaration of every
 * means necersarry to sync the calls to fl_check_forms used by omain.cc:process_poweron
 * and main.c::process_loop.
*/
#ifndef _GUISYNC_H_
#define _GUISYNC_H_
#include <pthread.h>

int intGuiEnabled;
extern pthread_mutex_t pmtxGuiRefresh;
#endif /* _GUISYNC_H_ */
