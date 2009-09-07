/*!
 *
 * \file        FAT32_Disk.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Disk.h,v 1.2 2009-09-07 11:43:30 ruckert Exp $
 * \brief       FAT32 Library, Disk
 * \details     {
 * }
 * \note        {
 *                Copyright (c) 2004-2007, Rob Riglar <rob@robriglar.com>, FAT32 File IO Library
 *                Copyright (c) 2007-2008, Bjoern Rennhak, Virtual Motherboard project.
 *                All rights reserved, see COPYRIGHT file for more details.
 * }
 *
 */

///! Custom includes
#include "../define.h"
#include "FAT32_Opts.h"

#ifndef __FAT32_DISK_H__
#define __FAT32_DISK_H__

bool FAT_InitDrive();
bool FAT_ReadSector( UINT32 sector, BYTE *buffer );
bool FAT_WriteSector( UINT32 sector, BYTE *buffer );

#endif // __FAT32_DISK_H__

// vim:ts=4:tw=100:wm=100
