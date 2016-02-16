/*!
 *
 * \file        FAT32_Table.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Table.h,v 1.1 2016-02-16 10:17:33 ruckert Exp $
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


///! Prototypes
void FAT32_InitFatBuffer( void );
bool FAT32_ReadFATSector( UINT32 sector );
bool FAT32_WriteFATSector( UINT32 sector );
bool FAT32_PurgeFATBuffer( void );
UINT32 FAT32_FindNextCluster( UINT32 Current_Cluster );
UINT32 FAT32_GetFsInfoNextCluster( void );
void FAT32_SetFsInfoNextCluster( UINT32 newValue );
bool FAT32_FindBlankCluster( UINT32 StartCluster, UINT32 *FreeCluster );
bool FAT32_SetClusterValue( UINT32 Cluster, UINT32 NextCluster );
bool FAT32_AddClusterToEndofChain( UINT32 StartCluster, UINT32 newEntry );
bool FAT32_FreeClusterChain( UINT32 StartCluster );

#endif  // __FAT32_TABLE_H__

// vim:ts=4:tw=100:wm=100
