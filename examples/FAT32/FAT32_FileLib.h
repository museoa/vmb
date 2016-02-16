/*!
 *
 * \file        FAT32_FileLib.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_FileLib.h,v 1.1 2016-02-16 10:17:32 ruckert Exp $
 * \brief       FAT32 Library, File Library
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

#ifndef __FILELIB_H__
#define __FILELIB_H__

///! Defines
#ifndef SEEK_CUR
    #define SEEK_CUR    1
#endif

#ifndef SEEK_END
    #define SEEK_END    2
#endif

#ifndef SEEK_SET
    #define SEEK_SET    0
#endif

///! Global structures
typedef struct
{
    UINT32  parentcluster;
    UINT32  startcluster;
    UINT32  bytenum;
    UINT32  currentBlock;
    UINT32  filelength;
    char    path[ MAX_LONG_FILENAME ];
    char    filename[ MAX_LONG_FILENAME ];
    BYTE    filebuf[ 512 ];
    BYTE    shortfilename[ 11 ];
    bool    inUse;
    bool    inRoot;

    bool    Read;
    bool    Write;
    bool    Append;
    bool    Binary;
    bool    Erase;
} FL_FILE;


///! External Prototypes
void        fat32_initialize(void);
void        fat32_shutdown(void);
int         fat32_fopen(BYTE handle, char *path, int mode );
int        fat32_fclose(BYTE handle);
int         fat32_fgetc(BYTE handle);
int         fat32_fputc( int c, BYTE handle);
int         fat32_fputs( const char * str, BYTE handle);
int         fat32_fwrite( const void * data, int size, int count, BYTE handle);
int         fat32_fread( BYTE handle, BYTE * buffer, UINT32 count );
int         fat32_fseek( BYTE handle, UINT32 offset , int origin );
int         fat32_fgetpos( BYTE handle, UINT32 * position);
int         fat32_remove( const char * filename );

#endif  // __FILELIB_H__

// vim:ts=4:tw=100:wm=100
