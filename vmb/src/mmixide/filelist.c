#include <stdio.h>
#include <windows.h>
#include "vmb.h"
#include "error.h"
#include "splitter.h"
#include "winmain.h"
#include "info.h"
#include "filelist.h"



void file_list_reset(void)
{  SendMessage(hFileList,LB_RESETCONTENT,0,0);	
}

#define MAX_LISTNAME 64
char *file_listname(int file_no)
{ static char listname[MAX_LISTNAME+7];
  static char noname[]="Unnamed";
  char *name = file2shortname(file_no);
  if (name==NULL) name=noname;
  if (file_no==0) return name;
  if(unique_shortname(file_no)) return name;
  sprintf_s(listname,MAX_LISTNAME,"%.64s (%d)",name,file_no);
  return listname;
}

void file_list_remove(int file_no)
{ int item;
  char *name = file_listname(file_no);
  item = (int)SendMessage(hFileList,LB_FINDSTRINGEXACT,-1,(LPARAM)name);
  if(item==LB_ERR) return;
  SendMessage(hFileList,LB_DELETESTRING,item,0);
}

void file_list_add(int file_no)
{ LPARAM item;
  char *name = file_listname(file_no);
  item = SendMessage(hFileList,LB_ADDSTRING,0,(LPARAM)name);
  SendMessage(hFileList,LB_SETITEMDATA,item,file_no);
}

void file_list_mark(int file_no)
{ int item;
  char *name = file_listname(file_no);
  item = (int)SendMessage(hFileList,LB_FINDSTRINGEXACT,-1,(LPARAM)name);
  if(item==LB_ERR) return;
  SendMessage(hFileList,LB_SETCURSEL,item,0);
}

void create_filelist(void)
{ sp_create_options(1,1,0.15,0,hEdit);
  hFileList = CreateWindow("LISTBOX","File List",
		     WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|LBS_SORT|LBS_HASSTRINGS|LBS_OWNERDRAWFIXED,
             0,0,0,0,
	         hSplitter, NULL, hInst, NULL);
  fill_file_list();
}

int file_list_measureitem(LPMEASUREITEMSTRUCT lm)
{   lm->itemHeight = 20; 
	return 1;
}

extern int file_list_drawitem(LPDRAWITEMSTRUCT di)
{ int len;
  char *str;
  TEXTMETRIC tm; 
  int y; 
  COLORREF cr;
  if (di->itemID <0 ) return 0;
  switch (di->itemAction) 
  { case ODA_SELECT: 
    case ODA_DRAWENTIRE: 
      len = (int)SendMessage(di->hwndItem, LB_GETTEXTLEN, di->itemID, 0); 
	  str = malloc(len+1);
	  if (str==NULL) 
	  { vmb_error(__LINE__,"Out of memory");
	    return 0;
	  }
      SendMessage(di->hwndItem, LB_GETTEXT, di->itemID, (LPARAM)str); 
      GetTextMetrics(di->hDC, &tm); 
      y = (di->rcItem.bottom + di->rcItem.top - tm.tmHeight) / 2;
      if (di->itemState & ODS_SELECTED)
		  cr = SetBkColor(di->hDC,RGB(0x80,0x80,0xff));
	  else
		  cr =SetBkColor(di->hDC,RGB(0xff,0xff,0xff));
	  ExtTextOut(di->hDC, 15, y,ETO_CLIPPED|ETO_OPAQUE, &di->rcItem,str,len,NULL);
	  SetBkColor(di->hDC,cr);
	  free(str);

    return 1;
  } 
  return 0;
}
 