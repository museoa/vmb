
#include <windows.h>
#include <commctrl.h>
#include "winopt.h"
#include "param.h"
#include "option.h"
#include "vmb.h"

HINSTANCE hInst;
HWND hMainWnd;
HBITMAP hBmp=NULL;
HMENU hMenu;
HBITMAP hon,hoff,hconnect;
device_info vmb = {0};


BOOL InitInstance(HINSTANCE hInstance)
{
  WNDCLASSEX wcex;
  BITMAP bm;
  int r;

#define MAX_LOADSTRING 100		
  static TCHAR szClassName[MAX_LOADSTRING];
  static TCHAR szTitle[MAX_LOADSTRING];
  hInst = hInstance; 

  r = LoadString(hInstance, IDS_CLASS, szClassName, MAX_LOADSTRING);
  if (r==0)
  { r = GetLastError();
    vmb_debugi(VMB_DEBUG_FATAL,"Unable to load class name (%X)",r);
	vmb_fatal_error(__LINE__,"Unable to load class name");
  }
  r = LoadString(hInstance, IDS_TITLE, szTitle, MAX_LOADSTRING);
  if (r==0)
  { r = GetLastError();
    vmb_debugi(VMB_DEBUG_FATAL,"Unable to load window title (%X)",r);
  }
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

	if (hBmp)
		GetObject(hBmp, sizeof(bm), &bm);
	else
		bm.bmWidth=bm.bmHeight=CW_USEDEFAULT;

    hMainWnd = CreateWindow(szClassName, szTitle ,WS_POPUP,
                            xpos, ypos, bm.bmWidth, bm.bmHeight,
	                        NULL, NULL, hInstance, NULL);

    if (hMainWnd && hBmp) 
	{ 
	  HRGN h = BitmapToRegion(hBmp);
	  if (h) SetWindowRgn(hMainWnd, h, TRUE);
	}

   return TRUE;
}

#define SUBWINDOW_MAX 100
static int  subwindow_num=0;
static HWND subwindow[SUBWINDOW_MAX] = {0};

void register_subwindow(HWND h)
{ if (subwindow_num<SUBWINDOW_MAX)
  { subwindow[subwindow_num]=h;
	subwindow_num++;
  }
  else
    vmb_error(__LINE__,"Too many subwindows, limited functionality");
}

void unregister_subwindow(HWND h)
{ int i;
  for (i=0; i< subwindow_num;i++)
    if (subwindow[i]==h)
	{ subwindow_num--;
	  while (i<subwindow_num)
	  { subwindow[i]=subwindow[i+1];
	    i++;
	  }
      return;
	}
    vmb_error(__LINE__,"Unregistering an unregistered window");
}

static BOOL do_subwindow_msg(MSG *msg)
{ int i;
  for (i=0; i<subwindow_num;i++)
	  if (IsDialogMessage(subwindow[i], msg)) 
		  return TRUE;
  return FALSE;
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	HACCEL hAccelTable;
    MSG msg;
    vmb_message_hook = win32_message;
	vmb_debug_hook = win32_debug;

	hMenu = LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU));
	hBmp = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_BITMAP), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    hon = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_ON), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
    hconnect = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_CONNECT), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);
    hoff = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OFF), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
    InitCommonControls();
	if (!InitInstance (hInstance)) return FALSE;
	
	param_init();
    get_pos_key(&xpos,&ypos,defined);
    init_device(&vmb);

	SetWindowPos(hMainWnd,HWND_TOP,xpos,ypos,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
	if (minimized)CloseWindow(hMainWnd); 
	UpdateWindow(hMainWnd);
	vmb_begin();
 	vmb_connect(&vmb,host,port);
	vmb_register(&vmb,HI32(vmb_address),LO32(vmb_address),vmb_size,0,0,defined);
    SendMessage(hMainWnd,WM_VMB_CONNECT,0,0); /* the connect button */
	if (vmb_debug_flag) vmb_debug_on(); else vmb_debug_off();
	if (vmb_debug_flag) hDebug= CreateDialog(hInst,MAKEINTRESOURCE(IDD_DEBUG),hMainWnd,DebugDialogProc);
	if (vmb_verbose_flag) vmb_debug_mask=0; 
	CheckMenuItem(hMenu,ID_DEBUG,MF_BYCOMMAND|(vmb_debug_flag?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_VERBOSE,MF_BYCOMMAND|(vmb_debug_mask==0?MF_CHECKED:MF_UNCHECKED));

	while (GetMessage(&msg, NULL, 0, 0)) 
	  if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg) &&
		  !do_subwindow_msg(&msg) 
		  ) 
	  { TranslateMessage(&msg);
	    DispatchMessage(&msg);
	  }
	vmb_disconnect(&vmb);
    set_pos_key(hMainWnd,defined);
	vmb_end();
	return (int)msg.wParam;
}

