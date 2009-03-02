#include <windows.h>
#include "winopt.h"
#include "error.h"


void win32_message(char *msg)
{
	MessageBox(NULL,msg,"Message",MB_OK);
}

HWND hDebug=NULL; /* debug output goes to this window, if not NULL */

void win32_debug(char *msg)
{ static char nl[] ="\r\n";	
  LRESULT  n;
  if (hDebug == NULL) return;
  n = SendDlgItemMessage(hDebug,IDC_DEBUG,EM_GETLINECOUNT,0,0);
  if (n>100)
  { n = SendDlgItemMessage(hDebug,IDC_DEBUG,EM_LINELENGTH,0,0);
    SendDlgItemMessage(hDebug,IDC_DEBUG,EM_SETSEL,0,n+2);
    SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)"");
    n = SendDlgItemMessage(hDebug,IDC_DEBUG,WM_GETTEXTLENGTH,0,0);
    SendDlgItemMessage(hDebug,IDC_DEBUG,EM_SETSEL,n,n);
  }
  SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)msg);
  SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)nl);
}


INT_PTR CALLBACK   
DebugDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{
  switch ( message )
  { case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { hDebug = NULL;
	    EndDialog(hDlg, TRUE);
	    CheckMenuItem(hMenu,ID_DEBUG,MF_BYCOMMAND|MF_UNCHECKED);
		vmb_debug_off();
        return TRUE;
      }
      break;
    case WM_SIZE: 
      MoveWindow(GetDlgItem(hDebug,IDC_DEBUG),5,5,LOWORD(lparam)-10,HIWORD(lparam)-10,TRUE); 
      return TRUE;
  }
  return FALSE;
}
