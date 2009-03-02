/*!
 *
 * \file        FAT32_Access.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Access.h,v 1.2 2009-03-02 12:27:59 ruckert Exp $
 * \brief       FAT32 Library, Access functions
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
#include "FAT32_Definitions.h"
#include "FAT32_Opts.h"

#ifndef __FAT32_ACCESS_H__
#define __FAT32_ACCESS_H__

///! Globals
struct
{
    BYTE currentsector[512];
    UINT32 SectorCurrentlyLoaded; ///< Initially Load to 0xffffffff;
    UINT32 NextFreeCluster;
} FATFS_Internal;

///! Prototypes
bool FAT32_InitFAT( void );
bool FAT32_SectorReader( UINT32 Startcluster, UINT32 offset );
bool FAT32_SectorWriter( UINT32 Startcluster, UINT32 offset );
void FAT32_ShowFATDetails( void );
UINT32 FAT32_GetRootCluster(  );
bool FAT32_GetFileEntry( UINT32 Cluster, char *nametofind, FAT32_ShortEntry *sfEntry );
bool FAT32_SFNexists( UINT32 Cluster, char *shortname );
bool FAT32_UpdateFileLength( UINT32 Cluster, char *shortname, UINT32 fileLength );
bool FAT32_MarkFileDeleted( UINT32 Cluster, char *shortname );
void ListDirectory( UINT32 StartCluster );

#endif // __FAT32_ACCESS_H__

// vim:ts=4:tw=100:wm=100
