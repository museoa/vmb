#include <windows.h>
#include "winopt.h"
#include "resource.h"

#define STATIC_BUILD
#include "../scintilla/include/scintilla.h"
extern sptr_t ed_send(unsigned int msg,uptr_t wparam,sptr_t lparam);


#define MAX_FIND_STR 1024
static char find_str[MAX_FIND_STR]={0};
static char replace_str[MAX_FIND_STR]={0};

#define MAX_HISTORY 10
static char *find_history[MAX_HISTORY]={NULL};
static char *replace_history[MAX_HISTORY]={NULL};
static int find_history_count = 0;
static int replace_history_count = 0;
static int find_flags=SCFIND_MATCHCASE;
static int find_wrap=1; 
static int find_direction = 1; /* direction ==1 forward  direction == -1 backward */

void add_list(HWND h, char *str)
{ 
  LRESULT i=0;
  if (str[0]==0) return;
  i=SendMessage(h,CB_FINDSTRINGEXACT ,-1,(LPARAM)(LPCTSTR)str);
  if(i!=CB_ERR)
  { SendMessage(h,CB_DELETESTRING ,i,0);
  }
  i = SendMessage(h,CB_INSERTSTRING,0,(LPARAM)str);
  SendMessage(h,CB_SETCURSEL,i,0);
}

void restore_history(HWND h,int history_count, char *history[MAX_HISTORY]) 
{ int i;
  for (i=0; i< history_count; i++)
	SendMessage(h,CB_ADDSTRING,0,(LPARAM) (LPCTSTR)history[i]);
  SendMessage(h,CB_SETCURSEL,0,0);
}

void save_history(HWND h,int *history_count, char *history[MAX_HISTORY]) 
{ int count;
  count = (int)SendMessage(h,CB_GETCOUNT,0,0);
  if (count>MAX_HISTORY) count=MAX_HISTORY;
  for (*history_count=0; *history_count< count;)
  { int len = (int)SendMessage(h,CB_GETLBTEXTLEN,*history_count,0);
    if (history[*history_count]!=NULL) free(history[*history_count]);
	history[*history_count] = malloc(len+1);
	if (history[*history_count]==NULL) continue;
    SendMessage(h, CB_GETLBTEXT,*history_count,(LPARAM)(LPCTSTR)history[*history_count]);
    (*history_count)++;
  }
}

int find_again(void)
/* return 1 if found, zero otherwise */
{ struct Sci_TextToFind ttf;
  int found;
  if (find_str[0]==0) return 0;
  ttf.lpstrText=find_str;
  if (find_direction<0)
  {	ttf.chrg.cpMin = (long)ed_send(SCI_GETANCHOR,0,0);
    ttf.chrg.cpMax = 0;
  }
  else
  { ttf.chrg.cpMin = (long)ed_send(SCI_GETCURRENTPOS,0,0);
    ttf.chrg.cpMax = (long)ed_send(SCI_GETLENGTH,0,0);
  }
  found = (int)ed_send(SCI_FINDTEXT,find_flags,(sptr_t)&ttf);
  if (found<0)
  { if (find_wrap)
    { if (find_direction<0)
      {	ttf.chrg.cpMin = (long)ed_send(SCI_GETLENGTH,0,0);
        ttf.chrg.cpMax = (long)ed_send(SCI_GETCURRENTPOS,0,0);;
      }
      else
      { ttf.chrg.cpMin = 0;
        ttf.chrg.cpMax = (long)ed_send(SCI_GETANCHOR,0,0);
      }
      found = (int)ed_send(SCI_FINDTEXT,find_flags,(sptr_t)&ttf);
      if (found<0) return 0;
    } 
    else
      return 0;
  }
  ed_send(SCI_SETSEL,ttf.chrgText.cpMin,ttf.chrgText.cpMax);
  return 1;
}

int replace_again(void)
/* return 1 if text was replaced and next text found, zero otherwise */
{ struct Sci_TextToFind ttf;
  int found;
  if (find_str[0]==0) return 0;

  ttf.chrg.cpMin = (long)ed_send(SCI_GETSELECTIONSTART,0,0);
  ttf.chrg.cpMax = (long)ed_send(SCI_GETSELECTIONEND,0,0);
  ttf.lpstrText=find_str;
  found = (int)ed_send(SCI_FINDTEXT,find_flags,(sptr_t)&ttf);
  if (found<0 && !find_again()) return 0;
  ed_send(SCI_REPLACESEL,0,(sptr_t)replace_str); 
  find_again();
  return 1;
}

int replace_all(void)
/* return 1 if text was replaced and next text found, zero otherwise */
{ int start, end, found, len;
  if (find_str[0]==0) return 0;
  len=(int)strlen(find_str);
  found = 0;
  start = 0;
  while(1)
  { end = (int)ed_send(SCI_GETLENGTH,0,0);
    ed_send(SCI_SETTARGETSTART,start,0);
    ed_send(SCI_SETTARGETEND,end,0);
	ed_send(SCI_SETSEARCHFLAGS,find_flags,0);
	start = (int)ed_send(SCI_SEARCHINTARGET,len,(sptr_t)find_str);
	if (start<0) break;
	start=start+(int)ed_send(SCI_REPLACETARGET,-1,(sptr_t)replace_str);
	found = 1;
  }
  return found;
}
	


INT_PTR CALLBACK 
FindDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{
switch ( message )
  { case WM_INITDIALOG:
	  restore_history(GetDlgItem(hDlg,IDC_COMBO_FIND),find_history_count,find_history);
	  restore_history(GetDlgItem(hDlg,IDC_COMBO_REPLACE),replace_history_count,replace_history);
	  SendDlgItemMessage(hDlg,IDC_CHECK_CASE,BM_SETCHECK,
		  (find_flags&SCFIND_MATCHCASE)?BST_CHECKED:BST_UNCHECKED,0);
	  SendDlgItemMessage(hDlg,IDC_CHECK_WORD,BM_SETCHECK,
		  (find_flags&SCFIND_WHOLEWORD)?BST_CHECKED:BST_UNCHECKED,0);
	  SendDlgItemMessage(hDlg,IDC_CHECK_WRAP,BM_SETCHECK,
		  find_wrap?BST_CHECKED:BST_UNCHECKED,0);
      return TRUE;
    case WM_COMMAND:
	  if (HIWORD(wparam) == BN_CLICKED) 
	  { HWND hFind = GetDlgItem(hDlg,IDC_COMBO_FIND); 
	    HWND hReplace = GetDlgItem(hDlg,IDC_COMBO_REPLACE); 
	    SendMessage(hFind,WM_GETTEXT,MAX_FIND_STR-1,(LPARAM)(LPCTSTR)find_str);
		SendMessage(hReplace,WM_GETTEXT,MAX_FIND_STR-1,(LPARAM)(LPCTSTR)replace_str);
		if( wparam == IDOK )
		{ HWND hFocus;
		  hFocus = GetFocus();
		  if (hFocus==GetWindow(hFind,GW_CHILD))
		  {	add_list(hFind,find_str);
		  	find_again();
		  }
		  else if (hFocus==GetWindow(hReplace,GW_CHILD))
		  { add_list(hReplace,replace_str);
		  }
          return TRUE;
        }
	    else if (HIWORD(wparam) == BN_CLICKED)
	    { if (LOWORD(wparam) == IDC_FIND_NEXT) 
	      { find_direction = 1;
		    add_list(hFind,find_str);
		    find_again();
		    return TRUE;
	      }
	      else if (LOWORD(wparam) == IDC_FIND_PREV) 
	      { find_direction = -1;
		    add_list(hFind,find_str);
		    find_again();
		    return TRUE;
	      }
		  else if (LOWORD(wparam) == IDC_REPLACE) 
	      { add_list(hFind,find_str);
		    add_list(hReplace,replace_str);
		    replace_again();
			return TRUE;
	      }
		  else if (LOWORD(wparam) == IDC_REPLACE_ALL) 
	      { add_list(hFind,find_str);
		    add_list(hReplace,replace_str);
		    replace_all();
			return TRUE;
	      }
		  else if (LOWORD(wparam) == IDC_CHECK_CASE)
		  { LRESULT check=SendDlgItemMessage(hDlg,IDC_CHECK_CASE,BM_GETCHECK,0,0);
            if (check==BST_CHECKED) find_flags|=SCFIND_MATCHCASE;
			else find_flags&=~SCFIND_MATCHCASE;
		  }
		  else if (LOWORD(wparam) == IDC_CHECK_WORD)
		  { LRESULT check=SendDlgItemMessage(hDlg,IDC_CHECK_WORD,BM_GETCHECK,0,0);
            if (check==BST_CHECKED) find_flags|=SCFIND_WHOLEWORD;
			else find_flags&=~SCFIND_WHOLEWORD;
		  }
		  else if (LOWORD(wparam) == IDC_CHECK_WRAP)
		  { LRESULT check=SendDlgItemMessage(hDlg,IDC_CHECK_WRAP,BM_GETCHECK,0,0);
            if (check==BST_CHECKED) find_wrap=1;
			else find_wrap=0;
		  }
	    }
	  }
      break;
	case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { DestroyWindow(hDlg); 
        return TRUE;
      }
      break;
	case WM_DESTROY:
	  save_history(GetDlgItem(hDlg,IDC_COMBO_FIND),&find_history_count,find_history);
	  save_history(GetDlgItem(hDlg,IDC_COMBO_REPLACE),&replace_history_count,replace_history);
      unregister_subwindow(hDlg);
      break;
  }
  return FALSE;
}
