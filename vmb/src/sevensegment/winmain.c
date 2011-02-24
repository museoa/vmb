#include <winsock2.h>
#include <windows.h>
#include <commctrl.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "winopt.h"
#include "param.h"
#include "option.h"



HINSTANCE hInst;
HWND hMainWnd;
HBITMAP hBmp=NULL;
HMENU hMenu;
HBITMAP hon,hoff,hconnect;
device_info vmb = {0};



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


static unsigned char segmentbits[8] = {0};
static unsigned char windowbits[8] = {0};
static HPEN hpenGreen, hpenBlack, hpenOld;

#define BORDER 6
#define HEIGHT 48
#define WIDTH  30
#define DISTANCE 50
#define SLANT 15
#define GAP 5
#define GAPSLANT ((GAP*SLANT)/HEIGHT)
#define THICK 5
#define WINHEIGHT (BORDER+HEIGHT+BORDER)
#define WINLENGTH (BORDER+DISTANCE*8+BORDER)

void seg_poweron(void);
void seg_poweroff(void);

void paint_digit(HDC hdc, unsigned char bits, int k)
{  int x, y, dx, dy; 
   int i;
   for (i=0;i<8;i++)
   { if (bits&0x1) 
	   SelectObject(hdc,hpenGreen);
	 else
	   SelectObject(hdc,hpenBlack);
     switch (i)
	 { case 0: /* top mid */
	     x = BORDER+k*DISTANCE+SLANT+GAP;
		 dx = WIDTH-2*GAP;
		 y = BORDER;
		 dy = 0;
	   break;
	   case 1: /* mid mid */
	     x = BORDER+k*DISTANCE+SLANT/2+GAP;
		 dx = WIDTH-2*GAP;
		 y = BORDER+HEIGHT/2;
		 dy = 0;
	   break;
	   case 2: /* bot mid */
	     x = BORDER+k*DISTANCE+GAP;
		 dx = WIDTH-2*GAP;
		 y = BORDER+HEIGHT;
		 dy = 0;
	   break;
	   case 3: /* left top */
	     x = BORDER+k*DISTANCE+SLANT-GAPSLANT;
		 dx = -SLANT/2+GAPSLANT;
		 y = BORDER+GAP;
		 dy = HEIGHT/2-2*GAP;
	   break;
	   case 4: /* left bot */
	     x = BORDER+k*DISTANCE+ SLANT/2-GAPSLANT;
		 dx = -SLANT/2+GAPSLANT;
		 y = BORDER+HEIGHT/2+GAP;
		 dy = HEIGHT/2-2*GAP;
	   break;
	   case 5: /* right top */
		 x = BORDER+k*DISTANCE+ SLANT+WIDTH-GAPSLANT;
		 dx = -SLANT/2+GAPSLANT;
		 y = BORDER+GAP;
		 dy = HEIGHT/2-2*GAP;
	   break;
	   case 6: /* right bot */
		 x = BORDER+k*DISTANCE+ SLANT/2+WIDTH-GAPSLANT;
		 dx = -SLANT/2+GAPSLANT;
		 y = BORDER+HEIGHT/2+GAP;
		 dy = HEIGHT/2-2*GAP;
	   break;
	   case 7: /* dot */
		 x = BORDER+k*DISTANCE+SLANT/2+WIDTH;
		 dx = 1;
		 y = BORDER+HEIGHT;
		 dy = 0;
	   break;
	 }
	 MoveToEx(hdc,x,y,NULL);
	 LineTo(hdc,x+dx,y+dy);
	 bits = bits>>1;
   }
}

 
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ switch (message) 
  {  
  case WM_VMB_ON: /* on*/
	  return 0;
  case WM_VMB_OFF: /* off */
	  return 0;
  case WM_VMB_CONNECT: /* Connected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Disconnect"))
	  DrawMenuBar(hMainWnd);
	if (vmb.power) 
		seg_poweron(); 
	else 
		seg_poweroff();
    return 0;
  case WM_VMB_DISCONNECT: /* Disconnected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Connect..."))
	  DrawMenuBar(hMainWnd);
	return 0;
  case WM_CREATE:
	  hpenGreen = CreatePen(PS_SOLID, THICK, RGB(0, 255, 0));
	  hpenBlack = CreatePen(PS_SOLID, THICK, RGB(0,   0, 0));
    return 0;
  case WM_DESTROY:
	  DeleteObject(hpenGreen);
	  DeleteObject(hpenBlack);
	  break;
  case WM_PAINT:
	{ PAINTSTRUCT ps;
      HDC hdc;
	  HPEN hpenOld;
	  int k;
      hdc = BeginPaint (hWnd, &ps);
	  hpenOld = SelectObject(hdc, hpenGreen);
      for (k=0;k<8;k++) 
	     paint_digit(hdc, segmentbits[k],k);
	  SelectObject(hdc, hpenOld);
      EndPaint (hWnd, &ps);
    }
    return 0;   
 }
 return (OptWndProc(hWnd, message, wParam, lParam));
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
    vmb_debugi(VMB_DEBUG_FATAL,"Unable to load class name (%X)",r);
	vmb_fatal_error(__LINE__,"Unable to load class name");
  }
  r = LoadString(hInstance, IDS_TITLE, szTitle, MAX_LOADSTRING);
  if (r==0)
  { r = GetLastError();
    vmb_debugi(VMB_DEBUG_FATAL,"Unable to load window title (%X)",r);
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

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
    InitCommonControls();
	if (!InitInstance (hInstance)) return FALSE;

	param_init();
	get_pos_key(&xpos,&ypos,defined);
    init_device(&vmb);
	SetWindowPos(hMainWnd,HWND_TOP,xpos,ypos,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
	UpdateWindow(hMainWnd);

	vmb_connect(&vmb,host,port);
	vmb_register(&vmb,HI32(vmb_address),LO32(vmb_address),vmb_size,0,0,defined);
    SendMessage(hMainWnd,WM_VMB_CONNECT,0,0); /* the connect button */
	if (vmb_debug_flag) vmb_debug_on(); else vmb_debug_off();
	if (vmb_verbose_flag) vmb_debug_mask=0; else vmb_debug_mask=VMB_DEBUG_DEFAULT;
	CheckMenuItem(hMenu,ID_DEBUG,MF_BYCOMMAND|(vmb_debug_flag?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu,ID_VERBOSE,MF_BYCOMMAND|(vmb_debug_mask==0?MF_CHECKED:MF_UNCHECKED));

	while (GetMessage(&msg, NULL, 0, 0)) 
      if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
	  { TranslateMessage(&msg);
	    DispatchMessage(&msg);
	  }
	vmb_disconnect(&vmb);
    set_pos_key(hMainWnd,defined);
    return (int)msg.wParam;
}



static void update_bits(void)
{
  InvalidateRect(hMainWnd,NULL,FALSE);
}
   

/* Interface to the virtual motherboard */


unsigned char *seg_get_payload(unsigned int offset,int size)
{ 
  return segmentbits+offset;
}

void seg_put_payload(unsigned int offset,int size, unsigned char *payload)
{ memmove(segmentbits+offset,payload,size);
  update_bits();
}

void seg_poweron(void)
{ memset(segmentbits,0xFF,8);
  update_bits();
}


void seg_poweroff(void)
{ memset(segmentbits,0x80,8);
  update_bits();
}

void seg_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{ memset(segmentbits,0x02,8);
  update_bits();
  PostMessage(hMainWnd,WM_VMB_DISCONNECT,0,0); /* the disconnect button */
}


void seg_reset(void)
{ memset(segmentbits,0xFF,8);
  update_bits();
}

void init_device(device_info *vmb)
{ 
  vmb_size = 8;

  vmb->poweron=seg_poweron;
  vmb->poweroff=seg_poweroff;
  vmb->disconnected=seg_disconnected;
  vmb->reset=seg_reset;
  vmb->terminate=vmb_terminate;
  vmb->put_payload=seg_put_payload;
  vmb->get_payload=seg_get_payload;
}
