#include <windows.h>
#include "resource.h"
#include "winmain.h"
#include "info.h"
#include "editor.h"
#include "sources.h"

static HWND hSources;

static void add_load_files(int file_no)
{ char *name;
  name = file2fullname(file_no);
  if (name!=NULL && file2assembly(file_no))
  { int i = (int)SendMessage(hSources,LB_ADDSTRING,0,(LPARAM)name);
    SendMessage(hSources,LB_SETITEMDATA,i,(LPARAM)file_no);}
}

static int application_index;
static void add_files(int file_no)
{  int i=(int)SendMessage(hSources,CB_ADDSTRING,0,(LPARAM)unique_name(file_no));
   SendMessage(hSources,CB_SETITEMDATA,i,(LPARAM)file_no);
   if (file_no==application_file_no) application_index=i;
}
static void set_assembly_files(int file_no)
{ char *name;
  int i;
  name = file2fullname(file_no);
  if (name==NULL) return;
  i=(int)SendMessage(hSources,LB_FINDSTRING,-1,(LPARAM)name);
  if (i==LB_ERR) file2assembly(file_no)=0;
  else file2assembly(file_no)=1;

}

INT_PTR CALLBACK    
OptionSourcesDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ 
  switch ( message )
  { case WM_INITDIALOG:
      { hSources=GetDlgItem(hDlg,IDC_LIST_FILES);
		for_all_files(add_load_files);
        hSources=GetDlgItem(hDlg,IDC_APPLICATION);
		application_index=-1;
		for_all_files(add_files);
        SendMessage(hSources,CB_SETCURSEL,application_index,0);
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
      { for_all_files(set_assembly_files);
        hSources=NULL;
		application_index = (int)SendMessage(GetDlgItem(hDlg,IDC_APPLICATION),CB_GETCURSEL,0,0);
		if (application_index!=CB_ERR)
			set_application((int)SendMessage(GetDlgItem(hDlg,IDC_APPLICATION),CB_GETITEMDATA,application_index,0));
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



