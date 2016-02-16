/*!
 *
 * \file        FAT32_Name.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Name.h,v 1.1 2015-09-24 13:00:43 ruckert Exp $
 * \brief       FAT32 Library, Name. Functions
 * \details     {
 * }
 * \note        {
 *                Copyright (c) 2004-2007, Rob Riglar <rob@robriglar.com>, FAT32 File IO Library
 *                Copyright (c) 2007-2008, Bjoern Rennhak, Virtual Motherboard project.
 *                All rights reserved, see COPYRIGHT file for more details.
 * }
 *
 */


#ifndef __FAT32_NAME_H__
#define __FAT32_NAME_H__


///! Prototypes
const char *Name_GetNextDirectory(const char * path, char * dir, int bound);
const char *Name_GetFirstDirectory(const char * path, char * dir, int bound);

int Name_CompareSN(BYTE* shortname, const char* name);
void Name_Trim(char *trimmed, const char *filename);

int FATName_is_lfn_entry( FAT32_ShortEntry *entry);
int FATName_is_sfn_entry( FAT32_ShortEntry *entry );
int FATName_is_dir_entry(FAT32_ShortEntry *entry);
int FATName_LFN_to_entry_count( char *filename );

void FATName_Create_sfn_entry( BYTE *shortfilename, UINT32 size, UINT32 startCluster, FAT32_ShortEntry *entry );
void FATName_Create_lfn_entrys(char *filename, int entryCount, int n, BYTE checksum,  FAT32_LongEntry *entry);

int FATName_Create_sfn_with_tail(UINT32 parentcluster, BYTE *shortFilename, char *filename);


BYTE FATName_ChkSum(BYTE * shortname);
int FATName_Compare_entry(FAT32_LongEntry *entry, const char *filename, 
			      BYTE checksum, int n);

#endif // __FAT32_NAME_H__


