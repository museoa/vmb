#include <windows.h>
#include "winopt.h"

void set_pos_key(DWORD Xpos, DWORD Ypos,char *name)
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
	    {
          r = RegSetValueEx(p,"xpos",0,REG_DWORD,(BYTE*)&Xpos,sizeof(DWORD));
          r = RegSetValueEx(p,"ypos",0,REG_DWORD,(BYTE*)&Ypos,sizeof(DWORD));
          RegCloseKey(p);
		}
       RegCloseKey(vmb);
	 }
    RegCloseKey(s);
  }
}

void get_pos_key(DWORD *Xpos, DWORD *Ypos, char *name)
{ HKEY p=0,vmb=0, s=0;
  DWORD bc = sizeof(DWORD);
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
	    {
          r = RegQueryValueEx(p,"xpos",0,NULL,(LPBYTE)Xpos,&bc);
          r = RegQueryValueEx(p,"ypos",0,NULL,(LPBYTE)Ypos,&bc);
          RegCloseKey(p);
		}
       RegCloseKey(vmb);
	 }
    RegCloseKey(s);
  }
}

