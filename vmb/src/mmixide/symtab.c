#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "libmmixal.h"
#include "error.h"
#include "splitter.h"
#include "winmain.h"
#include "mmixrun.h"
#include "info.h"
#include "editor.h"
#include "symtab.h"

int symtab_locals=0;
int symtab_registers=0;
int symtab_small=0;



INT_PTR CALLBACK    
OptionSymtabDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ 
  switch ( message )
  { case WM_INITDIALOG:
      CheckDlgButton(hDlg,IDC_CHECK_LOCALS,symtab_locals?BST_CHECKED:BST_UNCHECKED);
	  CheckDlgButton(hDlg,IDC_CHECK_REGISTERS,symtab_registers?BST_CHECKED:BST_UNCHECKED);
	  CheckDlgButton(hDlg,IDC_CHECK_SMALL,symtab_small?BST_CHECKED:BST_UNCHECKED);
      return TRUE;
    case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
	  { symtab_locals =IsDlgButtonChecked(hDlg,IDC_CHECK_LOCALS);
		symtab_registers =IsDlgButtonChecked(hDlg,IDC_CHECK_REGISTERS);
		symtab_small =IsDlgButtonChecked(hDlg,IDC_CHECK_SMALL);
		update_symtab();
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


void symtab_reset(void)
{  SendMessage(hSymbolTable,LB_RESETCONTENT,0,0);	
}

static void symtab_add(char *symbol, sym_node *sym)
{ LPARAM item;
  if ((symtab_registers && sym->link==REGISTER) || sym->link==DEFINED)
  { if (!symtab_locals && strchr(symbol+1,':')!=NULL) return;
    if (!symtab_small && sym->link==DEFINED && sym->equiv.h==0 && sym->equiv.l<0x100) return;
    item = SendMessage(hSymbolTable,LB_ADDSTRING,0,(LPARAM)symbol);
    SendMessage(hSymbolTable,LB_SETITEMDATA,item,(LPARAM)sym);
  }
}

#if 0
/* currently not used */
void symtab_mark(char *symbol)
{ int item;
  item = (int)SendMessage(hSymbolTable,LB_FINDSTRINGEXACT,-1,(LPARAM)symbol);
  if(item==LB_ERR) return;
  SendMessage(hSymbolTable,LB_SETCURSEL,item,0);
}
#endif

sym_node *find_symbol(char *symbol,int file_no)
{ trie_node *t;
  if (symbol==NULL || *symbol==0) return NULL;
  if (file_no<0) return NULL;
  t=(trie_node*)file2symbols(file_no);
  while(t!=NULL) 
  { if (*symbol==t->ch) 
	{ symbol++; 
	  if (*symbol==0) return t->sym;
      t=t->mid; 
	}
    else if (*symbol < t->ch) t=t->left;
    else t=t->right;
  }
  return NULL;
}

static char symtab_buf[1000];

static void enumerate_symtab(trie_node *t, char *sym_ptr)
{

  if (t->left) enumerate_symtab(t->left,sym_ptr);
  *sym_ptr=(char)t->ch;
  if (t->sym)
  { *(sym_ptr+1)=0;
     symtab_add(symtab_buf+1,t->sym); /* skip leading : */
  }
  if (t->mid) enumerate_symtab(t->mid,sym_ptr+1);
  if (t->right) enumerate_symtab(t->right,sym_ptr);
}

static void symtab_add_file_no(int file_no)
{ trie_node *t;
  if (!file2assembly(file_no)) return;
  t= (trie_node*)file2symbols(file_no);
  if (t!=NULL)
     enumerate_symtab(t, symtab_buf);
}


void update_symtab(void)
{ 
  symtab_reset();
  for_all_files(symtab_add_file_no);
}
void create_symtab(void)
{ sp_create_options(1,1,0.15,0,hEdit);
  hSymbolTable = CreateWindow("LISTBOX","Symbol Table",
		     WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|LBS_SORT|LBS_HASSTRINGS|LBS_OWNERDRAWFIXED,
             0,0,0,0,
	         hSplitter, NULL, hInst, NULL);
  update_symtab();
}

int symtab_measureitem(LPMEASUREITEMSTRUCT lm)
{   lm->itemHeight = 20; 
	return 1;
}

extern int symtab_drawitem(LPDRAWITEMSTRUCT di)
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
	  { win32_error(__LINE__,"Out of memory");
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
 
