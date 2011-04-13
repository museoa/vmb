#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "win32connect.h"
#include "winopt.h"
#include "error.h"
#include "winmem.h"

void win32_message(char *msg)
{
	MessageBox(NULL,msg,"Message",MB_OK);
}

HWND hDebug=NULL; /* debug output goes to this window, if not NULL */


#define MAX_MSG_BUFFER	0x10000
#define MAX_MSG_FILL (MAX_MSG_BUFFER/2)
#define MSG_BUFFER_MASK (MAX_MSG_BUFFER-1)
#define MSG_FILL ((msg_end-msg_begin)&MSG_BUFFER_MASK)
static char msg_buffer[MAX_MSG_BUFFER];
static int msg_begin, msg_end = 0;
static int msg_changed = 0;
static CRITICAL_SECTION   msg_section;

static void add_msg_char(char c)
{ if (MSG_FILL>=MAX_MSG_FILL) /* we keep it halve empty*/
  { int n;
    char c;
    n = MAX_MSG_FILL/16;
	c = 0;
	do { /* remove 1/16 of the buffer or one line */
      c = msg_buffer[msg_begin];
	  msg_begin = (msg_begin+1)&MSG_BUFFER_MASK; /*make space */
	  n--;
	} while (n>0 && c!='\n');
  }
  msg_buffer[msg_end] = c;
  msg_end = (msg_end+1)&MSG_BUFFER_MASK;
}

static int normalize_msg(void)
/* we make the msg_buffer into a string return its length*/
{ if (msg_begin!=0) 
  { if (msg_end>msg_begin)
    { memmove(msg_buffer,msg_buffer+msg_begin,msg_end-msg_begin);
      msg_end = msg_end-msg_begin;
	  msg_begin = 0;
    }
    else if (msg_end<msg_begin) /* split buffer */
    { int head, tail;
      head = MAX_MSG_BUFFER-msg_begin;
	  tail = msg_end;
	  memmove(msg_buffer+head,msg_buffer,tail);
	  memmove(msg_buffer,msg_buffer+msg_begin,head);
      msg_end = head+tail;
	  msg_begin = 0;
    }
    else
      msg_begin=msg_end=0;
  }
  msg_buffer[msg_end] = 0; /* trailing zero */
  return msg_end;
}

void win32_debug(char *msg)
{ if (hDebug == NULL) return;
  if (*msg == 0) return;
  EnterCriticalSection (&msg_section);
  while (*msg)
  { add_msg_char(*msg);
	msg++;
  }
  add_msg_char('\r');
  add_msg_char('\n');
  if (msg_changed==0)
    PostMessage(hDebug,WM_VMB_MSG,0,0);
  msg_changed++;
  LeaveCriticalSection (&msg_section);
}

#if 0
void win32_debug(char *msg)
{ static char nl[] ="\r\n";	
  LRESULT  n;
  if (hDebug == NULL) return;
  n = SendDlgItemMessage(hDebug,IDC_DEBUG,EM_GETLINECOUNT,0,0);
  if (n>500)
  { n = SendDlgItemMessage(hDebug,IDC_DEBUG,EM_LINELENGTH,0,0);
    SendDlgItemMessage(hDebug,IDC_DEBUG,EM_SETSEL,0,n+2);
    SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)"");
    n = SendDlgItemMessage(hDebug,IDC_DEBUG,WM_GETTEXTLENGTH,0,0);
    SendDlgItemMessage(hDebug,IDC_DEBUG,EM_SETSEL,n,n);
  }
  SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)msg);
  SendDlgItemMessage(hDebug,IDC_DEBUG,EM_REPLACESEL,(WPARAM)FALSE,(LPARAM)nl);
  peek

}
#endif

void show_tab(HWND hDlg,int t)
{ 
#define TABs		4
#define IDCs		11
	struct{int idc; int show;} showlist[TABs][IDCs] =
{ /* tab 0 */
  {{IDC_DEBUG,SW_SHOW},
   {IDC_HIDE_PAYLOAD,SW_HIDE},
   {IDC_HIDE_MSG,SW_HIDE},
   {IDC_HIDE_INFO,SW_HIDE},
   {IDC_HIDE_PROGRESS,SW_HIDE},
   {IDC_HIDE_NOTIFY,SW_HIDE},
   {IDC_HIDE_WARN,SW_HIDE},
   {IDC_HIDE_ERROR,SW_HIDE},
   {IDC_HIDE_FATAL,SW_HIDE},
   {IDC_HIDE_ALL,SW_HIDE},
   {IDD_MEMORY,SW_HIDE}
  },
  /* tab 1 */
  {{IDC_DEBUG,SW_HIDE},
   {IDC_HIDE_PAYLOAD,SW_SHOW},
   {IDC_HIDE_MSG,SW_SHOW},
   {IDC_HIDE_INFO,SW_SHOW},
   {IDC_HIDE_PROGRESS,SW_SHOW},
   {IDC_HIDE_NOTIFY,SW_SHOW},
   {IDC_HIDE_WARN,SW_SHOW},
   {IDC_HIDE_ERROR,SW_SHOW},
   {IDC_HIDE_FATAL,SW_SHOW},
   {IDC_HIDE_ALL,SW_SHOW},
   {IDD_MEMORY,SW_HIDE}
  },
  /* tab 2 */
  {{IDC_DEBUG,SW_HIDE},
   {IDC_HIDE_PAYLOAD,SW_HIDE},
   {IDC_HIDE_MSG,SW_HIDE},
   {IDC_HIDE_INFO,SW_HIDE},
   {IDC_HIDE_PROGRESS,SW_HIDE},
   {IDC_HIDE_NOTIFY,SW_HIDE},
   {IDC_HIDE_WARN,SW_HIDE},
   {IDC_HIDE_ERROR,SW_HIDE},
   {IDC_HIDE_FATAL,SW_HIDE},
   {IDC_HIDE_ALL,SW_HIDE},
   {IDD_MEMORY,SW_SHOW}
  },
  /* tab 3 */
  {{IDC_DEBUG,SW_HIDE},
   {IDC_HIDE_PAYLOAD,SW_HIDE},
   {IDC_HIDE_MSG,SW_HIDE},
   {IDC_HIDE_INFO,SW_HIDE},
   {IDC_HIDE_PROGRESS,SW_HIDE},
   {IDC_HIDE_NOTIFY,SW_HIDE},
   {IDC_HIDE_WARN,SW_HIDE},
   {IDC_HIDE_ERROR,SW_HIDE},
   {IDC_HIDE_FATAL,SW_HIDE},
   {IDC_HIDE_ALL,SW_HIDE},
   {IDD_MEMORY,SW_HIDE}
  }
};
  int i;
  if (t>=TABs) return;
  for (i=0;i<IDCs;i++)
	  if (showlist[t][i].idc==IDD_MEMORY)
	  {  if (hMemory!=NULL)ShowWindow(hMemory,showlist[t][i].show); }
	  else
	    ShowWindow(GetDlgItem(hDlg,showlist[t][i].idc),showlist[t][i].show);
}

void show_filter(HWND hDlg)
{ CheckDlgButton(hDlg,IDC_HIDE_FATAL,(vmb_debug_mask&VMB_DEBUG_FATAL)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hDlg,IDC_HIDE_ERROR,(vmb_debug_mask&VMB_DEBUG_ERROR)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hDlg,IDC_HIDE_WARN,(vmb_debug_mask&VMB_DEBUG_WARN)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hDlg,IDC_HIDE_NOTIFY,(vmb_debug_mask&VMB_DEBUG_NOTIFY)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hDlg,IDC_HIDE_PROGRESS,(vmb_debug_mask&VMB_DEBUG_PROGRESS)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hDlg,IDC_HIDE_INFO,(vmb_debug_mask&VMB_DEBUG_INFO)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hDlg,IDC_HIDE_MSG,(vmb_debug_mask&VMB_DEBUG_MSG)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hDlg,IDC_HIDE_PAYLOAD,(vmb_debug_mask&VMB_DEBUG_PAYLOAD)?BST_CHECKED:BST_UNCHECKED);
  CheckDlgButton(hDlg,IDC_HIDE_ALL,vmb_debug_mask==0?BST_UNCHECKED:
		  ((vmb_debug_mask&0xFF)==0xFF?BST_CHECKED:BST_INDETERMINATE));
}


void resize_dialog(HWND hDlg,int w, int h)
{ MoveWindow(GetDlgItem(hDlg,IDC_TAB_DEBUG),5,5,w-10,h-10,TRUE); 
  MoveWindow(GetDlgItem(hDlg,IDC_DEBUG),10,30,w-20,h-40,TRUE); 
  if (hMemory!=NULL) MoveWindow(hMemory,10,30,w-20,h-40,TRUE);
}

INT_PTR CALLBACK   
DebugDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ static int minw, minh;
  switch ( message )
  { case WM_INITDIALOG :
  	InitializeCriticalSection (&msg_section);
	if (mem_inspect!=NULL)
	{  hMemory=CreateDialog(hInst,MAKEINTRESOURCE(IDD_MEMORY),hDlg,MemoryDialogProc);
	   register_subwindow(hMemory);
	}
	else
      hMemory=NULL;
	{ TCITEM tie;
	  RECT rect;
	  int i=0;
      tie.mask = TCIF_TEXT;
      tie.iImage = -1;
 	  tie.pszText = "Debug";
	  TabCtrl_InsertItem (GetDlgItem (hDlg, IDC_TAB_DEBUG), i++, &tie);
 	  tie.pszText = "Filter";
	  TabCtrl_InsertItem (GetDlgItem (hDlg, IDC_TAB_DEBUG), i++, &tie);
	  if (hMemory!=NULL)
	  { tie.pszText = "Memory";
	    TabCtrl_InsertItem (GetDlgItem (hDlg, IDC_TAB_DEBUG), i++, &tie);
	  }
 	  tie.pszText = "Register";
	  TabCtrl_InsertItem (GetDlgItem (hDlg, IDC_TAB_DEBUG), i++, &tie);

	  TabCtrl_SetCurSel (GetDlgItem (hDlg, IDC_TAB_DEBUG), 0);
      show_tab(hDlg, 0);
	  GetWindowRect(hDlg,&rect);
      minw = rect.right-rect.left;
	  minh = rect.bottom-rect.top;
	  GetClientRect(hDlg,&rect);
      resize_dialog(hDlg,rect.right-rect.left,rect.bottom-rect.top);
	}
    return FALSE; /* no keyboard focus on the debug window */
  case WM_NOTIFY:
	{ NMHDR *p;
      p = (NMHDR*)lparam;
      if (p->code == TCN_SELCHANGE) 
      { int i = TabCtrl_GetCurSel (GetDlgItem (hDlg, IDC_TAB_DEBUG));
	    show_tab(hDlg, i);
	    if (i==1) show_filter(hDlg);
      }
	}
    break;
   case WM_COMMAND: 
     if (wparam==IDOK)
		 return FALSE;
     if (HIWORD(wparam) == BN_CLICKED) 
     { int flag = 0;
	   switch (LOWORD(wparam)) 
	   { case IDC_HIDE_FATAL: flag = VMB_DEBUG_FATAL; break;
		 case IDC_HIDE_ERROR: flag = VMB_DEBUG_ERROR; break;	
		 case IDC_HIDE_WARN: flag = VMB_DEBUG_WARN; break;	
		 case IDC_HIDE_NOTIFY: flag = VMB_DEBUG_NOTIFY; break;	
		 case IDC_HIDE_PROGRESS: flag = VMB_DEBUG_PROGRESS; break;	
		 case IDC_HIDE_INFO: flag = VMB_DEBUG_INFO; break;	
		 case IDC_HIDE_MSG: flag = VMB_DEBUG_MSG; break;	
		 case IDC_HIDE_PAYLOAD: flag = VMB_DEBUG_PAYLOAD; break;	
		 case IDC_HIDE_ALL:
		 { LRESULT bs; 
		   bs = SendDlgItemMessage(hDlg, IDC_HIDE_ALL, BM_GETSTATE, 0, 0);
		   if (bs & BST_CHECKED) vmb_debug_mask = 0xFFFF;
		   else if (bs & BST_INDETERMINATE) vmb_debug_mask = VMB_DEBUG_DEFAULT;
		   else vmb_debug_mask = 0;
		   show_filter(hDlg);
	       return FALSE;
		 }
	   }
       /* Retrieve the state of the check box. */
       if (SendDlgItemMessage(hDlg, LOWORD(wparam), BM_GETSTATE, 0, 0)&BST_CHECKED)
             vmb_debug_mask |= flag;
		   else
             vmb_debug_mask &= ~flag;
       return FALSE; 
	 }
     break;
  case WM_VMB_MSG:
		{ int n; 
		  EnterCriticalSection (&msg_section);
		  n = normalize_msg();
		  msg_changed = 0;
		  SetDlgItemText(hDlg,IDC_DEBUG,msg_buffer);
		  LeaveCriticalSection (&msg_section);
		  /* n = SendDlgItemMessage(hDlg,IDC_DEBUG,WM_GETTEXTLENGTH,0,0); */
          SendDlgItemMessage(hDlg,IDC_DEBUG,EM_SETSEL,n,n);
          SendDlgItemMessage(hDlg,IDC_DEBUG,EM_SCROLLCARET,0,0); 
		}

    return TRUE;
    case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { if (hMemory!=NULL) 
	    { unregister_subwindow(hMemory);
		  DestroyWindow(hMemory);
	    }
	    hMemory = NULL;
	    CheckMenuItem(hMenu,ID_DEBUG,MF_BYCOMMAND|MF_UNCHECKED);
		vmb_debug_off();
	    DeleteCriticalSection(&msg_section);
		unregister_subwindow(hDebug);
	    EndDialog(hDlg, TRUE);
		hDebug = NULL;
        return TRUE;
      }
      break;
    case WM_SIZE: 
	  resize_dialog(hDlg,LOWORD(lparam),HIWORD(lparam));
      return TRUE;
	case WM_GETMINMAXINFO:
		{ MINMAXINFO *p = (MINMAXINFO *)lparam;
		  p->ptMinTrackSize.x = minw;
          p->ptMinTrackSize.y = minh;
		}
		return 0;
  }
  return FALSE;
}
