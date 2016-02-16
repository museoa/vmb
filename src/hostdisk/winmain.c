#define _WIN32_DCOM

#include <winsock2.h>
#include <windows.h>
#include <shlobj.h>
#include <objbase.h>
#include <afxres.h>
#include "vmb.h"
#include "bus-arith.h"
#include "resource.h"
#include "winopt.h"
#include "param.h"
#include "option.h"

HBITMAP hbussy;


INT_PTR CALLBACK  
SettingsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{

  switch ( message )
  { case WM_INITDIALOG:
      uint64_to_hex(vmb_address,tmp_option);
      SetDlgItemText(hDlg,IDC_ADDRESS,tmp_option);
      SetDlgItemText(hDlg,IDC_FILE,filename);
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
        GetDlgItemText(hDlg,IDC_FILE,tmp_option,MAXTMPOPTION);
	    set_option(&filename,tmp_option);
		interrupt  = GetDlgItemInt(hDlg,IDC_INTERRUPT,NULL,FALSE);
      }
	  else if (HIWORD(wparam) == BN_CLICKED  && LOWORD(wparam) == IDC_BROWSE) 
      { BROWSEINFO bif = {0};
	    LPITEMIDLIST p;
		CoInitializeEx(NULL,COINIT_MULTITHREADED);
	    bif.hwndOwner = hMainWnd;
	    GetDlgItemText(hDlg,IDC_FILE,tmp_option,MAXTMPOPTION);
	    bif.pszDisplayName=tmp_option;
	    bif.lpszTitle="Autoname Folder";
	    p = SHBrowseForFolder(&bif);
        if(p!=0)
	    { SHGetPathFromIDList(p,tmp_option);
	      strcat(tmp_option,"\\");
	      SetDlgItemText(hDlg,IDC_FILE,tmp_option);
	    }
		CoUninitialize();
      }
#if 0
	  { OPENFILENAME ofn;       /* common dialog box structure */
         /* Initialize OPENFILENAME */
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hMainWnd;
		GetDlgItemText(hDlg,IDC_FILE,tmp_option,MAXTMPOPTION);
        ofn.lpstrFile = tmp_option;
        ofn.nMaxFile = MAXTMPOPTION;
        ofn.lpstrFilter = "All\0*.*\0Image\0*.img\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        /* Display the Open dialog box. */
        if (GetOpenFileName(&ofn)==TRUE) 
		   SetDlgItemText(hDlg,IDC_FILE,tmp_option);
	  }
#endif
	  if (wparam == IDOK || wparam == IDCANCEL)
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
  }
  return FALSE;
}




LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ switch (message) 
  {  
  case WM_VMB_OTHER+1: /* Disk bussy */
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hbussy);
	return 0;
  case WM_CREATE: 
	hpower = CreateWindow("STATIC",NULL,WS_CHILD|WS_VISIBLE|SS_BITMAP|SS_REALSIZEIMAGE,15,65,32,32,hWnd,NULL,hInst,0);
    SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hoff);
    return 0;

  default:
    return (OptWndProc(hWnd, message, wParam, lParam));
  }
 return (OptWndProc(hWnd, message, wParam, lParam));
}
