/*!
 *
 * \file        FAT32_Disk.c
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Disk.c,v 1.2 2009-09-07 11:43:30 ruckert Exp $  // 2.0
 * \brief       FAT32 Library, Disk
 * \details     {
 * }
 * \note        {
 *                Copyright (c) 2003-2007, Rob Riglar <rob@robriglar.com>, FAT32 File IO Library
 *                Copyright (c) 2007-2008, Bjoern Rennhak, Virtual Motherboard project.
 *                All rights reserved, see COPYRIGHT file for more details.
 * }
 *
 */

///! Custom includes
#include "FAT32_Disk.h"


/*!
 * \fn      bool FAT_InitDrive( void )
 * \brief   Initialise the choosen source
 * \return  True if success, returns False if not
 */
bool FAT_InitDrive( void )
{
  return disk_init();

}


/*!
 * \fn      bool FAT_ReadSector(UINT32 sector, BYTE *buffer)
 * \brief   Read a sector from disk
 * \param   sector  Needs an unsigned integer (uint32) which represents the given sector on the disk
 * \param   buffer  Needs a byte which represents the buffer to store one sector (512 byte).
 * \return  True if success, returns False if not
 */
bool FAT_ReadSector(UINT32 sector, BYTE *buffer)
{

    LOGVARS( "LOG4C_PRIORITY_VVERBOSE", "Called FAT_ReadSector(UINT32 sector, BYTE *buffer)", "ub", sector, buffer );

    return disk_read(sector, 1, buffer);
 
}


/*!
 * \fn      bool FAT_WriteSector(UINT32 sector, BYTE *buffer)
 * \brief   Write a sector to disk
 * \param   sector  Needs an unsigned integer (uint32) which represents the given sector on the disk
 * \param   buffer  Needs a byte which represents the buffer to store one sector (512 byte).
 * \return  True if success, returns False if not
 */
bool FAT_WriteSector(UINT32 sector, BYTE *buffer)
{

    LOGVARS( "LOG4C_PRIORITY_VVERBOSE", "Called FAT_WriteSector(UINT32 sector, BYTE *buffer)", "ub", sector, buffer );
    return disk_write(sector, 1, buffer);
 
}
         
