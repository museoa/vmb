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
#include "FAT32_FileString.h"

//-----------------------------------------------------------------------------
// FileString_PathTotalLevels: Take a filename and path and count the sub levels
// of folders. E.g. C:\folder\file.zip = 1 level
// Returns: -1 = Error, 0 or more = Ok
// in case there is no drive prefix, we assume C:
// we accept unix style filenames with '/' instead of '\'
//-----------------------------------------------------------------------------
int FileString_PathTotalLevels(char *path)
{
    int levels=0;


    // Check for C:\... and skip it,
    if (path[1]==':')
      path= path+2;
    // current working directory is always / skip it
    if ( path[0]=='\\' ||  path[0]=='/')
      path++;

    // If too short
    if (path[0]==0)
        return -1;

    // Count levels in path string
    while (*path)
    {
      if (*path == '\\' || *path == '/') levels++;
      path++;
    }
    
    return levels;
}

//-----------------------------------------------------------------------------
// FileString_GetSubString: Get a substring from 'Path' which contains the folder
// (or file) at the specified level.
// E.g. C:\folder\file.zip : Level 0 = folder, Level 1 = file.zip
// Returns: -1 = Error, lenght of substring if Ok
//-----------------------------------------------------------------------------
int FileString_GetSubString(char *path, int levelreq, char *output)
{
    int i, k;
    int level;


    // Check for C:\... and skip it,
    if (path[1]==':')
      path= path+2;
    // current working directory is always / skip it
    if ( path[0]=='\\' ||  path[0]=='/')
      path++;

    // If too short
    if (path[0]==0)
        return -1;

    // Loop through the number of times as characters in 'path'
    for (i = 0,level=0; path[i]!=0; i++)
    {
        // If a '\' or '/' is found then increase level
        if (path[i]=='\\' || path[i]=='/') 
          level++;
        else
        // If correct level and the character is not a '\' then copy text to 'output'
        if (level==levelreq)
        { for (k=0; path[i]!=0 && path[i]!='\\' && path[i]!='/'; i++,k++) 
            output[k] = path[i];
          output[k] = 0;
          return k;
	}
    }
    return -1;    // Error
}

//-----------------------------------------------------------------------------
// FileString_SplitPath: Full path contains the passed in string. 
// Returned is the Path string and file Name string
// E.g. C:\folder\sub\file.zip -> Path = folder\sub  FileName = file.zip
// E.g. C:\file.zip -> Path = [blank]  FileName = file.zip
//-----------------------------------------------------------------------------
int FileString_SplitPath(char *FullPath, char *Path, char *FileName)
{
    int n;
    int levels;

    // Count the levels to the filepath
    levels = FileString_PathTotalLevels(FullPath);
    if (levels<0)
        return -1;

    // Get filename part of string
    n = FileString_GetSubString(FullPath, levels, FileName);

    // If root file
    if (levels==0)
        Path[0]='\0';
    else
    {
       // Check for C:\... and skip it,
       if (FullPath[1]==':')
         FullPath= FullPath+2;
       // current working directory is always / skip it
       if ( FullPath[0]=='\\' || FullPath[0]=='/')
         FullPath++;

        n = (int)strlen(FullPath) - n;

        memcpy(Path, FullPath, n);
        Path[n-1] = '\0';
    }
    return 0;
}

//-----------------------------------------------------------------------------
// FileString_StrCmpNoCase: Compare two strings case with case sensitivity
//-----------------------------------------------------------------------------
int FileString_StrCmpNoCase(char *s1, char *s2, int n)
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
int FileString_GetExtension(char *str)
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
int FileString_TrimLength(char *str, int strLen)
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
