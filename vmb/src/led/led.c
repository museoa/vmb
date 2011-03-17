/*
    Copyright 2005 Martin Ruckert
    
    ruckertm@acm.org

    This file is part of the MMIX Motherboard project

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

#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#include "resource.h"
#pragma warning(disable : 4996)
extern HWND hMainWnd;
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#include "bus-arith.h"
#include "option.h"
#include "param.h"
#include "vmb.h"

int nleds=8;
unsigned char led;
#ifndef RGB
#define RGB(r,g,b) ((r)|((g)<<8)|((b)<<16))
#endif
int colors[8] = {RGB(0x80,0x80,0xFF),RGB(0xFF,0x80,0x80),RGB(0,0xFF,0xFF),RGB(0xFF,0,0xFF),
                 RGB(0xFF,0xFF,0),RGB(0,0,0xFF),RGB(0,0xFF,0),RGB(0xFF,0,0)};

char version[]="$Revision: 1.4 $ $Date: 2011-03-17 23:54:53 $";

char howto[] =
"\n"
"The program will contact the motherboard at host:port\n"
"and register itself with the given satrt address.\n"
"Then, the program will display a line of LEDs reflecting single bits.\n"
"\n"
;


#ifdef WIN32

void display_led( unsigned char diff, unsigned char led)
{  InvalidateRect(hMainWnd,NULL,FALSE); 
}

#else
void display_led(unsigned char diff, unsigned char led)
{ int i;
  for (i=nleds-1;i>=0;i--)
    if (led & (1<<i))
      printf("ON  ");
    else
      printf("OFF ");
  printf("\n");
}
#endif


/* Interface to the virtual motherboard */
unsigned char *led_get_payload(unsigned int offset,int size)

{  vmb_debugi(VMB_DEBUG_INFO, "LED GET: %2X",led);
   return &led;
}

void led_put_payload(unsigned int offset,int size, unsigned char *payload)
{ unsigned char diff;
  vmb_debugi(VMB_DEBUG_INFO, "LED SET: %2X",payload[0]);
  diff = led ^ payload[0];
  led = payload[0];
  display_led(diff,led); 
}

void led_poweroff(void)
{ display_led(led,0);
  led=0;
  vmb_debug(VMB_DEBUG_INFO, "POWER OFF");
#ifdef WIN32
   PostMessage(hMainWnd,WM_VMB_OFF,0,0);
#endif
}

void led_poweron(void)
{ display_led(led,0);
  led=0;
  vmb_debug(VMB_DEBUG_INFO, "POWER ON");
#ifdef WIN32
   PostMessage(hMainWnd,WM_VMB_ON,0,0);
#endif
}

void led_reset(void)
{ display_led(led,0);
  led=0;
  vmb_debug(VMB_DEBUG_INFO, "RESET");
#ifdef WIN32
   PostMessage(hMainWnd,WM_VMB_RESET,0,0);
#endif
}



void init_device(device_info *vmb)
{  vmb_debugi(VMB_DEBUG_INFO, "address hi: %x",HI32(vmb_address));
   vmb_debugi(VMB_DEBUG_INFO, "address lo: %x",LO32(vmb_address));
   vmb_size = 1;
   vmb_debugi(VMB_DEBUG_INFO, "size: %d",vmb_size);
   close(0);
   vmb->poweron=led_poweron;
   vmb->poweroff=led_poweroff;
   vmb->disconnected=vmb_disconnected;
   vmb->reset=led_reset;
   vmb->terminate=vmb_terminate;
   vmb->get_payload=led_get_payload;
   vmb->put_payload=led_put_payload;

}
