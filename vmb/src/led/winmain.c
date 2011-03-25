#include <winsock2.h>
#include <windows.h>
#include <commctrl.h>
#include <afxres.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "winopt.h"
#include "param.h"
#include "option.h"



extern unsigned char led;
extern int nleds; /* number of leds to display */
extern int colors[];

HBITMAP ledOnB, ledOffB, ledOnW, ledOffW, ledDisconnected;
int ledwidth, ledheight;
int hcolors[8] = {IDC_COLOR0, IDC_COLOR1, IDC_COLOR2, IDC_COLOR3, 
                  IDC_COLOR4, IDC_COLOR5, IDC_COLOR6, IDC_COLOR7};
HBITMAP hOn[8] = {0};
HBITMAP hOff[8] = {0};


unsigned char blend_color(unsigned char c, unsigned char b, unsigned char w)
{ 
   c = (unsigned char)(c *(w/255.0));
   if (b > c) c=b;
   return c;
}

#define bmRGB(r,g,b) ((b)|((g)<<8)|((r)<<16))
#define bmB(c) ((unsigned char)((c)&0xFF))
#define bmG(c) ((unsigned char)(((c)>>8)&0xFF))
#define bmR(c) ((unsigned char)(((c)>>16)&0xFF))


void set_color(BITMAP bm32, BITMAP bm32B, BITMAP bm32W, int color)
{ int i,n;
  unsigned char r, g, b;
  LONG *p, *pB, *pW;
  r = GetRValue(color);
  g = GetGValue(color);
  b = GetBValue(color);

  n = bm32.bmHeight*bm32.bmWidth;
  p = (LONG *)bm32.bmBits;
  pB = (LONG *)bm32B.bmBits;
  pW = (LONG *)bm32W.bmBits;
  for(i=0;i<n;i++)
  { int nr,ng,nb;
    nr = blend_color(r,bmR(*pB),bmR(*pW));
    ng = blend_color(g,bmG(*pB),bmG(*pW));
    nb = blend_color(b,bmB(*pB),bmB(*pW));
	*p = bmRGB(nr,ng,nb);
	p++; pB++; pW++;
  }
}

void color_led(int i, int color)
{ BITMAP bm, bmB, bmW;
  GetObject(hOn[i], sizeof(bm), &bm);
  GetObject(ledOnB, sizeof(bmB), &bmB);
  GetObject(ledOnW, sizeof(bmW), &bmW);
  set_color(bm,bmB,bmW,color);
  GetObject(hOff[i], sizeof(bm), &bm);
  GetObject(ledOffB, sizeof(bmB), &bmB);
  GetObject(ledOffW, sizeof(bmW), &bmW);
  set_color(bm,bmB,bmW,color);
  colors[i]=color;
}

void init_colors(void)
{ int i;
  for (i=0;i<8;i++)
  { 
    hOn[i] = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_ON_BLACK), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	hOff[i] = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_OFF_BLACK), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	color_led(i,colors[i]);
  }
}

void choose_color(HWND hDlg, int i)
	  { CHOOSECOLOR cc;                 // common dialog box structure 
        static COLORREF acrCustClr[16]; // array of custom colors 
	    // Initialize CHOOSECOLOR 
        ZeroMemory(&cc, sizeof(cc));
        cc.lStructSize = sizeof(cc);
        cc.hwndOwner = hDlg;
        cc.lpCustColors = (LPDWORD) acrCustClr;
		 cc.rgbResult = colors[i];
        cc.Flags = CC_FULLOPEN | CC_RGBINIT;
        if (ChooseColor(&cc)==TRUE) {
		    colors[i] = cc.rgbResult;
			color_led(i,cc.rgbResult);
	        SendMessage(GetDlgItem(hDlg,hcolors[i]),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOff[i]);
		}
	  }


extern HBITMAP hOn[], hOff[];

void paint_leds(HDC dest, HDC src, unsigned char led)
{ int i;
for (i=nleds-1;i>=0;i--) {
	if (!vmb.connected)
      SelectObject(src, ledOffW);
	else if (!vmb.power)
      SelectObject(src, ledOffB);
    else if (led & (1<<i))
      SelectObject(src, hOn[i]);
    else
      SelectObject(src, hOff[i]);
	BitBlt(dest, (nleds-1-i)*ledwidth, 0, ledwidth, ledheight, src, 0, 0, SRCCOPY);
  } 
}

void update_display(void)
{  InvalidateRect(hMainWnd,NULL,FALSE); 
}


INT_PTR CALLBACK  
SettingsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{

  switch ( message )
  { case WM_INITDIALOG:
      uint64tohex(vmb_address,tmp_option);
      SetDlgItemText(hDlg,IDC_ADDRESS,tmp_option);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR0),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOff[0]);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR1),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOff[1]);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR2),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOff[2]);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR3),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOff[3]);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR4),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOff[4]);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR5),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOff[5]);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR6),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOff[6]);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR7),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOff[7]);
	  SetDlgItemInt(hDlg,IDC_NLEDS,nleds,FALSE);
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
	    nleds = GetDlgItemInt(hDlg,IDC_NLEDS,NULL,FALSE);
	    if (nleds>8) nleds=8;
	    if (nleds<1) nleds=1;
		SetWindowPos(hMainWnd,HWND_TOP,0,0,ledwidth*nleds,ledheight,
			SWP_NOMOVE|SWP_NOZORDER|SWP_SHOWWINDOW);
      }	  
	  if (HIWORD(wparam) == STN_CLICKED)
	  { if (LOWORD(wparam)== IDC_CBUTTON0) choose_color(hDlg, 0);
        else if (LOWORD(wparam)== IDC_CBUTTON1) choose_color(hDlg, 1);
        else if (LOWORD(wparam)== IDC_CBUTTON2) choose_color(hDlg, 2);
        else if (LOWORD(wparam)== IDC_CBUTTON3) choose_color(hDlg, 3);
        else if (LOWORD(wparam)== IDC_CBUTTON4) choose_color(hDlg, 4);
        else if (LOWORD(wparam)== IDC_CBUTTON5) choose_color(hDlg, 5);
        else if (LOWORD(wparam)== IDC_CBUTTON6) choose_color(hDlg, 6);
        else if (LOWORD(wparam)== IDC_CBUTTON7) choose_color(hDlg, 7);
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
{  switch (message) 
  { case WM_VMB_ON: /* Power On */
	return 0;
    case WM_VMB_OFF: /* Power Off */
	return 0;
    case WM_VMB_RESET: /* Reset */
	return 0;
    case WM_VMB_CONNECT: /* Connected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Disconnect"))
	  DrawMenuBar(hMainWnd);
	  InvalidateRect(hWnd,NULL,FALSE);
 	return 0;
    case WM_VMB_DISCONNECT: /* Disconnected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Connect..."))
	  DrawMenuBar(hMainWnd);
	  InvalidateRect(hWnd,NULL,FALSE);
	return 0;
  case WM_CREATE: 
	  return 0;
  case WM_PAINT:
    { PAINTSTRUCT ps;
      HDC hdc = BeginPaint (hWnd, &ps);
      HDC memdc = CreateCompatibleDC(NULL);
      HBITMAP h = (HBITMAP)SelectObject(memdc, hOn[0]);
      paint_leds(hdc,memdc,led);
      SelectObject(memdc, h);
	  DeleteDC(memdc);
      EndPaint (hWnd, &ps);
    }
    return 0;
  }
  return (OptWndProc(hWnd, message, wParam, lParam));
}

HINSTANCE hInst;
HWND hMainWnd;
HBITMAP hBmp=NULL;
HMENU hMenu;
HBITMAP hon,hoff,hconnect;
device_info vmb = {0};




BOOL InitInstance(HINSTANCE hInstance)
{
  WNDCLASSEX wcex;
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
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName = szClassName;
	wcex.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
/*	wcex.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);
*/
	if (!RegisterClassEx(&wcex)) return FALSE;


    hMainWnd = CreateWindow(szClassName, szTitle ,WS_POPUP,
                            xpos, ypos,0, 0,
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
    vmb_message_hook = win32_message;
	vmb_debug_hook = win32_debug;

	hMenu = LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU));
	ledOnB = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_ON_BLACK), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	ledOffB= (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OFF_BLACK), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	ledOnW= (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_ON_WHITE), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	ledOffW= (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_OFF_WHITE), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	ledDisconnected= (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_DISCONNECT), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
    InitCommonControls();
	if (!InitInstance (hInstance)) return FALSE;
	
	param_init();
	if (nleds<1) nleds=1;
	if (nleds>8) nleds=8;
    get_pos_key(&xpos,&ypos,defined);
    init_device(&vmb);
    init_colors();
	{ BITMAP bm;
      GetObject(ledOnB,sizeof(bm),&bm);
	  ledwidth=bm.bmWidth;
	  ledheight=bm.bmHeight;
	  SetWindowPos(hMainWnd,HWND_TOP,xpos,ypos,ledwidth*nleds,ledheight,SWP_NOZORDER|SWP_SHOWWINDOW);
#if 0
	  HRGN hrg;
	  int i;

	  hrg = CreateEllipticRgn(0,0,ledwidth+1,ledheight+1);
      for(i=1;i<nleds;i++)
	  { HRGN rg;
	    rg = CreateEllipticRgn(ledwidth*i,0,ledwidth*(i+1)+1,ledheight+1);
	    CombineRgn(hrg, hrg, rg, RGN_OR);
	    DeleteObject(rg);
	  }	
	  SetWindowRgn(hMainWnd, hrg, TRUE);
#endif
	}
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
	  if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
	  { TranslateMessage(&msg);
	    DispatchMessage(&msg);
	  }
	vmb_disconnect(&vmb);
    set_pos_key(hMainWnd,defined);
	vmb_end();
	return (int)msg.wParam;
}

