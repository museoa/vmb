/*!
 *
 * \file        FAT32_Access.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Access.h,v 1.4 2009-09-08 15:56:57 ruckert Exp $
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


///! Prototypes
void FAT32_dir_shutdown(void);
bool FAT32_SectorReader( UINT32 Startcluster, UINT32 offset );
bool FAT32_SectorWriter( UINT32 *Startcluster, UINT32 offset );
bool FAT32_GetDirectory(const char *fullpath, char* path, char *name , UINT32 *parentcluster);
FAT32_ShortEntry *FAT32_GetFileEntry( UINT32 Cluster, char *nametofind, int *itemfound);
FAT32_ShortEntry *FAT32_GetFileShort( UINT32 Cluster, BYTE *shortname);
FAT32_ShortEntry *FAT32_GetFile( UINT32 Cluster, int item);
UINT32 FAT32_GetFilelength(FAT32_ShortEntry *sfEntry);
void FAT32_SetFilelength(FAT32_ShortEntry *sfEntry, UINT32 length);
UINT32 FAT32_GetFileStartcluster(FAT32_ShortEntry *sfEntry);
void FAT32_SetFileStartcluster(FAT32_ShortEntry *sfEntry, UINT32 cluster);
bool FAT32_UpdateFileLength( UINT32 Cluster, int item, UINT32 fileLength, UINT32 startcluster );
bool FAT32_MarkFileDeleted( UINT32 Cluster, int item );
bool FAT32_AddFileEntry(UINT32 dirCluster, char *filename, BYTE *shortfilename, UINT32 startCluster, UINT32 size, int *item);



#endif // __FAT32_ACCESS_H__

