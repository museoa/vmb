#include <winsock2.h>
#include <windows.h>
#include <afxres.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "winopt.h"
#include "param.h"
#include "option.h"
#pragma warning(disable : 4996)


static int counter;


INT_PTR CALLBACK   
SettingsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{
  switch ( message )
  { case WM_INITDIALOG:
      uint64tohex(vmb_address,tmp_option);
      SetDlgItemText(hDlg,IDC_ADDRESS,tmp_option);
	  SetDlgItemInt(hDlg,IDC_INTERRUPT,interrupt,FALSE);
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
		interrupt = GetDlgItemInt(hDlg,IDC_INTERRUPT,NULL,FALSE);
      }
      if (wparam == IDOK || wparam == IDCANCEL)
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
  }
  return FALSE;
}

static long int current_time(void)
/* current time in ms */
{ SYSTEMTIME now;
  GetSystemTime(&now);
  return (((now.wHour*60)+now.wMinute)*60+now.wSecond)*1000+now.wMilliseconds;
}

static long int timer_start_time;
static HWND hTime;
static HFONT hTimeFont;
static RECT timelineRect;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ static HDC BitmapDC ;
  static HBITMAP hold;
  switch (message) 
  {  
   case WM_USER+2: /* Power Off */
	if (counter!=0) 
	{ KillTimer(hMainWnd,1); counter = 0;}
   SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hoff);
	return 0;
  case WM_USER+4: /* Disconnected */
	if (counter!=0) 
	{ KillTimer(hMainWnd,1); counter = 0;}
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Connect..."))
	  DrawMenuBar(hMainWnd);
	   SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hconnect);
	return 0;
  case WM_USER+5: /* Stop Timer */
    KillTimer(hMainWnd,1);
	KillTimer(hMainWnd,2);
	counter = 0; 
    InvalidateRect(hMainWnd,&timelineRect,TRUE);
	SetWindowText(hTime,"0.000");
	return 0;
  case WM_USER+6: /* Start Timer */
	{ char timestr[20];
	  sprintf(timestr,"%6.3f",counter/1000.0);
	  SetWindowText(hTime,timestr);
	}
    SetTimer(hMainWnd,1,counter,NULL);
	timer_start_time = current_time();
#define MINUPDATE 50 	/* minimum time between updates */
#define NUMPIXELS 200 /* number of pixels in the timeline */
	{ int updates, update_interval;
	  updates = counter/MINUPDATE; /* number of updates rounded down */
	  if (updates <1) updates = 1;
	  if (updates > NUMPIXELS) 
		  updates = NUMPIXELS; /* we do not need more updates then pixels */
	  update_interval = (counter-1)/updates; /*rounded down */
	  SetTimer(hMainWnd,2,update_interval,NULL);
      InvalidateRect(hMainWnd,&timelineRect,TRUE);
	}
	return 0;
  case WM_TIMER: /* Timer expired */
	  if (wParam == 1) /* the Real Timer */
	  { vmb_raise_interrupt(interrupt);
		timer_start_time = current_time();
	  }
	  InvalidateRect(hMainWnd,&timelineRect,TRUE);
	  return 0;
  case WM_CREATE: 
	hpower = CreateWindow("STATIC",NULL,WS_CHILD|WS_VISIBLE|SS_BITMAP|SS_REALSIZEIMAGE,10,10,32,32,hWnd,NULL,hInst,0);
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hoff);
	hTime = CreateWindow( 
                "STATIC",     // predefined class 
                NULL,       // no window title 
                WS_CHILD | WS_VISIBLE | SS_RIGHT,
                45, 25, 200, 40, 
                hWnd,       // parent window 
                (HMENU)2,
                hInst, 
                NULL);                // pointer not needed 
    hTimeFont = CreateFont(40,0,0,0,0,FALSE,FALSE,FALSE,ANSI_CHARSET,
                    OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH|FF_MODERN,"Arial");
    SendMessage(hTime, WM_SETFONT, (WPARAM)hTimeFont, 0); 
    SetWindowText(hTime,"0.000");
		timelineRect.top=70-1;
		timelineRect.left=45-1; 
		timelineRect.bottom=70+3;
		timelineRect.right=45+200; 
 
    return 0;
  case WM_DESTROY:
	DeleteObject(hTimeFont);
	SelectObject(BitmapDC, hold);
	DeleteDC(BitmapDC);

    PostQuitMessage(0);
    return 0;
  case WM_CTLCOLORSTATIC:
	 { HDC hDC = (HDC) wParam;   // handle to display context 
 	   SetBkColor(hDC,0);
	   SetTextColor(hDC,RGB(35,135,0));
	 }
	 return (LRESULT)GetStockObject(BLACK_BRUSH);
  case WM_PAINT:
    { PAINTSTRUCT ps;
      HDC hdc;
      hdc = BeginPaint (hWnd, &ps);
	  if (BitmapDC==NULL)
	  {	    BitmapDC = CreateCompatibleDC(NULL);
            hold = (HBITMAP)SelectObject(BitmapDC, hBmp);
	  }
	  BitBlt(hdc,ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right-ps.rcPaint.left,ps.rcPaint.bottom-ps.rcPaint.top, 
			BitmapDC, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
      if (counter > 0) 
	  {
        HPEN hpen, hpenOld;
		long int delta;
		int linelength;

        hpen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
        hpenOld = SelectObject(hdc, hpen);
	    MoveToEx(hdc, 45, 70, (LPPOINT) NULL); 
	    delta = current_time() - timer_start_time;
        linelength = (200*(counter -delta))/counter;
        if (linelength>0) LineTo(hdc, 45 + linelength, 70); 
        SelectObject(hdc, hpenOld);
        DeleteObject(hpen);
      }
      EndPaint (hWnd, &ps);
    }
    return 0;

  }
 return (OptWndProc(hWnd, message, wParam, lParam));
}


void init_device(void)
{
	vmb_size = 8;
}

char version[]="$Revision: 1.8 $ $Date: 2008-09-26 08:58:55 $";

char howto[] =
"\n"
"The program will contact the motherboard at [host:]port\n"
"and register itself with the given start address.\n"
"It will offer one octa byte at the given address.\n"
"Of the eight byte only the low order four byte are used.\n"
"Then, the program will answer read and write requests from the bus.\n"
"Writing a value n into the given tetra byte will start a timer that will\n"
"deliver an interrupt every n miliseconds.\n"
"Writing before the time has expired will cancel the next interrupt.\n"
"Writing a zero value will terminate the interrupts.\n"
"Reading will give the number of miliseconds between interrupts.\n"
"\n"
;

static unsigned char counter_bytes[8] = {0};


/* Interface to the virtual motherboard */

unsigned char *vmb_get_payload(unsigned int offset,int size)
{
    inttochar(counter,counter_bytes+4);
	return counter_bytes+offset;
}


void vmb_put_payload(unsigned int offset,int size, unsigned char *payload)
{ if (counter != 0)
    SendMessage(hMainWnd,WM_USER+5,0,0); /* Stop the timer */
  if (offset+size<= 8)
	memmove(counter_bytes+offset,payload, size);
  counter = chartoint(counter_bytes+4);
  if (counter != 0)
    SendMessage(hMainWnd,WM_USER+6,0,0); /* Start the timer */
}



void vmb_reset(void)
{ if (counter!=0) SendMessage(hMainWnd,WM_USER+5,0,0); /* Stop the timer */
  counter = 0;
}

