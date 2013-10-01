#include <windows.h>
#include "resource.h"
#include "winmain.h"
#include "edit.h"

int autosave = 0;
int show_line_no = 0;
int show_profile = 0;

INT_PTR CALLBACK    
OptionEditorDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ 
  switch ( message )
  { case WM_INITDIALOG:
      CheckDlgButton(hDlg,IDC_CHECK_AUTOSAVE,autosave?BST_CHECKED:BST_UNCHECKED);
	  CheckDlgButton(hDlg,IDC_CHECK_LINENO,show_line_no?BST_CHECKED:BST_UNCHECKED);
	  CheckDlgButton(hDlg,IDC_CHECK_PROFILE,show_profile?BST_CHECKED:BST_UNCHECKED);
      return TRUE;
    case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
	  { autosave =IsDlgButtonChecked(hDlg,IDC_CHECK_AUTOSAVE);
		show_line_no =IsDlgButtonChecked(hDlg,IDC_CHECK_LINENO);
        set_lineno_width();
		show_profile =IsDlgButtonChecked(hDlg,IDC_CHECK_PROFILE);
        set_profile_width();
		update_profile();
		EndDialog(hDlg, TRUE);
        return TRUE;
      } 
	  else if (wparam==IDCANCEL)
	  { EndDialog(hDlg, TRUE);
        return TRUE;
	  }
     break;
  }
  return FALSE;
}
