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


extern char *label;
static int labelheight=0;
static int fontheight=0;
extern int vertical;
extern unsigned char led;
extern int nleds; /* number of leds to display */
extern int colors[];
extern char *pictures[];

HBITMAP ledOnB, ledOffB, ledOnW, ledOffW, ledDisconnected;
int ledwidth, ledheight;
int hcolors[8] = {IDC_COLOR0, IDC_COLOR1, IDC_COLOR2, IDC_COLOR3, 
                  IDC_COLOR4, IDC_COLOR5, IDC_COLOR6, IDC_COLOR7};
HBITMAP hOn[8] = {0};
HBITMAP hOff[8] = {0};

HBITMAP hpictures[8]={0};

static void init_metrics(void)
{ BITMAP bm;
  TEXTMETRIC tm;
  HDC hdc = CreateCompatibleDC(NULL);
  HGDIOBJ oldf, hf = GetStockObject(DEFAULT_GUI_FONT);
  oldf= SelectObject(hdc,hf);
  GetTextMetrics(hdc ,&tm);
  SelectObject(hdc,oldf);
  fontheight = tm.tmHeight;
  GetObject(ledOnB,sizeof(bm),&bm);
  ledwidth=bm.bmWidth;
  ledheight=bm.bmHeight;
}

unsigned char blend_color(unsigned char c, unsigned char b, unsigned char w, unsigned char p)
{  int n;
   n = (int)(b + (c *(w/255.0)*(p/255.0)));
   if (n > 255) return 255;
   else return (unsigned char)n;
}

#define bmRGB(r,g,b) ((b)|((g)<<8)|((r)<<16))
#define bmB(c) ((unsigned char)((c)&0xFF))
#define bmG(c) ((unsigned char)(((c)>>8)&0xFF))
#define bmR(c) ((unsigned char)(((c)>>16)&0xFF))

void load_pictures(void)
{ int i;
  for (i=0;i<8;i++)
    if (pictures[i]==NULL || pictures[0]==0)
        hpictures[i] =0;
    else
	{  hpictures[i]=LoadImage(NULL,pictures[i],IMAGE_BITMAP,32,32,LR_LOADFROMFILE|LR_CREATEDIBSECTION
); 
       if (hpictures[i]==0)
		   vmb_debugs(VMB_DEBUG_NOTIFY,"Unable to load picture %s",pictures[i]);
    }
}

static void set_color(BITMAP *bm32, BITMAP *bm32B, BITMAP *bm32W, int color, BITMAP *bm32P)
{ int i,n;
  unsigned char r, g, b;
  LONG *p, *pB, *pW, *pP, pPL;

  r = (color>>16)&0xFF;
  g = (color>>8)&0xFF;;
  b =  color&0xFF;;

  n = bm32->bmHeight*bm32->bmWidth;
  p = (LONG *)bm32->bmBits;
  pB = (LONG *)bm32B->bmBits;
  pW = (LONG *)bm32W->bmBits;
  if (bm32P==NULL)
  { pPL = 0xFFFFFFFF;
    pP = NULL;
  }
  else
  { pP = (LONG *)bm32P->bmBits;
  }
  for(i=0;i<n;i++)
  { int nr,ng,nb;
	if (pP!=NULL) pPL=*pP;
    nr = blend_color(r,bmR(*pB),bmR(*pW),bmR(pPL));
    ng = blend_color(g,bmG(*pB),bmG(*pW),bmG(pPL));
    nb = blend_color(b,bmB(*pB),bmB(*pW),bmB(pPL));
	*p = bmRGB(nr,ng,nb);
	p++; pB++; pW++; 
	if (pP!=NULL) pP++; 
  }
}

static void color_led(int i, int color)
{ BITMAP bm, bmB, bmW, bmP, *bmPP;
  if (hpictures[i]==NULL)
    bmPP=NULL;
  else
  { GetObject(hpictures[i],sizeof(bmP),&bmP);
    bmPP=&bmP;
  }
  GetObject(hOn[i], sizeof(bm), &bm);
  GetObject(ledOnB, sizeof(bmB), &bmB);
  GetObject(ledOnW, sizeof(bmW), &bmW);
  set_color(&bm,&bmB,&bmW,color,bmPP);
  GetObject(hOff[i], sizeof(bm), &bm);
  GetObject(ledOffB, sizeof(bmB), &bmB);
  GetObject(ledOffW, sizeof(bmW), &bmW);
  set_color(&bm,&bmB,&bmW,color,bmPP);
  colors[i]=color;
}

static void init_colors(void)
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
		 cc.rgbResult = RGB(bmR(colors[i]),bmG(colors[i]),bmB(colors[i]));
        cc.Flags = CC_FULLOPEN | CC_RGBINIT;
        if (ChooseColor(&cc)==TRUE) {
		    colors[i] = bmRGB(GetRValue(cc.rgbResult),
				GetGValue(cc.rgbResult),GetBValue(cc.rgbResult));
			color_led(i,colors[i]);
	        SendMessage(GetDlgItem(hDlg,hcolors[i]),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOn[i]);
		}
	  }


extern HBITMAP hOn[], hOff[];

void paint_leds(HDC dest, unsigned char led)
{ int i;
  HDC src = CreateCompatibleDC(NULL);
  HBITMAP h = (HBITMAP)SelectObject(src, hOn[0]);
  for (i=nleds-1;i>=0;i--) {
	if (!vmb.connected)
      SelectObject(src, ledOffW);
	else if (!vmb.power)
      SelectObject(src, ledOffB);
    else if (led & (1<<i))
      SelectObject(src, hOn[i]);
    else
      SelectObject(src, hOff[i]);
	if (vertical)
	  BitBlt(dest, 
	         0,(nleds-1-i)*ledheight, 
		     ledwidth, ledheight, src, 0, 0, SRCCOPY);
	else
	  BitBlt(dest, 
	         (nleds-1-i)*ledwidth, 0, 
		     ledwidth, ledheight, src, 0, 0, SRCCOPY);
  } 
  SelectObject(src, h);
  DeleteDC(src);
}

void paint_label(HDC dest, char *label)
{ if (label==NULL || label[0]==0)
    return;
  else
  {	int w,h;
    RECT rect;
	HGDIOBJ oldf, hf = GetStockObject(DEFAULT_GUI_FONT);
    oldf= SelectObject(dest,hf);
    SetTextColor(dest,RGB(0xff,0xff,0xff));
    SetBkColor(dest,RGB(0,0,0));
	SetTextAlign(dest,TA_CENTER|TA_TOP);
    if (vertical) w=ledwidth,h=ledheight*nleds;
	else          w=ledwidth*nleds,h=ledheight;
	rect.left=0;
	rect.right=w;
	rect.top=h;
	rect.bottom=h+labelheight;
	ExtTextOut(dest,w/2,h,
		       ETO_CLIPPED|ETO_OPAQUE,&rect,
		       label,(UINT)strlen(label),NULL);
	SelectObject(dest,oldf);
  }
}

void update_display(void)
{  InvalidateRect(hMainWnd,NULL,FALSE); 
}

static void resize_window(void)
{ if (label==NULL || label[0]==0) labelheight=0;
  else labelheight=fontheight;
  if (vertical)
    SetWindowPos(hMainWnd,HWND_TOP,0,0,ledwidth,ledheight*nleds+labelheight,
	SWP_NOMOVE|SWP_NOZORDER|SWP_SHOWWINDOW);
  else
    SetWindowPos(hMainWnd,HWND_TOP,0,0,ledwidth*nleds,ledheight+labelheight,
	SWP_NOMOVE|SWP_NOZORDER|SWP_SHOWWINDOW);
}

INT_PTR CALLBACK  
SettingsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{

  switch ( message )
  { case WM_INITDIALOG:
      uint64tohex(vmb_address,tmp_option);
      SetDlgItemText(hDlg,IDC_ADDRESS,tmp_option);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR0),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOn[0]);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR1),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOn[1]);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR2),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOn[2]);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR3),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOn[3]);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR4),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOn[4]);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR5),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOn[5]);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR6),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOn[6]);
	  SendMessage(GetDlgItem(hDlg,IDC_COLOR7),STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hOn[7]);
	  SetDlgItemInt(hDlg,IDC_NLEDS,nleds,FALSE);
	  SetDlgItemText(hDlg,IDC_LABEL,label?label:"");
	  SendMessage(GetDlgItem(hDlg,IDC_VERTICAL),BM_SETCHECK,
		  vertical?BST_CHECKED:BST_UNCHECKED,0);
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
        vertical = (BST_CHECKED == SendMessage(GetDlgItem(hDlg,IDC_VERTICAL),
			                        BM_GETCHECK,0,0));
        GetDlgItemText(hDlg,IDC_LABEL,tmp_option,MAXTMPOPTION);
		set_option(&label,tmp_option);
		resize_window();
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
      paint_leds(hdc,led);
	  paint_label(hdc,label);
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
	vmb_error_init_hook = win32_error_init;

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
	load_pictures();
	init_colors();
	init_metrics();
	SetWindowPos(hMainWnd,HWND_TOP,xpos,ypos,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
	resize_window();
	if (minimized)CloseWindow(hMainWnd); 
	UpdateWindow(hMainWnd);
	vmb_begin();
 	vmb_connect(&vmb,host,port);
	vmb_register(&vmb,HI32(vmb_address),LO32(vmb_address),vmb_size,0,0,defined);
    SendMessage(hMainWnd,WM_VMB_CONNECT,0,0); /* the connect button */
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

