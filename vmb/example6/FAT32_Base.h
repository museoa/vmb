/*!
 *
 * \file        FAT32_Base.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Base.h,v 1.1 2016-02-16 10:17:32 ruckert Exp $
 * \brief       FAT32 Library, Base methods
 * \details     {
 * }
 * \note        {
 *                Copyright (c) 2004-2007, Rob Riglar <rob@robriglar.com>, FAT32 File IO Library
 *                Copyright (c) 2007-2008, Bjoern Rennhak, Virtual Motherboard project.
 *                All rights reserved, see COPYRIGHT file for more details.
 * }
 *
 */

#include "../define.h"
#include "FAT32_Opts.h"

#ifndef __FAT32_BASE_H__
#define __FAT32_BASE_H__

// Custom includes
#include "FAT32_Disk.h"

///! Global variables
struct
{
    ///! Filesystem globals
    BYTE    SectorsPerCluster;
    UINT32  cluster_begin_lba;
    UINT32  RootDir_First_Cluster;
    UINT32  fat_begin_lba;
    UINT32  filenumber;
    UINT16  fs_info_sector;
    UINT32  lba_begin;
    UINT32  fat_sectors;
} FAT32;

///! Prototypes
bool    FAT32_Init( void );
UINT32 FAT32_LBAofCluster(UINT32 Cluster_Number);



#endif // __FAT32_BASE_H__

// vim:ts=4:tw=100:wm=100
