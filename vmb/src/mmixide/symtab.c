#include <stdio.h>
#include <windows.h>
#include "vmb.h"
#include "error.h"
#include "splitter.h"
#include "winmain.h"
#include "info.h"
#include "symtab.h"



void symtab_reset(void)
{  SendMessage(hSymbolTable,LB_RESETCONTENT,0,0);	
}

void symtab_add(char *symbol, sym_node *sym)
{ LPARAM item;
  item = SendMessage(hSymbolTable,LB_ADDSTRING,0,(LPARAM)symbol);
  SendMessage(hSymbolTable,LB_SETITEMDATA,item,(LPARAM)sym);
}

void symtab_mark(char *symbol)
{ int item;
  item = (int)SendMessage(hSymbolTable,LB_FINDSTRINGEXACT,-1,(LPARAM)symbol);
  if(item==LB_ERR) return;
  SendMessage(hSymbolTable,LB_SETCURSEL,item,0);
}

void create_symtab(void)
{ sp_create_options(1,1,0.15,0,hEdit);
  hSymbolTable = CreateWindow("LISTBOX","Symbol Table",
		     WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|LBS_SORT|LBS_HASSTRINGS|LBS_OWNERDRAWFIXED,
             0,0,0,0,
	         hSplitter, NULL, hInst, NULL);
  fill_symtab();
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
 

sym_node *find_symbol(char *symbol, trie_node *t)
{ if (symbol==NULL || *symbol==0 || t==NULL) return NULL;
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