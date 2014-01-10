#include <windows.h>
#include "option.h"
#include "winopt.h"
#include "winmain.h"
#include "edit.h"
#include "flags.h"

/* This file provides the functions set_flags and get_flags that store
   user preferences in the registry and retieve them again.
   Note: user preferences go in the registry, project settings should go 
         (in the future) in the default.vmb file.
 */

DWORD DFlags=0;

#define SET_FLAG(x,FLAG) DFlags = ((DFlags & ~(FLAG)) | ((x)?(FLAG):0))
#define GET_FLAG(x,FLAG) x = ((DFlags&(FLAG))!=0)

void set_flags(void)
{ 
	SET_FLAG(autosave,FLAG_AUTOSAVE);
	SET_FLAG(show_line_no,FLAG_SHOW_LINESNO);
	SET_FLAG(show_profile,FLAG_SHOW_PROFILE);
	set_reg_DWORD(defined,"flags",DFlags);
}

void get_flags(void)
{
	DFlags=get_reg_DWORD(defined,"flags");
	GET_FLAG(autosave,FLAG_AUTOSAVE);
	GET_FLAG(show_line_no,FLAG_SHOW_LINESNO);
	set_lineno_width();
	GET_FLAG(show_profile,FLAG_SHOW_PROFILE);
    set_profile_width();
}