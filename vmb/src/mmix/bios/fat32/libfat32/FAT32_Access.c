/*!
 *
 * \file        FAT32_Access.c
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Access.c,v 1.5 2009-09-08 13:12:02 ruckert Exp $ // 2.0
 * \brief       FAT32 Library, Access
 * \details     {
 * }
 * \note        {
 *                Copyright (c) 2003-2007, Rob Riglar <rob@robriglar.com>, FAT32 File IO Library
 *                Copyright (c) 2007-2008, Bjoern Rennhak, Virtual Motherboard project.
 *                All rights reserved, see COPYRIGHT file for more details.
 * }
 *
 */


///! Custom includes
#include <string.h>

#include "FAT32_Definitions.h"
#include "FAT32_Table.h"
#include "FAT32_Access.h"
#include "FAT32_Name.h"
#include "FAT32_Cache.h"

FAT32_Cache DirCache;


void FAT32_dir_shutdown(void)
{  FAT32_WriteCache(&DirCache);
}


bool FAT32_SectorReader( UINT32 Startcluster, UINT32 offset )
{
    UINT32 lba;

    lba = FAT32_ClusterOffset2lba(&Startcluster, offset,false);
    if (lba==0) return false;

    return FAT32_ReadCache(&DirCache,lba);
}


#ifdef INCLUDE_WRITE_SUPPORT

/*!
 * \fn      bool FAT32_SectorWriter(UINT32 *Startcluster, UINT32 offset)
 * \brief   Write to the provided startcluster and sector offset 
 * \return  True if success, returns False if not
 */
bool FAT32_SectorWriter(UINT32 *Startcluster, UINT32 offset)
{
    UINT32 lba;

    lba = FAT32_ClusterOffset2lba(Startcluster, offset, true);
    if (lba==0) return false;
    
    DirCache.lba=lba;
    DirCache.dirty=true;

    return FAT32_WriteCache(&DirCache);    
}

#endif


UINT32 FAT32_GetFilelength(FAT32_ShortEntry *sfEntry)
{ return GET_32BIT_WORD(sfEntry->Size,0);
}

void FAT32_SetFilelength(FAT32_ShortEntry *sfEntry, UINT32 length)
{ SET_32BIT_WORD(sfEntry->Size,0,length);
  DirCache.dirty=true;
}

UINT32 FAT32_GetFileStartcluster(FAT32_ShortEntry *sfEntry)
{ return (((UINT32)GET_16BIT_WORD(sfEntry->FstClusterHI,0))<<16)
                                     + GET_16BIT_WORD(sfEntry->FstClusterLO,0);
}
void FAT32_SetFileStartcluster(FAT32_ShortEntry *sfEntry, UINT32 cluster)
{ SET_16BIT_WORD(sfEntry->FstClusterHI,0, (cluster>>16)&0xFFFF);
  SET_16BIT_WORD(sfEntry->FstClusterLO,0, cluster&0xFFFF);
  DirCache.dirty=true;
}


int FAT32_FindNextFile(UINT32 cluster, int *psector, int *pitem, 
    UINT32 *lba1, int *offset1, UINT32 *lba2, int *offset2)
/* finds in the directory given by its Start cluster sector and item, 
   count contiguous entries that belong to one filename. 
   It returns the number of
   entries that belong to this file.
   a return value of 0 means no file found.
   a return value of 1 means a single short entry was found.
   a return value > 1 means a sequence of long entries was found,
   the first of these at lba1 and offset1, 
   the last at lba2 and offset2. (they are equal if entryCount=1).
*/
{  
    int sector = *psector;
    BYTE item = *pitem;
    UINT16 recordoffset;
    int count, n, expect;
    FAT32_ShortEntry *dirEntry;

    count = expect=0;

    while (FAT32_SectorReader(cluster, sector)) 
    { for (; item<16;item++)
      { recordoffset = (32*item);
        dirEntry=(FAT32_ShortEntry*)(DirCache.buffer+recordoffset);
        if ((n=FATName_is_lfn_entry(dirEntry))!=0) /* lfn entry */
	  { if (n<0) 
	    { count = -n;
              *lba1=DirCache.lba;
              *offset1=recordoffset;
	      expect = count -1;
	    }
	    else if (n == expect)
	      expect--;
	    else
	      count=expect=0;
	  }
	else if ( FATName_is_sfn_entry(dirEntry))
	  { if (count==0)
            { *lba1=*lba2=DirCache.lba;
              *offset1=*offset2=recordoffset;
            }
	    else
            { *lba2=DirCache.lba;
              *offset2=recordoffset;
            }
	    count++;
	    *psector=sector;
	    *pitem=item;
	    return count;
	  }
        else
	  { count=0;
	  }
      }
      item = 0;
      sector++;
    }
    return 0;
}


static int FAT32_Name_Equal(const char *filename, int count, 
                      UINT32 lba1, int offset1, UINT32 lba2, int offset2)
/* determine if the filename matches either the
short entry stored at lba2/offset2 or 
if (count>1) the long name that preceedes the short entry starting at lba1/offset1
with count-1 entries.
check the checksums to make sure the sequence of long entries is valid.
return 1 if equal,
return 0 if different.
According to the Microsoft white paper, the long name, if pressent, 
should be checked first. I do it the other way round.
first.
*/
{ FAT32_ShortEntry *shortEntry;
  FAT32_LongEntry *longEntry;
  BYTE checksum;
  int n,i;

  if (!FAT32_ReadCache(&DirCache,lba2)) return 0;
  shortEntry = (FAT32_ShortEntry*)(DirCache.buffer+offset2);
  if ( Name_CompareSN(shortEntry->Name,filename))
    return 1;
  if (count <=1)
    return 0;
  checksum =  FATName_ChkSum(shortEntry->Name); 
  /* we read backward through the long entries and compare */
  for (n=1; n<count; n++)
    { offset2 = offset2-32;
      if (offset2 < 0)
	{ lba2=lba1;
          offset2 = 512-32;
          if (!FAT32_ReadCache(&DirCache,lba2)) return 0;
	}
      longEntry = (FAT32_LongEntry*)(DirCache.buffer+offset2);
      i=FATName_Compare_entry(longEntry,filename,checksum,n);
      if (i==0) return 0;
      filename=filename+i;
    }
  return 1;
}

/*!
 * \fn      FAT32_ShortEntry *FAT32_GetFileEntry(UINT32 Cluster, char *nametofind)
 * \brief   Find the file entry for a filename
 * \param   Cluster     Needs an unsigned integer (uint32) which represents the start cluster of the directory
 * \param   nametofind  Needs a char pointer which represents the name for which to search
 * \return  Returns a pointer to the sfEntry or NULL (if unsuccessful)
 * \sideeffects has FAT32_LongFilename contain the files longname (if it exists) or "" otherwise
 *              and has long_start_lba and long_start_offset set also 

 */

FAT32_ShortEntry *FAT32_GetFileEntry( UINT32 cluster, char *nametofind)
{
  int sector, item, count;
    UINT32 lba1, lba2;
    int offset1, offset2;
    sector=0;
    item = 0;
    while ((count= FAT32_FindNextFile(cluster, &sector, &item, 
				      &lba1, &offset1, &lba2, &offset2))!=0)
      {	if (FAT32_Name_Equal(nametofind,count, lba1, offset1, lba2, offset2))
	  {  FAT32_ReadCache(&DirCache,lba2);
	     return (FAT32_ShortEntry *)(DirCache.buffer+offset2);
	  }
        else
          item++;
      }
    return 0;
}


FAT32_ShortEntry * FAT32_GetFileShort(UINT32 Cluster, BYTE *shortname)
{
  int item;
  int sector;
  FAT32_ShortEntry *entry;
  sector = 0;
  while (FAT32_SectorReader(Cluster, sector++)) 
  { for (item=0; item<16;item++)
    { entry = (FAT32_ShortEntry*)(DirCache.buffer+32*item); 
      if( FATName_is_sfn_entry(entry) &&  ///< Normal Entry, only 8.3 Text
          strncmp((const char*)entry->Name, (const char*)shortname, 11)==0)
        return entry;
    }
  }
  return NULL;
}

bool FAT32_GetDirectory(const char *fullpath, char* path, char *name , UINT32 *parentcluster)
/* from full path get path, filename, and startcluster of the directory */
{ const char *tail;
  char *out;
  UINT32 startcluster;
  FAT32_ShortEntry *sfEntry;
  
  startcluster = FAT32.RootDir_First_Cluster;
  out = path;
  *out++='/';
  tail = Name_GetFirstDirectory(fullpath, out, MAX_LONG_PATH -(out-path));
  while (tail!=NULL)
    { int d;
      sfEntry=FAT32_GetFileEntry(startcluster, out);
      if (sfEntry!=NULL && FATName_is_dir_entry(sfEntry))
        startcluster = FAT32_GetFileStartcluster(sfEntry);
      else
      {  *name = 0;
	 *path = 0;
         *parentcluster = 0;
         return false;
      }
      d = tail-fullpath;
      out[d-1]='/';
      out+=d;
      fullpath=tail;
      tail = Name_GetNextDirectory(fullpath, out, MAX_LONG_PATH -(out-path));
    }
  Name_Trim(name,out);
  *--out=0; /* remove trailing '/' */
  *parentcluster = startcluster;
  return true;
}

#ifdef INCLUDE_WRITE_SUPPORT

/*!
 * \fn      bool FAT32_UpdateFileLength(UINT32 Cluster, char *shortname, UINT32 fileLength)
 * \brief   Find a SFN entry and update it
 * \note    shortname is XXXXXXXXYYY not XXXXXXXX.YYY
 * \param   Cluster     Needs an unsigned integer (uint32) which represents a cluster on the Fat32 image
 * \param   shortname   Needs a char pointer which holds the shortname 
 * \param   fileLength  Needs an unsigned integer (uint32) which holds the file length
 * \return  Returns boolean, true if success, false if failure
 */
bool FAT32_UpdateFileLength(UINT32 parentcluster, BYTE *shortname, UINT32 fileLength)
{  FAT32_ShortEntry *Entry;

  Entry = FAT32_GetFileShort(parentcluster, shortname);
  if (Entry==NULL) return false;
  SET_32BIT_WORD(Entry->Size,0,fileLength);
        // TODO: Update last write time
  DirCache.dirty=true;
  return FAT32_WriteCache(&DirCache);   ///< Write sector back
}

#endif



#ifdef INCLUDE_WRITE_SUPPORT

/*!
 * \fn      bool FAT32_MarkFileDeleted(UINT32 Cluster, char *shortname)
 * \brief   Find a SFN entry and mark if as deleted
 * \note    shortname is XXXXXXXXYYY not XXXXXXXX.YYY
 * \param   Cluster     Needs an unsigned integer (uint32) which represents a cluster on the Fat32 image
 * \param   shortname   Needs a char pointer which holds the shortname 
 * \return  Returns boolean, true if success, false if failure
 */
bool FAT32_MarkFileDeleted(UINT32 Cluster, BYTE *shortname)
{  FAT32_ShortEntry *Entry;

  /* This should also remove associated long filename entries */
   Entry = FAT32_GetFileShort(Cluster, shortname);
   if (Entry==NULL) return false;
   Entry->Name[0] = 0xE5;                  
   DirCache.dirty=true;
   return FAT32_WriteCache(&DirCache);
}
#endif

//-----------------------------------------------------------------------------
// FAT32_FindFreeOffset: Find a free space in the directory for a new entry 
// which takes up 'entryCount' blocks (or allocate some more)
//-----------------------------------------------------------------------------
bool FAT32_FindFreeOffset(UINT32 dirCluster, int entryCount, UINT32 *lba1, int *offset1, UINT32 *lba2, int *offset2)
/* finds in the directory given by its Start Cluster, entryCount contiguous entries.
   the first of this at lba1 and offset1, 
   the last at lba2 and offset2. (they are equal if entryCount=1).
   extends the directory if needed and possible.
*/
{
    BYTE item;
    UINT16 recordoffset;
    int currentCount;
    int sector;
    FAT32_ShortEntry *dirEntry;
    UINT32 newCluster;
    int i;

    sector = 0;
    currentCount=0;
    while (FAT32_SectorReader(dirCluster, sector++)) 
    { for (item=0; item<16;item++)
      { recordoffset = (32*item);
        dirEntry=(FAT32_ShortEntry*)(DirCache.buffer+recordoffset);
	if (dirEntry->Attr ==  FILE_HEADER_BLANK ||
	    dirEntry->Attr ==  FILE_HEADER_DELETED )
	  currentCount++;
        else
          currentCount=0;
        if (currentCount==1)
	{ *lba1=DirCache.lba;
          *offset1=recordoffset;
        }  
        if (currentCount==entryCount)
	{ *lba2=DirCache.lba;
          *offset2=recordoffset;
          return true;
        }  
      }
    }

    // Run out of free space in the directory, allocate one additional cluster
    newCluster=FAT32_FindBlankCluster();
    if (newCluster==0)
       return false;
    if (!FAT32_AddClusterToEndofChain(dirCluster, newCluster))
       return false;
    *lba2 =  FAT32_LBAofCluster(newCluster);
    *offset2=32*(entryCount-currentCount-1);
    if (currentCount==0)
    { *lba1=*lba2;
      *offset1=0;
    }    
    for (i=0;i<FAT32.SectorsPerCluster;i++)
      FAT32_ZeroCache(&DirCache, *lba2+i);

    return true;
}  


//-----------------------------------------------------------------------------
// FAT32_AddFileEntry: Add a directory entry to a location found by FindFreeOffset
//-----------------------------------------------------------------------------
bool FAT32_AddFileEntry(UINT32 dirCluster,
                        char *filename, 
                        BYTE *shortfilename,
                        UINT32 startCluster, 
                        UINT32 size)
/* add a file entry to the directory given by dirCluster.
   if filename==NULL only a short entry is generated otherwise, a 
   sequence of long entries followed by a short entry.
*/
{
    int entryCount, n;
    FAT32_ShortEntry *shortEntry;
    FAT32_LongEntry *longEntry;
    UINT32 lba1, lba2;
    int offset1, offset2;
    BYTE checksum;
    if (filename!=NULL)
      entryCount = FATName_LFN_to_entry_count(filename);
    else
      entryCount = 0;

    if( !FAT32_FindFreeOffset(dirCluster, entryCount+1, &lba1, &offset1, &lba2, &offset2) )
        return false;
    if (entryCount>0) 
    { checksum =  FATName_ChkSum(shortfilename);
      for(n = entryCount;n>0;n--)
      { if (!FAT32_ReadCache(&DirCache,lba1)) 
          return false;
        longEntry = (FAT32_LongEntry*)(DirCache.buffer+offset1);
        FATName_Create_lfn_entrys(filename, entryCount,  n, checksum, longEntry); 
        DirCache.dirty=true;
        offset1+=32;
        if (offset1>=512)
        { lba1= lba2;
          offset1=offset2;
        }
      }
    }
    /* create short entry */
    if (!FAT32_ReadCache(&DirCache,lba2)) 
      return false;
    shortEntry = (FAT32_ShortEntry*)(DirCache.buffer+offset2);
    FATName_Create_sfn_entry(shortfilename, size, startCluster, shortEntry);
    DirCache.dirty=true;
    return FAT32_WriteCache(&DirCache);
}
