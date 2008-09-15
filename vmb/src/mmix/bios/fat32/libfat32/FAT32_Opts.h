/*!
 *
 * \file        FAT32_Opts.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Opts.h,v 1.1 2008-09-15 13:49:47 ruckert Exp $
 * \brief       FAT32 Library, FAT32 Options
 * \details     {
 * }
 * \note        {
 *                Copyright (c) 2004-2007, Rob Riglar <rob@robriglar.com>, FAT32 File IO Library
 *                Copyright (c) 2007-2008, Bjoern Rennhak, Virtual Motherboard project.
 *                All rights reserved, see COPYRIGHT file for more details.
 * }
 *
 */


#ifndef __FAT32_OPTS_H__
#define __FAT32_OPTS_H__

///! Configuration

///! Max filename Length 
#define MAX_LONG_FILENAME               260

///! Max open files here for all handles from 0 to 0xFF
#define MAX_OPEN_FILES                    256

///! Writes to FAT are done immediately
#define FATBUFFER_IMMEDIATE_WRITEBACK     1

///! Include support for writing files
#define INCLUDE_WRITE_SUPPORT             1

#endif // __FAT32_OPTS_H__


