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


/*!
 * \fn void Cache::reset()
 * \author Martin Hauser <info@martin-hauser.net>
 * 
 * \brief resets the cache contents
 *
 * This method resets the contents of the cache, cleares it.
 */

void Cache::reset()
{
    vmb_cache_clear(&vmb_i_cache);
}


/*!
 * \fn Word Cache::fetch_instr(Word wAddress)
 * \author Martin Hauser <info@martin-hauser.net>
 * 
 * \brief fetches instruction either through cache or from memory
 *
 * This Method uses the given adresss to fetch a Word from the cache or 
 * memory. It uses ::lookup to fetch a cache line.
 */

Word Cache::fetch_instr(Word wAddress)
{
    vmb_cache_line* clLine = cache_lookup(&vmb_i_cache,0,wAddress);
    unsigned int iSizeTest;
    unsigned char cpAddr[8];
    unsigned char cpTmpData[LINESIZE];
    unsigned int i;
    
    if(clLine == NULL)
    {
        clLine = cache_load(&vmb_i_cache,0,wAddress);
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
                fatal_error(__LINE__,"Failed to fix Cache alignment problems, bailing out.\n");
        }
        pthread_mutex_unlock(&pmtxBusAccess); /*!< we're done with the critic stuff */  
    }
    return vmb_cache_read(&vmb_i_cache,0,wAddress);
}
