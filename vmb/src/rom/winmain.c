#include <winsock2.h>
#include <windows.h>
#include <afxres.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "winopt.h"
#include "param.h"
#include "inspect.h"
#include "option.h"






INT_PTR CALLBACK  
SettingsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{

  switch ( message )
  { case WM_INITDIALOG:
      uint64tohex(vmb_address,tmp_option);
      SetDlgItemText(hDlg,IDC_ADDRESS,tmp_option);
      SetDlgItemText(hDlg,IDC_FILE,vmb_filename);
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
		inspector[0].address=vmb_address;
        GetDlgItemText(hDlg,IDC_FILE,tmp_option,MAXTMPOPTION);
	    set_option(&vmb_filename,tmp_option);
		open_file();
      }
	  else if (HIWORD(wparam) == BN_CLICKED  && LOWORD(wparam) == IDC_BROWSE) 
	  { OPENFILENAME ofn;       /* common dialog box structure */
         /* Initialize OPENFILENAME */
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hMainWnd;
		GetDlgItemText(hDlg,IDC_FILE,tmp_option,MAXTMPOPTION);
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

#define REFRESH_TIME 50
hFloor

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{  switch( message )
  { case WM_CREATE:
		SetTimer(hWnd,1,REFRESH_TIME,NULL);
		break;
	case WM_TIMER:
		{ RECT rect;
		GetWindowRect(hWnd,&rect);
		SetWindowPos(hWnd, HWND_TOP, rect.left+2, rect.top,0,0,SWP_NOACTIVATE|SWP_NOSIZE | SWP_NOZORDER |SWP_NOREDRAW);
		BitBlt(hdc,x,y,cx,cy,hdcSrc CAPTUREBLT
		StretchBlt()
		InvalidateRect(hWnd,NULL,FALSE);
		return 0; 

		aus winvram
{ PAINTSTRUCT ps;
	  BOOL rc;
	  DWORD dw;
	  int src_left, src_top,src_right,src_bottom;
	  EnterCriticalSection (&bitmap_section);
      BeginPaint(hWnd, &ps); 
	  src_left = (int)floor(ps.rcPaint.left/zoom);
	  src_right = (int)ceil(ps.rcPaint.right/zoom);
	  src_top = (int)floor(ps.rcPaint.top/zoom);
	  src_bottom = (int)ceil(ps.rcPaint.bottom/zoom);
	  if (zoom<1.0)
	    SetStretchBltMode(ps.hdc,HALFTONE);
	  else
	    SetStretchBltMode(ps.hdc,COLORONCOLOR);
	  rc = StretchBlt(ps.hdc, 
		          (int)(src_left*zoom),(int)(src_top*zoom),
				  (int)((src_right-src_left)*zoom), (int)((src_bottom-src_top)*zoom),
                  hCanvas, 
				  src_left,src_top, 
				  src_right-src_left, src_bottom-src_top,
				  SRCCOPY);
      if (!rc)
	    dw = GetLastError();
	  EndPaint(hWnd, &ps); 
	  LeaveCriticalSection (&bitmap_section);
    }



		}
  }
    return (OptWndProc(hWnd, message, wParam, lParam));
}

