/*!
 *
 * \file        FAT32_Longname.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Longname.h,v 1.1 2009-09-08 11:35:43 ruckert Exp $
 * \brief       FAT32 Library, Longname. Functions
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

#ifndef __FAT32_LONGNAME_H__
#define __FAT32_LONGNAME_H__


///! Prototypes
BYTE FATLongname_ChkSum(BYTE * shortname);
int FATLongname_Compare_entry(FAT32_LongEntry *entry, const char *filename, 
			      BYTE checksum, int n);

int FATLongname_Equal(const char *nametofind, int count, UINT32 lba1, int offset1, UINT32 lba2, int offset2);
int FATLongname_is_lfn_entry( FAT32_ShortEntry *entry);
int FATLongname_is_sfn_entry( FAT32_ShortEntry *entry );
int FATLongname_is_dir_entry(FAT32_ShortEntry *entry);
int FATLongname_LFN_to_entry_count( char *filename );

void FATLongname_Create_sfn_entry( BYTE *shortfilename, UINT32 size, UINT32 startCluster, FAT32_ShortEntry *entry );
void FATLongname_Create_lfn_entrys(char *filename, int entryCount, int n, BYTE checksum,  FAT32_LongEntry *entry);
int FATLongname_Create_sfn_with_tail(UINT32 parentcluster, BYTE *shortFilename, char *filename);

#endif // __FAT32_LONGNAME_H__


