#include <windows.h>
#include "winopt.h"


void set_xypos(HWND hWnd)
{ WINDOWPLACEMENT wndpl;
  wndpl.length=sizeof(WINDOWPLACEMENT);
  if(!GetWindowPlacement(hWnd,&wndpl))
  {   win32_error(__LINE__,"could not get window placement");
	  return;
  }
  xpos=wndpl.rcNormalPosition.left;
  ypos=wndpl.rcNormalPosition.top;
}

void get_xypos(void)
{ int w,h;
  w = GetSystemMetrics(SM_CXSCREEN)-50;
  h = GetSystemMetrics(SM_CYSCREEN)-50;
  if (xpos > 0 && xpos <w && ypos > 0 && ypos < h) 
	return;  /* values are reasonable */
  if (xpos < 0 || xpos >w) xpos = 0;
  if (ypos < 0 || ypos >h) ypos = 0;
}
