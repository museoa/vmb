/*!
 *
 * \file        FAT32_Cache.h
 * \author      Martin Ruckert
 * \version     $Id: FAT32_Cache.h,v 1.1 2015-09-24 13:00:43 ruckert Exp $
 * \brief       FAT32 Library, Caches for Sectors
 * \details     {
 * }
 * \note        {
 *                Copyright (c) 2009, Martin Ruckert
 *                All rights reserved, see COPYRIGHT file for more details.
 * }
 *
 */


#include "../define.h"
#include "FAT32_Opts.h"

#ifndef __FAT32_CACHE_H__
#define __FAT32_CACHE_H__

typedef struct 
{
	BYTE buffer[512];
	UINT32 lba;
	bool dirty;
} FAT32_Cache;


///! Prototypes
void FAT32_InitCache(FAT32_Cache *c);
bool FAT32_ReadCache(FAT32_Cache *c, UINT32 lba );
bool FAT32_ZeroCache(FAT32_Cache *c, UINT32 lba );
bool FAT32_MoveCache(FAT32_Cache *c, UINT32 lba );
bool FAT32_WriteCache(FAT32_Cache *c);

#define FAT32_GET_32BIT(c, location)	(GET_32BIT_WORD(c.buffer, location))
#define FAT32_SET_32BIT(c, location, value)	(SET_32BIT_WORD(c.buffer, location, value), c.dirty = true)
#define FAT32_GET_16BIT(c, location)	(GET_16BIT_WORD(c.buffer, location))
#define FAT32_SET_16BIT(c, location, value)	(SET_16BIT_WORD(c.buffer, location, value), c.dirty = true)
#define FAT32_GET_8BIT(c, location)	(c.buffer[location])
#define FAT32_SET_8BIT(c, location, value)	((c.buffer[location]=value), c.dirty = true)


#endif  // __FAT32_CACHE_H__


