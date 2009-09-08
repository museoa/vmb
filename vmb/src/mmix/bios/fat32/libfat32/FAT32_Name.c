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
#include "FAT32_Access.h"
#include "FAT32_Cache.h"
#include "FAT32_Name.h"

static int islower(int c) {return ('a'<= (c) && (c) <= 'z');}
static int toupper(int c) { return islower(c) ? (c) - 'a' + 'A' : (c);}
static int isspace(int c) { return  c==' ';}

/* the next two function make an iterator over a path */

const char *Name_GetNextDirectory(const char * path, char * dir, int bound)
/* copy the first directory of path into dir, terminate dir with 0
   return the pointer where the next directory starts in path
   return NULL if there was no directory in the path (only a file)
   bound is a bound on the characters that *dir may hold.
   truncate if needed. this will lead to a path that is probably not found
   in the volume but if yes, who knows ...
*/
{  char * q;
   q = dir; 
   if (bound<=0) return 0;
   while (*path!=0)
      if ( *path =='\\' || *path=='/')
	{ *q=0;
          return path+1;
	}
      else if (q-dir<bound-1) /* leave 1 char space fo the trailing 0 */
        *q++=*path++;
      else
        path++;
    *q = 0;
    return NULL;
}

static const char *skip_root(const char *path)
/* skip C:/ or C:\ or / or \ or ... */
{ if (path[0]==0) return path;
  if (path[1]==':') path=path+2;
  if (path[0]=='/') return path+1;
  if (path[0]=='\\') return path+1;
  return path;
}

const char *Name_GetFirstDirectory(const char * path, char * dir, int bound)
{  
  return Name_GetNextDirectory(skip_root(path),dir, bound);
}
 
//-----------------------------------------------------------------------------
// Name_Compare: Compare an internal short name to an external name.
// two filenames (without copying or changing originals)
// Returns 1 if match, 0 if not
//-----------------------------------------------------------------------------

int Name_CompareSN(BYTE* shortname, const char* name)
{ int i;

  for(i=0;i<8 && shortname[i]!=' ';i++,name++)
    if (shortname[i]!=toupper(*name)) return 0;

  if (*name=='.') 
    { name++;
      for(i=8;i<11 && shortname[i]!=' ';i++,name++)
        if (shortname[i]!=toupper(*name)) return 0;
    }
  else if (shortname[8]!=' ')
    return 0;

  return (*name==0);
}





void Name_Trim(char *trimmed, const char *filename)
/* external filenames can contain all kind of garbage, here we remove it and
place a neat and clean copy in trimmed. Filenames in the internal Files[] structure
should always be trimmed */
{ int i;
  while (isspace(*filename)) filename++; /* trim leading spaces */
  for (i=0;i<MAX_LONG_FILENAME-1 && *filename!=0;i++,filename++)
    trimmed[i]=*filename;
  while (i>0) /* trim trailing dots and spaces except for "." and ".."*/
  { i--;
    if (isspace(trimmed[i]))
      continue;
    if (trimmed[i]=='.' && i>1)
      continue;
    i++;
    break;
  }
  trimmed[i]=0;
  return;
}

//-----------------------------------------------------------------------------
// FATName_Create_sfn: Create a padded SFN 
//-----------------------------------------------------------------------------

static void FATName_Create_sfn(BYTE *shortname, char *filename)
/* create a basic short name (no tail) from the long name filename */
{	int i;
        char *lastdot;

	// special cases for . and ..
	if (filename[0]=='.')
	{ if (filename[1] == 0)
	  { shortname[0] ='.';
            i=1;
            goto padding;
	  }
	  else if (filename[1] == '.' && filename[2]==0)
 	  { shortname[0] = shortname[1]= '.';
            i=2;
            goto padding;
	  }
	}
	/* base name */
        while (*filename=='.') filename++; /* strip leading dots */
        i=0;
        lastdot=NULL;
        while (i<8&&*filename!=0)
	  { if (isspace(*filename))
              filename++;
	    else if (*filename=='.')
	    { lastdot = filename;
                 break;
	    }
	    else 
	      shortname[i++]=toupper(*filename++);
	  }
        if (*filename==0) goto padding;
        for (;i<8;i++) /* pad basename */
          shortname[i]=' ';
	for (;*filename!=0;filename++)
	  if (*filename=='.') lastdot=filename;
        if (lastdot!=NULL)
          while (i<11&&*lastdot!=0)
	  { if (isspace(*lastdot))
              lastdot++;
	    else 
	      shortname[i++]=toupper(*lastdot++);
	  }
 padding:
        for (;i<11;i++)
          shortname[i]=' ';
}




#define VALID_ENTRY(entry) (((entry)->Name[0]!=FILE_HEADER_BLANK) && \
                            ((entry)->Name[0]!=FILE_HEADER_DELETED))

//-----------------------------------------------------------------------------
// FATName_is_lfn_entry: If LFN text entry found returns sequence number, 0 otherwise
// for the first entry in a sequence the negative sequence number is returned.
//-----------------------------------------------------------------------------
int FATName_is_lfn_entry(FAT32_ShortEntry *entry)
{
  if ((entry->Attr&FILE_ATTR_LFN_MASK)==FILE_ATTR_LFN_TEXT && VALID_ENTRY(entry))
  { if  (((entry)->Name[0]&0x40)==0)
      return ((entry)->Name[0])&0x1F;
    else
      return -(((entry)->Name[0])&0x1F);
  }
  else
    return 0; 
}                          
//-----------------------------------------------------------------------------
// FATName_is_sfn: the entry is a simple short file entry
//-----------------------------------------------------------------------------
int FATName_is_sfn_entry(FAT32_ShortEntry *entry)
{
  return ((entry->Attr&(FILE_ATTR_VOLUME_ID))==0)
           && VALID_ENTRY(entry);
}

int FATName_is_dir_entry(FAT32_ShortEntry *entry)
{
  return ((entry->Attr&FILE_TYPE_DIR)!=0 && (entry->Attr&(FILE_ATTR_VOLUME_ID))==0 &&
           VALID_ENTRY(entry));
}

//-----------------------------------------------------------------------------
// FATName_LFN_to_entry_count:
//-----------------------------------------------------------------------------
int FATName_LFN_to_entry_count(char *filename)
/* determine whether filename needs a long filename entry and if yes, how many */
{
  return (strlen(filename)+12)/13; /* round up */
}

#define STORE_UNICODE(dest,src) ((dest)[0]=(src),(dest)[1]=0)

//-----------------------------------------------------------------------------
// FATName_LFN_to_lfn_entry:
//-----------------------------------------------------------------------------
void FATName_Create_lfn_entrys(char *filename, int entryCount, int n, BYTE checksum,  FAT32_LongEntry *entry)
{ 
  BYTE * buffer;
  int i;
  static const int nameIndexes[] = {1,3,5,7,9,0x0E,0x10,0x12,0x14,0x16,0x18,0x1C,0x1E};
         
	if (entryCount==n)
	  entry->Ord=0x40|n;
	else
	  entry->Ord=n;
	entry->Attr=FILE_ATTR_LFN_TEXT;
	entry->Type=0;
	entry->ChkSum=checksum;
	entry->FstClusterLO[0]=entry->FstClusterLO[0]=0;
 
        filename+=(n-1)*13;
        buffer = (BYTE*)entry;
	// Copy to buffer
	for (i=0;i<13 && *filename!=0 ; i++,filename++)
	  STORE_UNICODE(buffer+nameIndexes[i],*filename);
        if (i<13) 
	{ buffer[nameIndexes[i]] = buffer[nameIndexes[i]+1] = 0x00;
	  i++;    
	}
  	for (;i<13;i++)
	  buffer[nameIndexes[i]] = buffer[nameIndexes[i]+1] = 0xFF;
}

//-----------------------------------------------------------------------------
// FATName_Create_sfn_entry: Create the short filename directory entry
//-----------------------------------------------------------------------------
#ifdef INCLUDE_WRITE_SUPPORT
void FATName_Create_sfn_entry(BYTE *shortfilename, UINT32 size, UINT32 startCluster, FAT32_ShortEntry *entry)
{
	 memcpy(entry->Name,shortfilename,11);

	// Unless we have a RTC we might as well set these to 1980
	entry->CrtTimeTenth = 0x00;
	entry->CrtTime[1] = entry->CrtTime[0] = 0x00;
	entry->CrtDate[1] = 0x00;
	entry->CrtDate[0] = 0x20;
	entry->LstAccDate[1] = 0x00;
	entry->LstAccDate[0] = 0x20;
	entry->WrtTime[1] = entry->WrtTime[0] = 0x00;
	entry->WrtDate[1] = 0x00;
	entry->WrtDate[0] = 0x20;	

	entry->Attr = FILE_TYPE_FILE;
	entry->NTRes = 0x00;

	SET_16BIT_WORD(entry->FstClusterHI,0,((startCluster>>16) & 0xFFFF));
	SET_16BIT_WORD(entry->FstClusterLO,0,(startCluster & 0xFFFF));
	SET_32BIT_WORD(entry->Size,0,size);
}
#endif






//-----------------------------------------------------------------------------
// FATName_GenerateTail:
// sfn_input = Input short filename, spaced format & in upper case
// sfn_output = Output short filename with tail
//-----------------------------------------------------------------------------
static void itoa(UINT32 n, char * str)
     /* write decimal representation of n to str */

#define to_char(n) ((n)+'0')
#define MAX_CHAR_UINT32 6
{ char tmp[MAX_CHAR_UINT32];
  int i;
   i = MAX_CHAR_UINT32;
   while (n >= 10) {
      tmp[--i] = to_char (n % 10);
      n /= 10;
   }
   tmp[--i] = to_char (n);
   while (i<MAX_CHAR_UINT32)
     *str++ = tmp[i++];
}


static bool FATName_GenerateTail(BYTE *sfn, UINT32 tailNum)
{
	int tail_chars;
	char tail_str[8];

	if (tailNum > 999999)
		return false;

	// Convert to number
	memset(tail_str, 0x00, sizeof(tail_str)); 
	tail_str[0] = '~';
        itoa(tailNum, tail_str+1);
	
	// Overwrite with tail
	tail_chars = (int)strlen(tail_str);
	memcpy(sfn+(8-tail_chars), tail_str, tail_chars);

	return true;
}

//-----------------------------------------------------------------------------
// FATName_Create_sfn_with_tail: Create a padded SFN 
//-----------------------------------------------------------------------------

int FATName_Create_sfn_with_tail(UINT32 parentcluster, BYTE *shortFilename, char *filename)
/* create a short filename with tail if needed. return the tail number, zero if no tail */
{   int tailNum = 0;
    FATName_Create_sfn(shortFilename, filename);
    do 
    {   // Check if entry exists already or not
        if (FAT32_GetFileShort(parentcluster, shortFilename)==NULL)
	  return 0;

        tailNum++;
        FATName_GenerateTail(shortFilename, tailNum);
    }
    while (tailNum<999999);
    
    return tailNum;
}

BYTE FATName_ChkSum(BYTE * shortname)
{ int i;
  BYTE Sum;

  Sum = 0;
  for (i=11; i>0;i--)
    Sum = ((Sum&1) ? 0x80 : 0) + (Sum >> 1) + *shortname++;
  return Sum;
}


#define GET_UNICODE(src) ((char)*(src))
  static const int nameIndexes[] = {1,3,5,7,9,0x0E,0x10,0x12,0x14,0x16,0x18,0x1C,0x1E};


int FATName_Compare_entry(FAT32_LongEntry *entry, const char *filename, 
                               BYTE checksum, int n)
/* compare the sequence number, the cchecksum, and the leading part
   of the filename.
   return 0 if there is no match.
   return the number of characters found equal.
   in case this is the last enty, it might be less than 13 
   in this case check that the filename ends properly.
*/
{ 
  BYTE * buffer;
  int i;
  char c;
         

  if ((entry->Ord&0x1F)!=n || entry->ChkSum!=checksum)
    return 0;
  buffer = (BYTE*)entry;
  for (i=0;i<13 && *filename!=0 ; i++,filename++)
  { c = GET_UNICODE(buffer+nameIndexes[i]);
    if (c==0) break;
    if (toupper(c)!=toupper(*filename))
      return 0;
  }
  if (entry->Ord&0x40 && *filename!=0)
    return 0;
  return i;
}

