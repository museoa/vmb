/*!
 *
 * \file        FAT32_Table.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Table.h,v 1.1 2015-09-24 13:00:43 ruckert Exp $
 * \brief       FAT32 Library, FAT, File Allocation Table
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

#ifndef __FAT32_TABLE_H__
#define __FAT32_TABLE_H__

///! Global variables
struct
{
    ///! Filesystem globals
    BYTE    SectorsPerCluster;
    UINT16  BytePerSector;
    UINT16  Reserved_Sectors;
    BYTE    NumFATs;
    UINT32  RootDir_First_Cluster;
    UINT32  lba_begin;
    UINT32  fat_begin_lba;
    UINT32  cluster_begin_lba;
    UINT32  FATSz;
    UINT32  TotSec;
    UINT32  DataSec;
    UINT32  CountofClusters;
    UINT16  fs_info_sector;

} FAT32;

///! Prototypes
bool    FAT32_Init( void );
void FAT32_FAT_shutdown(void);
void FAT32_InitFatBuffer( void );
bool FAT32_PurgeFATBuffer( void );
UINT32 FAT32_FindNextCluster( UINT32 Current_Cluster );
UINT32 FAT32_FindBlankCluster(void);
bool FAT32_AddClusterToEndofChain( UINT32 StartCluster, UINT32 newEntry );
bool FAT32_AddFreeSpaceToChain(UINT32 *lastCluster);
bool FAT32_FreeClusterChain( UINT32 StartCluster );
UINT32 FAT32_ClusterOffset2lba(UINT32 *ClusterChain, UINT32 offset, int extend);
UINT32 FAT32_LBAofCluster(UINT32 Cluster_Number);
bool FAT32_AddFreeSpaceToChain(UINT32 *startCluster);
bool FAT32_AllocateFreeSpace(bool newFile, UINT32 *startCluster, UINT32 size);

#endif  // __FAT32_TABLE_H__

// vim:ts=4:tw=100:wm=100
