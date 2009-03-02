/*!
 *
 * \file        FAT32_Access.c
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_Access.c,v 1.2 2009-03-02 12:27:59 ruckert Exp $ // 2.0
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
#include "FAT32_Base.h"
#include "FAT32_Table.h"
#include "FAT32_Access.h"
#include "FAT32_Write.h"
#include "FAT32_FileString.h"
#include "FAT32_Misc.h"


/*!
 * \fn      bool FAT32_InitFAT(void)
 * \brief   Load FAT32 Parameters
 * \return  Boolean if the initialization went ok or not
 */
bool FAT32_InitFAT( void )
{
    FATFS_Internal.SectorCurrentlyLoaded  = 0xFFFFFFFF;
    FATFS_Internal.NextFreeCluster        = 0xFFFFFFFF;
    return FAT32_Init();
}

/*!
 * \fn      bool FAT32_SectorReader(UINT32 Startcluster, UINT32 offset)
 * \brief   From the provided startcluster and sector offset
 * \return  True if success, returns False if not (including if read out of range)
 */
bool FAT32_SectorReader( UINT32 Startcluster, UINT32 offset )
{
    UINT32 SectortoRead, ClustertoRead, ClusterChain   = 0;
    UINT32 i;
    UINT32 lba;

    LOGVARS( "LOG4C_PRIORITY_VVERBOSE", "Inside of FAT32_SectorReader(UINT32 Startcluster, UINT32 offset).",
             "uu", Startcluster, offset );

    ClusterChain  = Startcluster;                                         ///< Set start of cluster chain to initial value

    ClustertoRead = offset / FAT32.SectorsPerCluster;                     ///< Find parameters
    SectortoRead  = offset - (ClustertoRead*FAT32.SectorsPerCluster);
    for (i=0; i<ClustertoRead; i++)                                       ///< Follow chain to find cluster to read
        ClusterChain = FAT32_FindNextCluster( ClusterChain );


    LOGVARS( "LOG4C_PRIORITY_VVERBOSE", "ClusterChain = FAT32_FindNextCluster(ClusterChain);", "x", ClusterChain );
    //LOGVARS( "LOG4C_PRIORITY_VVERBOSE", "Clusterchain should be (to end):", "x", 0xFFFFFF8 );

    // If end of cluster chain then return false
    //   -- this was the original value, produces a infinite loop when doing a dirlisting
    //   -- If the value is e.g. 0xFFFFFF8 the ClusterChain ends prematurely.
    if( ClusterChain == 0xFFFFFFFF )
        {
            LOG( "LOG4C_PRIORITY_VVERBOSE", "We reached the end of the cluster chain, nothing to read anymore !" );
            return false;
        }

    lba = FAT32_LBAofCluster(ClusterChain)+SectortoRead;                   ///< Calculate sector address

    if (lba!=FATFS_Internal.SectorCurrentlyLoaded)                         ///< Else read sector if not already loaded
    {
        FATFS_Internal.SectorCurrentlyLoaded = lba;
        return FAT_ReadSector( FATFS_Internal.SectorCurrentlyLoaded, FATFS_Internal.currentsector );
    }
    else
        return true;

}


#ifdef INCLUDE_WRITE_SUPPORT

/*!
 * \fn      bool FAT32_SectorWriter(UINT32 Startcluster, UINT32 offset)
 * \brief   Write to the provided startcluster and sector offset 
 * \return  True if success, returns False if not
 */
bool FAT32_SectorWriter(UINT32 Startcluster, UINT32 offset)
{
    UINT32 SectortoWrite, ClustertoWrite, ClusterChain  = 0;
    UINT32 LastClusterChain                             = 0xFFFFFFFF;
    UINT32 i;

    ClusterChain    = Startcluster;                                   ///< Set start of cluster chain to initial value

    ClustertoWrite  = offset / FAT32.SectorsPerCluster;               ///< Find parameters
    SectortoWrite   = offset - ( ClustertoWrite * FAT32.SectorsPerCluster );

    for( i=0; i<ClustertoWrite; i++ )                                 ///< Follow chain to find cluster to read
    {
        LastClusterChain  = ClusterChain;                             ///< Find next link in the chain
        ClusterChain      = FAT32_FindNextCluster( ClusterChain );

        if( ClusterChain==0xFFFFFFFF )                                ///< Dont keep following a dead end
            break;
    }

    if( ClusterChain==0xFFFFFFFF )                                    ///< If end of cluster chain 
    {
        if( !FAT32_AddFreeSpaceToChain( &LastClusterChain ) )         ///< Add another cluster to the last good cluster chain
            return false;
        ClusterChain = LastClusterChain;
    }

    FATFS_Internal.SectorCurrentlyLoaded = FAT32_LBAofCluster( ClusterChain ) + SectortoWrite;  ///< Calculate write address

    return FAT_WriteSector(FATFS_Internal.SectorCurrentlyLoaded, FATFS_Internal.currentsector); ///< Write to disk
}

#endif


/*!
 * \fn      void FAT32_ShowFATDetails(void)
 * \brief   Show the details about the filesystem
 */

void FAT32_ShowFATDetails( void )
{
  LOG( "LOG4C_PRIORITY_DEBUG", "Current Disc FAT details\r\n------------------------\r\nRoot Dir First Cluster = " );
  LOGVARS( "LOG4C_PRIORITY_DEBUG", "x", FAT32.RootDir_First_Cluster);
  LOG( "LOG4C_PRIORITY_DEBUG", "\r\nFAT Begin LBA = ");
  LOGVARS( "LOG4C_PRIORITY_DEBUG", "x", FAT32.fat_begin_lba);
  LOG( "LOG4C_PRIORITY_DEBUG", "\r\nCluster Begin LBA = ");
  LOGVARS( "LOG4C_PRIORITY_DEBUG", "x", FAT32.cluster_begin_lba);
  LOG( "LOG4C_PRIORITY_DEBUG", "\r\nSectors Per Cluster = ");
  LOGVARS( "LOG4C_PRIORITY_DEBUG", "d", FAT32.SectorsPerCluster);
  LOG( "LOG4C_PRIORITY_DEBUG", "\r\n\r\nFormula for conversion from Cluster num to LBA is;");
  LOG( "LOG4C_PRIORITY_DEBUG", "\r\nLBA = (cluster_begin_lba + ((Cluster_Number-2)*SectorsPerCluster)))\r\n");
}



/*!
 * \fn      UINT32 FAT32_GetRootCluster(void)
 * \brief   Get the root dir cluster
 * \return  Returns a unsigned integer (uint32) which represents the first cluster from the Root
 * directory.
 */
UINT32 FAT32_GetRootCluster( void )
{
    LOGVARS( "LOG4C_PRIORITY_DEBUG", "Returning FAT32.RootDir_First_Cluster", "x", FAT32.RootDir_First_Cluster);
    return FAT32.RootDir_First_Cluster;
}


/*!
 * \fn      UINT32 FAT32_GetFileEntry(UINT32 Cluster, char *nametofind, FAT32_ShortEntry *sfEntry)
 * \brief   Find the file entry for a filename
 * \param   Cluster     An unsigned integer (uint32) which represents a cluster on the Fat32 image
 * \param   nametofind  A char pointer which represents the name for which to search
 * \param   sfEntry     A FAT32_ShortEntry pointer of the sfEntry, will contain the
 *                      entry data if return is true.
 * \return  Returns true if the file was found, false otherwise
 */
bool FAT32_GetFileEntry(UINT32 Cluster, char *nametofind, FAT32_ShortEntry *sfEntry)
{
    LOGVARS( "LOG4C_PRIORITY_DEBUG", "FAT32_GetFileEntry(UINT32 Cluster, char *nametofind, ..)", "uc", Cluster, nametofind );
    BYTE item           = 0;
    UINT16 recordoffset = 0;
    BYTE i              = 0;
    int x               = 0;
    char LongFilename[ MAX_LONG_FILENAME ];
    char ShortFilename[ 13 ];
    FAT32_ShortEntry *directoryEntry;

    FATMisc_ClearLFN(true);

    while( true )                                                                      ///< Main cluster following loop
    {
        ///! Read sector
        if( FAT32_SectorReader(Cluster, x++) )                                         ///< If sector read was successfull
        {
            for( item=0; item<=15;item++ )                                             ///< Analyse Sector
            {
                recordoffset    = (32*item);                                           ///< Create the multiplier for sector access

                directoryEntry = (FAT32_ShortEntry*)(FATFS_Internal.currentsector+recordoffset);          ///< Overlay directory entry over buffer

                if (FATMisc_If_LFN_TextOnly(directoryEntry) )                          ///< Long File Name Text Found
                    FATMisc_CacheLFN(FATFS_Internal.currentsector+recordoffset);
                else if (FATMisc_If_LFN_Invalid(directoryEntry) )                      ///< If Invalid record found delete any long file name information collated
                    FATMisc_ClearLFN(false);
                else if (FATMisc_If_LFN_Exists(directoryEntry) )                       ///< Normal SFN Entry and Long text exists 
                {
                    FATMisc_GetLFNCache((BYTE*)LongFilename);

                    if (FileString_Compare(LongFilename, nametofind))                  ///< Compare names to see if they match
                    {
                        memcpy(sfEntry,directoryEntry,sizeof(FAT32_ShortEntry));
                        return true;
                    }

                     FATMisc_ClearLFN(false);
                }
                else if (FATMisc_If_noLFN_SFN_Only(directoryEntry) )                   ///< Normal Entry, only 8.3 Text
                {
                    memset(ShortFilename, 0, sizeof(ShortFilename));

                    for (i=0; i<8; i++)                                                ///< Copy name to string
                        ShortFilename[i] = directoryEntry->Name[i];

                    if (ShortFilename[0]!='.')                                         ///< If not . or .. entry
                        ShortFilename[8] = '.';
                    else
                        ShortFilename[8] = ' ';

                    for (i=8; i<11; i++)                                               ///< Extension
                        ShortFilename[i+1] = directoryEntry->Name[i];

                    if (FileString_Compare(ShortFilename, nametofind))                 ///< Compare names to see if they match
                    {
                        memcpy(sfEntry,directoryEntry,sizeof(FAT32_ShortEntry));
                        return true;
                    }

                    FATMisc_ClearLFN(false);
                }
            }
        }
        else
            break;

    } // End of while loop

    return false;
}

#ifdef INCLUDE_WRITE_SUPPORT


/*!
 * \fn      bool FAT32_SFNexists(UINT32 Cluster, char *shortname)
 * \brief   Check if a short filename exists
 * \note    shortname is XXXXXXXXYYY not XXXXXXXX.YYY
 * \param   Cluster     Needs an unsigned integer (uint32) which represents a cluster on the Fat32 image
 * \param   shortname   Needs a char pointer which holds the shortname 
 * \return  Returns a unsigned integer (uint32) which represents the file entry for a given cluster and name.
 */
bool FAT32_SFNexists(UINT32 Cluster, char *shortname)
{
    BYTE item           = 0;
    UINT16 recordoffset = 0;
    int x               = 0;
    FAT32_ShortEntry *directoryEntry;

    while (true)                                                                ///< Main cluster following loop
    {
        ///! Read sector
        if (FAT32_SectorReader(Cluster, x++))                                   ///< If sector read was successfull
        {
            ///! Analyse Sector
            for (item=0; item<=15;item++)
            {
                recordoffset = (32*item);                                       ///< Create the multiplier for sector access

                directoryEntry = (FAT32_ShortEntry*)(FATFS_Internal.currentsector+recordoffset);  ///< Overlay directory entry over buffer

                if (FATMisc_If_LFN_TextOnly(directoryEntry) )                                     ///< Long File Name Text Found
                    ; // FIXME
                else if (FATMisc_If_LFN_Invalid(directoryEntry) )               ///< If Invalid record found delete any long file name information collated
                    ; // FIXME
                else if (FATMisc_If_noLFN_SFN_Only(directoryEntry) )            ///< Normal Entry, only 8.3 Text
                {
                    if (strncmp((const char*)directoryEntry->Name, shortname, 11)==0)
                        return true;
                }
            }
        }
        else
            break;
    } // End of while loop

    return false;
}
#endif


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
bool FAT32_UpdateFileLength(UINT32 Cluster, char *shortname, UINT32 fileLength)
{
    BYTE item           = 0;
    UINT16 recordoffset = 0;
    int x               = 0;
    FAT32_ShortEntry *directoryEntry;

    while (true)                                                                  ///< Main cluster following loop
    {
        ///! Read sector
        if (FAT32_SectorReader(Cluster, x++))                                     ///< If sector read was successfull
        {
            ///! Analyse Sector
            for (item=0; item<=15;item++)
            {
                recordoffset = (32*item);                                         ///< Create the multiplier for sector access

                directoryEntry = (FAT32_ShortEntry*)(FATFS_Internal.currentsector+recordoffset);      ///< Overlay directory entry over buffer

                if (FATMisc_If_LFN_TextOnly(directoryEntry) )                     ///< Long File Name Text Found
                    ;     // FIXME
                else if (FATMisc_If_LFN_Invalid(directoryEntry) )                 ///< If Invalid record found delete any long file name information collated
                    ;     // FIXME

                else if(FATMisc_If_noLFN_SFN_Only(directoryEntry) )              ///< Normal Entry, only 8.3 Text
                {
                    if(strncmp((const char*)directoryEntry->Name, shortname, 11)==0)
                    {
                        SET_32BIT_WORD(directoryEntry->Size,0,fileLength);
                        // TODO: Update last write time

                        memcpy((BYTE*)(FATFS_Internal.currentsector+recordoffset), (BYTE*)directoryEntry, sizeof(FAT32_ShortEntry));    ///< Update sfn entry
                        return FAT_WriteSector(FATFS_Internal.SectorCurrentlyLoaded, FATFS_Internal.currentsector);   ///< Write sector back
                    }
                }
            }
        }
        else
            break;
    } // End of while loop

    return false;
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
bool FAT32_MarkFileDeleted(UINT32 Cluster, char *shortname)
{
    BYTE item           = 0;
    UINT16 recordoffset = 0;
    int x               = 0;
    FAT32_ShortEntry *directoryEntry;

    while (true)                                                                    ///< Main cluster following loop
    {
        ///< Read sector
        if (FAT32_SectorReader(Cluster, x++))                                       ///< If sector read was successfull
        {
            ///< Analyse Sector
            for (item=0; item<=15;item++)
            {
                recordoffset = (32*item);                                           ///< Create the multiplier for sector access

                directoryEntry = (FAT32_ShortEntry*)(FATFS_Internal.currentsector+recordoffset);          ///< Overlay directory entry over buffer

                if (FATMisc_If_LFN_TextOnly(directoryEntry) )                       ///< Long File Name Text Found
                    ; // FIXME
                else if (FATMisc_If_LFN_Invalid(directoryEntry) )                   ///< If Invalid record found delete any long file name information collated
                    ; // FIXME
                else if (FATMisc_If_noLFN_SFN_Only(directoryEntry) )                ///< Normal Entry, only 8.3 Text
                {
                    if (strncmp((const char *)directoryEntry->Name, shortname, 11)==0)
                    {
                        directoryEntry->Name[0] = 0xE5;                             ///< Mark as deleted

                        memcpy((BYTE*)(FATFS_Internal.currentsector+recordoffset), (BYTE*)directoryEntry, sizeof(FAT32_ShortEntry));  ///< Update sfn entry

                        return FAT_WriteSector(FATFS_Internal.SectorCurrentlyLoaded, FATFS_Internal.currentsector);                   ///< Write sector back
                    }
                }
            }
        }
        else
            break;
    } // End of while loop

    return false;
}
#endif


/*!
 * \fn      void ListDirectory(UINT32 StartCluster)
 * \brief   Using starting cluster number of a directory and the FAT, list all directories and files
 * \param   StartCluster  Needs and unsigned integer (uint32) which represents the Start cluster of the File System.
 */
void ListDirectory(UINT32 StartCluster)
{
    LOGVARS( "LOG4C_PRIORITY_DEBUG", "Begin of ListDirectory(UINT32 StartCluster)", "u", StartCluster );

    BYTE i,item;
    UINT16 recordoffset;
    BYTE LFNIndex=0;
    UINT32 x=0;
    FAT32_ShortEntry *directoryEntry;
    char LongFilename[MAX_LONG_FILENAME];
    char ShortFilename[13];

    FAT32.filenumber=0;
    LOG( "LOG4C_PRIORITY_DEBUG", "\r\nNo.\tFilename\r\n" );

    FATMisc_ClearLFN(true);
    
    while(true)
    {
        bool retVal = FAT32_SectorReader(StartCluster, x);
        ++x;
        LOGVARS( "LOG4C_PRIORITY_VVERBOSE", "FAT32_SectorReader(StartCluster, ++x)", "uu", StartCluster, x);
        if( retVal )                                                                                ///< If data read OK
        {
            LFNIndex = 0;

            for (item=0; item<=15; item++)                                                          ///< Maximum of 15 directory entries
            {
                recordoffset = (32*item);                                                           ///< Increase directory offset 

                directoryEntry = (FAT32_ShortEntry*)(FATFS_Internal.currentsector+recordoffset);    ///< Overlay directory entry over buffer

                if ( FATMisc_If_LFN_TextOnly(directoryEntry) )                                      ///< Long File Name Text Found
                {
                    LOG( "LOG4C_PRIORITY_VVERBOSE", "if ( FATMisc_If_LFN_TextOnly(directoryEntry) == TRUE" );
                    FATMisc_CacheLFN(FATFS_Internal.currentsector+recordoffset);
                }
                else if ( FATMisc_If_LFN_Invalid(directoryEntry) )  ///< If Invalid record found delete any long file name information collated
                    {
                        LOG( "LOG4C_PRIORITY_VVERBOSE", "if ( FATMisc_If_LFN_Invalid(directoryEntry) == TRUE" );
                        FATMisc_ClearLFN(false);
                    }
                    else if (FATMisc_If_LFN_Exists(directoryEntry) )                                ///< Normal SFN Entry and Long text exists 
                        {
                            LOG( "LOG4C_PRIORITY_VVERBOSE", "if (FATMisc_If_LFN_Exists(directoryEntry) == TRUE" );
                            FAT32.filenumber++;                                                     ///< File / Dir Count

                            FATMisc_GetLFNCache((BYTE*)LongFilename);                               ///< Get text

                            if (FATMisc_If_dir_entry(directoryEntry)) LOG( "LOG4C_PRIORITY_DEBUG", "\r\nDirectory ");
                            if (FATMisc_If_file_entry(directoryEntry)) LOG( "LOG4C_PRIORITY_DEBUG", "\r\nFile ");

                            ///! Print Filename
                            #ifdef BIOS
                                LOGVARS( "LOG4C_PRIORITY_DEBUG", "dsdx", FAT32.filenumber, LongFilename, directoryEntry->FileSize, (directoryEntry->FstClusHI<<16)|directoryEntry->FstClusLO);
                            #endif

                            #ifdef STANDALONE
                                printf("%d - %s [%d bytes] (0x%08lx)\n",FAT32.filenumber, LongFilename, directoryEntry->FileSize, (directoryEntry->FstClusHI<<16)|directoryEntry->FstClusLO);
                            #endif

                             FATMisc_ClearLFN(false);
                        }
                        else if( FATMisc_If_noLFN_SFN_Only(directoryEntry) )                        ///< Normal Entry, only 8.3 Text
                        {
                            LOG( "LOG4C_PRIORITY_VVERBOSE", "if( FATMisc_If_noLFN_SFN_Only(directoryEntry) == TRUE" );
                            FATMisc_ClearLFN(false);
                            FAT32.filenumber++;                                                     ///< File / Dir Count
                            
                            if (FATMisc_If_dir_entry(directoryEntry)) LOG( "LOG4C_PRIORITY_DEBUG", "\r\nDirectory ");
                            if (FATMisc_If_file_entry(directoryEntry)) LOG( "LOG4C_PRIORITY_DEBUG", "\r\nFile ");

                            memset(ShortFilename, 0, sizeof(ShortFilename));

                            for (i=0; i<8; i++)                                                     ///< Copy name to string
                                ShortFilename[i] = directoryEntry->Name[i];

                            if (ShortFilename[0]!='.')                                              ///< If not . or .. entry
                                ShortFilename[8] = '.';
                            else
                                ShortFilename[8] = ' ';

                            for (i=8; i<11; i++)                                                    ///< Extension
                                ShortFilename[i+1] = directoryEntry->Name[i];

                            LOGVARS( "LOG4C_PRIORITY_DEBUG", "ds", FAT32.filenumber, ShortFilename);  ///< Print Filename
                        }

            
            }

            // FIXME : Remove this
            //if( getLengthOfNumber( x ) > 2 )
            //   break;

        }
        else
        {
            break;
        }
    } // end of while
}



