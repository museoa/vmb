/* File: $Id: aout.h,v 1.1 2007-08-29 09:19:37 ruckert Exp $ */

/**************************************************************************** 
 *
 * This header file contains constant & macro definitions for .aout/.core 
 * file types.
 * 
 ****************************************************************************/

// default kernel ASID identifier
#define KERNID	0

// BIOS reserved page frames number
#define BIOSPAGES	1

// core file header size in words: core file id tag (1W) + 1 full page frames
// (FRAMESIZE) for BIOS exclusive use
#define COREHDRSIZE	((BIOSPAGES * FRAMESIZE) + 1)

// .aout header entries number and position within header itself
#define AOUTENTNUM 10
#define AOUTTAG	0
#define PROGSTART	1
#define TEXTVSTART	2
#define TEXTVSIZE	3
#define TEXTFOFFS	4
#define TEXTFSIZE	5
#define DATAVSTART	6
#define DATAVSIZE	7
#define DATAFOFFS	8
#define DATAFSIZE	9


