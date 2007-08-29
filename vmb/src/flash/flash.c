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
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include "message.h"
#include "bus-arith.h"
#include "bus-util.h"
#include "option.h"
#include "param.h"
#include "error.h"
#include "main.h"

#ifdef WIN32

static void open_file(void);

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
      SetDlgItemText(hDlg,IDC_FILE,filename);
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
        GetDlgItemText(hDlg,IDC_FILE,tmp_option,MAXTMPOPTION);
	    set_option(&filename,tmp_option);
		open_file();
      }
	  else if (HIWORD(wparam) == BN_CLICKED  && LOWORD(wparam) == IDC_BROWSE) 
	  { OPENFILENAME ofn;       /* common dialog box structure */
         /* Initialize OPENFILENAME */
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hMainWnd;
        ofn.lpstrFile = tmp_option;
        ofn.nMaxFile = MAXTMPOPTION;
        ofn.lpstrFilter = "All\0*.*\0Rom\0*.rom\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        /* Display the Open dialog box. */
        if (GetOpenFileName(&ofn)==TRUE) 
		   SetDlgItemText(hDlg,IDC_FILE,tmp_option);
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

char version[]="$Revision: 1.1 $ $Date: 2007-08-29 09:19:34 $";

char howto[] =
"\n"
"The program will contact the motherboard at host:port\n"
"and register itself with the given satrt address.\n"
"Then, the program will read the file and answer\n"
"read and write requests from the bus.\n"
"\n"
;

static unsigned char *flash=NULL;

#define PAGESIZE (1<<10) /* one kbyte */

static void open_file(void)
{ FILE *f;
  struct stat fs;
  if (filename==NULL)
    fatal_error(__LINE__,"No filename");
  f = fopen(filename,"rb");
  if (f==NULL) fatal_error(__LINE__,"Unable to open file");
  if (fstat(fileno(f),&fs)<0) fatal_error(__LINE__,"Unable to get file size");
  size = ((fs.st_size+7)/8)*8; /* make it a multiple of octas */
  if (flash!=NULL) free(flash);
  flash = malloc(size);
  if (flash==NULL) fatal_error(__LINE__,"Out of memory");
  size=fread(flash,1,size,f);
  if (size<0) fatal_error(__LINE__,"Unable to read file");
  if (size==0) fatal_error(__LINE__,"Empty file");
  size = (size+PAGESIZE-1)&~(PAGESIZE-1); /*round up to whole pages*/
  fclose(f);
}


static void write_file(void)
{ FILE *f;
  int i;
  if (filename==NULL)
    errormsg("No filename");
  if (flash==NULL)
    errormsg("No flash memory");
  f = fopen(filename,"wb");
  if (f==NULL) errormsg("Unable to open file");
  i = fwrite(flash,1,size,f);
  if (i<size) errormsg("Unable to write file");
  fclose(f);
}

unsigned char *get_payload(unsigned int offset,int size)
{ return flash+offset;
}

int reply_payload(unsigned char address[8], int size,unsigned char *payload)
{ return 1;
}

void put_payload(unsigned int offset,int size, unsigned char *payload)
{    memmove(flash+offset,payload,size);
}


void init_device(void)
{  debugs("address: %s",hexaddress);
   close(0);
}

void process_input(unsigned char c) 
{ 
}

int process_interrupt(unsigned char interrupt)
{ return 0;
}	

int process_poweron(void)
{ open_file();
  debugi("size: %d",size);
#ifdef WIN32
	SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hon); 
#endif
  return 0;
}

int process_poweroff(void)
{  write_file();
#ifdef WIN32
  SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hconnect);
#endif
  return 0;
}

int process_reset(void)
{ write_file();
  return 0;
}

