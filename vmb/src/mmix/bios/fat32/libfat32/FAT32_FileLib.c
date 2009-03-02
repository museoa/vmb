/*!
 *
 * \file        FAT32_FileLib.c
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_FileLib.c,v 1.2 2009-03-02 12:27:59 ruckert Exp $ // 2.0
 * \brief       FAT32 Library, File Library
 * \details     {
 * }
 * \note        {
 *                Copyright (c) 2003-2007, Rob Riglar <rob@robriglar.com>, FAT32 File IO Library
 *                Copyright (c) 2007-2008, Bjoern Rennhak, Virtual Motherboard project.
 *                All rights reserved, see COPYRIGHT file for more details.
 * }
 *
 */

///! Custom Includes
#include "FAT32_Definitions.h"
#include "FAT32_Base.h"
#include "FAT32_Table.h"
#include "FAT32_Access.h"
#include "FAT32_Write.h"
#include "FAT32_Misc.h"
#include "FAT32_FileString.h"
#include "FAT32_FileLib.h"
#include <string.h>

///! Local variables

FL_FILE        Files[MAX_OPEN_FILES];
int            Filelib_Init;


///! Macro for checking if file lib is initialised
#define CHECK_FL_INIT()        { if (Filelib_Init==false) fat32_initialize(); }

///! Macro for checking if a handle is in a valid range
#if 0
#define CHECK_HANDLE(handle) if((handle < 0) || (handle > MAX_OPEN_FILES)) \
    { LOG( "LOG4C_PRIORITY_ERROR", "handle out of range" ); \
      return -1;  }
#else
/// since handles are of type BYTE and MAX_OPEN_FILES == 256 
#define CHECK_HANDLE(handle) 
#endif

///! Macro for Checking if file open
#define CHECK_OPEN(handle) if (Files[handle].inUse==false) \
    { LOG( "LOG4C_PRIORITY_ERROR", "file already open" ); \
      return -1;  }


///! Local Functions
static bool        _open_directory(char *path, UINT32 *pathCluster);
static bool        open_read_file(BYTE handle, char *path);
static bool        _write_block(BYTE handle, BYTE *data, UINT32 length);
static bool        create_file(BYTE handle,char *filename, UINT32 size);

/*!
 * \fn      static bool _open_directory(char *path, UINT32 *pathCluster)
 * \brief   Cycle through path string to find the start cluster address of the highest subdir.
 * \param   path          Needs an char pointer which holds the path
 * \param   pathCluster   Needs an unsigned int (uint32) which represents the path cluster 
 * \return  Returns true if the dir is already open 
 */
static bool _open_directory(char *path, UINT32 *pathCluster)
{
    int levels;
    int sublevel;
    char currentfolder[MAX_LONG_FILENAME];
    FAT32_ShortEntry sfEntry;
    UINT32 startcluster;

    startcluster    = FAT32_GetRootCluster();                                           ///< Set starting cluster to root cluster
    levels          = FileString_PathTotalLevels(path);                                 ///< Find number of levels

    for (sublevel=0;sublevel<(levels+1);sublevel++)                                     ///< Cycle through each level and get the start sector
    {
        FileString_GetSubString(path, sublevel, currentfolder);

        if (FAT32_GetFileEntry(startcluster, currentfolder,&sfEntry))                   ///< Find clusteraddress for folder (currentfolder) 
            startcluster = (((UINT32)GET_16BIT_WORD(sfEntry.FstClusterHI,0))<<16) 
                                   + GET_16BIT_WORD(sfEntry.FstClusterLO,0);
        else
            return false;
    }

    *pathCluster = startcluster;
    return true;
}


///! External API

/*!
 * \fn      void fat32_initialize( void )
 * \brief   Initialise File Library, called once before using any of the funczions below
 */
void fat32_initialize( void )
{
    int i;

    LOG( "LOG4C_PRIORITY_DEBUG", "Initializing File Library" );

    Filelib_Init=false;

    if (!FAT32_InitDrive()) return;
    if (!FAT32_Init()) return;

    for (i=0;i<MAX_OPEN_FILES;i++)
        Files[i].inUse = false;

    Filelib_Init = true;
}


/*!
 * \fn      void fat32_shutdown( void )
 * \brief   Call before shutting down system
 */
void fat32_shutdown( void )
{
  if (!Filelib_Init) return;
  FAT32_PurgeFATBuffer();
}

/*!
 * \fn      FL_FILE* fat32_fopen(BYTE handle, char *path, int mode)
 * \brief   Open or Create a file for reading or writing
 * \param   path  FIXME
 * \param   mode  FIXME
 * \return  Returns FIXME
 */

int fat32_fopen(BYTE handle, char *path, int mode)
{
    bool read = false;
    bool write = false;
    bool append = false;
    bool binary = false;
    bool create = false;
    bool erase = false;

    LOGVARS( "LOG4C_PRIORITY_DEBUG", "Inside fat32_fopen(char *path, char *mode)", "ss", path, mode );

    // If first call to library, and still not initialized try to initialize again
    CHECK_FL_INIT();
    CHECK_HANDLE(handle);

    if(path==NULL)
    {
        LOG( "LOG4C_PRIORITY_ERROR", "fat32_fopen - Path was null" );
        return -1;
    }

    if(mode>4 || mode<0) 
    {
        LOG( "LOG4C_PRIORITY_ERROR", "fat32_fopen - mode was out of range" );
        return -1;
    }

    if (Files[handle].inUse)
      fat32_fclose(handle);

    // Supported Modes:
    // 0  Open a file for reading in Text mode. The file must exist. 
    // 1  Create an empty file for writing in Text mode. 
    //    If a file with the same name already exists its content is erased 
    //    and the file is treated as a new empty file. 
    // 2  Open a file for reading in Binary mode. The file must exist. 
    // 3  Create an empty file for writing in binary mode. 
    //    If a file with the same name already exists its content is erased 
    //    and the file is treated as a new empty file. 
    // 4  Append to a file. Writing operations append data at the end of the file. 
    //    The file is created if it does not exist. 
    switch (mode)
    {
        case 0:
            read = true;
            break;
        case 1:
            write = true;
            erase = true;
            create = true;
            break;
        case 2:
            read = true;
            binary = true;
            break;
         case 3:
            write = true;
            erase = true;
            create = true;
            binary = true;
            break;
         case 4:
            write = true;
            append = true;
            create = true;
            break;
    }

    // Read
    if( read )
    {
        LOGVARS( "LOG4C_PRIORITY_DEBUG", "Reading file", "s", path );
        open_read_file(handle, path);
    }

    // Create New
#ifdef INCLUDE_WRITE_SUPPORT
    if( (!Files[handle].inUse) && (create) )
    {
        LOG( "LOG4C_PRIORITY_DEBUG", "Creating file" );
        Files[handle].inUse = create_file(handle, path, 0);
    }
#else
    create = false;
    write = false;
    append = false;
#endif

    // Write Existing
    if ( !create && !read && (write || append) )
       open_read_file(handle, path);

    if (Files[handle].inUse)
    {
        Files[handle].Read = read;
        Files[handle].Write = write;
        Files[handle].Append = append;
        Files[handle].Binary = binary;
        Files[handle].Erase = erase;
    }

    return 0;    
}
//-----------------------------------------------------------------------------
// open_read_file: Open a file for reading
//-----------------------------------------------------------------------------
static bool open_read_file(BYTE handle, char *path)
{
    FL_FILE* file; 
    FAT32_ShortEntry sfEntry;

    file = &(Files[handle]);
    // Check if file already open
    if (file->inUse) return false;

    // Clear filename
    memset(file->path, '\n', sizeof(file->path));
    memset(file->filename, '\n', sizeof(file->filename));

    // Split full path into filename and directory path
    FileString_SplitPath(path, file->path, file->filename);

    // If file is in the root dir
    if (file->path[0]==0)
    {
        file->parentcluster = FAT32_GetRootCluster();
        file->inRoot = true;
    }
    else
    {
        file->inRoot = false;

        // Find parent directory start cluster
        if (!_open_directory(file->path, &file->parentcluster))
            return false;
    }

    // Using dir cluster address search for filename
    if (FAT32_GetFileEntry(file->parentcluster, file->filename,&sfEntry))
    {
        // Initialise file details
        memcpy(file->shortfilename, sfEntry.Name, 11);
        file->filelength = GET_32BIT_WORD(sfEntry.Size,0);
        file->bytenum = 0;
        file->startcluster = (((UINT32)GET_16BIT_WORD(sfEntry.FstClusterHI,0))<<16)
                                     + GET_16BIT_WORD(sfEntry.FstClusterLO,0);
        file->currentBlock = 0xFFFFFFFF;
        file->inUse = true;

        FAT32_PurgeFATBuffer();

        return true;
    }

    return false;
}
//-----------------------------------------------------------------------------
// _create_file: Create a new file
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
static bool create_file(BYTE handle, char *filename, UINT32 size)
{
    FL_FILE* file; 
    FAT32_ShortEntry sfEntry;
    char shortFilename[11];
    int tailNum;

    LOG( "LOG4C_PRIORITY_DEBUG", "Inside of _create_file" );

    file = &(Files[handle]);
    // Check if file already open
    if (file->inUse) return false;

    // Clear filename
    memset(file->path, '\n', sizeof(file->path));
    memset(file->filename, '\n', sizeof(file->filename));

    // Split full path into filename and directory path
    FileString_SplitPath(filename, file->path, file->filename);

    // If file is in the root dir
    if (file->path[0]==0)
    {
        file->parentcluster = FAT32_GetRootCluster();
        file->inRoot = true;
    }
    else
    {
        file->inRoot = false;

        // Find parent directory start cluster
        if( !_open_directory(file->path, &file->parentcluster) )
        {
            LOG( "LOG4C_PRIORITY_ERROR", "Couldn't find the parent directory start cluster." );
            return false;
        }
    }

    // Check if same filename exists in directory
    if (FAT32_GetFileEntry(file->parentcluster, file->filename,&sfEntry))
        LOG( "LOG4C_PRIORITY_ERROR", "File already exists in directory" );
    else 
      {
    // Create the file space for the file
    file->startcluster = 0;
    file->filelength = size;
    if( !FAT32_AllocateFreeSpace(true, &file->startcluster, (file->filelength==0)?1:file->filelength) )
    {
        LOG( "LOG4C_PRIORITY_ERROR", "Couldn't allocate free space for the file" );
        return false;
    }

    // Generate a short filename & tail
    tailNum = 0;
    do 
    {
        // Create a standard short filename (without tail)
        FATMisc_CreateSFN(shortFilename, file->filename);

        // If second hit or more, generate a ~n tail        
        if (tailNum!=0)
            FATMisc_GenerateTail((char*)file->shortfilename, shortFilename, tailNum);
        // Try with no tail if first entry
        else
            memcpy(file->shortfilename, shortFilename, 11);

        // Check if entry exists already or not
        if (FAT32_SFNexists(file->parentcluster, (char*)file->shortfilename)==false)
            break;

        tailNum++;
    }
    while (tailNum<9999);

    if( tailNum == 9999 )
    {
        LOG( "LOG4C_PRIORITY_ERROR", "Couldn't generate a short filename & tail" );
        return false;
    }

    // Add file to disk
    if( !FAT32_AddFileEntry(file->parentcluster, (char*)file->filename, (char*)file->shortfilename, file->startcluster, file->filelength) )
    {
        LOG( "LOG4C_PRIORITY_ERROR", "Couldn't add file to disk - FAT32_AddFileEntry failed" );
        return false;
    }
    }
    // General
    file->bytenum = 0;
    file->currentBlock = 0xFFFFFFFF;
    file->inUse = true;
    
    FAT32_PurgeFATBuffer();

    return true;
}
#endif

//-----------------------------------------------------------------------------
// fat32_fclose: Close an open file
//-----------------------------------------------------------------------------
int fat32_fclose(BYTE handle)
{   FL_FILE* file; 
 
    LOGVARS( "LOG4C_PRIORITY_DEBUG", "Closing file", "s", file );
    // If first call to library, initialise
    CHECK_FL_INIT();
    CHECK_HANDLE(handle);

    file = &(Files[handle]);
 
    file->bytenum = 0;
    file->filelength = 0;
    file->startcluster = 0;
    file->currentBlock = 0xFFFFFFFF;
    file->inUse = false;
    FAT32_PurgeFATBuffer();
    return 0;
}

//-----------------------------------------------------------------------------
// fat32_fread: Read a block of data from the file
// return number of byte read, -1 on error
//-----------------------------------------------------------------------------
int fat32_fread(BYTE handle, BYTE * buffer, UINT32 count)
{
    UINT32 sector;
    UINT32 offset;
    UINT32 totalSectors;
    UINT32 bytesRead;
    UINT32 thisReadCount;
    UINT32 i;

    // If first call to library, initialise
    CHECK_FL_INIT();
    CHECK_HANDLE(handle);
    CHECK_OPEN(handle);

    if (buffer==NULL)
        return -1;

    // No read permissions
    if (Files[handle].Read==false)
        return -1;

    // Nothing to be done
    if (count==0)
        return 0;

    // Check if read starts past end of file
    if (Files[handle].bytenum>=Files[handle].filelength)
        return -1;

    // Limit to file size
    if ( (Files[handle].bytenum + count) > Files[handle].filelength )
        count = Files[handle].filelength - Files[handle].bytenum;

    // Calculations for file position
    sector = Files[handle].bytenum / 512;
    offset = Files[handle].bytenum - (sector*512);

    // Calculate how many sectors this is
    totalSectors = (count+offset) / 512;

    // Take into account partial sector read
    if ((count+offset) % 512)
        totalSectors++;

    bytesRead = 0;
    for (i=0;i<totalSectors;i++)
    {
        // Read sector of file
        if ( FAT32_SectorReader(Files[handle].startcluster, (sector+i)) )
        {
            // Read length - full sector or remainder
            if ( (bytesRead+512) > count )
                thisReadCount = count - bytesRead;
            else
                thisReadCount = 512;

            // Copy to file buffer (for continuation reads)
            memcpy(Files[handle].filebuf, FATFS_Internal.currentsector, 512);
            Files[handle].currentBlock = (sector+i);

            // Copy to application buffer
            // Non aligned start
            if ( (i==0) && (offset!=0) )
                memcpy( (BYTE*)(buffer+bytesRead), (BYTE*)(Files[handle].filebuf+offset), thisReadCount);
            else
                memcpy( (BYTE*)(buffer+bytesRead), Files[handle].filebuf, thisReadCount);
        
            bytesRead+=thisReadCount;
            Files[handle].bytenum+=thisReadCount;

            if (thisReadCount>=count)
                return bytesRead;
        }
        // Read failed - out of range (probably)
        else
        {
            return (int)bytesRead;
        }
    }

    return bytesRead;
}


//-----------------------------------------------------------------------------
// fat32_fgetc: Get a character in the stream
//-----------------------------------------------------------------------------
int fat32_fgetc(BYTE handle)
{
    UINT32 sector;
    UINT32 offset;    
    BYTE returnchar=0;

    // If first call to library, initialise
    CHECK_FL_INIT();
    CHECK_HANDLE(handle);
    CHECK_OPEN(handle);

    // No read permissions
    if (Files[handle].Read==false)
        return -1;

    // Check if read past end of file
    if (Files[handle].bytenum>=Files[handle].filelength)
        return -1;

    // Calculations for file position
    sector = Files[handle].bytenum / 512;
    offset = Files[handle].bytenum - (sector*512);

    // If file block not already loaded
    if (Files[handle].currentBlock!=sector)
    {
        // Read the appropriate sector
        if (!FAT32_SectorReader(Files[handle].startcluster, sector)) 
            return -1;

        // Copy to file's buffer
        memcpy(Files[handle].filebuf, FATFS_Internal.currentsector, 512);
        Files[handle].currentBlock=sector;
    }

    // Get the data block
    returnchar = Files[handle].filebuf[offset];

    // Increase next read position
    Files[handle].bytenum++;

    // Return character read
    return returnchar;
}


//-----------------------------------------------------------------------------
// fat32_fseek: Seek to a specific place in the file
// TODO: This should support -ve numbers with SEEK END and SEEK CUR
//-----------------------------------------------------------------------------
int fat32_fseek( BYTE handle , UINT32 offset , int origin )
{
    // If first call to library, initialise
    CHECK_FL_INIT();
    CHECK_HANDLE(handle);
    CHECK_OPEN(handle);

   if ( (origin == SEEK_END) && (offset!=0) )
        return -1;

    // Invalidate file buffer
    Files[handle].currentBlock = 0xFFFFFFFF;

    if (origin==SEEK_SET)
    {
        Files[handle].bytenum = offset;

        if (Files[handle].bytenum>Files[handle].filelength)
            Files[handle].bytenum = Files[handle].filelength;

        return 0;
    }
    else if (origin==SEEK_CUR)
    {
        Files[handle].bytenum+= offset;

        if (Files[handle].bytenum>Files[handle].filelength)
            Files[handle].bytenum = Files[handle].filelength;

        return 0;
    }
    else if (origin==SEEK_END)
    {
        Files[handle].bytenum = Files[handle].filelength;
        return 0;
    }
    else
        return -1;
}
//-----------------------------------------------------------------------------
// fat32_fgetpos: Get the current file position
//-----------------------------------------------------------------------------
int fat32_fgetpos(BYTE handle , UINT32 * position)
{
    CHECK_HANDLE(handle);
    CHECK_OPEN(handle);

    // Get position
    *position = Files[handle].bytenum;

    return 0;
}

//-----------------------------------------------------------------------------
// fat32_fputc: Write a character to the stream
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
int fat32_fputc(int c, BYTE handle)
{
    BYTE Buffer[1];

    // If first call to library, initialise
    CHECK_FL_INIT();
    CHECK_HANDLE(handle);
    CHECK_OPEN(handle);

    // Append writes to end of file
    if (Files[handle].Append)
        Files[handle].bytenum = Files[handle].filelength;
    // Else write to current position

    // Write single byte
    Buffer[0] = (BYTE)c;
    if (_write_block(handle, Buffer, 1))
        return c;
    else
        return -1;
}
#endif

//-----------------------------------------------------------------------------
// fat32_fwrite: Write a block of data to the stream
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
int fat32_fwrite(const void * data, int size, int count, BYTE handle )
{
    LOG( "LOG4C_PRIORITY_DEBUG", "Inside fat32_fwrite" );

    // If first call to library, initialise
    CHECK_FL_INIT();
    CHECK_HANDLE(handle);
    CHECK_OPEN(handle);

    // Append writes to end of file
    if( Files[handle].Append )
    {
        LOG( "LOG4C_PRIORITY_DEBUG", "Appending to end of file" );
        Files[handle].bytenum = Files[handle].filelength;
    }

    // Else write to current position

    if( _write_block(handle, (BYTE*)data, (size*count) ) )
    {
        LOG( "LOG4C_PRIORITY_DEBUG", "Writing to current position." );
        return count;
    }
    else
        return -1;
}
#endif
//-----------------------------------------------------------------------------
// fat32_fputs: Write a character string to the stream
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
int fat32_fputs(const char * str, BYTE handle)
{
    // If first call to library, initialise
    CHECK_FL_INIT();
    CHECK_HANDLE(handle);
    CHECK_OPEN(handle);

    // Append writes to end of file
    if (Files[handle].Append)
        Files[handle].bytenum = Files[handle].filelength;
    // Else write to current position

    if (_write_block(handle, (BYTE*)str, (UINT32)strlen(str)))
        return (int)strlen(str);
    else
        return -1;
}
#endif
//-----------------------------------------------------------------------------
// _write_block: Write a block of data to a file
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
static bool _write_block(BYTE handle, BYTE *data, UINT32 length) 
{
    UINT32 sector;
    UINT32 offset;    
    UINT32 i;
    bool dirtySector = false;

    // No write permissions
    if (Files[handle].Write==false)
        return false;

    for (i=0;i<length;i++)
    {
        // Calculations for file position
        sector = Files[handle].bytenum / 512;
        offset = Files[handle].bytenum - (sector*512);

        // If file block not already loaded
        if (Files[handle].currentBlock!=sector)
        {
            if (dirtySector)
            {
                // Copy from file buffer to FAT driver buffer
                memcpy(FATFS_Internal.currentsector, Files[handle].filebuf, 512);

                // Write back current sector before loading next
                if (!FAT32_SectorWriter(Files[handle].startcluster, Files[handle].currentBlock)) 
                    return false;
            }

            // Read the appropriate sector
            // NOTE: This does not have succeed; if last sector of file
            // reached, no valid data will be read in, but write will 
            // allocate some more space for new data.
            FAT32_SectorReader(Files[handle].startcluster, sector);

            // Copy to file's buffer
            memcpy(Files[handle].filebuf, FATFS_Internal.currentsector, 512);
            Files[handle].currentBlock=sector;
            dirtySector = false;
        }

        // Get the data block
        Files[handle].filebuf[offset] = data[i];
        dirtySector = true;

        // Increase next read/write position
        Files[handle].bytenum++;
    }

    // If some write data still in buffer
    if (dirtySector)
    {
        // Copy from file buffer to FAT driver buffer
        memcpy(FATFS_Internal.currentsector, Files[handle].filebuf, 512);

        // Write back current sector before loading next
        if (!FAT32_SectorWriter(Files[handle].startcluster, Files[handle].currentBlock)) 
            return false;
    }

    // Increase file size
    Files[handle].filelength+=length;

    // Update filesize in directory
    FAT32_UpdateFileLength(Files[handle].parentcluster, (char*)Files[handle].shortfilename, Files[handle].filelength);

    return true;
}
#endif

//-----------------------------------------------------------------------------
// fat32_remove: Remove a file from the filesystem
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT

int fat32_remove( const char * filename )
{
    FL_FILE dir; 
    FAT32_ShortEntry sfEntry;

    // If first call to library, initialise
    CHECK_FL_INIT();

    dir.inUse = true;
    // Clear filename
    memset(dir.path, '\n', sizeof(dir.path));
    memset(dir.filename, '\n', sizeof(dir.filename));

    // Split full path into filename and directory path
    FileString_SplitPath((char*)filename, dir.path, dir.filename);

    // If file is in the root dir
    if (dir.path[0]==0)
    {
        dir.parentcluster = FAT32_GetRootCluster();
        dir.inRoot = true;
    }
    else
    {
        dir.inRoot = false;

        // Find parent directory start cluster
        if (!_open_directory(dir.path, &dir.parentcluster))
            return -1;
    }

    // Using dir cluster address search for filename
    if (FAT32_GetFileEntry(dir.parentcluster, dir.filename,&sfEntry))
    {
        // Initialise file details
        memcpy(dir.shortfilename, sfEntry.Name, 11);
        dir.filelength = GET_32BIT_WORD(sfEntry.Size,0);
        dir.bytenum = 0;
        dir.startcluster = (((UINT32)GET_16BIT_WORD(sfEntry.FstClusterHI,0))<<16) 
                                   + GET_16BIT_WORD(sfEntry.FstClusterLO,0);
         dir.currentBlock = 0xFFFFFFFF;

        // Delete allocated space
        if (!FAT32_FreeClusterChain(dir.startcluster))
            return -1;

        // Remove directory entries
        if (!FAT32_MarkFileDeleted(dir.parentcluster, (char*)dir.shortfilename))
            return -1;

        FAT32_PurgeFATBuffer();
        return 0;
    }
    else
        return -1;
}

#endif  // INCLUDE_WRITE_SUPPORT
