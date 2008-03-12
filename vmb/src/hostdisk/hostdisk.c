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
#include <fcntl.h>

#include "vmb.h"
#include "bus-arith.h"
#include "option.h"
#include "param.h"

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
      uint64tohex(vmb_address,tmp_option);
      SetDlgItemText(hDlg,IDC_ADDRESS,tmp_option);
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
	    vmb_address = strtouint64(tmp_option);
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

char version[]="$Revision: 1.3 $ $Date: 2008-03-12 16:49:38 $";

char howto[] =
"\n"
"The program will contact the motherboard at host:port\n"
"and register itself with the given start address.\n"
"\n"
"The HOSTDISK device allows simple access to the file system of the host computer.\n"
"It is written to mimic the Unix system calls \n"
"open, close, read, write, lseek, and dup\n"
"as documented in section two of the unix manual pages. \n"
"The memory interface consists of\n" 
"a command register (write only),\n"
"a result register (read only), and \n"
"a parameter space of up to 64k Byte + 16 Byte.\n"
"Input parameters are written to the parameter space.\n"
"Numbers are stored as octabytes, even if often only the low 32 bits are used.\n"
"The order of parameters is rearanged such that non pointer parameters come first,\n"
"instead of pointer parameters, the data pointed to must be in the parameter space.\n"
"Output parameters can be read from the parameter space.\n"
"Again, instead of pointers, the data pointed to is found in the parameter space.\n"
"\n"
"Writing the command code \n"
"(open=1, close=2, read=3, write=4, lseek=5, and dup=6)\n"
"to the command register will trigger the corresponding command. \n"
"\n"
;

/* the registers */
#define HD_COMMAND (0*8)
#define HD_RESULT  (1*8)
#define HD_PARAM   (2*8)
#define HD_PARAM_SIZE (0x10000+16)
#define HD_SIZE    (HD_PARAM+HD_PARAM_SIZE)

#define HD_IGNORE 0
#define HD_OPEN   1
#define HD_CLOSE  2
#define HD_READ   3
#define HD_WRITE  4
#define HD_LSEEK  5
#define HD_DUP    6


static unsigned char mem[HD_SIZE] = {0};

#define disk_command (mem[HD_COMMAND+7])


static void execute_command(int cmd)
{ unsigned char *p;
 int result;
 int fd;
 vmb_debugi("Command %d received", cmd);
 p = mem+HD_PARAM;
 switch(cmd)
   { case HD_IGNORE :
       break;
   case HD_OPEN   :
     {
     char *pathname;
     int flags;
     mode_t mode;
     flags = chartoint(p+4);
     p = p+8;
     mode = chartoint(p+4);  
     p = p+8;
     pathname = (char *)p;
     result = open(pathname,flags, mode);
     inttochar(result,mem+HD_RESULT+4);
     }
     break;
   case HD_CLOSE  :
     { fd = chartoint(p+4);
     result = close(fd);
     inttochar(result,mem+HD_RESULT+4);
     }
     break;
   case HD_READ   :
     { size_t count;
     fd = chartoint(p+4); 
     p = p+8;
     count = chartoint(p+4);
     if (count > HD_PARAM_SIZE)
       vmb_errormsg("Parameter count of read too large");
     else   
       { result = read(fd, mem+HD_PARAM, count);
       inttochar(result,mem+HD_RESULT+4);
       }
     }
     break;
   case HD_WRITE  :
     { size_t count;
     fd = chartoint(p+4); 
     p = p+8;
     count = chartoint(p+4);
     p = p+8;
     if (count > HD_PARAM_SIZE-16)
       vmb_errormsg("Parameter count of write too large");
     else   
       { vmb_debugi("write to %d",fd);
         vmb_debugi("%d byte", count);
         result = write(fd, p, count);
         vmb_debugi("got result %d",result);
         inttochar(result,mem+HD_RESULT+4);
       }
     }
     break;
   case HD_LSEEK  :
     { off_t offset;
     int whence;
     fd = chartoint(p+4); 
     p = p+8;
     offset = chartoint(p+4);
     p = p+8;
     whence = chartoint(p+4);
     result = lseek(fd, offset, whence);
     inttochar(result,mem+HD_RESULT+4);
     }
     break;
   case HD_DUP    :
     { fd = chartoint(p+4);
     result = dup(fd);
     inttochar(result,mem+HD_RESULT+4);
     }
     break;
   default:
     vmb_errormsg("Undefined command.");
   }
}



unsigned char *get_payload(unsigned int offset,int size)
{   return mem+offset;
}

void put_payload(unsigned int offset,int size, unsigned char *payload)
{ memmove(mem+offset,payload,size);
 vmb_debugi("writing to %d",offset);
 vmb_debugi(" %d bytes", size);
  if (offset+size > HD_COMMAND && offset <  HD_COMMAND+8)
   execute_command(disk_command);
}

int reply_payload(unsigned char address[8], int size,unsigned char *payload)
{ return 1;
}



void init_device(void)
{  vmb_size = HD_SIZE;
   vmb_debugi("address hi: %x",vmb_address_hi);
   vmb_debugi("address lo: %x",vmb_address_lo);
   vmb_debugi("size: %d",vmb_size);
   close(0);
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
#endif
	return 0;
}

int process_poweroff(void)
{  
#ifdef WIN32
  SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hconnect);
#endif
  return 0;
}

int process_reset(void)
{ return 0;
}

