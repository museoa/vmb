#include <winsock2.h>
#include <windows.h>
#include <afxres.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "param.h"
#include "option.h"

extern int width, height, framewidth,frameheight;

/* Global Variables loaded form the Resourcefile*/
#define MAX_LOADSTRING 100			
static TCHAR szClassName[MAX_LOADSTRING];
static TCHAR szTitle[MAX_LOADSTRING];
static HDC hCanvas;
static HBITMAP hBmp;
static HMENU hMenu;
static CRITICAL_SECTION   bitmap_section;

/* Global Variables for important Windows */
static HWND hDebug=NULL; /* debug output goes to this window, if not NULL */
HWND hMainWnd;
static HINSTANCE hInst;

/* Global variables defining the properties of the device */
#define MAXHOST 1024
unsigned int address_hi=0;
unsigned int address_lo=0;


void win32_message(char *msg)
{
	MessageBox(NULL,msg,"Message",MB_OK);
}

void win32_debug(char *msg)
{ static char nl[] ="\r\n";	
  LRESULT  n;
  if (hDebug == NULL) return;
  n = SendDlgItemMessage(hDebug,IDC_DEBUG,EM_GETLINECOUNT,0,0);
  if (n>100)
  { n = SendDlgItemMessage(hDebug,IDC_DEBUG,EM_LINELENGTH,0,0);
    SendDlgItemMessage(hDebug,IDC_DEBUG,EM_SETSEL,0,n+2);
    SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)"");
    n = SendDlgItemMessage(hDebug,IDC_DEBUG,WM_GETTEXTLENGTH,0,0);
    SendDlgItemMessage(hDebug,IDC_DEBUG,EM_SETSEL,n,n);
  }
  SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)msg);
  SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)nl);
}


INT_PTR CALLBACK   
DebugDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{
  switch ( message )
  { case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { vmb_debug_flag = 0;
	    hDebug = NULL;
		EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
	case WM_SIZE: 
      MoveWindow(GetDlgItem(hDebug,IDC_DEBUG),5,5,LOWORD(lparam)-10,HIWORD(lparam)-10,TRUE); 
       return TRUE;
    break;
  }
  return FALSE;
}

INT_PTR CALLBACK    
AboutDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{
  switch ( message )
  { case WM_INITDIALOG:
      return TRUE;
    case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
      {
        EndDialog(hDlg, TRUE);
        return TRUE;
      }
     break;
  }
  return FALSE;
}




INT_PTR CALLBACK    
ConnectDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{

  switch ( message )
  { case WM_INITDIALOG:
      SetDlgItemText(hDlg,IDC_THE_SERVER,host);
      SetDlgItemInt(hDlg,IDC_THE_PORT,port,FALSE);
      return TRUE;
   case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
        { 
	      GetDlgItemText(hDlg,IDC_THE_SERVER,host,MAXHOST);
          port = GetDlgItemInt(hDlg,IDC_THE_PORT,NULL,FALSE);
		  if (!vmb_connected)
		  {  vmb_connect(host,port);
		     vmb_register(vmb_address_hi, vmb_address_lo,vmb_size,0,0,defined);
			 SendMessage(hMainWnd,WM_USER+3,0,0); /* the connect button */
		  }
		  EndDialog(hDlg, TRUE);
          return TRUE;
      }
      else if( wparam == IDCANCEL )
      {
        EndDialog(hDlg, TRUE);
        return TRUE;
      }
     break;
  }
  return FALSE;
}





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

HBITMAP hBmp;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ switch (message) 
  {  
  case WM_NCHITTEST:
    return HTCAPTION;
  case WM_USER+3: /* Connected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Disconnect"))
	  DrawMenuBar(hMainWnd);
 	return 0;
  case WM_USER+4: /* Disconnected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Connect..."))
	  DrawMenuBar(hMainWnd);
	return 0;
  case WM_CREATE:
	{ HDC hdc; 
	  hdc = GetDC(hWnd);
	  hCanvas = CreateCompatibleDC(hdc);	 
	  if (hCanvas==NULL) return 0;
      hBmp = CreateCompatibleBitmap(hdc,framewidth,frameheight);
      if (hBmp==NULL) return 0;
      SelectObject(hCanvas, hBmp); 
      ReleaseDC(hWnd, hdc); 
	}
    return 0; 
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
      return 0;
    }
    return 0;
  case WM_NCRBUTTONDOWN: /* right Mouse Button -> Context Menu */
    TrackPopupMenu(GetSubMenu(hMenu,0),TPM_LEFTALIGN|TPM_TOPALIGN,
		   LOWORD(lParam),HIWORD(lParam),0 ,hWnd,NULL);
    return 0;
  case WM_COMMAND:
    if (HIWORD(wParam)==0) /* Menu */
      switch(LOWORD(wParam))
	{ case ID_EXIT:
	    PostQuitMessage(0);
	    return 0;
	case ID_CONNECT:
	  if (!vmb_connected)
	    DialogBox(hInst,MAKEINTRESOURCE(IDD_CONNECT),hWnd,ConnectDialogProc);
	  else
	    vmb_disconnect();
	  return 0;
	case ID_SETTINGS:
	  DialogBox(hInst,MAKEINTRESOURCE(IDD_SETTINGS),hMainWnd,SettingsDialogProc);
	  return 0; 
	case ID_DEBUG:
	  if (hDebug==NULL)
	  {  vmb_debug_flag = 1;
	     hDebug= CreateDialog(hInst,MAKEINTRESOURCE(IDD_DEBUG),hWnd,DebugDialogProc);
	  }
	  return 0; 
	case ID_HELP_ABOUT:
	  DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),hWnd,AboutDialogProc);
	  return 0; 
	case ID_HELP:
	  DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),hWnd,AboutDialogProc);
	  return 0; 
	case ID_MINIMIZE:
	  CloseWindow(hWnd);
	  return 0; 
	}
    return 0;

  case WM_DESTROY:
    DeleteDC(hCanvas); 
    DeleteObject(hBmp); 
    PostQuitMessage(0);
    return 0;
  default:
    return (DefWindowProc(hWnd, message, wParam, lParam));
  }
 return (DefWindowProc(hWnd, message, wParam, lParam));
}



BOOL InitInstance(HINSTANCE hInstance)
{
  WNDCLASSEX wcex;

    hInst = hInstance; 
   	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; /*(HBRUSH)(COLOR_WINDOW+1);*/
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
/*	wcex.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);
*/
	if (!RegisterClassEx(&wcex)) return FALSE;

    hMainWnd = CreateWindow(szClassName, szTitle ,WS_POPUP,
                            0, 0, width, height,
	                        NULL, NULL, hInstance, NULL);

   return TRUE;
}




int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	HACCEL hAccelTable;
    MSG msg;
	InitializeCriticalSection (&bitmap_section);
	LoadString(hInstance, IDS_CLASS, szClassName, MAX_LOADSTRING);
	LoadString(hInstance, IDS_TITLE, szTitle, MAX_LOADSTRING);
	hMenu = LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU));

	param_init();
	vmb_size = frameheight*framewidth*4;

	if (!InitInstance (hInstance)) return FALSE;

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
	ShowWindow(hMainWnd, nCmdShow);
	UpdateWindow(hMainWnd);

    vmb_message_hook = win32_message;
	vmb_debug_hook = win32_debug;
	vmb_connect(host,port);
	vmb_register(vmb_address_hi,vmb_address_lo,vmb_size,0,0,defined);
	SendMessage(hMainWnd,WM_USER+3,0,0); /* connected */
	if (vmb_debug_flag)
	  SendMessage(hMainWnd,WM_COMMAND,(WPARAM)ID_DEBUG,0);

	while (GetMessage(&msg, NULL, 0, 0)) 
	  if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
	  { TranslateMessage(&msg);
	    DispatchMessage(&msg);
	  }
	vmb_disconnect();
	DeleteCriticalSection(&bitmap_section);
    return (int)msg.wParam;
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


void vmb_terminate(void)
/* this function is called when the motherboard politely asks the device to terminate.*/
{ 
   PostMessage(hMainWnd,WM_QUIT,0,0);
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
