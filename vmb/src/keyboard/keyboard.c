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
#include "win32main.h"
#else
#include <unistd.h>
#include <termios.h>
#endif
#include "message.h"
#include "bus-arith.h"
#include "bus-util.h"
#include "option.h"
#include "param.h"
#include "error.h"
#include "main.h"

void display_char(char c);

#ifdef WIN32

#define ID_CHILD 1
HWND hwndEdit; 
HBITMAP hBmpActive, hBmpInactive;
void InitControlls(HINSTANCE hInst,HWND hWnd)
{ hBmpInactive = hBmp;  
  hBmpActive = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_BITMAPACTIVE), 
                                IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
}


void PositionControlls(HWND hWnd,int width, int height)
{  
   power_led_position(230,1);
}

void process_focus(int on)
{ if (on) hBmp = hBmpActive;
  else hBmp = hBmpInactive;
  RedrawWindow(hMainWnd,NULL,NULL,RDW_INVALIDATE);
}

BOOL APIENTRY   
SettingsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{
  switch ( message )
  { case WM_INITDIALOG:
      SetDlgItemText(hDlg,IDC_ADDRESS,hexaddress);
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
        set_option(&hexaddress,tmp_option);
    hextochar(hexaddress,address,8);
        interrupt  = GetDlgItemInt(hDlg,IDC_INTERRUPT,NULL,FALSE);
      }
    if (wparam == IDOK || wparam == IDCANCEL)
      { EndDialog(hDlg, TRUE);
        return TRUE;
    }
    break;

  }
  return FALSE;
}



 void get_settings(void)
 {
   DialogBox(hInst,MAKEINTRESOURCE(IDD_SETTINGS),hMainWnd,SettingsDialogProc);
 }
 
#endif

char version[]="$Revision: 1.2 $ $Date: 2007-09-12 07:22:40 $";

char howto[] =
"\n"
"The program will contact the motherboard at [host:]port\n"
"and register itself with the given address and interrupt.\n"
"Then, the program will read bytes from standard input.\n"
"For each byte read it will signal an interrupt with the\n"
"given interrupt number.\n"
"At the given address it will provide a read only octabyte\n"
"in the following format:\n"
"\n"
"   XX00 00YY 0000 00ZZ\n"
"\n"
"The XX byte signals errors it will be 0x80 if an error occured\n"
"and 0x00 otherwise.\n"
"\n"
"The YY byte contains the number of characters taken from \n"
"the input since the last read operation.  This should be 0\n"
"if no new character was received and 1 if one charcter was\n"
"received. Any other value will indicate that character were\n"
"lost since the last read operation.\n"
"\n"
"The ZZ byte contains the last character received. It is valid only\n"
"if YYYY is not zero.\n"
"\n"
"The complete ocatbyte will be reset to zero after a read operation to ZZ.\n"
"\n"
;

static unsigned char data[8];
#define ERROR 0
#define COUNT 3
#define DATA  7


unsigned char *get_payload(unsigned int offset, int size)
{  
    static unsigned char payload[8];
    
    memmove(payload,data,8);
    // memset(data,0,8);
    
    if(offset +size>=8) /* reset to zero */
        memset(data,0,8);    
    
    return payload+offset;
}

int reply_payload(unsigned char address[8], int size,unsigned char *payload)
{ return 1;
}

void put_payload(unsigned int offset, int size, unsigned char *payload)
{
}

#ifdef WIN32
#else
static struct termios *tio=NULL;

static void prepare_input(void)
{ static struct termios oldtio;
  struct termios newtio;
  
  tcgetattr(0,&oldtio); /* save current port settings */
  tio = &oldtio;

  memset(&newtio, 0, sizeof(newtio));
  newtio.c_cflag = CS8 | CLOCAL | CREAD; /* input modes */
  newtio.c_iflag = IGNBRK;  /* control modes */
  newtio.c_oflag = 0;      /* output modes */

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;     /* local modes */
 
  /* control characters */
  newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */

  tcflush(0, TCIFLUSH);
  tcsetattr(0,TCSANOW,&newtio);
}
#endif

void init_device(void)
{  debugs("address: %s",hexaddress);
   debugi("interrupt: %d",interrupt);
   size = 8;
#ifdef WIN32
#else
   prepare_input();
#endif
}

void process_input(unsigned char c) 
{ /* The keyboard Interface */

  if (bus_power)

  { data[DATA] = c;
    if (data[COUNT]<0xFF) data[COUNT]++;
    if (data[COUNT]>1) data[ERROR] = 0x80;
    set_interrupt(bus_fd, interrupt);
    debug("Raised interrupt");

  }

  else

  {

#ifdef WIN32

   Beep(800,100);

#else

#endif

  }
}

int process_interrupt(unsigned char interrupt)
{ return 0;
}  

int process_poweron(void)
{ 
#ifdef WIN32
  SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hon);
  SendMessage(hwndEdit, WM_SETTEXT, 0, 
               (LPARAM) ""); 
#endif
  return 0;
}

int process_poweroff(void)
{  
#ifdef WIN32
  SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hconnect);
  SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM) howto); 
#endif
  return 0;
}

int process_reset(void)
{ return 0;
}

