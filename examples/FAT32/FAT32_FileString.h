/*!
 *
 * \file        FAT32_FileString.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_FileString.h,v 1.1 2016-02-16 10:17:32 ruckert Exp $
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
int FileString_PathTotalLevels( char *path );
int FileString_GetSubString( char *Path, int levelreq, char *output );
int FileString_SplitPath( char *FullPath, char *Path, char *FileName );
int FileString_StrCmpNoCase( char *s1, char *s2, int n );
int FileString_GetExtension( char *str );
int FileString_TrimLength( char *str, int strLen );
int FileString_Compare( char* strA, char* strB );

#ifndef NULL
    #define NULL 0
#endif

#endif  // __FILESTRING_H__

// vim:ts=4:tw=100:wm=100
