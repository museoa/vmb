/*
    Copyright 2005 Alexander Ukas, Martin Ruckert
    
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

#include <io.h>
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

#ifdef WIN32

static void open_file(void);

static void init_ram(void);


void InitControlls(HINSTANCE hInst,HWND hWnd)
{
}

void PositionControlls(HWND hWnd,int width, int height)
{  
   power_led_position(2,8);
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
      SetDlgItemInt(hDlg,IDC_RAMSIZE,size,FALSE);
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
        size = GetDlgItemInt(hDlg,IDC_RAMSIZE,NULL,FALSE);

		init_ram();

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

char version[]="$Revision: 1.2 $ $Date: 2007-09-11 15:53:09 $";

char howto[] =
"\n"
"The program will contact the motherboard at [host:]port\n"
"and register itself with the given start address.\n"
"Then, the program will answer read and write requests from the bus.\n"
"\n"
;

#define BITS 32

#define ROOTBITS 8
#define ROOTSIZE (1<<ROOTBITS)
#define ROOTMASK (ROOTSIZE-1)

#define MIDBITS 8
#define MIDSIZE (1<<MIDBITS)
#define MIDMASK (MIDSIZE-1)

#define PAGEBITS 16
#define PAGESIZE (1<<PAGEBITS)
#define PAGEMASK (PAGESIZE-1)


/* simulated ram as 0x100 * 0x100 * 0x10000 byte */
/* page level functions */
int ram_write_page(unsigned char *page[], int j, int offset,int size,unsigned char *payload)
/* write size byte from payload into the ith root entry at offset.
   return the number of byte written */
{ if (page[j]==NULL) {
    page[j] = calloc(PAGESIZE,sizeof(unsigned char));
    if (page[j]==NULL) {
      errormsg("Out of memory");
      return size;
    }
  }
  if (size+offset > PAGESIZE)
    size = PAGESIZE-offset;
  memmove(page[j]+offset,payload,size);
  return size;
}

int ram_read_page(unsigned char *page, int offset,int size,unsigned char *payload)
/* read size byte from the page at offset into payload.
   return the number of byte read */
{ if (size+offset > PAGESIZE)
    size = PAGESIZE-offset;
  if (page==NULL)
    memset(payload,0,size);
  else
    memmove(payload,page+offset,size);
  return size;
}



unsigned char **root[ROOTSIZE];


/* mid level functions */
int ram_read_mid(int i, int offset,int size,unsigned char *payload)
/* read size byte from the ith root entry at offset into payload
   return the number of byte read */
{ int j;
  j = (offset>>(BITS-ROOTBITS-MIDBITS))&MIDMASK;
  offset = offset & PAGEMASK;
  if (root[i]==NULL) 
    return ram_read_page(NULL,offset,size,payload);
  return ram_read_page(root[i][j],offset,size,payload);
}

int ram_write_mid(int i, int offset,int size,unsigned char *payload)
/* write size byte from payload into the ith root entry at offset.
   return the number of byte written */
{ int j;
  if (root[i]==NULL) {
    root[i] = calloc(MIDSIZE,sizeof(unsigned char *));
    if (root[i]==NULL) { 
      errormsg("Out of memory");
      return size;
    }
  }
  j = (offset>>(BITS-ROOTBITS-MIDBITS))&MIDMASK;
  offset = offset & PAGEMASK;
  return ram_write_page(root[i],j,offset,size,payload);
}



/* root level functions */

void ram_read(unsigned int offset,int size,unsigned char *payload)
/* read size byte from the ram at offset into payload */
{ int n;
  int i,k;
  i = (offset>>(BITS-ROOTBITS))&ROOTMASK;
  k = offset & ((MIDMASK<<PAGEBITS)|PAGEMASK);
  n = ram_read_mid(i,k,size,payload);
  if (n<size)
    ram_read_mid(i+1,0,size-n,payload+n);
}

void ram_write(unsigned int offset,int size,unsigned char *payload)
/* write size byte from payload at offset into the ram */
{ int n;
  int i,k;
  i = (offset>>(BITS-ROOTBITS))&ROOTMASK;
  k = offset &  ((MIDMASK<<PAGEBITS)|PAGEMASK);
  n = ram_write_mid(i,k,size,payload);
  if (n<size)
    ram_write_mid(i+1,0,size-n,payload+n);
}

void ram_clean(void)
{ int i,j;
  for (i=0;i<ROOTSIZE;i++)
    if (root[i]!=NULL)
    { for (j=0;j<MIDSIZE;j++)
        if (root[i][j]!=NULL) {
          free(root[i][j]);
          root[i][j]=NULL;
        }
      root[i]=NULL;
    }
}

void init_ram(void)
{ ram_clean();
}

void init_device(void){
	close(0);
	ram_clean();
}


unsigned char *get_payload(unsigned int offset,int size){
  static unsigned char payload[258*8];
  ram_read(offset,size,payload);
  return payload;
}

int reply_payload(unsigned char address[8], int size,unsigned char *payload)
{
 return 1;
}

void put_payload(unsigned int offset,int size, unsigned char *payload){
    ram_write(offset,size,payload);
}

int process_poweron(void)
{ 
#ifdef WIN32
	SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hon); 
#endif
 return 0;
}


int process_poweroff(void)
{ ram_clean();
#ifdef WIN32
  SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hconnect);
#endif
 return 0;
}


int process_reset(void)
{ 
  ram_clean();
  return 0;
}

int process_interrupt(unsigned char interrupt){
  return 0;
}


void process_input(unsigned char c) 
{ /* ignore input */
}

