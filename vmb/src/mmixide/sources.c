#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "winmain.h"
#include "info.h"
#include "editor.h"
#include "sources.h"


static HWND hSourceDlg;
static HWND hFileTab;
static int current_tab;


static int application_index;

static void add_application(int file_no)
{  HWND hSources=GetDlgItem(hSourceDlg,IDC_APPLICATION);
   int i=(int)SendMessage(hSources,CB_ADDSTRING,0,(LPARAM)unique_name(file_no));
   SendMessage(hSources,CB_SETITEMDATA,i,(LPARAM)file_no);
   if (file_no==application_file_no) application_index=i;
}

#define MAX_CMD 512
static void save_file(void)
/* call before changing current_tab */
{ int file_no;
  TCITEM tie;
  tie.mask=TCIF_PARAM;
  TabCtrl_GetItem(hFileTab,current_tab,&tie);
  file_no=(int)tie.lParam;
  file2assembly(file_no)=IsDlgButtonChecked(hSourceDlg,IDC_CHECK_SYMBOLS);
  if (IsDlgButtonChecked(hSourceDlg,IDC_CHECK_EXECUTE))
  { char cmd[MAX_CMD+1];
    int n;
	n= GetDlgItemText(hSourceDlg,IDC_EDIT_COMMAND,cmd,MAX_CMD);
	if (n>=MAX_CMD) MessageBox(hSourceDlg,"Command too long, truncated","Warning",MB_ICONWARNING|MB_OK);
    command[file_no]=realloc(command[file_no],n+1);
	if (command[file_no]==NULL)
	{ MessageBox(hSourceDlg,"Out of memmory","Error",MB_ICONWARNING|MB_OK);
	  return;
	}
	strncpy_s(command[file_no],n+1,cmd,n+1);
  }
  else if (command[file_no]!=NULL)
  { free(command[file_no]);
    command[file_no]=NULL;
  }
}

static void show_file(void)
/* call after changing current_tab */
{ int file_no;
  TCITEM tie;
  char *name;
  tie.mask=TCIF_PARAM;
  TabCtrl_GetItem(hFileTab,current_tab,&tie);
  file_no=(int)tie.lParam;
  name = file2fullname(file_no);
  if (name==NULL) name = unique_name(file_no);
  SetDlgItemText(hSourceDlg,IDC_TAB_FULLNAME,name);
  CheckDlgButton(hSourceDlg,IDC_CHECK_SYMBOLS,file2assembly(file_no)?BST_CHECKED:BST_UNCHECKED);
  if (command[file_no]!=NULL)
  {   CheckDlgButton(hSourceDlg,IDC_CHECK_EXECUTE,BST_CHECKED);
      SetDlgItemText(hSourceDlg,IDC_EDIT_COMMAND,command[file_no]);
	  EnableWindow(GetDlgItem(hSourceDlg,IDC_EDIT_COMMAND),TRUE);
  }
  else
  {   CheckDlgButton(hSourceDlg,IDC_CHECK_EXECUTE,BST_UNCHECKED);
      SetDlgItemText(hSourceDlg,IDC_EDIT_COMMAND,"");
	  EnableWindow(GetDlgItem(hSourceDlg,IDC_EDIT_COMMAND),FALSE);
  }

}




static void select_tab(int index)
/* call to change current_tab */
{ if (index<0) return;
  if (current_tab>=0) 
	save_file();
  current_tab=index;
  TabCtrl_SetCurSel (hFileTab, current_tab);
  show_file();
}

static void add_file(int file_no)
{ TCITEM tie;
  int index;
  if (hFileTab==NULL) return;
  if (file_no<0) return;
  index = TabCtrl_GetItemCount(hFileTab);
  tie.mask = TCIF_TEXT|TCIF_PARAM;
  tie.pszText = unique_name(file_no);
  tie.lParam=file_no;
  current_tab=TabCtrl_InsertItem (hFileTab, index, &tie);
}


INT_PTR CALLBACK    
OptionSourcesDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ 
  switch ( message )
  { case WM_INITDIALOG:
      { hSourceDlg=hDlg;
	    hFileTab=GetDlgItem(hDlg,IDC_TAB_FILES);
		for_all_files(add_file);
		current_tab=0;
		TabCtrl_SetCurSel (hFileTab, current_tab);
        show_file();
		for_all_files(add_application);
        SendMessage(GetDlgItem(hDlg,IDC_APPLICATION),CB_SETCURSEL,application_index,0);
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
      { save_file();
		application_index = (int)SendMessage(GetDlgItem(hDlg,IDC_APPLICATION),CB_GETCURSEL,0,0);
		if (application_index!=CB_ERR)
			set_application((int)SendMessage(GetDlgItem(hDlg,IDC_APPLICATION),CB_GETITEMDATA,application_index,0));
        hSourceDlg=NULL;
		hFileTab=NULL;
		EndDialog(hDlg, TRUE);
        return TRUE;
      } 
	  else if (wparam==IDC_ADD)
	  { int file_no = ed_open();
		if (file_no>=0) 
		{  add_file(file_no);
		   select_tab(current_tab);
		}
        return TRUE;
	  } 
	  else if (wparam==IDC_CHECK_EXECUTE)
	  { EnableWindow(GetDlgItem(hSourceDlg,IDC_EDIT_COMMAND),IsDlgButtonChecked(hSourceDlg,IDC_CHECK_EXECUTE));
        return TRUE;
	  }
     break;
	 case WM_NOTIFY:
	  { NMHDR *n = (NMHDR*)lparam;
	    if (n->hwndFrom==hFileTab)
		{ if (n->code == TCN_SELCHANGE){ 
			save_file();
			current_tab = TabCtrl_GetCurSel(hFileTab);
			show_file();
		  }
		}
		return TRUE;
	  }


  }
  return FALSE;
}



