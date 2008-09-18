#include <winsock2.h>
#include <windows.h>
#include <afxres.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "param.h"
#include "option.h"



/* Global Variables loaded form the Resourcefile*/
	

static HBITMAP hBmp;
static HMENU hMenu;
static HBITMAP hon,hoff,hconnect;

/* Global Variables for important Windows */
static HWND hpower;
HWND hMainWnd;
static HINSTANCE hInst;

/* Global variables defining the properties of the device */
#define MAXHOST 1024
unsigned int address_hi=0;
unsigned int address_lo=0;

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


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ switch (message) 
  {  
  case WM_NCHITTEST:
    return HTCAPTION;
  case WM_USER+1: /* Power On */
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hon);
	return 0;
  case WM_USER+2: /* Power Off */
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hoff);
	return 0;
  case WM_USER+3: /* Connected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Disconnect"))
	  DrawMenuBar(hMainWnd);
	SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hoff);
 	return 0;
  case WM_USER+4: /* Disconnected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Connect..."))
	  DrawMenuBar(hMainWnd);
	   SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hconnect);
	return 0;
  case WM_CREATE: 
	hpower = CreateWindow("STATIC",NULL,WS_CHILD|WS_VISIBLE|SS_BITMAP|SS_REALSIZEIMAGE,10,10,32,32,hWnd,NULL,hInst,0);
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hoff);
    return 0;
  case WM_PAINT:
    { PAINTSTRUCT ps;
      HDC hdc;
      hdc = BeginPaint (hWnd, &ps);
      if (hBmp)
      {	HDC memdc = CreateCompatibleDC(NULL);
        HBITMAP h = (HBITMAP)SelectObject(memdc, hBmp);
        BITMAP bm;
        GetObject(hBmp, sizeof(bm), &bm);
        BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, memdc, 0, 0, SRCCOPY);
        SelectObject(memdc, h);
      }
      EndPaint (hWnd, &ps);
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
          if (vmb_debug_flag) vmb_debug_off(); else vmb_debug_on();
	    CheckMenuItem(hMenu,ID_DEBUG,MF_BYCOMMAND|(vmb_debug_flag?MF_CHECKED:MF_UNCHECKED));
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
    PostQuitMessage(0);
    return 0;
  default:
    return (DefWindowProc(hWnd, message, wParam, lParam));
  }
 return (DefWindowProc(hWnd, message, wParam, lParam));
}



extern HRGN BitmapToRegion (HBITMAP hBmp);


BOOL InitInstance(HINSTANCE hInstance)
{
  WNDCLASSEX wcex;
  BITMAP bm;

#define MAX_LOADSTRING 100		
  static TCHAR szClassName[MAX_LOADSTRING];
  static TCHAR szTitle[MAX_LOADSTRING];
  hInst = hInstance; 

  LoadString(hInstance, IDS_CLASS, szClassName, MAX_LOADSTRING);
  LoadString(hInstance, IDS_TITLE, szTitle, MAX_LOADSTRING);

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

	GetObject(hBmp, sizeof(bm), &bm);

    hMainWnd = CreateWindow(szClassName, szTitle ,WS_POPUP,
                            xpos, ypos, bm.bmWidth, bm.bmHeight,
	                        NULL, NULL, hInstance, NULL);

    if (hMainWnd) 
	{ 
	  HRGN h = BitmapToRegion(hBmp);
	  if (h) SetWindowRgn(hMainWnd, h, TRUE);
	}

   return TRUE;
}



extern int ramsize;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	HACCEL hAccelTable;
    MSG msg;


	hMenu = LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU));
	hBmp = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_BITMAP), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    hon = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_ON), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
    hconnect = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_CONNECT), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
    hoff = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OFF), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
	if (hBmp==NULL) return FALSE;

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

	if (!InitInstance (hInstance)) return FALSE;

	param_init();

	SetWindowPos(hMainWnd,HWND_TOP,xpos,ypos,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
	UpdateWindow(hMainWnd);

	vmb_connect(host,port);
	vmb_register(vmb_address_hi,vmb_address_lo,vmb_size,0,0,defined);
    SendMessage(hMainWnd,WM_USER+3,0,0); /* the connect button */
	if (vmb_debug_flag)
	  SendMessage(hMainWnd,WM_COMMAND,(WPARAM)ID_DEBUG,0);

	while (GetMessage(&msg, NULL, 0, 0)) 
	  if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
	  { TranslateMessage(&msg);
	    DispatchMessage(&msg);
	  }
	vmb_disconnect();
    return (int)msg.wParam;
}

