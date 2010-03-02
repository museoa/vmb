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
static HBITMAP hblink,hold;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ static HDC BitmapDC ;
  static HBITMAP hold;
  switch (message) 
  {  
   case WM_VMB_OFF: /* Power Off */
	if (counter!=0) 
	{ KillTimer(hMainWnd,1); 
	}
	break;
  case WM_VMB_DISCONNECT: /* Disconnected */
	if (counter!=0) 
	{ KillTimer(hMainWnd,1);  
	}
	break;
  case WM_USER+5: /* Stop Timer */
    KillTimer(hMainWnd,1);
	SetWindowText(hTime,"0.000");
	vmb_debug(VMB_DEBUG_INFO,"Timer stoped");
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hon);
	return 0;
  case WM_USER+6: /* Start Timer */
	{ char timestr[20];
	  sprintf(timestr,"%6.3f",counter/1000.0);
	  SetWindowText(hTime,timestr);
	  SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hon);
	}
    SetTimer(hMainWnd,1,counter,NULL);
	vmb_debugi(VMB_DEBUG_INFO,"Timer started %d",counter);
	return 0;
  case WM_TIMER: /* Timer expired */
	  if (wParam == 1) /* the Real Timer */
	  { vmb_raise_interrupt(&vmb,interrupt);
	  	vmb_debugi(VMB_DEBUG_INFO,"Timer expired (interrupt %X)",interrupt);
	    hold = (HBITMAP)SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hblink);
#define BLINKTIME 100
		if (counter>2*BLINKTIME)
	      SetTimer(hMainWnd,2,BLINKTIME,NULL);	 
	  } 
	  else if (wParam == 2) /* the Blink Timer */
	  {  SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hold);
	     KillTimer(hMainWnd,2);
	  }
	  return 0;
  case WM_CREATE: 
	hpower = CreateWindow("STATIC",NULL,WS_CHILD|WS_VISIBLE|SS_BITMAP|SS_REALSIZEIMAGE,10,20,32,32,hWnd,NULL,hInst,0);
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hoff);

	hTime = CreateWindow( 
                "STATIC",     // predefined class 
                NULL,       // no window title 
                WS_CHILD | WS_VISIBLE | SS_RIGHT,
                50, 15, 200, 40, 
                hWnd,       // parent window 
                (HMENU)2,
                hInst, 
                NULL);                // pointer not needed 
    hTimeFont = CreateFont(40,0,0,0,0,FALSE,FALSE,FALSE,ANSI_CHARSET,
                    OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH|FF_MODERN,"Arial");
    SendMessage(hTime, WM_SETFONT, (WPARAM)hTimeFont, 0); 
    SetWindowText(hTime,"0.000");
	return 0;
  case WM_DESTROY:
	DeleteObject(hTimeFont);
    break;
  case WM_CTLCOLORSTATIC:
	 { HDC hDC = (HDC) wParam;   // handle to display context 
 	   SetBkColor(hDC,0);
	   SetTextColor(hDC,RGB(35,135,0));
	 }
	 return (LRESULT)GetStockObject(BLACK_BRUSH);
  }
 return (OptWndProc(hWnd, message, wParam, lParam));
}




char version[]="$Revision: 1.9 $ $Date: 2010-03-02 10:48:24 $";

char howto[] =
"\n"
"The program will contact the motherboard at [host:]port\n"
"and register itself with the given start address.\n"
"It will offer two octa byte at the given address.\n"
"Then, the program will answer read and write requests from the bus.\n"
"Writing a value n into the first tetra byte will start a timer that will\n"
"deliver an interrupt every n miliseconds.\n"
"Writing before the time has expired will cancel the next interrupt.\n"
"Writing a zero value will terminate the interrupts.\n"
"Reading this tetra will give the number of miliseconds between interrupts.\n"
"\n"
;

#define TIMER_MEM	16
static unsigned char tmem[TIMER_MEM] = {0};
/* memory layout:
tmem[0]  4 Byte int, timer interval = time in msec between interrupts
tmem[1]
tmem[2]
tmem[3]

tmem[4]	4Byte int current year
tmem[5]
tmem[6]
tmem[7]

tmem[8] 2 Byte int current month
tmem[9]
tmem[a] 1 Byte int current day
tmem[b] 1 Byte int current day of the week
tmem[c] 4 byte int time in ms since midnight.
tmem[d]
tmem[e]
tmem[f]
*/

/* Interface to the virtual motherboard */

unsigned char *timer_get_payload(unsigned int offset,int size)
{ SYSTEMTIME now;
  GetSystemTime(&now);
    inttochar(counter,tmem);
	inttochar(now.wYear,tmem+4);
    shorttochar(now.wMonth,tmem+8);
    tmem[0xa] = (unsigned char)now.wDay;
	tmem[0xb] = (unsigned char)now.wDayOfWeek;
	inttochar((((now.wHour*60)+now.wMinute)*60+now.wSecond)*1000+now.wMilliseconds, tmem+0xc);
	return tmem+offset;
}


void timer_put_payload(unsigned int offset,int size, unsigned char *payload)
{ if (!vmb.power)
  { vmb_debug(VMB_DEBUG_NOTIFY,"Power off, Write ignored.");
    return;
  }
  if (offset+size>4)  /* only the first 4 byte are writable */
	  size = 4-offset;
  if (size<=0) return;
	memmove(tmem+offset,payload, size);
  if (counter != 0)
    PostMessage(hMainWnd,WM_USER+5,0,0); /* Stop the timer */  
  counter = chartoint(tmem);
  if (counter != 0)
    PostMessage(hMainWnd,WM_USER+6,0,0); /* Start the timer */
}

void timer_poweroff(void)
/* this function is called when the virtual power is turned off */
{ if (counter!=0) PostMessage(hMainWnd,WM_USER+5,0,0); /* Stop the timer */
  counter = 0;
  PostMessage(hMainWnd,WM_VMB_OFF,0,0);
}

void timer_poweron(void)
/* this function is called when the virtual power is turned off */
{
   counter = 0;
   PostMessage(hMainWnd,WM_VMB_ON,0,0);
}

void timer_reset(void)
{ if (counter!=0) PostMessage(hMainWnd,WM_USER+5,0,0); /* Stop the timer */
  counter = 0;
}

void timer_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{ if (counter!=0) PostMessage(hMainWnd,WM_USER+5,0,0); /* Stop the timer */
  counter = 0;
  PostMessage(hMainWnd,WM_VMB_DISCONNECT,0,0);
}


void init_device(device_info *vmb)
{
  vmb_size = TIMER_MEM;
  vmb->poweron=timer_poweron;
  vmb->poweroff=timer_poweroff;
  vmb->disconnected=timer_disconnected;
  vmb->reset=timer_reset;
  vmb->terminate=vmb_terminate;
  vmb->put_payload=timer_put_payload;
  vmb->get_payload=timer_get_payload;
  hblink = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_BLINK), 
				IMAGE_BITMAP, 32, 32, LR_CREATEDIBSECTION);

}