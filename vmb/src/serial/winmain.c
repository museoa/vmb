#include <winsock2.h>
#include <windows.h>
#include <afxres.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "winopt.h"
#include "param.h"
#include "option.h"


HBITMAP hBmpPinOn,hBmpPinOff;


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
extern void process_input_file(char *filename);

extern int pins, npins;

void display_pins(void)
{ RECT Rect;
  Rect.top=18;
  Rect.bottom=27;
  Rect.left=25;
  Rect.right=25+25*npins+9;
  InvalidateRect(hMainWnd,&Rect,FALSE);
}

void paint_pins(HDC memdc, HDC hdc)
{ int i;
  for (i=0;i<npins;i++)  
  { if ((pins>>i)&1)
	  SelectObject(memdc, hBmpPinOn);
    else
	  SelectObject(memdc, hBmpPinOff);
    BitBlt(hdc, 25+25*i, 18, 9, 9, memdc, 0, 0, SRCCOPY);
  }
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ switch (message) 
  {  case WM_CREATE: 
	hpower = CreateWindow("STATIC",NULL,WS_CHILD|WS_VISIBLE|SS_BITMAP|SS_REALSIZEIMAGE,145,80,32,32,hWnd,NULL,hInst,0);
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hoff);
	hBmpPinOn= (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_PIN_ON), 
				IMAGE_BITMAP, 9, 9, LR_CREATEDIBSECTION);
	hBmpPinOff= (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_PIN_OFF), 
				IMAGE_BITMAP, 9, 9, LR_CREATEDIBSECTION);

    return 0;
  case WM_PAINT:
    { PAINTSTRUCT ps;
      HDC hdc = BeginPaint (hWnd, &ps);
	  if (hBmp)
	  { HDC memdc = CreateCompatibleDC(NULL);
        HBITMAP h = (HBITMAP)SelectObject(memdc, hBmp);
        BITMAP bm;
        GetObject(hBmp, sizeof(bm), &bm);
        BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, memdc, 0, 0, SRCCOPY);

        paint_pins(memdc, hdc);

        SelectObject(memdc, h);
	    DeleteDC(memdc);
	  }
      EndPaint (hWnd, &ps);
    }
    return 0;
   case WM_DROPFILES:
	  { HDROP hDrop;
	    char filename[500];
	    hDrop = (HDROP)wParam;
		DragQueryFile(hDrop,0,filename,500);
	    process_input_file(filename);
		DragFinish(hDrop);
	  }
	  return 0;
    case WM_VMB_ON: /* Power On */
	  DragAcceptFiles(hWnd,TRUE);
	  	  if (filename!=NULL)
	    process_input_file(filename);
      break;
	case WM_VMB_RESET:
	  if (filename!=NULL)
	    process_input_file(filename);
      break;
   return 0;
    case WM_VMB_OFF: /* Power Off */
	  DragAcceptFiles(hWnd,FALSE);
	  break;
  }
  return (OptWndProc(hWnd, message, wParam, lParam));
}
