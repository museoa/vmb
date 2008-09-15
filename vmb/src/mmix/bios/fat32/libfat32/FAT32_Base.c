/*!
 *
 * \file        FAT32_Base.c
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Base.c,v 1.1 2008-09-15 13:49:47 ruckert Exp $    // 2.0
 * \brief       FAT32 Library, Base methods
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
#include "FAT32_Definitions.h"
#include "FAT32_Base.h"
#include "FAT32_Table.h"


/*!
 * \fn      bool FAT32_FindLBABegin(BYTE *buffer, UINT32 *lbaBegin)
 * \brief   This function is used to find the LBA Address of the first volume on the disc. Also
 *          checks are performed on the signature and identity codes to make sure the partition is FAT32
 * \param   buffer      FIXME
 * \param   lbaBegin    FIXME
 * \return  Boolean, success if something was found otherwise false
 */
static bool FAT32_FindLBABegin(BYTE *buffer, UINT32 *lbaBegin)
{
    if (buffer==NULL)
        return false;

    if (!FAT_ReadSector(0, buffer))                                                         ///< Load MBR (LBA 0) into the 512 byte buffer
        return false;
    
    if (GET_16BIT_WORD(buffer, Signature_Position)!=Signature_Value)                        ///< Make Sure 0x55 and 0xAA are at end of sector
        return false;
        
    // TODO: Verify this 

    switch(buffer[PARTITION1_TYPECODE_LOCATION])                                            ///< Check the partition type code
    {
    case 0x0B: break;
    case 0x06: break;
    case 0x0C: break;
    case 0x0E: break;
    case 0x0F: break;
    case 0x05: break;
    default:
        if (buffer[PARTITION1_TYPECODE_LOCATION] > 0x06)
            return false;
        break;
    }

    *lbaBegin=GET_32BIT_WORD(buffer, PARTITION1_LBA_BEGIN_LOCATION);                        ///< Read LBA Begin for FAT32 File system is located for partition

    return true;                                                                            ///< Return the LBA address of FAT table
}

/*!
 * \fn      UINT32 FAT32_LBAofCluster(UINT32 Cluster_Number) 
 * \brief   This function converts a cluster number into a sector / LBA number.
 * \param   Cluster_Number
 * \return  Returns an unsigned integer (uint32) which represents the Logical Block Address of a given cluster number
 */
UINT32 FAT32_LBAofCluster(UINT32 Cluster_Number)
{
    return ((FAT32.cluster_begin_lba + ((Cluster_Number-2)*FAT32.SectorsPerCluster)));
}

BYTE tmp_buffer[512];

/*!
 * \fn      bool FAT32_Init(void)
 * \brief   Uses FAT32_FindLBABegin to find the LBA for the volume, and loads into memory some 
 *          specific details of the partition which are used in further calculations.
 * \return  Boolean, success returns true otherwise false
 */
bool FAT32_Init(void)
{
    LOG( "LOG4C_PRIORITY_DEBUG", "FAT32_Init() begin" );

    BYTE Number_of_FATS;
    UINT16 Reserved_Sectors;
    UINT32 LBA_BEGIN;
    UINT32 FATSz;
    UINT32 RootDirSectors;
    UINT32 TotSec;
    UINT32 DataSec;
    UINT32 CountofClusters;

    FAT32_InitFatBuffer();

    if (FAT32_FindLBABegin(tmp_buffer, &LBA_BEGIN))      ///< Check Volume 1 and find LBA address
    {
        LOG( "LOG4C_PRIORITY_DEBUG", "FAT32_FindLBABegin() is ok" );
        FAT32.lba_begin = LBA_BEGIN;

        if (!FAT_ReadSector(LBA_BEGIN, tmp_buffer))     ///< Load Volume 1 table into sector buffer
            return false;

        if (GET_16BIT_WORD(tmp_buffer, BPB_BytsPerSec)!=0x200)    ///< Make sure there are 512 bytes per cluster
            return false;

        FAT32.SectorsPerCluster         = tmp_buffer[BPB_SecPerClus];    ///< Load Parameters of FAT32
        Reserved_Sectors                = GET_16BIT_WORD(tmp_buffer, BPB_RsvdSecCnt);
        Number_of_FATS                  = tmp_buffer[BPB_NumFATs];
        FAT32.fat_sectors               = GET_32BIT_WORD(tmp_buffer, BPB_FAT32_FATSz32);
        FAT32.RootDir_First_Cluster     = GET_32BIT_WORD(tmp_buffer, BPB_FAT32_RootClus);
        FAT32.fs_info_sector            = GET_16BIT_WORD(tmp_buffer, BPB_FAT32_FSInfo);

        FAT32.fat_begin_lba = LBA_BEGIN + Reserved_Sectors;     ///< First FAT LBA address

        FAT32.cluster_begin_lba = FAT32.fat_begin_lba + (Number_of_FATS * FAT32.fat_sectors);   ///< The address of the first data cluster on this volume

        if (GET_16BIT_WORD(tmp_buffer, FAT32_SIGNATURE_OFFSET)!=FAT32_SIGNATURE)  ///< This signature should be AA55
            return false;

        ///! Calculate the root dir sectors
        RootDirSectors = ((GET_16BIT_WORD(tmp_buffer, BPB_RootEntCnt) * 32) + (GET_16BIT_WORD(tmp_buffer, BPB_BytsPerSec) - 1)) / GET_16BIT_WORD(tmp_buffer, BPB_BytsPerSec);

        if(GET_16BIT_WORD(tmp_buffer, BPB_FATSz16) != 0)
            FATSz = GET_16BIT_WORD(tmp_buffer, BPB_FATSz16);
        else
            FATSz = GET_32BIT_WORD(tmp_buffer, BPB_FAT32_FATSz32);

        if(GET_16BIT_WORD(tmp_buffer, BPB_TotSec16) != 0)
            TotSec = GET_16BIT_WORD(tmp_buffer, BPB_TotSec16);
        else
            TotSec = GET_32BIT_WORD(tmp_buffer, BPB_TotSec32);

        DataSec = TotSec - (GET_16BIT_WORD(tmp_buffer, BPB_RsvdSecCnt) + (tmp_buffer[BPB_NumFATs] * FATSz) + RootDirSectors);

       CountofClusters = DataSec / FAT32.SectorsPerCluster;
#if 0
	/* we will permit even small FAT32 disks */ 

        if(CountofClusters < 4085)
        {
            LOG( "LOG4C_PRIORITY_DEBUG", "Detected FAT12 - aborting" );
            return false;
        }
        else
        {
            if(CountofClusters < 65524)
            {
                LOGVARS( "LOG4C_PRIORITY_DEBUG", "Count of clusters", "d", CountofClusters );
                LOG( "LOG4C_PRIORITY_DEBUG", "Detected FAT16 - aborting" );
                return false;
            }
        }
#endif
        LOG( "LOG4C_PRIORITY_DEBUG", "Ok - found a FAT32" );
        return true;
     }
     else
     {
        LOG( "LOG4C_PRIORITY_DEBUG", "FAT32_FindLBABegin is *NOT* ok" );
        return false;
     }

    return true;
}

// vim:ts=4:tw=100:wm=100
