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


#define WAYS     2                    /* a n-way cache */
#define LINEBITS 5                    /* bits to index a cache line */
#define LINESIZE (1<<LINEBITS)        /* bytes per cache line */
#define LINEMASK (LINESIZE-1)         /* mask to get line bits */
#define CACHEBITS 13                  /* bits to index the cache */
#define CACHEMASK ((1<<CACHEBITS)-1)  /* mask to get cache bits */
#define CACHEOFFSET(address) (address & LINEMASK)
#define CACHEINDEX(address)  ((address & CACHEMASK)>>LINEBITS)
#define CACHESIZE (1<<(CACHEBITS-LINEBITS))

typedef struct {
  unsigned char data[LINESIZE]; 
  Word address;  
  unsigned int use;
  unsigned int dirty;
} cache_line;

typedef struct {
  unsigned int access;
  cache_line line[WAYS];
} cache_t[CACHESIZE];


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
    cache_line* lookup(Word address);
    cache_line* lru(Word address);
    void reset();
    Word fetch_instr(Word wAddress);
    
  private:
      cache_t lines;
};

#endif /* _CACHE_H_ */
