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
void InitControlls(HINSTANCE hInst,HWND hWnd)
{   

        hwndEdit = CreateWindow( 
                "EDIT",     // predefined class 
                NULL,       // no window title 
                WS_CHILD | WS_VISIBLE | 
                    ES_LEFT | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL
					/* | WS_VSCROLL */, 
                0, 0, 0, 0, // set size in WM_SIZE message 
                hWnd,       // parent window 
                (HMENU) ID_CHILD, // edit control ID 
                hInst, 
                NULL);                // pointer not needed 
             SendMessage(hwndEdit, WM_SETFONT, (WPARAM) GetStockObject(ANSI_FIXED_FONT), 0); 
             // Add text to the window. 
             SendMessage(hwndEdit, WM_SETTEXT, 0, 
                (LPARAM) howto); 
}


void PositionControlls(HWND hWnd,int width, int height)
{    
  MoveWindow(hwndEdit,25,30,315,210,TRUE); 
  power_led_position(307, 240);
}

void process_focus(int on)

{ 

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

char version[]="$Revision: 1.1 $ $Date: 2007-08-29 09:19:36 $";

char howto[] =
"The program will contact the motherboard at [host:]port\r\n"
"and register itself with the given address and interrupt.\r\n"
"Then, the program will be ready to receive bytes and display them\r\n"
"by sending them to standard output.\r\n"
"Whenever ready to receive a byte, it will signal an interrupt with the\r\n"
"given interrupt number.\r\n"
"At the given address it will provide a read/write octabyte\r\n"
"in the following format:\r\n"
"\r\n"
"   XX00 00YY 0000 00ZZ\r\n"
"\r\n"
"The XX byte signals errors it will be 0x80 if an error occured\r\n"
"and 0x00 otherwise.\r\n"
"\r\n"
"The YY byte contains the number of characters that were written\r\n"
"to this address since the last output. This should be 0\r\n"
"if the address is ready to receive a character or 1 if the hardware\r\n"
"is bussy with writing a byte. Any other value will indicate that characters were\r\n"
"lost since the last write operation.\r\n"
"\r\n"
"The ZZ byte contains the last character received. It is valid only\r\n"
"if YY is not zero.\r\n"
"\r\n"
"The complete ocatbyte will be reset to zero after a byte is output.\r\n"
"\r\n"
;

static unsigned char data[8];
#define ERROR 0
#define COUNT 3
#define DATA  7

unsigned char *get_payload(unsigned int offset, int size)
{ return data+offset;
}

int reply_payload(unsigned char address[8], int size,unsigned char *payload)
{ return 1;
}

void put_payload(unsigned int offset, int size, unsigned char *payload)
{   
    unsigned char cpData[8]; //!< to fix payload smaller then 8
    int i;

    if( offset + size < 8 )
    {
	 for(i=0;i<7;i++) //!< cleanout memory (just in case)
	   cpData[i] = 0;
	 cpData[DATA] = payload[size-1-offset]; //!< copy the char on the right place
	 printf("%d,%d\n",offset,size);
	 payload = cpData;
    }
    if (data[COUNT]<0xFF) data[COUNT]++;
    else data[ERROR]=0x80;
    data[DATA] = payload[7-offset];
    debugi("(%02X)",data[DATA]);
    display_char(data[DATA]);
    memset(data,0,8);
    set_interrupt(bus_fd, interrupt);
}

void init_device(void)
{  debugs("address: %s",hexaddress);
   debugi("interrupt: %d",interrupt);
   size = 8;
#ifdef WIN32
#else
   setvbuf(stdout,NULL,_IONBF,0); /* make ouput unbuffered */
#endif
}

void display_char(char c)
{ 
#ifdef WIN32	
  char str[3];
  int n;

     n = SendMessage(hwndEdit,EM_GETLINECOUNT,0,0);

   if (n>100)

   { n = SendMessage(hwndEdit,EM_LINELENGTH,0,0);

     SendMessage(hwndEdit,EM_SETSEL,0,n+2);

     SendMessage(hwndEdit,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)"");

   }
  if (c=='\n')
  { str[0]='\r'; str[1]='\n'; str[2]=0; } 
  else
  { str[0]=c; str[1]=0; }
  n = SendMessage(hwndEdit, WM_GETTEXTLENGTH,0,0);
  SendMessage(hwndEdit, EM_SETSEL,n,n);
  SendMessage(hwndEdit, EM_REPLACESEL, 0,(LPARAM)str); 

  SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0); 
#else
    printf("%c",c);
#endif
} 

void process_input(unsigned char c) 
{ 
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
{
#ifdef WIN32
    SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM) ""); 
#endif
    return 0;
}

