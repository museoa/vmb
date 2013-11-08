#include <windows.h>
#include "resource.h"
#include "winmain.h"
#include "info.h"
#include "sources.h"




static HWND hSources;

static void add_load_files(int file_no)
{ if (file2loading(file_no))
  SendMessage(hSources,LB_ADDSTRING,0,(LPARAM)file2fullname(file_no));
}
static void set_load_files(int file_no)
{ 
  int i=(int)SendMessage(hSources,LB_FINDSTRING,-1,(LPARAM)file2fullname(file_no));
  if (i==LB_ERR) file2loading(file_no)=0;
  else file2loading(file_no)=1;

}

INT_PTR CALLBACK    
OptionSourcesDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ 
  switch ( message )
  { case WM_INITDIALOG:
      { hSources=GetDlgItem(hDlg,IDC_LIST_FILES);
		for_all_files(add_load_files);
      }
      return TRUE;
    case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
      { for_all_files(set_load_files);
        hSources=NULL;
        EndDialog(hDlg, TRUE);
        return TRUE;
      } else if (wparam==IDCANCEL)
	  { EndDialog(hDlg, TRUE);
        return TRUE;
	  } else if (wparam==IDC_ADD)
	  { int file_no = ed_open();
	    if (file_no>=0) {
		  char *str = file2fullname(file_no);
		  int item = (int)SendMessage(GetDlgItem(hDlg,IDC_LIST_FILES),LB_ADDSTRING,0,(LPARAM)str);
		      SendMessage(GetDlgItem(hDlg,IDC_LIST_FILES),LB_SETITEMDATA,item,(LPARAM)file_no);
	    }
        return TRUE;
	  } else if (wparam==IDC_REMOVE)
	  { int item = (int)SendMessage(GetDlgItem(hDlg,IDC_LIST_FILES),LB_GETCURSEL,0,0);
        if (item!=LB_ERR)
          SendMessage(GetDlgItem(hDlg,IDC_LIST_FILES),LB_DELETESTRING,item,0);
        return TRUE;
	  }
     break;
  }
  return FALSE;
}



