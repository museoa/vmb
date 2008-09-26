#include <windows.h>
#include "winopt.h"

void set_pos_key(int xpos, int ypos,char *name)
{ HKEY p=0,vmb=0, s=0;
  LONG r;
  r = RegOpenKeyEx(HKEY_CURRENT_USER,"Software",0,KEY_ALL_ACCESS,&s);
  if (r==ERROR_SUCCESS)
  {  DWORD n;
	  r = RegCreateKeyEx(s,"VMB",0,NULL,REG_OPTION_NON_VOLATILE,
               KEY_ALL_ACCESS,NULL,&vmb,&n);
     if (r==ERROR_SUCCESS)
	 {
	    r = RegCreateKeyEx(vmb,name,0,NULL,REG_OPTION_NON_VOLATILE,
               KEY_ALL_ACCESS,NULL,&p,&n);
        if (r==ERROR_SUCCESS)
	    { DWORD n;
		  n = xpos;
          r = RegSetValueEx(p,"xpos",0,REG_DWORD,(BYTE*)&n,sizeof(DWORD));
		  n = ypos;
          r = RegSetValueEx(p,"ypos",0,REG_DWORD,(BYTE*)&n,sizeof(DWORD));
          RegCloseKey(p);
		}
       RegCloseKey(vmb);
	 }
    RegCloseKey(s);
  }
}

void get_pos_key(int *xpos, int *ypos, char *name)
{ HKEY p=0,vmb=0, s=0;
  DWORD bc = sizeof(DWORD);
  LONG r;
  int w,h;

  w = GetSystemMetrics(SM_CXSCREEN)-50;
  h = GetSystemMetrics(SM_CYSCREEN)-50;
  if (*xpos > 0 && *xpos <w && *ypos > 0 && *ypos < h) 
	return;  /* values are reasonable */
  *xpos = *ypos = 0;
  /* get values from the registry */
  r = RegOpenKeyEx(HKEY_CURRENT_USER,"Software",0,KEY_ALL_ACCESS,&s);
  if (r==ERROR_SUCCESS)
  {  DWORD n;
	  r = RegCreateKeyEx(s,"VMB",0,NULL,REG_OPTION_NON_VOLATILE,
               KEY_ALL_ACCESS,NULL,&vmb,&n);
     if (r==ERROR_SUCCESS)
	 {
	    r = RegCreateKeyEx(vmb,name,0,NULL,REG_OPTION_NON_VOLATILE,
               KEY_ALL_ACCESS,NULL,&p,&n);
        if (r==ERROR_SUCCESS)
	    { DWORD n;
          r = RegQueryValueEx(p,"xpos",0,NULL,(LPBYTE)&n,&bc);
		  *xpos = n;
          r = RegQueryValueEx(p,"ypos",0,NULL,(LPBYTE)&n,&bc);
		  *ypos = n;
          RegCloseKey(p);
		}
       RegCloseKey(vmb);
	 }
    RegCloseKey(s);
  }
  if (*xpos < 0 || *xpos >w) *xpos = 0;
  if (*ypos < 0 || *ypos >h) *ypos = 0;
}

