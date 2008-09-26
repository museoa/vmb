#include <winsock2.h>
#include <windows.h>
#include <afxres.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "winopt.h"
#include "param.h"
#include "option.h"






INT_PTR CALLBACK   
SettingsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{
  switch ( message )
  { case WM_INITDIALOG:
      uint64tohex(vmb_address,tmp_option);
      SetDlgItemText(hDlg,IDC_ADDRESS,tmp_option);
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
      }
      if (wparam == IDOK || wparam == IDCANCEL)
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
  }
  return FALSE;
}


static HWND hBits[64];
static HBITMAP hhor,hvert,hdot;
static int xpixelpos[8] = {39,25,11,  20, 5, 80,65,  82};
static int ypixelpos[8] = {5,53,101,   10,59,  12,61, 101};
#define DIGITLENGTH 100
#define WINHEIGHT 111
#define WINLENGTH (DIGITLENGTH*8)
static enum {vert, hor, dot} bittyp[8] = {hor,hor,hor,vert,vert,vert,vert,dot};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ switch (message) 
  {  
  case WM_USER+1: /* on*/
	  return 0;
  case WM_USER+2: /* off */
	  return 0;
  case WM_USER+3: /* Connected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Disconnect"))
	  DrawMenuBar(hMainWnd);
 	return 0;
  case WM_USER+4: /* Disconnected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Connect..."))
	  DrawMenuBar(hMainWnd);
	return 0;
  case WM_CREATE:
      /* create main dialog */ 
      { int i,k;
	    for (k=0;k<8;k++)
     	  for (i=0;i<8;i++)
		  {	hBits[i+k*8] = CreateWindow("STATIC",NULL,WS_CHILD|WS_VISIBLE|SS_BITMAP|SS_REALSIZEIMAGE,
		                    xpixelpos[i]+k*DIGITLENGTH,ypixelpos[i],0,0,hWnd,NULL,hInst,0);
            if (bittyp[i] ==vert)
		      SendMessage(hBits[i+k*8],STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hvert);
            else if (bittyp[i] == hor)
		      SendMessage(hBits[i+k*8],STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hhor);
            else if (bittyp[i] ==dot)
		      SendMessage(hBits[i+k*8],STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hdot);
			ShowWindow(hBits[i+k*8],SW_SHOW);
		  }
      }	
    return 0; 
  }
 return (OptWndProc(hWnd, message, wParam, lParam));
}



void init_device(void)
{
	hvert = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_VERT), 
				IMAGE_BITMAP, 0, 0, 0);
    hhor = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_HOR), 
				IMAGE_BITMAP, 0, 0, 0);
    hdot = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_DOT), 
				IMAGE_BITMAP, 0, 0, 0);
	vmb_size = 8;

}



static unsigned char segmentbits[8];

static void update_bits(void)
{ int i, k;
  unsigned char c;
  for (k=0;k<8;k++)
  { c = segmentbits[k];
    for (i=0;i<8;i++)
    {  if (c&0x1) 
         ShowWindow(hBits[i+8*k],SW_SHOW); 
        else  
	      ShowWindow(hBits[i+8*k],SW_HIDE);
        c = c>>1;
    }
  }
  UpdateWindow(hMainWnd);
}
   

/* Interface to the virtual motherboard */


unsigned char *vmb_get_payload(unsigned int offset,int size)
{ 
  return segmentbits+offset;
}

void vmb_put_payload(unsigned int offset,int size, unsigned char *payload)
{ memmove(segmentbits+offset,payload,size);
  update_bits();
}

void vmb_poweron(void)
{ memset(segmentbits,0xFF,8);
  update_bits();
}


void vmb_poweroff(void)
{ memset(segmentbits,0x80,8);
  update_bits();
}

void vmb_disconnected(void)
/* this function is called when the reading thread disconnects from the virtual bus. */
{ memset(segmentbits,0x02,8);
  update_bits();
  SendMessage(hMainWnd,WM_USER+4,0,0); /* the disconnect button */
}


void vmb_terminate(void)
/* this function is called when the motherboard politely asks the device to terminate.*/
{ 
   PostMessage(hMainWnd,WM_QUIT,0,0);
}


void vmb_reset(void)
{ memset(segmentbits,0xFF,8);
  update_bits();
}
