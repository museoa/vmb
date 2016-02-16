/*!
 * \file cache.h
 * \author Martin Hauser <info@martin-hauser.net>
 * \author Martin Ruckert <ruckertm@acm.org>
 * \brief Standard instruction Cache support
 *
 * This File contains classes to enable cached read of instructions. The
 * Orginal Implementation was done by Martin Ruckert but was stripped down
 * and converted to C++ for the use with the UMPS Simulator by Martin Hauser.
 */



#ifndef _CACHE_H_
#define _CACHE_H_


/*!
 * \class Cache
 * \author Martin Hauser <info@martin-hauser.net>
 * \author Martin Ruckert <ruckertm@acm.org>
 * \brief Class simulating an standard cache.
 * 
 * This cache class implements a standard ReadOnly Cache for
 * the Instructions read from the Simulator core. This class
 * will be hooked up into the systembus.cc::InstrRead method
 * which is used to fetch instructions.
 */

class Cache
{
  public:
    void reset();
    Word fetch_instr(Word wAddress);
};

#endif /* _CACHE_H_ */
