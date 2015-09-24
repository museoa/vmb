/*!
 *
 * \file        FAT32_FileLib.c
 * \author      Rob Riglar <rob@robriglar.com>
 * \author      Bjoern Rennhak <bjoern@rennhak.de>
 * \version     $Id: FAT32_FileLib.c,v 1.1 2015-09-24 13:00:43 ruckert Exp $ // 2.0
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
#include "FAT32_Opts.h"
#include "FAT32_Definitions.h"
#include "FAT32_Table.h"
#include "FAT32_Access.h"
#include "FAT32_Name.h"
#include "FAT32_Cache.h"
#include "FAT32_Disk.h"
#include "FAT32_FileLib.h"
#include <string.h>

///! Global structures
typedef struct
{
    UINT32  parentcluster;
    int     parentitem;
    UINT32  startcluster;
    UINT32  bytenum;
    UINT32  currentsector;
    UINT32  currentlba;      // lba for currentsector
    UINT32  filelength;
    char    path[ MAX_LONG_PATH ];
    char    filename[ MAX_LONG_FILENAME ];
    BYTE    shortfilename[ 11 ];
    bool    inUse;
    bool    Read;
    bool    Write;
    bool    Append;
    bool    Binary;
    bool    Erase;
} FL_FILE;

///! Local variables

int            Filelib_Init;
FL_FILE        Files[MAX_OPEN_FILES];
#define MAX_FILE_CACHE 4
FAT32_Cache FileCache[MAX_FILE_CACHE];  // share buffers for all handles%MAX_FILE_CACHE
#define CachePtr(handle) (&(FileCache[handle%MAX_FILE_CACHE]))

///! Local Functions
static bool open_read_file(BYTE handle, char *path);
static bool create_file(BYTE handle,char *filename);
static UINT32 sector_to_lba( FL_FILE* file, UINT32 sector, int extend);
static int read_block(BYTE * buffer, unsigned int count, int handle);
static bool write_block(int handle, const BYTE *data, UINT32 count);
static int update_file_cache(int handle, int extend);

///! External API

/*!
 * \fn      void fat32_init( void )
 * \brief   Initialise File Library, called once before using any of the functions below
 */
void fat32_initialize(void)
{
    int i;

    LOG( "LOG4C_PRIORITY_DEBUG", "Initializing File Library" );

    Filelib_Init=false;

    if (!FAT_InitDrive()) return;
    if (!FAT32_Init()) return;

    for (i=0;i<MAX_OPEN_FILES;i++)
        Files[i].inUse = false;
    for (i=0;i<MAX_FILE_CACHE;i++)
      FAT32_InitCache(&(FileCache[i]));

    Filelib_Init = true;
}


///! Macro for checking if file lib is initialised
#define CHECK_FL_INIT()        { if (!Filelib_Init) fat32_initialize(); }


/*!
 * \fn      void fat32_shutdown( void )
 * \brief   Call before shutting down system
 */
void fat32_shutdown( void )
{ int i;
  if (!Filelib_Init) return;
  for (i=0;i<MAX_OPEN_FILES;i++)
    if (Files[i].inUse) fat32_fclose(i);
  for (i=0;i<MAX_FILE_CACHE;i++)
    FAT32_WriteCache(&(FileCache[i]));
  FAT32_dir_shutdown();
  FAT32_FAT_shutdown();
}

/*!
 * \fn      FL_FILE* fat32_fopen(BYTE handle, char *path, int mode)
 * \brief   Open or Create a file for reading or writing
 * \param   path  FIXME
 * \param   mode  FIXME
 * \return  Returns FIXME
 */


int fat32_fopen(char *fullpath, int mode, int handle)
{
    bool read = false;
    bool write = false;
    bool append = false;
    bool binary = false;
    bool create = false;
    bool erase = false;

    LOG( "LOG4C_PRIORITY_DEBUG", "Inside fat32_fopen");
    CHECK_FL_INIT();


    if ((handle < 0) || (handle >= MAX_OPEN_FILES) ||
       (fullpath==NULL) || (mode>4) || (Files[handle].inUse)) 
    { LOG( "LOG4C_PRIORITY_ERROR", "fat32_fopen - Path or mode was null handle out of range" );
      return -1;
    }

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
    {   case 0:
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
        open_read_file(handle, fullpath);
    }

    // Create New
#ifdef INCLUDE_WRITE_SUPPORT
    if( (!Files[handle].inUse) && (create) )
    {
        LOG( "LOG4C_PRIORITY_DEBUG", "Creating file" );
        create_file(handle, fullpath);
    }
#else
    create = false;
    write = false;
    append = false;
#endif

    // Write Existing
    if ( !create && !read && (write || append) )
       open_read_file(handle, fullpath);

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
// fat32_fclose: Close an open file
//-----------------------------------------------------------------------------
int fat32_fclose(int handle)
{   FL_FILE* file; 
 
    LOG( "LOG4C_PRIORITY_DEBUG", "Closing file");
    CHECK_FL_INIT();

    if((handle<0) || (handle >= MAX_OPEN_FILES))
    { LOG( "LOG4C_PRIORITY_ERROR", "fat32_fclose - handle out of range" );
      return -1;
    }

    file = &(Files[handle]);
 
    file->bytenum = 0;
    file->filelength = 0;
    file->startcluster = 0;
    file->currentsector = FAT32_INVALID_SECTOR;
    file->inUse = false;
    FAT32_WriteCache(CachePtr(handle));
    return 0;
}

//-----------------------------------------------------------------------------
// fat32_fread: Read a block of data from the file
//-----------------------------------------------------------------------------
int fat32_fread(void *buffer, unsigned int size, int handle )
{  
    LOG( "LOG4C_PRIORITY_DEBUG", "Read file");
    CHECK_FL_INIT();

    if((handle<0) || (handle >= MAX_OPEN_FILES) || 
       (!Files[handle].inUse) ||
       (!Files[handle].Read) ||
       (buffer==NULL) )
    { LOG( "LOG4C_PRIORITY_ERROR", "fat32_fread - invalid parameters" );
      return -1;
    }

    // Check if read past end of file
    if (Files[handle].bytenum>=Files[handle].filelength)
        return 0;

    return read_block(buffer,size,handle);
}

//-----------------------------------------------------------------------------
// fat32_fgetc: Get a character in the stream
//-----------------------------------------------------------------------------
int fat32_fgetc(int handle)
{
    int offset;

    LOG( "LOG4C_PRIORITY_DEBUG", "Read char");
    CHECK_FL_INIT();
    if((handle<0) || (handle >= MAX_OPEN_FILES) || 
       (!Files[handle].inUse) ||
       (!Files[handle].Read) )
    { LOG( "LOG4C_PRIORITY_ERROR", "fat32_fgetc - invalid parameters" );
      return -1;
    }

    // Check if read past end of file
    if (Files[handle].bytenum>=Files[handle].filelength)
        return -1;

    offset = update_file_cache(handle,false);

    if (offset<0) return -1;

    return CachePtr(handle)->buffer[offset];
}

char* fat32_fgets(char *s, unsigned int size, int handle)
{
  BYTE *p;
  int part;
  int offset;
  int i, byteRead;;

    LOG( "LOG4C_PRIORITY_DEBUG", "Read string");
    CHECK_FL_INIT();
    if((handle<0) || (handle >= MAX_OPEN_FILES) || 
       (!Files[handle].inUse) ||
       (!Files[handle].Read) )
    { LOG( "LOG4C_PRIORITY_ERROR", "fat32_fgets - invalid parameters" );
      return NULL;
    }

    // Check if read past end of file
    if (Files[handle].bytenum>=Files[handle].filelength)
        return NULL;

    // Limit to file size
    if ( (Files[handle].bytenum + size) > Files[handle].filelength )
      size = Files[handle].filelength - Files[handle].bytenum;

    byteRead = 0;

    while (size>0)
      { offset = update_file_cache(handle,false);
      if (offset<0) return NULL;
      p = CachePtr(handle)->buffer+offset;
      if (offset+size>512) part = 512-offset;
      else part = size;
      for (i=0; i<part;i++,p++)
      {  s[byteRead++] = *p;
         if (*p == '\n')
	 { s[byteRead]='\0';
           Files[handle].bytenum+=i+1;
           return s; 
	 }
      }
      size -= part;
      Files[handle].bytenum+=part;
    }
    s[byteRead]='\0';
    return s;
}

char* fat32_fgetws(char *s, unsigned int size, int handle)
{
  BYTE *p;
  int part;
  int offset;
  int i, byteRead;;

    LOG( "LOG4C_PRIORITY_DEBUG", "Read string");
    CHECK_FL_INIT();
    if((handle<0) || (handle >= MAX_OPEN_FILES) || 
       (!Files[handle].inUse) ||
       (!Files[handle].Read) )
    { LOG( "LOG4C_PRIORITY_ERROR", "fat32_fgets - invalid parameters" );
      return NULL;
    }

    // Check if read past end of file
    if (Files[handle].bytenum>=Files[handle].filelength)
        return NULL;

    // Limit to file size
    if ( (Files[handle].bytenum + size) > Files[handle].filelength )
      size = Files[handle].filelength - Files[handle].bytenum;

    byteRead = 0;

    while (size>0)
      { offset = update_file_cache(handle,false);
      if (offset<0) return NULL;
      p = CachePtr(handle)->buffer+offset;
      if (offset+size>512) part = 512-offset;
      else part = size;
      for (i=0; i<part;i++,p++)
      {  s[byteRead++] = *p;
	if (*p == '\n')
	 { s[byteRead]='\0';
           Files[handle].bytenum+=i+1;
           return s; 
	 }
      }
      size -= part;
      Files[handle].bytenum+=part;
    }
    s[byteRead]='\0';
    return s;
}

//-----------------------------------------------------------------------------
// fat32_fwrite: Write a block of data to the stream
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
int fat32_fwrite(const void *buffer, unsigned int size, int handle)
{

    // If first call to library, initialise
    CHECK_FL_INIT();

    if((handle<0) || (handle >= MAX_OPEN_FILES) || 
       (!Files[handle].inUse) ||
       (!Files[handle].Write) ||
       (buffer==NULL) )
    { LOG( "LOG4C_PRIORITY_ERROR", "fat32_fwrite - invalid parameters" );
      return -1;
    }

    // Append writes to end of file
    if( Files[handle].Append )
    { LOG( "LOG4C_PRIORITY_DEBUG", "Appending to end of file" );
      Files[handle].bytenum = Files[handle].filelength;
    }

    // Else write to current position
    return write_block(handle, buffer, size );
}

//-----------------------------------------------------------------------------
// fat32_fputc: Write a character to the stream
//-----------------------------------------------------------------------------
int fat32_fputc(int c, int handle)
{   int offset;

    LOG( "LOG4C_PRIORITY_DEBUG", "Write char");
    CHECK_FL_INIT();
    if((handle<0) || (handle >= MAX_OPEN_FILES) || 
       (!Files[handle].inUse) ||
       (!Files[handle].Write) )
    { LOG( "LOG4C_PRIORITY_ERROR", "fat32_fputc - invalid parameters" );
      return -1;
    }

    offset = update_file_cache(handle,true);

    if (offset<0) return -1;

    CachePtr(handle)->buffer[offset]=(unsigned char)c;
    CachePtr(handle)->dirty=true;
   
    Files[handle].bytenum++;
    
    // Increase file size
    if (Files[handle].filelength<Files[handle].bytenum)
    { Files[handle].filelength=Files[handle].bytenum;
      // Update filesize in directory
      FAT32_UpdateFileLength(Files[handle].parentcluster, 
         Files[handle].parentitem, Files[handle].filelength, Files[handle].startcluster);
    }
 
    return (int)(unsigned char)c;
}

//-----------------------------------------------------------------------------
// fat32_fputs: Write a character string to the stream
//-----------------------------------------------------------------------------
int fat32_fputs(const char * str, int handle)
{ BYTE *p;
  const char *s;
  int part;
  int offset;
  int i;

  LOG( "LOG4C_PRIORITY_DEBUG", "Write string");
  CHECK_FL_INIT();
  if((handle<0) || (handle >= MAX_OPEN_FILES) || 
    (!Files[handle].inUse) ||
    (!Files[handle].Write) ||
    (str==NULL) )
  { LOG( "LOG4C_PRIORITY_ERROR", "fat32_fputs - invalid parameters" );
    return -1;
  }

  // Append writes to end of file
  if (Files[handle].Append)
    Files[handle].bytenum = Files[handle].filelength;
  // Else write to current position

  s = str;

  while (*s != 0)
  { offset = update_file_cache(handle,true);
    if (offset<0) return -1;
    p = CachePtr(handle)->buffer+offset;
    part = 512-offset;
    for (i=0; i<part && (*s!=0);i++)
      *p++ = *s++;
    CachePtr(handle)->dirty=true;
    Files[handle].bytenum+=i;
  }

  // Increase file size
  if (Files[handle].filelength<Files[handle].bytenum)
  { Files[handle].filelength=Files[handle].bytenum;
    // Update filesize in directory
    FAT32_UpdateFileLength(Files[handle].parentcluster, 
         Files[handle].parentitem, Files[handle].filelength, Files[handle].startcluster);
  }
  return s-str;
}


//-----------------------------------------------------------------------------
// fat32_fputws: Write a wide (16bit) character string to the stream
//-----------------------------------------------------------------------------
int fat32_fputws(const char * str, int handle)
{ BYTE *p;
  int part;
  int offset;
  int i,k;

  LOG( "LOG4C_PRIORITY_DEBUG", "Write string");
  CHECK_FL_INIT();
  if((handle<0) || (handle >= MAX_OPEN_FILES) || 
    (!Files[handle].inUse) ||
    (!Files[handle].Write) ||
    (str==NULL) )
  { LOG( "LOG4C_PRIORITY_ERROR", "fat32_fputs - invalid parameters" );
    return -1;
  }

  // Append writes to end of file
  if (Files[handle].Append)
    Files[handle].bytenum = Files[handle].filelength;
  // Else write to current position

  k=0;

  while (!(((k&1)==0) && str[k] == 0 && str[k+1]==0))
  { offset = update_file_cache(handle,true);
    if (offset<0) return -1;
    p = CachePtr(handle)->buffer+offset;
    part = 512-offset;
    for (i=0; i<part;i++)
    { if (((k&1)==0) && str[k] == 0 && str[k+1]==0) break;
      *p++ = str[k++];
    }
    CachePtr(handle)->dirty=true;
    Files[handle].bytenum+=i;
  }

  // Increase file size
  if (Files[handle].filelength<Files[handle].bytenum)
  { Files[handle].filelength=Files[handle].bytenum;
    // Update filesize in directory
    FAT32_UpdateFileLength(Files[handle].parentcluster, 
         Files[handle].parentitem, Files[handle].filelength, Files[handle].startcluster);
  }
  return k;
}

#endif



//-----------------------------------------------------------------------------
// fat32_fseek: Seek to a specific place in the file
// TODO: This should support -ve numbers with SEEK END and SEEK CUR
//-----------------------------------------------------------------------------
int fat32_fseek(long offset , int origin, int handle)
{
  LOG( "LOG4C_PRIORITY_DEBUG", "File seek");
  CHECK_FL_INIT();

  if((handle<0) || (handle >= MAX_OPEN_FILES) || 
     (!Files[handle].inUse) )
  { LOG( "LOG4C_PRIORITY_ERROR", "fat32_fseek - invalid parameters" );
    return -1;
  }

  switch(origin)
  { case SEEK_CUR:
      offset = Files[handle].bytenum + offset;
      break;
    case SEEK_SET:
      break; 
    case SEEK_END:
      offset = Files[handle].filelength + offset;
      break;
    default:
      return -1;
  }
  if(offset<0 || offset>Files[handle].filelength)
     return -1;
  Files[handle].bytenum = offset;
  return 0;
}

//-----------------------------------------------------------------------------
// fat32_fgetpos: Get the current file position
//-----------------------------------------------------------------------------
long fat32_ftell(int handle)
{ 
  LOG( "LOG4C_PRIORITY_DEBUG", "File tell");
  CHECK_FL_INIT();

  if((handle<0) || (handle >= MAX_OPEN_FILES) || 
     (!Files[handle].inUse) )
  { LOG( "LOG4C_PRIORITY_ERROR", "fat32_ftell - invalid parameters" );
    return -1;
  }
  return  Files[handle].bytenum;
}


//-----------------------------------------------------------------------------
// fat32_remove: Remove a file from the filesystem
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT

int fat32_remove( const char *fullpath )
{ 
 
    FAT32_ShortEntry *sfEntry;
    UINT32 startcluster, parentcluster;
    BYTE shortname[11];
    char path[ MAX_LONG_FILENAME];
    char name[ MAX_LONG_FILENAME];
    int item;

    // If first call to library, initialise
    LOG( "LOG4C_PRIORITY_DEBUG", "File remove");
    CHECK_FL_INIT();

    if (!FAT32_GetDirectory(fullpath,path,name,&parentcluster))
      return -1;

    sfEntry=FAT32_GetFileEntry(parentcluster, name, &item);
    if (sfEntry==NULL || FATName_is_dir_entry(sfEntry))
      return -1;
    
    memcpy(shortname, sfEntry->Name, 11);
    startcluster = FAT32_GetFileStartcluster(sfEntry);

    // Delete allocated space
    if (!FAT32_FreeClusterChain(startcluster))
      return -1;

    // Remove directory entries
    if (!FAT32_MarkFileDeleted(parentcluster, item))
      return -1;

    return 0;
}

#endif  // INCLUDE_WRITE_SUPPORT

int fat32_mkdir(const char *fullpath)
{     // If first call to library, initialise
    LOG( "LOG4C_PRIORITY_DEBUG", "Make Directory");
    CHECK_FL_INIT();
    return -1;
}


// Local Functions


static UINT32 sector_to_lba( FL_FILE* file, UINT32 sector, int extend)
{ UINT32 lba;
  // we cache sector/lba pairs
  if (sector == file->currentsector) return file->currentlba;
  if (sector/FAT32.SectorsPerCluster == file->currentsector/ FAT32.SectorsPerCluster)
  {  //different sector within same cluster
    lba = file->currentlba +(sector-file->currentsector);
  }
  else
    lba = FAT32_ClusterOffset2lba(&file->startcluster, sector, extend);
  if (lba!=0) 
    { file->currentsector=sector;
      file->currentlba=lba;
    }
  return lba; 
} 


//-----------------------------------------------------------------------------
// open_read_file: Open a file for reading
//-----------------------------------------------------------------------------
static bool open_read_file(BYTE handle, char *fullpath)
{
    FL_FILE* file; 
    FAT32_ShortEntry *sfEntry;

    file = &(Files[handle]);

    if (!FAT32_GetDirectory(fullpath,file->path,file->filename,&file->parentcluster))
      return false;

    // Using dir cluster address search for filename
    sfEntry=FAT32_GetFileEntry(file->parentcluster, file->filename, &(file->parentitem));
    if (sfEntry!=NULL && !FATName_is_dir_entry(sfEntry))
    {
        // Initialise file details
        memcpy(file->shortfilename, sfEntry->Name, 11);
        file->filelength = FAT32_GetFilelength(sfEntry);
        file->bytenum = 0;
        file->startcluster =  FAT32_GetFileStartcluster(sfEntry);
        file->currentsector = FAT32_INVALID_SECTOR;
        file->inUse = true;

        return true;
    }

    return false;
}


//-----------------------------------------------------------------------------
// create_file: Create a new, empty file
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
static bool create_file(BYTE handle, char *fullpath)

{
    FL_FILE* file; 
    int tail;
    FAT32_ShortEntry *entry;
    file = &(Files[handle]);
    file->startcluster = 0; /* a start cluster of 0 means empty */
    file->filelength = 0;
    file->bytenum = 0;
    file->currentsector = FAT32_INVALID_SECTOR;

    if (!FAT32_GetDirectory(fullpath,file->path,file->filename,&file->parentcluster))
      return false;
    // Check if same filename exists in directory
    entry = FAT32_GetFileEntry(file->parentcluster, file->filename, &file->parentitem);
    if (entry == NULL) /* make a new file */
    { tail = FATName_Create_sfn_with_tail(file->parentcluster, 
                     file->shortfilename, file->filename);
      if( !FAT32_AddFileEntry(file->parentcluster, 
                            (tail==0)? NULL:file->filename,
                            file->shortfilename, 
                            file->startcluster, 
                            file->filelength,
                            &(file->parentitem)) )
        return false;
    }
    else 
    {  FAT32_FreeClusterChain(FAT32_GetFileStartcluster(entry));
       FAT32_SetFileStartcluster(entry,file->startcluster);
       FAT32_SetFilelength(entry,file->filelength);
    }
    file->inUse = true;
    return true;
}
#endif


 static int update_file_cache(int handle, int extend)
// return offset into file cache or -1 if unsuccessful
 // if extend==true then extend the file if necessary
{   register FL_FILE* file; 
    UINT32 sector;
    UINT32 offset;
    UINT32 lba;
  
    file = &(Files[handle]);

    // Calculations for file position
    sector = file->bytenum / 512;
    offset = file->bytenum - (sector*512);
    lba = sector_to_lba(file, sector,extend);
    if (lba==0) 
      return -1;
    if (extend && sector>file->filelength / 512) 
    { if (!FAT32_ZeroCache(CachePtr(handle),lba))
        return -1;
    }
    else if (!FAT32_ReadCache(CachePtr(handle),lba))
      return -1;
    return offset;
}



static int read_block(BYTE * buffer, unsigned int count, int handle)
{   register FL_FILE* file; 
    UINT32 sector;
    UINT32 offset;
    UINT32 lba;
    UINT32 totalSectors;
    UINT32 bytesRead;
    UINT32 thisReadCount;
    UINT32 i;

    // Nothing to be done
    if (count==0)
        return 0;

    file = &(Files[handle]);

    // Check if read starts past end of file
    if (file->bytenum>=file->filelength)
        return -1;

    // Limit to file size
    if ( (file->bytenum + count) > file->filelength )
      count = file->filelength - file->bytenum;

    // Calculations for file position
    sector = file->bytenum / 512;
    offset = file->bytenum - (sector*512);

    // Calculate how many sectors this is
    totalSectors = (count+offset+511) / 512;

    bytesRead = 0;
    for (i=0;i<totalSectors;i++)
    {  lba = sector_to_lba(file, sector,false);
       if (lba==0) 
         return (int)bytesRead;
      if (!FAT32_ReadCache(CachePtr(handle),lba))
         return (int)bytesRead;
       // Read length - full sector or part
       if ( offset+count>512 )
         thisReadCount = 512-offset;
       else
         thisReadCount = count;

       // Copy to application buffer
       memcpy( (BYTE*)(buffer+bytesRead), CachePtr(handle)->buffer+offset, thisReadCount);
       bytesRead+=thisReadCount;
       count -=thisReadCount;
       if (count<=0)
         break;
       sector++;
       offset=0;
    }
    file->bytenum+=bytesRead;

    return bytesRead;
}

//-----------------------------------------------------------------------------
// write_block: Write a block of data to a file
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
static bool write_block(int handle, const BYTE *data, UINT32 count) 
{   register FL_FILE* file; 
    UINT32 sector;
    UINT32 offset;
    UINT32 totalSectors;  
    UINT32 oldSectors;  
    UINT32 BytesWritten;
    UINT32 thisWriteCount;
    UINT32 lba;  
    UINT32 i;

    // Nothing to be done
    if (count==0)
      return 0;

    file = &(Files[handle]);
    
    // Calculations for file position
    sector = file->bytenum / 512;
    offset = file->bytenum - (sector*512);
    // Calculate how many sectors this is
    totalSectors = (count+offset+511) / 512;
    oldSectors = (file->filelength+511)/512;

    BytesWritten = 0;
    for (i=0;i<totalSectors;i++)
    {
      lba = sector_to_lba(file, sector, true);
       if (lba==0) 
         return -1;
       // Write length - full sector or part
       if (offset+count>512)
         thisWriteCount = 512-offset;
       else
         thisWriteCount = count;

       if (thisWriteCount==512)
         FAT32_MoveCache(CachePtr(handle),lba);
       else if(sector<oldSectors)
	 FAT32_ReadCache(CachePtr(handle),lba);
       else
	 FAT32_ZeroCache(CachePtr(handle),lba);

       memcpy(CachePtr(handle)->buffer+offset, (data+BytesWritten),thisWriteCount);
       CachePtr(handle)->dirty=true;
       BytesWritten+=thisWriteCount;
       count -= thisWriteCount;
       if (count<=0) break;
       sector++;
       offset=0;
     }

    file->bytenum+=BytesWritten;

    // Increase file size
    if (file->filelength<file->bytenum)
    { file->filelength=file->bytenum;

      // Update filesize in directory
      FAT32_UpdateFileLength(file->parentcluster, file->parentitem, file->filelength, file->startcluster);
    }
    return true;
}
#endif
