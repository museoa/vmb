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
#include "FAT32_Cache.h"
#include "FAT32_Table.h"

//-----------------------------------------------------------------------------
//							FAT Sector Buffer
//-----------------------------------------------------------------------------

FAT32_Cache FATCache, FAT2Cache; /* the secont is only to write multiple FATs */



#ifdef FATBUFFER_IMMEDIATE_WRITEBACK
#define FAT32_UpdateFAT() FAT32_WriteCache(&FATCache)
#else
#define FAT32_UpdateFAT() 
#endif



#ifdef INCLUDE_WRITE_SUPPORT
static bool FAT32_SetClusterValue(UINT32 Cluster, UINT32 value)
/* this should be the only function writing to the FAT! 
   it should make sure that each time we write the FATCache
   we write the secont FAT too.
*/
{ int i;
	UINT32 sector, offset;
	sector = Cluster / 128;
	offset = (Cluster - (sector * 128)) * 4; 

	FAT32_ReadCache(&FATCache,FAT32.fat_begin_lba+sector);
	FAT32_SET_32BIT(FATCache, offset, value);
	 
        for (i=1;i<FAT32.NumFATs;i++)
	  { /* write the other FATs too */
	  }

        FAT32_UpdateFAT(); 

	return true;					 
} 
#endif

void FAT32_FAT_shutdown(void)
{ FAT32_WriteCache(&FATCache);
  FAT32_WriteCache(&FAT2Cache);
}

/*!
 * \fn      bool FAT32_FindLBABegin(BYTE *buffer, UINT32 *lbaBegin)
 * \brief   This function is used to find the LBA Address of the first volume on the disc. Also
 *          checks are performed on the signature and identity codes to make sure the partition is FAT32
 * \param   lbaBegin    FIXME
 * \return  Boolean, success if something was found otherwise false
 */




static bool FAT32_FindLBABegin(UINT32 *lbaBegin)
{ BYTE typecode;

  if (!FAT32_ReadCache(&FATCache,0))         ///< Load MBR (LBA 0)
        return false;

  ///< Make Sure 0x55 and 0xAA are at end of sector
  if (FAT32_GET_16BIT(FATCache, FAT32_SIGNATURE_OFFSET)!=FAT32_SIGNATURE)    
    return false;
        
  ///< Check the partition type code
  typecode = FAT32_GET_8BIT(FATCache, PARTITION1_TYPECODE_LOCATION);
  switch(typecode)   
  { case 0x05: break;
    case 0x06: break;
    case 0x0B: break;
    case 0x0C: break;
    case 0x0E: break;
    case 0x0F: break;
    default:   if (typecode > 0x06)
                 return false;
               break;
  }
  ///< Read LBA Begin for FAT32 File system is located for partition
  *lbaBegin=FAT32_GET_32BIT(FATCache, PARTITION1_LBA_BEGIN_LOCATION);    
  return true; 
}



/*!
 * \fn      bool FAT32_Init(void)
 * \brief   Uses FAT32_FindLBABegin to find the LBA for the volume, and loads into memory some 
 *          specific details of the partition which are used in further calculations.
 * \return  Boolean, success returns true otherwise false
 */
bool FAT32_Init(void)
{ 
    LOG( "LOG4C_PRIORITY_DEBUG", "FAT32_Init() begin" );
 
    FAT32_InitCache(&FATCache);
    FAT32_InitCache(&FAT2Cache);

    if (!FAT32_FindLBABegin(&(FAT32.lba_begin)))      ///< Check Volume 1 and find LBA address
    {
        LOG( "LOG4C_PRIORITY_DEBUG", "FAT32_FindLBABegin is *NOT* ok" );
        return false;
    }

    if (!FAT32_ReadCache(&FATCache,FAT32.lba_begin))     ///< Load Volume 1 table into sector buffer
        return false;
      
    if (FAT32_GET_16BIT(FATCache, FAT32_SIGNATURE_OFFSET)!=FAT32_SIGNATURE) 
        return false;

    FAT32.BytePerSector =FAT32_GET_16BIT(FATCache, BPB_BytsPerSec);
    if (FAT32.BytePerSector!=512)    ///< Make sure there are 512 bytes per cluster
        return false;
    FAT32.SectorsPerCluster         = FAT32_GET_8BIT(FATCache,BPB_SecPerClus); 
    FAT32.Reserved_Sectors          = FAT32_GET_16BIT(FATCache, BPB_RsvdSecCnt);
    FAT32.NumFATs                   = FAT32_GET_8BIT(FATCache,BPB_NumFATs);

    FAT32.FATSz = FAT32_GET_16BIT(FATCache, BPB_FATSz16);
    if(FAT32.FATSz== 0)
      FAT32.FATSz = FAT32_GET_32BIT(FATCache, BPB_FAT32_FATSz32);

    FAT32.RootDir_First_Cluster     = FAT32_GET_32BIT(FATCache, BPB_FAT32_RootClus);
    FAT32.fs_info_sector            = FAT32_GET_16BIT(FATCache, BPB_FAT32_FSInfo);

    FAT32.fat_begin_lba = FAT32.lba_begin + FAT32.Reserved_Sectors;     ///< First FAT LBA address
    ///< The address of the first data cluster on this volume
    FAT32.cluster_begin_lba = FAT32.fat_begin_lba + (FAT32.NumFATs * FAT32.FATSz);  

    if ((FAT32_GET_16BIT(FATCache, BPB_RootEntCnt))!=0)
      return false; /* we deal only with a FAT32 partition */
         
    FAT32.TotSec = FAT32_GET_16BIT(FATCache, BPB_TotSec16);
    if(FAT32.TotSec== 0)
      FAT32.TotSec = FAT32_GET_32BIT(FATCache, BPB_TotSec32);

    FAT32.DataSec = FAT32.TotSec - (FAT32.Reserved_Sectors +
                        (FAT32.NumFATs*FAT32.FATSz));

    FAT32.CountofClusters = FAT32.DataSec / FAT32.SectorsPerCluster;

#if 0
    /* we will permit even small FAT32 disks */ 
    if(CountofClusters < 4085)
    {
        LOG( "LOG4C_PRIORITY_DEBUG", "Detected FAT12 - aborting" );
        return false;
    }
    else if(CountofClusters < 65524)
    {
        LOG( "LOG4C_PRIORITY_DEBUG", "Detected FAT16 - aborting" );
        return false;
    }
    else
    {
        LOG( "LOG4C_PRIORITY_DEBUG", "Ok - found a FAT32" );
        return true;
    }
    }
#endif

    return true;
}


//-----------------------------------------------------------------------------
//						General FAT Table Operations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// FAT32_FindNextCluster: Return Cluster number of next cluster in chain by 
// reading FAT table and traversing it. Return 0xffffffff for end of chain.
//-----------------------------------------------------------------------------
UINT32 FAT32_FindNextCluster(UINT32 Current_Cluster)
{
	UINT32 FAT_sector_offset, position;
	UINT32 nextcluster;

	// Why is '..' labelled with cluster 0 when it should be 2 ??
	if (Current_Cluster==0) Current_Cluster=2;

	// Find which sector of FAT table to read
	FAT_sector_offset = Current_Cluster / 128;

	// Read FAT sector into buffer
	if (!FAT32_ReadCache(&FATCache,FAT32.fat_begin_lba+FAT_sector_offset))
		return (FAT32_EOC_FLAG); 

	// Find 32 bit entry of current sector relating to cluster number 
	position = (Current_Cluster - (FAT_sector_offset * 128)) * 4; 

	// Read Next Clusters value from Sector Buffer
	nextcluster = FAT32_GET_32BIT(FATCache,position);	 

	// Mask out MS 4 bits (its 28bit addressing)
	nextcluster = nextcluster & FAT32_C_MASK;		

	if (FAT32_EOC(nextcluster)) 
	   return FAT32_EOC_FLAG; 
	else 
	   return nextcluster;						 
} 

//-----------------------------------------------------------------------------
// FAT32_FindBlankCluster: Find a free cluster entry by reading the FAT
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
UINT32 FAT32_FindBlankCluster(void)
/* find a free cluster by searching in the FAT.
   returns 0 if no free cluster is found
*/
{       UINT32 cluster;
        UINT32 clusterbound;
        UINT32 lba;
        int offset;

	cluster = 2; /* first possibly free cluster */
        clusterbound = FAT32.CountofClusters+2;
        offset = cluster*4;
        lba = FAT32.fat_begin_lba;
	FAT32_ReadCache(&FATCache,lba);

        while (cluster<clusterbound)
	{ if (offset>=512)
          { lba++;
            offset=0;
            if (!FAT32_ReadCache(&FATCache,lba)) return 0;
          }
	  if((FAT32_GET_32BIT(FATCache,offset) & FAT32_C_MASK)==0)
            return cluster;
          offset+=4;
          cluster++;
	}
        return 0;
}	 
#endif


//-----------------------------------------------------------------------------
// FAT32_FreeClusterChain: Follow a chain marking each element as free
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
bool FAT32_FreeClusterChain(UINT32 StartCluster)
{
	UINT32 last_cluster;
	UINT32 next_cluster = StartCluster;
	
	// Loop until end of chain
	while ( !FAT32_EOC(next_cluster) && (next_cluster!=0x00000000) )
	{
		last_cluster = next_cluster;

		// Find next link
		next_cluster = FAT32_FindNextCluster(next_cluster);

		// Clear last link
		FAT32_SetClusterValue(last_cluster, 0x00000000);
	}

	return true;
} 
#endif
//-----------------------------------------------------------------------------
// FAT32_AddClusterToEndofChain: Follow a chain marking and then add a new entry
// to the current tail.
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
bool FAT32_AddFreeSpaceToChain(UINT32 *lastCluster)
{
    UINT32 nextcluster;

    // Start looking for free clusters from the beginning
    nextcluster =FAT32_FindBlankCluster();
    if (nextcluster!=0)
    {
        if (*lastCluster!=0)
          // Point last to this
          FAT32_SetClusterValue(*lastCluster, nextcluster);
        
        // Point this to end of file
        FAT32_SetClusterValue(nextcluster, FAT32_EOC_FLAG);

        // Adjust argument reference
        *lastCluster = nextcluster;

        return true;
    }
    else
        return false;
}

bool FAT32_AddClusterToEndofChain(UINT32 StartCluster, UINT32 newEntry)
{
	UINT32 last_cluster = FAT32_EOC_FLAG;
	UINT32 next_cluster = StartCluster;

	if (FAT32_EOC(StartCluster))
		return false;
	
	// Loop until end of chain
	while ( !FAT32_EOC(next_cluster))
	{
		last_cluster = next_cluster;

		// Find next link
		next_cluster = FAT32_FindNextCluster(next_cluster);

		if (next_cluster==0x00000000)
			return false;
	}

	// Add link in for new cluster
	FAT32_SetClusterValue(last_cluster, newEntry);

	// Mark new cluster as end of chain
	FAT32_SetClusterValue(newEntry, FAT32_EOC_FLAG);

	return true;
} 
#endif

/*!
 * \fn      bool FAT32_FileOffset2lba(UINT32 *Startcluster, UINT32 offset)
 * \brief   From the provided files startcluster and sector offset compute lba
 *          if extend is set, extend the chain as needed.
 * \return  True if success, returns False if not (including if read out of range)
 */

UINT32 FAT32_ClusterOffset2lba(UINT32 *Cluster, UINT32 offset, int extend)
{
  UINT32 SectortoRead, ClustertoRead, LastCluster, ClusterChain;
    UINT32 i;

    LOGVARS( "LOG4C_PRIORITY_VVERBOSE", "Inside of FAT32_ClusterOffset2lba.",
             "uu", ClusterChain, offset );

    if (*Cluster==0 && /* empty chain */
        (!extend || !FAT32_AddFreeSpaceToChain(Cluster) ))
      return 0;
   
    ClusterChain = *Cluster;
    ClustertoRead = offset / FAT32.SectorsPerCluster;                   
    SectortoRead  = offset - (ClustertoRead*FAT32.SectorsPerCluster);
    for (i=0; i<ClustertoRead; i++)              ///< Follow chain to find cluster to read
    {  LastCluster = ClusterChain;  
       ClusterChain = FAT32_FindNextCluster( ClusterChain );
       if( FAT32_EOC(ClusterChain) )
       {
          LOG( "LOG4C_PRIORITY_VVERBOSE", "We reached the end of the cluster chain" );
          if (!extend)
            return 0;
          if( !FAT32_AddFreeSpaceToChain( &LastCluster ) )   ///< Add another cluster
	    return 0;
          ClusterChain = LastCluster;
        }
    }

    LOGVARS( "LOG4C_PRIORITY_VVERBOSE", "ClusterChain = FAT32_FileOffset2lba;", "x", ClusterChain );

    return FAT32_LBAofCluster(ClusterChain)+SectortoRead;          
}



/*!
 * \fn      UINT32 FAT32_LBAofCluster(UINT32 Cluster_Number) 
 * \brief   This function converts a cluster number into a sector / LBA number.
 * \param   Cluster_Number
 * \return  Returns an unsigned integer (uint32) which represents 
 *           the Logical Block Address of a given cluster number
 */
UINT32 FAT32_LBAofCluster(UINT32 Cluster_Number)
{
    return ((FAT32.cluster_begin_lba + ((Cluster_Number-2)*FAT32.SectorsPerCluster)));
}

//-----------------------------------------------------------------------------
// FAT32_AllocateFreeSpace: Add an ammount of free space to a file either from
// 'startCluster' if newFile = false, or allocating a new start to the chain if
// newFile = true.
//-----------------------------------------------------------------------------
bool FAT32_AllocateFreeSpace(bool newFile, UINT32 *startCluster, UINT32 size)
{
    UINT32 clusterSize;
    UINT32 clusterCount;
    UINT32 nextcluster;

    if (size==0)
        return false;

    // Work out size and clusters
    clusterSize = FAT32.SectorsPerCluster * 512;
    clusterCount = (size / clusterSize);

    // If any left over
    if (size-(clusterSize*clusterCount))
        clusterCount++;

    // Allocated first link in the chain if a new file
    if (newFile)
    {
       nextcluster=FAT32_FindBlankCluster();
       if (nextcluster==0)
            return false;

        // If this is all that is needed then all done
        if (clusterCount==1)
        {
            FAT32_SetClusterValue(nextcluster, FAT32_EOC_FLAG);
            *startCluster = nextcluster;
            return true;
        }
    }
    // Allocate from end of current chain (startCluster is end of chain)
    else
        nextcluster = *startCluster;

    while (clusterCount)
    {
        if (!FAT32_AddFreeSpaceToChain(&nextcluster))
            return false;

        clusterCount--;
    }

    return true;
}
