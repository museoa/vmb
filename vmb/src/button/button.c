#include <winsock2.h>
#include <windows.h>
#include <winuser.h>
#include <afxres.h>
#include "vmb.h"
#include "resource.h"
#include "winopt.h"
#include "param.h"
#include "option.h"

HBITMAP hOn, hOff;
int color;
int upinterrupt;
int pushbutton;
int pushstate;
int enable_interrupts;


unsigned char blend_color(int c, int w)
{  int n;
   w = (w-0x7F) * 2 ;
    n = c+w;
	if (n>0xFF) n = 0xFF;
	if (n<0) n=0;
	return n;
}


void color_bitmap(HBITMAP hBmp, int color )
{ BITMAP bm32;
  int i,n;
  unsigned char r, g, b;
  LONG *p;
  r = (color>>16)&0xFF;
  g = (color>>8)&0xFF;;
  b =  color&0xFF;;

  GetObject(hBmp, sizeof(bm32), &bm32);
  if (bm32.bmBitsPixel!=32)
    return;
  while (bm32.bmWidthBytes % 4)
	bm32.bmWidthBytes++;
  n = bm32.bmHeight*bm32.bmWidth;
  p = (LONG *)bm32.bmBits;
  for(i=0;i<n;i++)
  { int nr,ng,nb;
    nr = blend_color(r,(*p>>16)&0xFF);
    ng = blend_color(g,(*p>>8)&0xFF);
    nb = blend_color(b,(*p)&0xFF);
	*p = (nr<<16)|(ng<<8)|nb;
	p++;
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
	  SetDlgItemInt(hDlg,IDC_UPINTERRUPT,upinterrupt,FALSE);
	  SetDlgItemInt(hDlg,IDC_COLOR,color,FALSE);
	  CheckDlgButton(hDlg,IDC_INTENABLE,enable_interrupts&1?BST_CHECKED:BST_UNCHECKED);
	  CheckDlgButton(hDlg,IDC_UPINTENABLE,enable_interrupts&2?BST_CHECKED:BST_UNCHECKED);
	  CheckDlgButton(hDlg,IDC_PUSHBUTTON,pushbutton?BST_CHECKED:BST_UNCHECKED);
      return TRUE;
   case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
      { int new_color;
		GetDlgItemText(hDlg,IDC_ADDRESS,tmp_option,MAXTMPOPTION);
        vmb_address = strtouint64(tmp_option);
		interrupt =GetDlgItemInt(hDlg,IDC_INTERRUPT,NULL,FALSE);
		upinterrupt =GetDlgItemInt(hDlg,IDC_UPINTERRUPT,NULL,FALSE);
		new_color =GetDlgItemInt(hDlg,IDC_COLOR,NULL,FALSE);
		if (new_color!=color)
		{ color=new_color;
		  DeleteObject(hOff);
		  hOff = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_BITMAPOFF), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
 		  DeleteObject(hOn);
          hOn = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_BITMAPON), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
		  color_bitmap(hOn,color);
	      color_bitmap(hOff,color);
		  if (pushstate) hBmp=hOn; else hBmp=hOff;
		  InvalidateRect(hMainWnd,NULL,FALSE);	
		}
		if (IsDlgButtonChecked(hDlg,IDC_INTENABLE)==BST_CHECKED)
			enable_interrupts|=1;
		else
			enable_interrupts&=~1;
		if (IsDlgButtonChecked(hDlg,IDC_UPINTENABLE)==BST_CHECKED)
			enable_interrupts|=2;
		else
			enable_interrupts&=~2;
		pushbutton=IsDlgButtonChecked(hDlg,IDC_PUSHBUTTON)==BST_CHECKED;
      }
	  if (HIWORD(wparam) == STN_CLICKED && LOWORD(wparam)== IDC_COLORCHOOSE)
	  { CHOOSECOLOR cc;                 // common dialog box structure 
        static COLORREF acrCustClr[16]; // array of custom colors 

          
	    // Initialize CHOOSECOLOR 
        ZeroMemory(&cc, sizeof(cc));
        cc.lStructSize = sizeof(cc);
        cc.hwndOwner = hDlg;
        cc.lpCustColors = (LPDWORD) acrCustClr;
		{int c;
		 c = GetDlgItemInt(hDlg,IDC_COLOR,NULL,FALSE);
		 cc.rgbResult = RGB((c>>16)&0xFF,(c>>8)&0xFF,c&0xFF);
		}
        cc.Flags = CC_FULLOPEN | CC_RGBINIT;
        if (ChooseColor(&cc)==TRUE) {
			int c;
			c = GetBValue(cc.rgbResult);
			c |=GetGValue(cc.rgbResult)<<8;
			c |=GetRValue(cc.rgbResult)<<16;
		   SetDlgItemInt(hDlg,IDC_COLOR,c,FALSE);
		}
	  }
    if (wparam == IDOK || wparam == IDCANCEL)
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
  }
  return FALSE;
}

static int top, left;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ 
  switch (message) 
  {  
 case WM_VMB_ON: /* Power On */
 	return 0;
  case WM_VMB_OFF: /* Power Off */
	return 0;
  case WM_VMB_CONNECT: /* Connected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Disconnect"))
	  DrawMenuBar(hMainWnd);
 	return 0;
  case WM_VMB_DISCONNECT: /* Disconnected */
	if (ModifyMenu(hMenu,ID_CONNECT, MF_BYCOMMAND|MF_STRING,ID_CONNECT,"Connect..."))
	  DrawMenuBar(hMainWnd);
	return 0;
  case WM_CREATE: 
	hpower = NULL;
	hOff = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_BITMAPOFF), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	hOn = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_BITMAPON), 
		                            IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	
    return 0;
 
  case WM_NCLBUTTONDOWN:
  case WM_NCLBUTTONDBLCLK:
  {   int dx, dy;
	  WINDOWPLACEMENT wndpl;
      wndpl.length=sizeof(WINDOWPLACEMENT);
      if(GetWindowPlacement(hWnd,&wndpl))
	  { xpos = wndpl.rcNormalPosition.left;   // horizontal position 
        ypos = wndpl.rcNormalPosition.top;   // vertical position 
	  }
	  dx = xpos + 32 - LOWORD(lParam); 
      dy = ypos +32 - HIWORD(lParam);
      if (dx*dx+dy*dy>25*25) break;
	  if (pushbutton && pushstate)
	  { hBmp=hOff;
	    pushstate = 0;
	    if (enable_interrupts&2)
    	  vmb_raise_interrupt(&vmb, upinterrupt);
	  }
	  else
	  { hBmp=hOn;
	    pushstate = 1;
	    if (enable_interrupts&1)
    	  vmb_raise_interrupt(&vmb, interrupt);
	  }
	  InvalidateRect(hWnd,NULL,FALSE);
	  return 0;
  }
  case WM_WINDOWPOSCHANGED:
	  top = ((WINDOWPOS *)lParam)->y;
	  left = ((WINDOWPOS *)lParam)->x;
	  break;
  case WM_NCLBUTTONUP:
  {   int dx, dy;
	  dx = left + 32 - LOWORD(lParam); 
      dy = top +32 - HIWORD(lParam);
      if (dx*dx+dy*dy>20*20) break;
	  if (pushbutton) return 0;
	  hBmp=hOff;
	  pushstate = 0;
	  InvalidateRect(hWnd,NULL,FALSE);
	  if (enable_interrupts&2)
    	vmb_raise_interrupt(&vmb, upinterrupt);
	  return 0;
  }
  }
 return (OptWndProc(hWnd, message, wParam, lParam));
}


void init_device(device_info *vmb)
{ BITMAP bm;
  hBmp = hOff;
  pushstate = 0;
  color_bitmap(hOn,color);
  color_bitmap(hOff,color);
  GetObject(hBmp, sizeof(bm), &bm);
  SetWindowPos(hMainWnd,HWND_TOP,0,0,bm.bmWidth, bm.bmHeight,
		  SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_SHOWWINDOW);
  vmb->poweron=vmb_poweron;
  vmb->poweroff=vmb_poweroff;
  vmb->disconnected=vmb_disconnected;
  vmb->terminate=vmb_terminate;
}
