#include <winsock2.h>
#include <windows.h>
#include <afxres.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "winopt.h"
#include "param.h"
#include "option.h"


HBITMAP hBmpActive, hBmpInactive;


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

extern void process_input(unsigned char c);
extern void process_input_file(char *filename);


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ switch (message) 
  {  
  case WM_SETFOCUS:
    vmb_debug(VMB_DEBUG_PROGRESS, "got focus");
	hBmp = hBmpActive;
    RedrawWindow(hMainWnd,NULL,NULL,RDW_INVALIDATE);
    break;
  case WM_KILLFOCUS:
    vmb_debug(VMB_DEBUG_PROGRESS, "lost focus");
	hBmp = hBmpInactive;
	RedrawWindow(hMainWnd,NULL,NULL,RDW_INVALIDATE);
	break;
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
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hon);
	DragAcceptFiles(hWnd,TRUE);
	return 0;
  case WM_VMB_OFF: /* Power Off */
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hoff);
	DragAcceptFiles(hWnd,FALSE);
	return 0;
 case WM_CHAR:
    process_input((unsigned char) wParam); 
    return 0;
  }
 return (OptWndProc(hWnd, message, wParam, lParam));
}
