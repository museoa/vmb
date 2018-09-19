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
  width=wndpl.rcNormalPosition.right-wndpl.rcNormalPosition.left;
  height=wndpl.rcNormalPosition.bottom-wndpl.rcNormalPosition.top;
}

void get_xypos(void)
{ int w,h;
  w = screen_width-64;
  h = screen_height-64;
  if (xpos < 0) xpos = 0;
  if (xpos > w) xpos = 3*screen_width/4;
  if (ypos < 0) ypos = 0;
  if (ypos >h) ypos = 3*screen_height/4;
  if (width<=0) width=screen_width/2;
  if (width>screen_width) width=screen_width;
  if (height<=0) height=screen_height/2;
  if (height>screen_height) height=screen_height;
}
