/*
    Copyright 2011  Martin Ruckert
    
    ruckertm@acm.org

    This file is part of the Virtual Motherboard project

    This file is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this software; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifdef WIN32
#include <windows.h>
extern HWND hMainWnd;
#else
#include <unistd.h>
#endif
#include <string.h>
#include "vmb.h"
#include "bus-arith.h"
#include "param.h"
#include "option.h"
#include "timer.h"

/* contains operating system independent timer code */

#define TIMER_MEM	0x20
unsigned char tmem[TIMER_MEM] = {0};
/* memory layout:
tmem[0]	2 Byte int current year
tmem[1]
tmem[2] current month 0=January - 11=December
tmem[3] current day

tmem[4] (unix only) 1 if daylight savings time else 0
tmem[5] (unix only) 2 byte current day of the year
tmem[6] 
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

unsigned int tt=0, ti=0, t0=0, dt=0; /* copies of tmem fields */

/* functions to operate the timer simulation */
static unsigned int last_time=0;
static unsigned int time_delay=0;
static unsigned int expire_time=0; 
/* setting a timer will set this to the expected expire time */

static void debug_last_time(void)
{ char tstr[15];
  int t,h,m,s,ms; 
  if (!VMB_DEBUGGING(VMB_DEBUG_INFO)) return;
  ms = last_time%1000;
  t = last_time/1000;
  s = t%60;
  t=t/60;
  m= t%60;
  t= t/60;
  h = t%24;
  sprintf(tstr,"%02d:%02d:%02d:%03d",h,m,s,ms);
  vmb_debugs(VMB_DEBUG_INFO,"Time: %s",tstr);
}



/*problem: det timer ist etwas zu schnell dadurch ist t0 
  nach der erh�hung etwas nach timer_get_now
  damit geht dann hier gar nichts mehr t0 l�uft timer_get_now davon (weil
			 er aufholen will!)
*/

static void advance_time(unsigned int new_time)
{ if (((signed int)(new_time-last_time))>0) 
       /* should always happen except if last time is set by expire_time */
       /* we never step the clock backward */
    last_time=new_time;
  else
    time_delay = last_time-new_time; /* our time is past the host time */
}


void timer_start(void)
/* arrange the timer to signal at absolute time T0 + t0 +dt */
{ unsigned int d, ms;
  
  update_display();
  ms = timer_get_now(); /* might be too small by timer_delay */
  d = ms-t0; /* t0 is always in the past */
  if (d>=dt)
  { vmb_debugi(VMB_DEBUG_NOTIFY,"Timer signaled in the past: %u",d-dt);
    if (time_delay>0)
    { expire_time=ms+time_delay;
      timer_set(time_delay);
      vmb_debugi(VMB_DEBUG_NOTIFY,"Timer delayed %u",time_delay);
    }
    else
      timer_signal(); /* too late */
  }
  else
  { expire_time=t0+dt;
    timer_set(dt+time_delay-d);
    vmb_debugi(VMB_DEBUG_PROGRESS,"Timer started %u",dt-d);
  }
}

unsigned int timer_get_now(void)
/* return the simulated time in ms since T0 
   if the host time wraps around, make sure to call this function
   often enough (once per hour) to make sure that the host time
   since T0 and the time returned from this function last time
   dont differ by (wrap around)/2 or more.
   The values from this function are monoton increasing.
  
   If timer_signal() is called
   this time will advance at least to T0+delay from the last call
   of timer_set(delay).  If this happens to early (due to jitter),
   host time and time returned from this will differ still the
   clock will not step backwards by calling it again and setting it
   from host time.
*/
{  advance_time(timer_since_T0());
   debug_last_time();
   return last_time;
}


void timer_signal()
/* raise the timer interrupt after the timer has expired */
{ advance_time(expire_time);
  vmb_raise_interrupt(&vmb,interrupt);
  vmb_debugi(VMB_DEBUG_PROGRESS,"Timer expired (interrupt %X)",interrupt);
  debug_last_time();
  t0 = t0+dt;
  dt = ti;
  SETT0(t0);
  SETDT(dt);
  if (ti==0) 
  { tt = 0;
    SETTT(0);
	update_display();
  }
  else
    timer_start();
}




char version[]="$Revision: 1.3 $ $Date: 2011-03-17 23:54:53 $";

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
  { timer_get_DateTime();
    if (offset<0x08)
    { vmb_debugi(VMB_DEBUG_INFO,"year:        %d", YEAR);
      vmb_debugi(VMB_DEBUG_INFO,"month:       %d", MONTH);
      vmb_debugi(VMB_DEBUG_INFO,"day:         %d", DAY);
      vmb_debugi(VMB_DEBUG_INFO,"weekday:     %d", WEEKDAY);
    }
    if (offset<0x0C && offset+size>0x08)
    { vmb_debugi(VMB_DEBUG_INFO,"hour:        %d", HOUR);
      vmb_debugi(VMB_DEBUG_INFO,"minute:      %d", MIN);
      vmb_debugi(VMB_DEBUG_INFO,"second:      %d", SEC);
    }
    if (offset+size>0x0C)
      vmb_debugi(VMB_DEBUG_INFO,"millisecond: %d", MILLISEC);
  } 
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
	size = size-d;
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
    {  timer_stop();
       vmb_debug(VMB_DEBUG_PROGRESS,"stopped");
    }
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
#ifdef WIN32
  PostMessage(hMainWnd,WM_VMB_OFF,0,0);
#endif
}

void timer_poweron(void)
/* this function is called when the virtual power is turned off */
{ tt = ti = t0 = dt = 0;
  SETTT(tt);
  SETTI(ti);
  SETT0(t0);
  SETDT(dt);
#ifdef WIN32
  PostMessage(hMainWnd,WM_VMB_ON,0,0);
#endif
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
{ 
  vmb_debug(VMB_DEBUG_PROGRESS,"disconnected.");
  timer_reset();
#ifdef WIN32
  PostMessage(hMainWnd,WM_VMB_DISCONNECT,0,0);
#endif
}


void init_device(device_info *vmb)
{ tt = ti = t0 = dt = 0;
  memset(tmem,0,sizeof(tmem));
  timer_set_T0();
  last_time = timer_since_T0();
  time_delay = 0;
  timer_get_DateTime();
  vmb_size = TIMER_MEM;
  vmb->poweron=timer_poweron;
  vmb->poweroff=timer_poweroff;
  vmb->disconnected=timer_disconnected;
  vmb->reset=timer_reset;
  vmb->terminate=timer_terminate;
  vmb->put_payload=timer_put_payload;
  vmb->get_payload=timer_get_payload;
}

