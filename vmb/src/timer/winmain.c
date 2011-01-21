#include <winsock2.h>
#include <windows.h>
#include <afxres.h>
#include <time.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "winopt.h"
#include "param.h"
#include "option.h"
#pragma warning(disable : 4996)


#define TIMER_MEM	0x20
static unsigned char tmem[TIMER_MEM] = {0};
/* memory layout:
tmem[0]	2 Byte int current year
tmem[1]
tmem[2] current month 0=January - 11=December
tmem[3] current day

tmem[4] zero
tmem[5] (unix only) 1 if daylight savings time else 0
tmem[6] (unix only) current day of the year
tmem[7] current day of the week 0=Sunday - 6=Saturday

tmem[8] zero
tmem[9] current hour
tmem[A] current minute
tmem[B] current second

tmem[C]  4 Byte int, current total time since midnight in milliseconds
tmem[D]
tmem[E]
tmem[F]

tmem[10] 4 Byte int, offset
tmem[11]
tmem[12]
tmem[13]

tmem[14] 4 Byte int, interval
tmem[15]
tmem[16]
tmem[17]

tmem[18] 4 Byte int, t0
tmem[19]
tmem[1A]
tmem[1B]

tmem[1C] 4 Byte int, dt
tmem[1D]
tmem[1E]
tmem[1F]

see help.html
*/

#define YEAR     GET2(tmem+0x00)
#define MONTH    (tmem[0x02])
#define DAY      (tmem[0x03])
#define DST      (tmem[0x05])
#define YEARDAY  (tmem[0x06])
#define WEEKDAY  (tmem[0x07])
#define HOUR     (tmem[0x09])
#define MIN      (tmem[0x0A])
#define SEC      (tmem[0x0B])
#define MILLISEC GET4(tmem+0x0C)
#define TT       GET4(tmem+0x10)
#define TI       GET4(tmem+0x14)
#define TT0      GET4(tmem+0x18)
#define TDT      GET4(tmem+0x1C)

#define SETYEAR(x)     SET2(tmem,x)
#define SETMILLISEC(x) SET4(tmem+0x0C,x)
#define SETTT(x)       SET4(tmem+0x10,x)
#define SETTI(x)       SET4(tmem+0x14,x)
#define SETT0(x)       SET4(tmem+0x18,x)
#define SETDT(x)       SET4(tmem+0x1C,x)

/* variables to operate the timer */
static uint64_t T0;   /* global base time T0, filetime at midnight of system start */
static SYSTEMTIME now; /* current host time information */
static unsigned int tt=0, ti=0, t0=0, dt=0; /* copies of tmem fields */


/* functions to operate the timer simulation */
static unsigned int last_time=0;
static unsigned int time_delay=0;

void timer_set_T0(void)
{ SYSTEMTIME now, midnight;
  FILETIME fnow;
  ULARGE_INTEGER ulnow,ulmidnight;
  GetLocalTime(&now);
  SystemTimeToFileTime(&now, &fnow); 
  ulnow.HighPart = fnow.dwHighDateTime;
  ulnow.LowPart = fnow.dwLowDateTime;
  midnight=now;
  midnight.wHour=midnight.wMinute=midnight.wSecond=midnight.wMilliseconds=0;
  SystemTimeToFileTime(&midnight, &fnow); 
  ulmidnight.HighPart = fnow.dwHighDateTime;
  ulmidnight.LowPart = fnow.dwLowDateTime;
  T0 = ulmidnight.QuadPart;
  last_time = (unsigned int)((ulnow.QuadPart-T0)/10000);
  time_delay=0;
}

/*problem: det timer ist etwas zu schnell dadurch ist t0 nach der erhöhung etwas nach timer_get_now
			 damit geht dann hier gar nichts mehr t0 läuft timer_get_now davon (weil
			 er aufholen will!)
*/

static void advance_time(unsigned int new_time)
{	if (((signed int)(new_time-last_time))>0) 
       /* should always happen except if last time is set by expire_time */
       /* we never step the clock backward */
	   last_time=new_time;
    else
       time_delay = last_time-new_time; /* our time is past the host time */
}

unsigned int timer_get_now(void)
/* set now and return time in ms since T0 
   we make shure that the current host time and the time stored in now
   never differ by more than 1 hour, by calling this function once every hour.
*/
{   FILETIME fnow;
	ULARGE_INTEGER ul;
	unsigned int new_time;
    GetLocalTime(&now);
	SystemTimeToFileTime(&now, &fnow);  
	ul.HighPart = fnow.dwHighDateTime;
	ul.LowPart = fnow.dwLowDateTime;
	new_time = (unsigned int)((ul.QuadPart-T0)/10000);
	advance_time(new_time);
	return last_time;
}

void timer_get_DateTime(void) 
/* set the first two octas of the timer from host time */
{   unsigned int ms;
    ms = timer_get_now();
	SETYEAR(now.wYear);
	MONTH = (unsigned char)(now.wMonth-1);
	DAY = (unsigned char)now.wDay;
	WEEKDAY = (unsigned char)now.wDayOfWeek;
	HOUR = (unsigned char)now.wHour;
	MIN = (unsigned char)now.wMinute;
	SEC = (unsigned char)now.wSecond;
	SETMILLISEC(ms);
}

void timer_init(void)
{   timer_set_T0();
	timer_get_DateTime();
}

void timer_stop(void)
/* cancel a running timer */
{ PostMessage(hMainWnd,WM_USER+5,0,0); /* Stop the timer */  
}

void timer_start(void)
/* arrange the timer to signal at absolute time T0 + t0 +dt */
{
    PostMessage(hMainWnd,WM_USER+6,0,0); /* Start the timer */
}

void timer_signal()
/* raise the timer interrupt after the timer has expired */
{ vmb_raise_interrupt(&vmb,interrupt);
  vmb_debugi(VMB_DEBUG_INFO,"Timer expired (interrupt %X)",interrupt);
  if (ti==0) 
  { tt = 0;
    SETTT(0);
  }
  else
  { t0 = t0+dt;
    dt = ti;
    SETT0(t0);
	SETDT(dt);
    timer_start();
  }
}

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

static HWND hTime;
static HFONT hTimeFont;
static HBITMAP hblink,hold;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ static HDC BitmapDC ;
  static HBITMAP hold;
  static unsigned int expire_time=0; /* setting a timer set this to the expected expirte time */
  switch (message) 
  {  
   case WM_VMB_OFF: /* Power Off */
	if (tt!=0) 
	{ KillTimer(hMainWnd,1); 
	}
	break;
  case WM_VMB_DISCONNECT: /* Disconnected */
	if (tt!=0) 
	{ KillTimer(hMainWnd,1);  
	}
	break;
  case WM_USER+5: /* Stop Timer */
    KillTimer(hMainWnd,1);
	SetWindowText(hTime,"0.000");
	vmb_debug(VMB_DEBUG_INFO,"Timer stoped");
	return 0;
  case WM_USER+6: /* Start Timer */
	  { static char timestr[20] = {0};
	  unsigned int d,ms;
	  static unsigned int last_dt=0;
	  if (last_dt!=dt)
	  { sprintf(timestr,"%6.3f",dt/1000.0);
	    SetWindowText(hTime,timestr);
		last_dt=dt;
	  }
	  ms = timer_get_now(); /* might be too small by timer_delay */
	  d = ms-t0; /* t0 is always in the past */
	  if (d>=dt)
	  {	vmb_debugi(VMB_DEBUG_NOTIFY,"Timer signaled in the past t0: %u",t0);
	    vmb_debugi(VMB_DEBUG_NOTIFY,"Timer signaled in the past now: %u",ms);
		if (time_delay>0)
	    { expire_time=ms+time_delay;
	      SetTimer(hMainWnd,1,time_delay,NULL);
	  	  vmb_debugi(VMB_DEBUG_INFO,"Timer started %u",dt-d);
	    }
		else
		  timer_signal(); /* too late */
	  }
	  else
	  { expire_time=t0+dt;
	    SetTimer(hMainWnd,1,dt+time_delay-d,NULL);
	  	vmb_debugi(VMB_DEBUG_INFO,"Timer started %u",dt-d);
	  }
	}
	return 0;
  case WM_TIMER: /* Timer expired */
	  if (wParam == 1) /* the Real Timer */
	  { KillTimer(hMainWnd,1); 
	    advance_time(expire_time);
	    timer_signal();
		vmb_debugi(VMB_DEBUG_INFO,"Timer signaled at %u",timer_get_now());
#define BLINKTIME 200
		if (ti>2*BLINKTIME || ti==0)
		{	    
		  hold = (HBITMAP)SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hblink);
	      SetTimer(hMainWnd,2,BLINKTIME,NULL);	 
		}
	  } 
	  else if (wParam == 2) /* the Blink Timer */
	  {  SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hold);
	     KillTimer(hMainWnd,2); 
	  }
	  else if (wParam == 3) /* the Hour Timer */
         timer_get_DateTime();
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
	{ int hour = 60*60*1000;
	  if (hour > USER_TIMER_MAXIMUM)
		  hour = USER_TIMER_MAXIMUM;
	  SetTimer(hMainWnd,3,hour,NULL); /* every hour */
	}
	return 0;
  case WM_DESTROY:
	KillTimer(hMainWnd,1);
	KillTimer(hMainWnd,2);
	KillTimer(hMainWnd,3);
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




char version[]="$Revision: 1.12 $ $Date: 2011-01-21 08:41:52 $";

char howto[] =
"\n"
"The program will contact the motherboard at [host:]port\n"
"and register itself with the given start address.\n"
"It will offer four octa byte at the given address.\n"
"Then, the program will answer read and write requests from the bus.\n"
"The first two octabyte contain the host date and time\n"
"The next two octa implement a millisecond timer\n"
"\n"
;


/* command interface to the timer simulation */

/* Interface to the virtual motherboard */

unsigned char *timer_get_payload(unsigned int offset,int size)
{ if (offset<0x10)
    timer_get_DateTime();
  return tmem+offset;
}


void timer_put_payload(unsigned int offset,int size, unsigned char *payload)
{ 
  if (!vmb.power)
  { vmb_debug(VMB_DEBUG_NOTIFY,"Power off, Write ignored.");
	return;
  }
  if (offset<0x10)  /* the first 16 byte are read only */
  { int d = 0x10-offset;
	offset = offset+d;
	size = size+d;
  }
  if (size>0)
  { int to_tt=offset<0x14; /* write to offset */
	int to_ti=offset<0x18&& offset+size>0x14; 
	int to_t0= offset<0x1C&& offset+size>0x18; 
	int to_dt= offset+size>0x1C;
	memmove(tmem+offset,payload, size);

    if (to_ti) ti = TI;
    if (to_tt) tt = TT;
    if (to_tt && tt==0)
	   timer_stop();
    else
    { if (to_t0 || to_dt)
	  { /* takes precedence over writing to tt */
        t0 = TT0;
	    dt = TDT;
		tt = 1;
		SETTT(tt);
		timer_start();
	  } 
	  else if (to_tt && tt!=0)
	  { t0 = timer_get_now();;
	    dt = tt;
	    SETT0(t0);
	    SETDT(dt);
	    timer_start();
	  }
    }
  }
}

void timer_poweroff(void)
/* this function is called when the virtual power is turned off */
{ if (tt!=0) timer_stop();
  tt = 0; 
  SETTT(tt);
  PostMessage(hMainWnd,WM_VMB_OFF,0,0);
}

void timer_poweron(void)
/* this function is called when the virtual power is turned off */
{ tt = ti = t0 = dt = 0;
  SETTT(tt);
  SETTI(ti);
  SETT0(t0);
  SETDT(dt);
  PostMessage(hMainWnd,WM_VMB_ON,0,0);
}

void timer_reset(void)
{ if (tt!=0) timer_stop();
  tt = ti = t0 = dt = 0;
  SETTT(tt);
  SETTI(ti);
  SETT0(t0);
  SETDT(dt);
}

void timer_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{ timer_reset();
  PostMessage(hMainWnd,WM_VMB_DISCONNECT,0,0);
}


void init_device(device_info *vmb)
{ timer_init();
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