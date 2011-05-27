//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//					        FAT32 File IO Library
//								    V2.0
// 	  							 Rob Riglar
//						    Copyright 2003 - 2007
//
//   					  Email: rob@robriglar.com
//
//-----------------------------------------------------------------------------
//
// This file is part of FAT32 File IO Library.
//
// FAT32 File IO Library is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// FAT32 File IO Library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FAT32 File IO Library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#include <string.h>
#include "FAT32_Definitions.h"
#include "FAT32_Disk.h"
#include "FAT32_Cache.h"

#define INVALID_LBA 0xFFFFFFFF

char somebuf[100] = {0};

void FAT32_InitCache(FAT32_Cache *c)
{ c->lba =  INVALID_LBA;
  c->dirty = false;
}


bool FAT32_ReadCache(FAT32_Cache *c, UINT32 lba)
{
  // Only do anything if sector not already loaded
  if (lba == c->lba) return true;
  
  // Writeback
  if (!FAT32_WriteCache(c)) return false;
  
  // Read next sector
  if (!FAT_ReadSector(lba, c->buffer)) return false;

  c->lba = lba;
  c->dirty=false;
  return true;
}

bool FAT32_ZeroCache(FAT32_Cache *c, UINT32 lba)
{
  // Writeback
  if (!FAT32_WriteCache(c)) return false;

  memset(c->buffer,0,512);  
  c->lba = lba;
  c->dirty=false;
  return true;
}

bool FAT32_MoveCache(FAT32_Cache *c, UINT32 lba)
{
  // Writeback
  if (!FAT32_WriteCache(c)) return false;

  c->lba = lba;
  c->dirty=false;
  return true;
}

bool FAT32_WriteCache(FAT32_Cache *c)
{ if (!c->dirty)
    return true;
  if (c->lba ==  INVALID_LBA)
    return true;
  if (!FAT_WriteSector(c->lba, c->buffer))
    return false;
  c->dirty = false;
  return true;
}

