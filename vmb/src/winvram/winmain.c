#include <winsock2.h>
#include <windows.h>
#include <afxres.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "param.h"
#include "option.h"
#include "winopt.h"

extern int width, height, framewidth,frameheight;


static HDC hCanvas;
static CRITICAL_SECTION   bitmap_section;




INT_PTR CALLBACK   
SettingsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{
  switch ( message )
  { case WM_INITDIALOG:
      uint64tohex(vmb_address,tmp_option);
      SetDlgItemText(hDlg,IDC_ADDRESS,tmp_option);
      return TRUE;
   case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
      { GetDlgItemText(hDlg,IDC_ADDRESS,tmp_option,MAXTMPOPTION);
        vmb_address = strtouint64(tmp_option); 
      }
      if (wparam == IDOK || wparam == IDCANCEL)
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
  }
  return FALSE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ switch (message) 
  {  
case WM_USER+1: /* On */
	return 0;
	case WM_USER+2: /* Off */
		return 0;
	case WM_USER+3: /* Connected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Disconnect"))
	  DrawMenuBar(hMainWnd);
 	return 0;
  case WM_USER+4: /* Disconnected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Connect..."))
	  DrawMenuBar(hMainWnd);
	return 0;
case WM_CREATE:

    return (DefWindowProc(hWnd, message, wParam, lParam));

 case WM_PAINT:
	{ PAINTSTRUCT ps;
	  BOOL rc;
	  DWORD dw;
	  EnterCriticalSection (&bitmap_section);
      BeginPaint(hWnd, &ps); 
	  rc = BitBlt(ps.hdc, 
		          ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right-ps.rcPaint.left, ps.rcPaint.bottom-ps.rcPaint.top,
                  hCanvas, ps.rcPaint.left,ps.rcPaint.top, SRCCOPY);
      if (!rc)
	    dw = GetLastError();
	  EndPaint(hWnd, &ps); 
	  LeaveCriticalSection (&bitmap_section);
    }
    return 0;

  case WM_DESTROY:
    DeleteDC(hCanvas); 
    DeleteObject(hBmp);
	DeleteCriticalSection(&bitmap_section);
    PostQuitMessage(0);
    return 0;
  }
 return (OptWndProc(hWnd, message, wParam, lParam));
}




void init_device(void)
{	
	HDC hdc; 
	SetWindowPos(hMainWnd,HWND_TOP,xpos,ypos,width,height,SWP_NOREDRAW);
	hdc = GetDC(hMainWnd);
	  hCanvas = CreateCompatibleDC(hdc);	 
	  if (hCanvas==NULL) return;
      hBmp = CreateCompatibleBitmap(hdc,framewidth,frameheight);
      if (hBmp==NULL) return;
      SelectObject(hCanvas, hBmp); 
      ReleaseDC(hMainWnd, hdc); 
   InitializeCriticalSection (&bitmap_section);
   vmb_size = frameheight*framewidth*4;
}


/* Interface to the virtual motherboard */

unsigned char *vmb_get_payload(unsigned int offset,int size)
{ static unsigned char payload[258*8];
  int i = 0;
  EnterCriticalSection (&bitmap_section);
  while (size >= 4)
  { int x, y;
    COLORREF c;
    y = offset / framewidth;
	x = offset % framewidth;
	c = GetPixel(hCanvas,x,y);
    payload[i++]= 0;
    payload[i++]= GetRValue(c);
    payload[i++]= GetGValue(c);
    payload[i++]= GetBValue(c);
	offset = offset+4;
	size = size-4;
  }
  if (size > 0)
  { int x, y;
    COLORREF c;
    y = offset / framewidth;
	x = offset % framewidth;
	c = GetPixel(hCanvas,x,y);
    payload[i++]= 0;
    if (size>1) payload[i++]= GetRValue(c);
    if (size>2) payload[i++]= GetGValue(c);
    if (size>3) payload[i++]= GetBValue(c);
  }
  LeaveCriticalSection (&bitmap_section);
  return payload;
}

void vmb_put_payload(unsigned int offset,int size, unsigned char *payload)
{ RECT rect;
  int i = 0;
  int minx=framewidth-1,miny=frameheight-1,maxx=0,maxy=0;
  EnterCriticalSection (&bitmap_section);
  while (size >= 4)
  { int x, y;
    COLORREF c;
    y = (offset/4) / framewidth;
	x = (offset/4) % framewidth;
	if (x>maxx)maxx=x;
	if (y>maxy)maxy=y;
	if (x<minx)minx=x;
	if (y<miny)miny=y;
	c = RGB(payload[i+1],payload[i+2],payload[i+3]);
	c = SetPixel(hCanvas,x,y,c);
	i = i+4;
	offset = offset+4;
	size = size-4;
  }
  if (size > 0)
  { int x, y;
    unsigned char r,g,b;
    COLORREF c;
    y = (offset/4) / framewidth;
	x = (offset/4) % framewidth;
	if (x>maxx)maxx=x;
	if (y>maxy)maxy=y;
	if (x<minx)minx=x;
	if (y<miny)miny=y;
	c = GetPixel(hCanvas,x,y);
	r = GetRValue(c);
    g = GetGValue(c);
    b = GetBValue(c);

	if (size > 1) r = payload[i+1];
    if (size > 2) g = payload[i+2];
	if (size > 3) b = payload[i+3];
	c = RGB(r,g,b);
 	c = SetPixel(hCanvas,x,y,c);
  }
  rect.top=miny, rect.left=minx, rect.bottom=maxy+1, rect.right=maxx+1;
  InvalidateRect(hMainWnd,&rect,FALSE);
  LeaveCriticalSection (&bitmap_section);
}

void vmb_poweron(void)
{ RECT rect;
  int rc;
  HBRUSH hbr;
  EnterCriticalSection (&bitmap_section);
  hbr = CreateSolidBrush(RGB(0,0,0));
  rect.top=0, rect.left=0, rect.bottom=frameheight, rect.right=framewidth;
  rc = FillRect(hCanvas,&rect,hbr);
  InvalidateRect(hMainWnd,&rect,FALSE);
  LeaveCriticalSection (&bitmap_section);
}


void vmb_poweroff(void)
{ RECT rect;
  int rc;
  HBRUSH hbr;
  EnterCriticalSection (&bitmap_section);
  hbr = CreateSolidBrush(RGB(127,127,127));
  rect.top=0, rect.left=0, rect.bottom=frameheight, rect.right=framewidth;
  rc = FillRect(hCanvas,&rect,hbr);
  InvalidateRect(hMainWnd,&rect,FALSE);
  LeaveCriticalSection (&bitmap_section);
}

void vmb_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{ RECT rect;
  int rc;
  HBRUSH hbr;
  EnterCriticalSection (&bitmap_section);
  hbr = CreateSolidBrush(RGB(127,0,0));
  rect.top=0, rect.left=0, rect.bottom=frameheight, rect.right=framewidth;
  rc = FillRect(hCanvas,&rect,hbr);
  InvalidateRect(hMainWnd,&rect,FALSE);
  LeaveCriticalSection (&bitmap_section);
  SendMessage(hMainWnd,WM_USER+4,0,0); /* the disconnect button */

}


void vmb_reset(void)
{ RECT rect;
  int rc;
  HBRUSH hbr;
  EnterCriticalSection (&bitmap_section);
  hbr = CreateSolidBrush(RGB(0,0,0));
  rect.top=0, rect.left=0, rect.bottom=height, rect.right=width;
  rc = FillRect(hCanvas,&rect,hbr);
  InvalidateRect(hMainWnd,&rect,FALSE);
  LeaveCriticalSection (&bitmap_section);
}
