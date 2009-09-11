/*!
 *
 * \file        FAT32_FileLib.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_FileLib.h,v 1.3 2009-09-11 09:13:03 ruckert Exp $
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

///! Defines for fat32_fseek
#ifndef SEEK_CUR
    #define SEEK_CUR    1
#endif

#ifndef SEEK_END
    #define SEEK_END    2
#endif

#ifndef SEEK_SET
    #define SEEK_SET    0
#endif

///! External Prototypes
void        fat32_initialize(void);
void        fat32_shutdown(void);
int         fat32_fopen(char *path, int mode, int handle );
int         fat32_fclose(int handle);
int         fat32_fread(void *buffer, unsigned int size, int handle );
int         fat32_fgetc(int handle);
char*       fat32_fgets(char *s, unsigned int size, int handle);
char*       fat32_fgetws(char *s, unsigned int size, int handle);
int         fat32_fwrite(const void *buffer, unsigned int size, int handle);
int         fat32_fputc(int c, int handle);
int         fat32_fputs(const char * str, int handle);
int         fat32_fputws(const char * str, int handle);
int         fat32_fseek(long offset , int origin, int handle);
long        fat32_ftell(int handle);
int         fat32_remove(const char * filename);
int         fat32_mkdir(const char *pathname);
#endif  // __FILELIB_H__

