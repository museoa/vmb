//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//                            FAT32 File IO Library
//                                    V2.0
//                                    Rob Riglar
//                            Copyright 2003 - 2007
//
//                         Email: rob@robriglar.com
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
#include "FAT32_FileString.h"




/* the next two function make an iterator over a path */

const char *FileString_GetNextDirectory(const char * path, char * dir, int bound)
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

const char *FileString_GetFirstDirectory(const char * path, char * dir, int bound)

{  
  return FileString_GetNextDirectory(skip_root(path),dir, bound);
}
 


//-----------------------------------------------------------------------------
// FileString_StrCmpNoCase: Compare two strings case with case sensitivity
//-----------------------------------------------------------------------------
static int FileString_StrCmpNoCase(char *s1, char *s2, int n)
{
    int diff;
    char a,b;

    do
    {
        a = tolower(*s1++);
        b = tolower(*s2++);

        diff = a - b;

        // If different
        if (diff!=0)
            return diff;

    } while (--n > 0 && *s1!=0 && *s2!=0);

    return diff;
}

//-----------------------------------------------------------------------------
// FileString_GetExtension: Get index to extension within filename
// Returns -1 if not found or index otherwise
//-----------------------------------------------------------------------------
static int FileString_GetExtension(char *str)
{
    int dotPos = -1;
    char *strSrc = str;
    
    // Find last '.' in string (if at all)
    while (*strSrc)
    {
        if (*strSrc=='.')
            dotPos = (int)(strSrc-str);

        strSrc++;
    }

    return dotPos;
}

//-----------------------------------------------------------------------------
// FileString_TrimLength: Get length of string excluding trailing spaces
// Returns -1 if not found or index otherwise
//-----------------------------------------------------------------------------
static int FileString_TrimLength(char *str, int strLen)
{
    char *strSrc = str+strLen-1;
    
    // Find last non white space
    while (strLen>0 && *strSrc==' ' )
    {
        strSrc--;
        strLen--;
    }

    return strLen;
}

//-----------------------------------------------------------------------------
// FileString_Compare: Compare an internal short name to an external name.
// two filenames (without copying or changing originals)
// Returns 1 if match, 0 if not
//-----------------------------------------------------------------------------

int FileString_CompareSN(BYTE* shortname, const char* name)
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



//-----------------------------------------------------------------------------
// FileString_Compare: Compare two filenames (without copying or changing originals)
// Returns 1 if match, 0 if not
//-----------------------------------------------------------------------------
int FileString_Compare(char* strA, char* strB)
{
    char *ext1 = NULL;
    char *ext2 = NULL;
    int ext1Pos, ext2Pos;
    int file1Len, file2Len;

    // Get both files extension
    ext1Pos = FileString_GetExtension(strA);
    ext2Pos = FileString_GetExtension(strB);

    // NOTE: Extension position can be different for matching 
    // filename if trailing space are present before it!
    // Check that if one has an extension, so does the other
    if ((ext1Pos==-1) && (ext2Pos!=-1))
        return 0;
    if ((ext2Pos==-1) && (ext1Pos!=-1))
        return 0;

    // If they both have extensions, compare them
    if (ext1Pos!=-1)
    {
        // Set pointer to start of extension
        ext1 = strA+ext1Pos+1;
        ext2 = strB+ext2Pos+1;
        
        // If they dont match
        if (FileString_StrCmpNoCase(ext1, ext2, (int)strlen(ext1))!=0) 
            return 0;
        
        // Filelength is upto extensions
        file1Len = ext1Pos;
        file2Len = ext2Pos;
    }
    // No extensions
    else
    {
        // Filelength is actual filelength
        file1Len = (int)strlen(strA);
        file2Len = (int)strlen(strB);
    }

    // Find length without trailing spaces (before ext)
    file1Len = FileString_TrimLength(strA, file1Len);
    file2Len = FileString_TrimLength(strB, file2Len);

    // Check the file lengths match
    if (file1Len!=file2Len)
        return 0;

    // Compare main part of filenames
    if (FileString_StrCmpNoCase(strA, strB, file1Len)!=0)
        return 0;
    else
        return 1;
}


#if 0
bool FileString_is_short(const char *filename)
/* a filename is short, if it has a maximum of 8 characters basename followed by
   a an optional dot and a maximum 3 characters extension */
{ int i;
  i=0;
  for (i=0;i<9;i++)
  { if (filename[i]=='.') /* extension starts */
      { filename=filename+i+1; 
        for (i=0;i<4;i++)
	{ if (filename[i]==0) return true;
	  if (filename[i]=='.') return false; /* dot in the extension */
	}
        return false; /* extension too long */
      }
    if (filename[i]==0) return true;
  }
  return false; /* base too long */  
}
#endif

void FileString_Trim(char *trimmed, const char *filename)
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


