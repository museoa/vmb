/*!
 *
 * \file        FAT32_Definitions.h
 * \author      Rob Riglar <rob@robriglar.com>
 * \version     $Id: FAT32_Definitions.h,v 1.1 2015-09-24 13:00:43 ruckert Exp $
 * \brief       FAT32 Library, Definitions
 * \details     {
 * }
 * \note        {
 *                Copyright (c) 2004-2007, Rob Riglar <rob@robriglar.com>, FAT32 File IO Library
 *                All rights reserved, see COPYRIGHT file for more details.
 * }
 *
 */




#ifndef __FAT32_DEFINITIONS_H__
#define __FAT32_DEFINITIONS_H__

#include "../define.h"

///! FAT32 Offsets
///! Name -> Offset

///! Boot Sector
#define BS_jmpBoot                 0    // Length = 3
#define BS_OEMName                 3    // Length = 8
#define BPB_BytsPerSec            11    // Length = 2
#define BPB_SecPerClus            13    // Length = 1
#define BPB_RsvdSecCnt            14    // Length = 2
#define BPB_NumFATs               16    // Length = 1
#define BPB_RootEntCnt            17    // Length = 2
#define BPB_TotSec16              19    // Length = 2
#define BPB_Media                 21    // Length = 1
#define BPB_FATSz16               22    // Length = 2
#define BPB_SecPerTrk             24    // Length = 2
#define BPB_NumHeads              26    // Length = 2
#define BPB_HiddSec               28    // Length = 4
#define BPB_TotSec32              32    // Length = 4

///! FAT 12/16
#define BS_FAT_DrvNum             36    // Length = 1
#define BS_FAT_BootSig            38    // Length = 1
#define BS_FAT_VolID              39    // Length = 4
#define BS_FAT_VolLab             43    // Length = 11
#define BS_FAT_FilSysType         54    // Length = 8

///! FAT 32
#define BPB_FAT32_FATSz32         36    // Length = 4
#define BPB_FAT32_ExtFlags        40    // Length = 2
#define BPB_FAT32_FSVer           42    // Length = 2
#define BPB_FAT32_RootClus        44    // Length = 4
#define BPB_FAT32_FSInfo          48    // Length = 2
#define BPB_FAT32_BkBootSec       50    // Length = 2
#define BS_FAT32_DrvNum           64    // Length = 1
#define BS_FAT32_BootSig          66    // Length = 1
#define BS_FAT32_VolID            67    // Length = 4
#define BS_FAT32_VolLab           71    // Length = 11
#define BS_FAT32_FilSysType       82    // Length = 8

///! FAT Types
#define FAT_TYPE_FAT12             1
#define FAT_TYPE_FAT16             2
#define FAT_TYPE_FAT32             3


///! VFAT Max filename Length 
#define MAX_LONG_FILENAME               256
#define MAX_LONG_PATH                   260



///! FAT32 Specific Statics
#define PARTITION1_TYPECODE_LOCATION        450
#define FAT32_TYPECODE1                    0x0B
#define FAT32_TYPECODE2                    0x0C
#define PARTITION1_LBA_BEGIN_LOCATION       454

///! FAT32 File Attributes and Types
#define FILE_ATTR_READ_ONLY                0x01
#define FILE_ATTR_HIDDEN                   0x02
#define FILE_ATTR_SYSTEM                   0x04
#define FILE_ATTR_SYSHID                   0x06
#define FILE_ATTR_VOLUME_ID                0x08
#define FILE_ATTR_DIRECTORY                0x10
#define FILE_ATTR_ARCHIVE                  0x20
#define FILE_ATTR_LFN_TEXT                 0x0F
#define FILE_ATTR_LFN_MASK                 0x3F

#define FILE_HEADER_BLANK                  0x00
#define FILE_HEADER_DELETED                0xE5
#define FILE_TYPE_DIR                      0x10
#define FILE_TYPE_FILE                     0x20


///! FAT32 Other Defines
#define FAT32_SIGNATURE              0xAA55
#define FAT32_SIGNATURE_OFFSET       0x1FE            
#define FAT32_EOC_FLAG               0x0FFFFFF8
#define FAT32_C_MASK                 0x0FFFFFFF
#define FAT32_EOC(cluster)           ((cluster&FAT32_C_MASK) >= FAT32_EOC_FLAG)
#define FAT32_INVALID_SECTOR         0xFFFFFFFF

#ifdef TARGET_WINDOWS 
    #pragma pack(1)
#endif


typedef struct
{
    BYTE Name[11];
    BYTE Attr;
    BYTE NTRes;
    BYTE CrtTimeTenth;
    BYTE CrtTime[2];
    BYTE CrtDate[2];
    BYTE LstAccDate[2];
    BYTE FstClusterHI[2]; /* Little endian Number */
    BYTE WrtTime[2];
    BYTE WrtDate[2];
    BYTE FstClusterLO[2]; /* Little endian Number */
    BYTE Size[4];         /* Little endian Number */
} FAT32_ShortEntry;


typedef struct
{
    BYTE Ord;
    BYTE Name1[10];
    BYTE Attr;
    BYTE Type;
    BYTE ChkSum;
    BYTE Name2[12];
    BYTE FstClusterLO[2]; /* Little endian Number */
    BYTE Name3[4];         /* Little endian Number */
} FAT32_LongEntry;



#endif // __FAT32_DEFINITIONS_H__

