/*!
 *
 * \file        FAT32_Misc.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Misc.h,v 1.1 2016-02-16 10:17:32 ruckert Exp $
 * \brief       FAT32 Library, Misc. Functions
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

#ifndef __FAT32_MISC_H__
#define __FAT32_MISC_H__

///! Defines
#define MAX_LONGFILENAME_ENTRIES    20

///! Globals
struct
{
    ///! Long File Name Structure (max 260 LFN length)
    BYTE String[MAX_LONGFILENAME_ENTRIES][13];
    BYTE no_of_strings;
} FAT32_LFN;

///! Prototypes
void FATMisc_ClearLFN( bool wipeTable );
void FATMisc_CacheLFN( BYTE *entryBuffer );
void FATMisc_GetLFNCache( BYTE *strOut );
int FATMisc_If_LFN_TextOnly( FAT32_ShortEntry *entry );
int FATMisc_If_LFN_Invalid( FAT32_ShortEntry *entry );
int FATMisc_If_LFN_Exists( FAT32_ShortEntry *entry );
int FATMisc_If_noLFN_SFN_Only( FAT32_ShortEntry *entry );
int FATMisc_If_dir_entry( FAT32_ShortEntry *entry );
int FATMisc_If_file_entry( FAT32_ShortEntry *entry );
int FATMisc_LFN_to_entry_count( char *filename );
void FATMisc_LFN_to_lfn_entry( char *filename, BYTE *buffer, int entry, BYTE sfnChk );
void FATMisc_Create_sfn_entry( char *shortfilename, UINT32 size, UINT32 startCluster, FAT32_ShortEntry *entry );
bool FATMisc_CreateSFN( char *sfn_output, char *filename );
bool FATMisc_GenerateTail( char *sfn_output, char *sfn_input, UINT32 tailNum );

#endif // __FAT32_MISC_H__

// vim:ts=4:tw=100:wm=100

