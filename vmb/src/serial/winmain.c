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
extern void process_input_file(char *filename);


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ switch (message) 
  { case WM_DROPFILES:
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
