/*!
 * \file cache.cc
 * \author Martin Hauser <info@martin-hauser.net>
 * \author Martin Ruckert <ruckertm@acm.org>
 * \brief Standard instruction Cache support
 *
 * This File contains classes to enable cached read of instructions. The
 * Orginal Implementation was done by Martin Ruckert but was stripped down
 * and converted to C++ for the use with the UMPS Simulator by Martin Hauser.
*/
extern "C" {

#include "../../message.h"
#include "../../bus-arith.h"
#include "../../bus-util.h"
#include <stdio.h>
#include "h/bussync.h"
#include "../../error.h"
}

#include "h/types.h" /*!< be able to use Word and WORDLEN */
#include "h/cache.h" /*!< The Class declaration of Cache class */


void Cache::reset()
{
    int i,k;
    for (i=0;i<CACHESIZE;i++)
    { 
        lines[i].access = 0;
        for(k=0; k<WAYS; k++)
            lines[i].line[k].use = 0;
    }    
}

cache_line* Cache::lookup(Word address)
{ 
  int i = CACHEINDEX(address);
  cache_line *line = lines[i].line;
  int w;
  for (w=0; w<WAYS; w++)
  {    
    if (line[w].use != 0 && ((line[w].address&~CACHEMASK) == (address&~CACHEMASK)))
    { 
        if (++lines[i].access == 0) 
          ++lines[i].access;
        line[w].use = lines[i].access;
        return line+w;
    }
  }
  
  return NULL;
}
/*
 * \fn cache_line* Cache::lru(Word address)  
 * \author Martin Hauser <info@martin-hauser.net>
 * \param address The address which cache line shall be fetched
 * \brief finds the least recently used cache line
 * \return a pointer to the least recently used cache.h::cache_line
 *
 * This methods walks it's way through the cache_line bundle 
 * corresponding to address, trying to find the line with the 
 * lowest use count. Once the line is found, access values are
 * altered accordingly and the pointer to the cache line is
 * returned
 */


cache_line* Cache::lru(Word address)  
{
    int i = CACHEINDEX(address);
    /*! get line corresponding to address */
    cache_line *line = lines[i].line; 
    
    /*! retrieve access for cache.h::cache_line bundle and do some setup*/
    int access = lines[i].access;
    int lru = 0;
    int age = access - line[lru].use;
    int w;
    
    /*! loop through all parts of a cache.h::cache_line bundle */
    for (w=1; w<WAYS; w++) 
    {
        /*! check whether new line is older then current line */
        if ((access - line[w].use) > age) 
        {
            age = access - line[w].use;
            lru = w; 
        }
    }
    access++;
    if (access == 0) /*!< make sure access has been raised above 0 */
        ++access;
    
    /*! store new access and use values */
    lines[i].access  = access;
    line[lru].use = access;
    return line+lru;
}

Word Cache::fetch_instr(Word wAddress)
{
    cache_line* clLine = lookup(wAddress);
    unsigned int iSizeTest;
    unsigned char cpAddr[8];
    unsigned char cpTmpData[LINESIZE];
    unsigned int i;
    
    if(clLine == NULL)
    {
        clLine = lru(wAddress);
        clLine->address = wAddress & ~LINEMASK;
        inttochar(0UL,cpAddr);
        inttochar(clLine->address,cpAddr+4);
        pthread_mutex_lock(&pmtxBusAccess); /*!< get the mutex for bus access */
        if(load_bus_data(bus_fd,cpAddr,clLine->data,LINESIZE) != 1)
        {
            fprintf(stderr,"Address 0x%x not properly aligned for cache operations. Trying to compensate...\n",
                            wAddress);
            iSizeTest = LINESIZE;
            while(load_bus_data(bus_fd,cpAddr,clLine->data,iSizeTest) != 1 && iSizeTest > 0)
                iSizeTest -= 4;
            if(iSizeTest > 0)
                fprintf(stderr,"Compensation worked.\nAlignment of Address 0x%x is of by %d bytes, please "
                               "fix if possible.\n",wAddress,LINESIZE-iSizeTest);
            else
                fatal_error(__LINE__,"Failed to fix alignment problems, bailing out.\n");
        }
        pthread_mutex_unlock(&pmtxBusAccess); /*!< we're done with the critic stuff */  
    }
    return chartoint(clLine->data + (CACHEOFFSET(wAddress)&~(3)));
}
