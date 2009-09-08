//-------------------------------------------------------------
// Global Defines
//-------------------------------------------------------------
#ifndef __DEFINE_H__
#define __DEFINE_H__

#ifndef BYTE
  typedef unsigned char BYTE;
#endif

#ifndef bool
  typedef int bool;
#endif

#ifndef UINT16
  typedef unsigned short int UINT16;
#endif

#ifndef UINT32
  typedef unsigned long UINT32;
#endif

#ifndef true
  #define true (0==0)
#endif

#ifndef false
  #define false (!true)
#endif

//-------------------------------------------------------------
//					 Disk Configuration
//-------------------------------------------------------------
//

#define SOURCE_MOUNT_FILE_AS_DRIVE		1		// Mount an image file of a FAT32 drive

// NOTE: The project does not come with any IDE/SD/CF driver: 
// you must connect one to the stub functions in FAT32_Disk.c 
// or use SOURCE_WINDOWS_PHYSICAL_DRIVE or SOURCE_MOUNT_FILE_AS_DRIVE


#define DEF_LITTLE		1
#define DEF_BIG			2





/* includes to make the compilation work for the mmix bios */

/* These macros are used to convert numbers on disk
   which are stored there littleendian into host numbers */

#define TARGET_ENDIAN  DEF_LITTLE		

//-------------------------------------------------------------
//						Endian Specific
//-------------------------------------------------------------
// Little Endian
#if TARGET_ENDIAN == DEF_LITTLE	
  #define GET_32BIT_WORD(buffer, location)	( ((UINT32)(buffer)[(location)+3]<<24) + \
                                                  ((UINT32)(buffer)[(location)+2]<<16) + \
                                                  ((UINT32)(buffer)[(location)+1]<<8)  + \
                                                  (UINT32)(buffer)[(location)+0] )
  #define GET_16BIT_WORD(buffer, location)	( ((UINT16)(buffer)[(location)+1]<<8) + \
                                                  (UINT16)(buffer)[(location)+0] )

  #define SET_32BIT_WORD(buffer, location, value) ( (buffer)[(location)+0] = (BYTE)(((value))&0xFF), \
					            (buffer)[(location)+1] = (BYTE)(((value)>>8)&0xFF), \
						    (buffer)[(location)+2] = (BYTE)(((value)>>16)&0xFF), \
						    (buffer)[(location)+3] = (BYTE)(((value)>>24)&0xFF))

  #define SET_16BIT_WORD(buffer, location, value) ( (buffer)[(location)+0] = (BYTE)(((value))&0xFF), \
					            (buffer)[(location)+1] = (BYTE)(((value)>>8)&0xFF))

// Big Endian
#else
  #define GET_32BIT_WORD(buffer, location)	( ((UINT32)(buffer)[(location)+0]<<24) + \
                                                  ((UINT32)(buffer)[(location)+1]<<16) + \
                                                  ((UINT32)(buffer)[(location)+2]<<8)  + \
                                                  (UINT32)(buffer)[(location)+3] )
  #define GET_16BIT_WORD(buffer, location)	( ((UINT16)(buffer)[(location)+0]<<8) + \
                                                  (UINT16)(buffer)[(location)+1] )

  #define SET_32BIT_WORD(buffer, location, value) ( (buffer)[(location)+3] = (BYTE)(((value))&0xFF), \
					 	    (buffer)[(location)+2] = (BYTE)(((value)>>8)&0xFF), \
					     	    (buffer)[(location)+1] = (BYTE)(((value)>>16)&0xFF), \
						    (buffer)[(location)+0] = (BYTE)(((value)>>24)&0xFF) )

  #define SET_16BIT_WORD(buffer, location, value) ( (buffer)[(location)+1] = (BYTE)(((value))&0xFF), \
					            (buffer)[(location)+0] = (BYTE)(((value)>>8)&0xFF) )

#endif

/* definitions for logging (currently none) */

#define LOG(level,message)
#define LOGVARS(level,message,format,...)


/* includes nedded to interface with the target and with utilities */

#include "diskio.h"




#endif
