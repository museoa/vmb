/*!
 *
 * \file        FAT32_FileString.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_FileString.h,v 1.2 2009-09-07 11:43:30 ruckert Exp $
 * \brief       FAT32 Library, File String capsulation
 * \details     {
 * }
 * \note        {
 *                Copyright (c) 2004-2007, Rob Riglar <rob@robriglar.com>, FAT32 File IO Library
 *                Copyright (c) 2007-2008, Bjoern Rennhak, Virtual Motherboard project.
 *                All rights reserved, see COPYRIGHT file for more details.
 * }
 *
 */


#ifndef __FILESTRING_H__
#define __FILESTRING_H__

#include "../define.h"

///! Prototypes

int FileString_Compare( char* strA, char* strB );
int FileString_CompareSN(BYTE* shortname, const char* name);
const char *FileString_GetFirstDirectory(const char * path, char * dir,int bound);
const char *FileString_GetNextDirectory(const char * path, char * q, int bound);
bool FileString_is_short(const char *filename);
void FileString_Trim(char *trimmed, const char *filename);

#ifndef NULL
    #define NULL 0
#endif

#endif  // __FILESTRING_H__

// vim:ts=4:tw=100:wm=100
