#include <winsock2.h>
#include <windows.h>
#include <afxres.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "param.h"
#include "option.h"
#include "winopt.h"

extern int width, height, framewidth, frameheight, zoom;


static HDC hCanvas = NULL;
static CRITICAL_SECTION   bitmap_section;



INT_PTR CALLBACK   
SettingsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{
  switch ( message )
  { case WM_INITDIALOG:
      uint64tohex(vmb_address,tmp_option);
      SetDlgItemText(hDlg,IDC_ADDRESS,tmp_option);
	  SetDlgItemInt(hDlg,IDC_FRAMEHEIGHT,frameheight,FALSE);
	  SetDlgItemInt(hDlg,IDC_FRAMEWIDTH,framewidth,FALSE);
	  SetDlgItemInt(hDlg,IDC_HEIGHT,height,FALSE);
	  SetDlgItemInt(hDlg,IDC_WIDTH,width,FALSE);
	  SetDlgItemInt(hDlg,IDC_ZOOM,zoom,FALSE);
      return TRUE;
   case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
      { int newframewidth, newframeheight;
	    GetDlgItemText(hDlg,IDC_ADDRESS,tmp_option,MAXTMPOPTION);
        vmb_address = strtouint64(tmp_option); 
		newframeheight=GetDlgItemInt(hDlg,IDC_FRAMEHEIGHT,FALSE,FALSE);
		newframewidth=GetDlgItemInt(hDlg,IDC_FRAMEWIDTH,FALSE,FALSE);

		height=GetDlgItemInt(hDlg,IDC_HEIGHT,FALSE,FALSE);
		width=GetDlgItemInt(hDlg,IDC_WIDTH,FALSE,FALSE);
		zoom=GetDlgItemInt(hDlg,IDC_ZOOM,FALSE,FALSE);
		if(newframeheight!=frameheight || newframewidth!=framewidth)
		  init_device();
		SetWindowPos(hMainWnd,HWND_TOP,xpos,ypos,width*zoom,height*zoom,SWP_SHOWWINDOW);
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
	InitializeCriticalSection (&bitmap_section);
    return (DefWindowProc(hWnd, message, wParam, lParam));

 case WM_PAINT:
	{ PAINTSTRUCT ps;
	  BOOL rc;
	  DWORD dw;
	  int src_left, src_top,src_right,src_bottom;
	  EnterCriticalSection (&bitmap_section);
      BeginPaint(hWnd, &ps); 
	  src_left = ps.rcPaint.left/zoom;
	  src_right = (ps.rcPaint.right+zoom-1)/zoom;
	  src_top = ps.rcPaint.top/zoom;
	  src_bottom = (ps.rcPaint.bottom+zoom-1)/zoom;
	  SetStretchBltMode(ps.hdc,COLORONCOLOR);
	  rc = StretchBlt(ps.hdc, 
		          src_left*zoom,src_top*zoom,
				  (src_right-src_left)*zoom, (src_bottom-src_top)*zoom,
                  hCanvas, 
				  src_left,src_top, 
				  src_right-src_left, src_bottom-src_top,
				  SRCCOPY);
      if (!rc)
	    dw = GetLastError();
	  EndPaint(hWnd, &ps); 
	  LeaveCriticalSection (&bitmap_section);
    }
    return 0;

  case WM_DESTROY:
    if (hCanvas != NULL) DeleteDC(hCanvas); 
    if (hBmp!=NULL) DeleteObject(hBmp);
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
	if (hCanvas!=NULL) DeleteDC(hCanvas); 
	hCanvas = CreateCompatibleDC(hdc);	 
	if (hCanvas==NULL) return;
    if (hBmp!=NULL) DeleteObject(hBmp);
    hBmp = CreateCompatibleBitmap(hdc,framewidth,frameheight);
    if (hBmp==NULL) return;
    SelectObject(hCanvas, hBmp); 
    ReleaseDC(hMainWnd, hdc); 
    vmb_size = frameheight*framewidth*4;
	SetWindowPos(hMainWnd,HWND_TOP,xpos,ypos,width*zoom,height*zoom,SWP_NOREDRAW);
	if (vmb_power) vmb_poweron(); else vmb_poweroff();
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
  rect.top=miny*zoom, rect.left=minx*zoom, rect.bottom=(maxy+1)*zoom, rect.right=(maxx+1)*zoom;
  InvalidateRect(hMainWnd,&rect,FALSE);
  LeaveCriticalSection (&bitmap_section);
}

void vmb_poweron(void)
{ RECT rect;
  int rc;
  HBRUSH hbr;
  EnterCriticalSection (&bitmap_section);
  hbr = CreateSolidBrush(RGB(0,0,0));
  rect.top=0, rect.left=0, rect.bottom=frameheight*zoom, rect.right=framewidth*zoom;
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
  rect.top=0, rect.left=0, rect.bottom=frameheight*zoom, rect.right=framewidth*zoom;
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
  rect.top=0, rect.left=0, rect.bottom=frameheight*zoom, rect.right=framewidth*zoom;
  rc = FillRect(hCanvas,&rect,hbr);
  InvalidateRect(hMainWnd,&rect,FALSE);
  LeaveCriticalSection (&bitmap_section);
  SendMessage(hMainWnd,WM_USER+4,0,0); /* the disconnect button */
}


void vmb_reset(void)
{ vmb_poweron();
}
