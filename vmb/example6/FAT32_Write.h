/*!
 *
 * \file        FAT32_Write.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Write.h,v 1.1 2016-02-16 10:17:33 ruckert Exp $
 * \brief       FAT32 Library, FAT32 Write functions
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

#ifndef __FAT32_WRITE_H__
#define __FAT32_WRITE_H__


///! Prototypes
bool FAT32_AddFileEntry(UINT32 dirCluster, char *filename, char *shortfilename, UINT32 startCluster, UINT32 size);
bool FAT32_AddFreeSpaceToChain(UINT32 *startCluster);
bool FAT32_AllocateFreeSpace(bool newFile, UINT32 *startCluster, UINT32 size);

#endif  // __FAT32_WRITE_H__

// vim:ts=4:tw=100:wm=100
