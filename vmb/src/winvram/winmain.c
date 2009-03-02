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

static CRITICAL_SECTION   bitmap_section;


static unsigned char *vram=NULL;

static void init_bitmap(void)
{	
	SetWindowPos(hMainWnd,HWND_TOP,xpos,ypos,width*zoom,height*zoom,SWP_NOREDRAW);
	{ unsigned int new_size = frameheight*framewidth*4;
      if (vram==NULL)
	  { vram= malloc(new_size);
	    vmb_size = new_size;
		memset(vram,0,new_size);
	  }
	  else
	  { unsigned char * new_vram;
	    new_vram = realloc(vram,new_size);
		if (new_vram)
		{ vram = new_vram;
		  if (new_size> vmb_size)
			memset(vram+vmb_size,0, new_size-vmb_size);
		  vmb_size=new_size;
		}
	  }
	}
	if (vram == NULL)
      vmb_fatal_error(__LINE__,"Out of memory initializing video memory");
}

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
      { GetDlgItemText(hDlg,IDC_ADDRESS,tmp_option,MAXTMPOPTION);
        vmb_address = strtouint64(tmp_option); 
		frameheight=GetDlgItemInt(hDlg,IDC_FRAMEHEIGHT,FALSE,FALSE);
		framewidth=GetDlgItemInt(hDlg,IDC_FRAMEWIDTH,FALSE,FALSE);
		height=GetDlgItemInt(hDlg,IDC_HEIGHT,FALSE,FALSE);
		width=GetDlgItemInt(hDlg,IDC_WIDTH,FALSE,FALSE);
		zoom=GetDlgItemInt(hDlg,IDC_ZOOM,FALSE,FALSE);
		init_bitmap();
      }
      if (wparam == IDOK || wparam == IDCANCEL)
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
  }
  return FALSE;
}

static void paint_pixel(HDC hdc, int x, int y)
{ RECT rect;
  HBRUSH color;
  rect.top=y*zoom;
  rect.bottom=y*zoom+zoom;
  rect.left=x*zoom;
  rect.right=x*zoom+zoom;

  color = CreateSolidBrush(RGB(vram[(y*framewidth+x)*4+1],
	                           vram[(y*framewidth+x)*4+2],
							   vram[(y*framewidth+x)*4+3]));
  FillRect(hdc,&rect,color);
  DeleteObject(color);
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
	  HDC hdc;
	  int x, y;
	  hdc =BeginPaint(hWnd,&ps);
	  EnterCriticalSection (&bitmap_section);
	  for (y=ps.rcPaint.top/zoom;y<=(ps.rcPaint.bottom-1)/zoom;y++)
	    for (x=ps.rcPaint.left/zoom;x<=(ps.rcPaint.right-1)/zoom;x++)
	     paint_pixel(hdc,x,y);	  
	  LeaveCriticalSection (&bitmap_section);
	  EndPaint(hWnd, &ps); 
    }
    return 0;

  case WM_DESTROY:
	DeleteCriticalSection(&bitmap_section);
    PostQuitMessage(0);
    return 0;
  }
 return (OptWndProc(hWnd, message, wParam, lParam));
}



void init_device(void)
{	
   init_bitmap();
   InitializeCriticalSection (&bitmap_section);
}


/* Interface to the virtual motherboard */

unsigned char *vmb_get_payload(unsigned int offset,int size)
{ static unsigned char payload[258*8];
  EnterCriticalSection (&bitmap_section);
  memmove(payload,vram+offset,size);
  LeaveCriticalSection (&bitmap_section);
  return payload;
}

void vmb_put_payload(unsigned int offset,int size, unsigned char *payload)
{ RECT rect;
  int minx, miny, maxx, maxy;
  EnterCriticalSection (&bitmap_section);
  memmove(vram+offset,payload,size);
  miny=offset/(4*framewidth);
  maxy=(offset+size-1)/(4*framewidth);
  if (maxy>miny) 
  { minx=0; 
    maxx=width; 
  }
  else
  { minx = (offset%(4*framewidth))/4;
    maxx = ((offset+size-1)%(4*framewidth))/4;
  }
  rect.top=miny*zoom, rect.left=minx*zoom, rect.bottom=(maxy+1)*zoom, rect.right=(maxx+1)*zoom;
  InvalidateRect(hMainWnd,&rect,FALSE);
  LeaveCriticalSection (&bitmap_section);
}

void vmb_poweron(void)
{ RECT rect;
  EnterCriticalSection (&bitmap_section);
  memset(vram,0,vmb_size);
  rect.top=0, rect.left=0, rect.bottom=height*zoom, rect.right=width*zoom;
  InvalidateRect(hMainWnd,&rect,FALSE);
  LeaveCriticalSection (&bitmap_section);
}


void vmb_poweroff(void)
{ RECT rect;
  EnterCriticalSection (&bitmap_section);
  memset(vram,0x7F,vmb_size);
  rect.top=0, rect.left=0, rect.bottom=height*zoom, rect.right=width*zoom;
  InvalidateRect(hMainWnd,&rect,FALSE);
  LeaveCriticalSection (&bitmap_section);
}


void vmb_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{  RECT rect;
   unsigned int i;
  EnterCriticalSection (&bitmap_section);
  for(i = 1; i < vmb_size; i= i+8)
	  vram[i]= 0xFF;
  rect.top=0, rect.left=0, rect.bottom=height*zoom, rect.right=width*zoom;
  InvalidateRect(hMainWnd,&rect,FALSE);
  LeaveCriticalSection (&bitmap_section);
  SendMessage(hMainWnd,WM_USER+4,0,0); /* the disconnect button */
}


void vmb_reset(void)
{ vmb_poweron();
}
