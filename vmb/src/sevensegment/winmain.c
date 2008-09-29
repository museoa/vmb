#include <winsock2.h>
#include <windows.h>
#include <afxres.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "winopt.h"
#include "param.h"
#include "option.h"



HINSTANCE hInst;
HWND hMainWnd;
HBITMAP hBmp;
HMENU hMenu;
HBITMAP hon,hoff,hconnect;




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


static HWND hBits[64];
static HBITMAP hhor,hvert,hdot;
static int xpixelpos[8] = {39,25,11,  20, 5, 80,65,  82};
static int ypixelpos[8] = {5,53,101,   10,59,  12,61, 101};
#define DIGITLENGTH 100
#define WINHEIGHT 111
#define WINLENGTH (DIGITLENGTH*8)
static enum {vert, hor, dot} bittyp[8] = {hor,hor,hor,vert,vert,vert,vert,dot};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ switch (message) 
  {  
  case WM_USER+1: /* on*/
	  return 0;
  case WM_USER+2: /* off */
	  return 0;
  case WM_USER+3: /* Connected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Disconnect"))
	  DrawMenuBar(hMainWnd);
 	return 0;
  case WM_USER+4: /* Disconnected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Connect..."))
	  DrawMenuBar(hMainWnd);
	return 0;
  case WM_PAINT:
    return (DefWindowProc(hWnd, message, wParam, lParam));
  case WM_CREATE:
      /* create main dialog */ 
      { int i,k;
	    for (k=0;k<8;k++)
     	  for (i=0;i<8;i++)
		  {	hBits[i+k*8] = CreateWindow("STATIC",NULL,WS_CHILD|WS_VISIBLE|SS_BITMAP|SS_REALSIZEIMAGE,
		                    xpixelpos[i]+k*DIGITLENGTH,ypixelpos[i],0,0,hWnd,NULL,hInst,0);
            if (bittyp[i] ==vert)
		      SendMessage(hBits[i+k*8],STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hvert);
            else if (bittyp[i] == hor)
		      SendMessage(hBits[i+k*8],STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hhor);
            else if (bittyp[i] ==dot)
		      SendMessage(hBits[i+k*8],STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hdot);
			ShowWindow(hBits[i+k*8],SW_SHOW);
		  }
      }	
    return 0; 
  }
 return (OptWndProc(hWnd, message, wParam, lParam));
}



void init_device(void)
{
}



BOOL InitInstance(HINSTANCE hInstance)
{ int r;
  WNDCLASSEX wcex;
#define MAX_LOADSTRING 100		
  static TCHAR szClassName[MAX_LOADSTRING];
  static TCHAR szTitle[MAX_LOADSTRING];
  r = LoadString(hInstance, IDS_CLASS, szClassName, MAX_LOADSTRING);
  if (r==0)
  { r = GetLastError();
    vmb_debugi(1,"Unable to load class name (%X)",r);
	vmb_fatal_error(__LINE__,"Unable to load class name");
  }
  r = LoadString(hInstance, IDS_TITLE, szTitle, MAX_LOADSTRING);
  if (r==0)
  { r = GetLastError();
    vmb_debugi(1,"Unable to load window title (%X)",r);
  }
 
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
	wcex.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
/*	wcex.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);
*/
	if (!RegisterClassEx(&wcex)) return FALSE;

    hMainWnd = CreateWindow(szClassName, szTitle ,WS_POPUP,
                            xpos, ypos, WINLENGTH, WINHEIGHT,
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

	hMenu = LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU));

    hvert = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_VERT), 
				IMAGE_BITMAP, 0, 0, 0);
    hhor = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_HOR), 
				IMAGE_BITMAP, 0, 0, 0);
    hdot = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_DOT), 
				IMAGE_BITMAP, 0, 0, 0);

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

	if (!InitInstance (hInstance)) return FALSE;

	param_init();
	get_pos_key(&xpos,&ypos,defined);
 
	vmb_size = 8;

	SetWindowPos(hMainWnd,HWND_TOP,xpos,ypos,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
	UpdateWindow(hMainWnd);

	vmb_connect(host,port);
	vmb_register(vmb_address_hi,vmb_address_lo,vmb_size,0,0,defined);
    SendMessage(hMainWnd,WM_USER+3,0,0); /* the connect button */
	if (vmb_debug_flag) vmb_debug_on(); else vmb_debug_off();
	CheckMenuItem(hMenu,ID_DEBUG,MF_BYCOMMAND|(vmb_debug_flag?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_VERBOSE,MF_BYCOMMAND|(vmb_verbose_level==0?MF_CHECKED:MF_UNCHECKED));

	while (GetMessage(&msg, NULL, 0, 0)) 
      if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
	  { TranslateMessage(&msg);
	    DispatchMessage(&msg);
	  }
	vmb_disconnect();
    set_pos_key(xpos,ypos,defined);
    return (int)msg.wParam;
}


static unsigned char segmentbits[8];

static void update_bits(void)
{ int i, k;
  unsigned char c;
  for (k=0;k<8;k++)
  { c = segmentbits[k];
    for (i=0;i<8;i++)
    {  if (c&0x1) 
         ShowWindow(hBits[i+8*k],SW_SHOW); 
        else  
	      ShowWindow(hBits[i+8*k],SW_HIDE);
        c = c>>1;
    }
  }
  UpdateWindow(hMainWnd);
}
   

/* Interface to the virtual motherboard */


unsigned char *vmb_get_payload(unsigned int offset,int size)
{ 
  return segmentbits+offset;
}

void vmb_put_payload(unsigned int offset,int size, unsigned char *payload)
{ memmove(segmentbits+offset,payload,size);
  update_bits();
}

void vmb_poweron(void)
{ memset(segmentbits,0xFF,8);
  update_bits();
}


void vmb_poweroff(void)
{ memset(segmentbits,0x80,8);
  update_bits();
}

void vmb_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{ memset(segmentbits,0x02,8);
  update_bits();
  SendMessage(hMainWnd,WM_USER+4,0,0); /* the disconnect button */
}


void vmb_terminate(void)
/* this function is called when the motherboard politely asks the device to terminate.*/
{ 
   PostMessage(hMainWnd,WM_QUIT,0,0);
}


void vmb_reset(void)
{ memset(segmentbits,0xFF,8);
  update_bits();
}
