#include <windows.h>
#include "vmb.h"
#include "winopt.h"

void set_pos_key(HWND hWnd,char *name)
{ int xpos,ypos;
  HKEY p=0,vmbkey=0, s=0;
  LONG r;
  WINDOWPLACEMENT wndpl;
  wndpl.length=sizeof(WINDOWPLACEMENT);
  if(!GetWindowPlacement(hWnd,&wndpl))
  { DWORD e = GetLastError();
      vmb_errori(__LINE__,"could not get window placement",e);
	  return;
  }
  xpos = wndpl.rcNormalPosition.left;   // horizontal position 
  ypos = wndpl.rcNormalPosition.top;   // vertical position 
  r = RegOpenKeyEx(HKEY_CURRENT_USER,"Software",0,KEY_ALL_ACCESS,&s);
  if (r==ERROR_SUCCESS)
  {  DWORD n;
	  r = RegCreateKeyEx(s,"VMB",0,NULL,REG_OPTION_NON_VOLATILE,
               KEY_ALL_ACCESS,NULL,&vmbkey,&n);
     if (r==ERROR_SUCCESS)
	 {
	    r = RegCreateKeyEx(vmbkey,name,0,NULL,REG_OPTION_NON_VOLATILE,
               KEY_ALL_ACCESS,NULL,&p,&n);
        if (r==ERROR_SUCCESS)
	    { DWORD n;
		  n = xpos;
          r = RegSetValueEx(p,"xpos",0,REG_DWORD,(BYTE*)&n,sizeof(DWORD));
		  n = ypos;
          r = RegSetValueEx(p,"ypos",0,REG_DWORD,(BYTE*)&n,sizeof(DWORD));
          RegCloseKey(p);
		}
       RegCloseKey(vmbkey);
	 }
    RegCloseKey(s);
  }
}

void get_pos_key(int *xpos, int *ypos, char *name)
{ HKEY p=0,vmbkey=0, s=0;
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
               KEY_ALL_ACCESS,NULL,&vmbkey,&n);
     if (r==ERROR_SUCCESS)
	 {
	    r = RegCreateKeyEx(vmbkey,name,0,NULL,REG_OPTION_NON_VOLATILE,
               KEY_ALL_ACCESS,NULL,&p,&n);
        if (r==ERROR_SUCCESS)
	    { DWORD n;
          r = RegQueryValueEx(p,"xpos",0,NULL,(LPBYTE)&n,&bc);
		  *xpos = n;
          r = RegQueryValueEx(p,"ypos",0,NULL,(LPBYTE)&n,&bc);
		  *ypos = n;
          RegCloseKey(p);
		}
       RegCloseKey(vmbkey);
	 }
    RegCloseKey(s);
  }
  if (*xpos < 0 || *xpos >w) *xpos = 0;
  if (*ypos < 0 || *ypos >h) *ypos = 0;
}

