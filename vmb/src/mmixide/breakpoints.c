#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include "mmixlib.h"
#include "winopt.h"
#include "error.h"
#include "splitter.h"
#include "winmain.h"
#include "resource.h"
#include "info.h"
#include "debug.h"
#include "editor.h"
#include "symtab.h"
#include "mmixrun.h"
#include "breakpoints.h"


static HWND hBreakList=NULL;
static void add_breakpoint(int i);
static void del_breakpoint(int i);

/* The datastructure representing all breakpoints */
#define MAX_BREAKPOINTS 256
static
struct {
	int line_no;
	int file_no;
	unsigned char bkpt;
	unsigned char active;
	octa loc;
} breakpoints[MAX_BREAKPOINTS];
static int blimit = 0, bcount=0;

/* there are two kinds of breakpoinst with a file!=-1 and witha loc!=-1 */
#define loc_unknown(i) (breakpoints[i].loc.h==-1 && breakpoints[i].loc.l==-1)
#define file_unknown(i) (breakpoints[i].file_no<0)
#define unused(i) (loc_unknown(i)&& file_unknown(i))

/* constructing a new breakpoint */
static int new_breakpoint(void)
{ int n;
  if (bcount>=MAX_BREAKPOINTS)
  { win32_error(__LINE__,"Too many breakpoints");
    n=bcount-1; /* reuse last breakpoint */
  }
  else
  { n=blimit;
    if (blimit==bcount) 
	  blimit++;
	else
	  do n--; while(!unused(n));
	bcount++;
  }
  breakpoints[n].line_no=0;
  breakpoints[n].file_no=-1; /* no file */
  breakpoints[n].bkpt=0;
  breakpoints[n].active=1;
  breakpoints[n].loc.h = breakpoints[n].loc.l = -1; /* loc unknown */
  return n;
}

/* destructing an old breakpoint */
static void remove_breakpoint(int i)
{ if (i<0 ||i>=blimit) return;
  del_breakpoint(i);
  if (blimit==i+1) 
    blimit--;
  else
  { breakpoints[i].file_no=-1;
    breakpoints[i].loc.h = breakpoints[i].loc.l = -1; /* loc unknown */
  }
  bcount--;
}

int find_file_breakpoint(int file_no, int line_no)
{ int i;
  for (i=0; i<blimit;i++)
    if ( breakpoints[i].file_no==file_no && 
		 breakpoints[i].line_no==line_no)
	 return i;
  return -1;
}


int find_loc_breakpoint(octa loc)
{ int i;
  for (i=0; i<blimit;i++)
    if ( breakpoints[i].loc.l==loc.l && 
		 breakpoints[i].loc.h==loc.h)
	 return i;
  return -1;
}
/* Interfacing the breakpoints with the memory representation */

static
void sync_loc(octa loc, mem_tetra *dat)
/* syncronizing all breakpoints with the loaded memory */
{ int i;
  dat->bkpt=0;
  for (i=0;i<blimit;i++)
  { if (dat->file_no==breakpoints[i].file_no &&
        dat->line_no==breakpoints[i].line_no)
	  dat->bkpt=breakpoints[i].bkpt;
	else if (breakpoints[i].loc.l==loc.l && breakpoints[i].loc.h==loc.h)
	  dat->bkpt=breakpoints[i].bkpt;
  }    
}

void mem_sync_breakpoints(void)
/* syncronizing all breakpoints with the loaded memory */
{  if (!mmix_active()) return;
   mem_iterator(sync_loc);
}

static int break_bits=0, break_file_no=-1, break_line_no=0;

static void file_line_break(octa loc, mem_tetra *dat)
{ if (dat->line_no==break_line_no &&
	  dat->file_no==break_file_no) 
	dat->bkpt = break_bits;
}
static void file_break(octa loc, mem_tetra *dat)
{ if (dat->file_no==break_file_no) 
	dat->bkpt = break_bits;
}

void mem_file_line_breakpoints(int file_no, int line_no, int bits)
/* set breakpoints for whole line */
{ if (!mmix_active()) return;
  break_bits=bits;
  break_file_no=file_no; 
  break_line_no=line_no;
  mem_iterator(file_line_break);
}	

void mem_file_breakpoints(int file_no, int bits)
/* set breakpoint for whole file */
{ if (!mmix_active()) return;
  break_bits=bits;
  break_file_no=file_no; 
  mem_iterator(file_break);
}	


void mem_set_breakpoint(int i)
{ if(mmix_active())
  { if (!loc_unknown(i))
       loc2bkpt(breakpoints[i].loc)=breakpoints[i].bkpt;
    else if (!file_unknown(i))
		mem_file_line_breakpoints(breakpoints[i].file_no, breakpoints[i].line_no, breakpoints[i].bkpt);
  }
}


/* interfacing the breakpoint structure with the editor */

/* removing all breakpoints refering to a file */
void remove_file_breakpoints(int file_no)
{ int i=0;
  if (file_no<0) return;
  for (i=0;i<blimit;i++)
	if (breakpoints[i].file_no==file_no)
	  remove_breakpoint(i);
  mem_file_breakpoints(file_no,0); /* delete them from memory */
}


/* setting a breakpoint in a file*/
int set_file_breakpoint(int file_no, int line_no, int mask)
/* return breakpoint set */
{ int i;
  i =find_file_breakpoint(file_no,line_no);
  if (i>=0)
  {  breakpoints[i].bkpt|=mask;
     InvalidateRect(hBreakList,NULL,FALSE);
  }
  else
  { i = new_breakpoint();
    breakpoints[i].file_no=file_no;
    breakpoints[i].line_no=line_no;
    breakpoints[i].bkpt=mask;
    breakpoints[i].active=1;
    breakpoints[i].loc.h=breakpoints[i].loc.l=-1; /* unknown */
    add_breakpoint(i);
  }
  mem_set_breakpoint(i);
  return i;
}	

/* deleting a breakpoint in a file */
void del_file_breakpoint(int file_no, int line_no, int mask)
{ int i;
  for (i=0; i<blimit;i++)
    if ( breakpoints[i].file_no==file_no && 
		 breakpoints[i].line_no==line_no)
	{ breakpoints[i].bkpt&=~mask;
      mem_set_breakpoint(i);
      if (breakpoints[i].bkpt==0) 
	  { remove_breakpoint(i);
	  }
	  else
        InvalidateRect(hBreakList,NULL,FALSE);
      return;
    }
}

/* interfacing the symbol table with breakpoints */
int break_at_symbol(int file_no,char *symbol)
{ sym_node *sym=find_symbol(symbol,file_no);
  if (sym!=NULL&& sym->link==DEFINED)
  { int i=set_file_breakpoint(sym->file_no,sym->line_no,exec_bit);
	ed_set_break(sym->file_no,sym->line_no,exec_bit);
	return 1;
  }
  else
    return 0;
}




/* Interfacing the breakpoints with the GUI */
static void break_main(int file_no)
{ if (file2loading(file_no))
	break_at_symbol(file_no,":Main");
}

/* call after loading the object files in the debugger */
void update_breakpoints(void)
{ if (break_at_Main) 
	  for_all_files(break_main);
  ed_refresh_breaks(); /* syncronize editor with breakpoints */
  mem_sync_breakpoints(); /* synchronize breakpoints with mem */
}

#define MAX_BREAK_NAME 32

char *break_name(int i)
{ static char name[MAX_BREAK_NAME];
  if (!loc_unknown(i))
    sprintf_s(name,MAX_BREAK_NAME,"#%04X %04X %04X %04X",
	          (breakpoints[i].loc.h>>16)&0xFFFF, 
	          breakpoints[i].loc.h&0xFFFF, 
			  (breakpoints[i].loc.l>>16)&0xFFFF, 
			  breakpoints[i].loc.l&0xFFFF);
  else if (!file_unknown(i))
	  sprintf_s(name,32,"%s:%3d",unique_name(breakpoints[i].file_no),breakpoints[i].line_no);
  else
	  return "ERROR";
  return name;
}


static void add_breakpoint(int i)
{ LRESULT item;
  //item = (int)SendMessage(hBreakList,LB_FINDSTRING,-1,i);
  //if(item!=LB_ERR) return;
  if (hBreakList==NULL) return;
  item = SendMessage(hBreakList,LB_ADDSTRING,0,i);
}

static void del_breakpoint(int i)
{ LRESULT item;
  if (hBreakList==NULL) return;
  item = (int)SendMessage(hBreakList,LB_FINDSTRINGEXACT,-1,i);
  if(item!=LB_ERR) 
    SendMessage(hBreakList,LB_DELETESTRING,item,0);
}
  
static POINT list={0,0}; /* top left coordinates of list box */
static void resize(HWND hDlg, int w, int h)
{ 
  MoveWindow(hBreakList,list.x,list.y,w-2*list.x,h-list.y-list.x,TRUE);
}


INT_PTR CALLBACK    
BreakpointsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ static int min_w, min_h;
  static HANDLE hBreakX,hBreakR,hBreakW,hBreakT;

  switch ( message )
  { case WM_INITDIALOG:
      { RECT r,d;
		int i;
		hBreakList=GetDlgItem(hDlg,IDC_LIST_BREAKPOINTS);
	    GetWindowRect(hBreakList,&d);
		list.x=d.left; list.y=d.top;
		ScreenToClient(hDlg,&list);
	  	GetWindowRect(hDlg,&r);
	    min_w = r.right-r.left;
	    min_h = r.bottom-r.top;
		resize(hDlg,min_w,min_h);
		for (i=0; i<blimit;i++) add_breakpoint(i);
      }
	    hBreakX = LoadImage(hInst, MAKEINTRESOURCE(IDI_BREAKX),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	    hBreakR = LoadImage(hInst, MAKEINTRESOURCE(IDI_BREAKR),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	    hBreakW = LoadImage(hInst, MAKEINTRESOURCE(IDI_BREAKW),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	    hBreakT = LoadImage(hInst, MAKEINTRESOURCE(IDI_BREAKT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
        SendDlgItemMessage(hDlg,IDC_EXEC,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hBreakX);
        SendDlgItemMessage(hDlg,IDC_READ,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hBreakR);
        SendDlgItemMessage(hDlg,IDC_WRITE,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hBreakW);
        SendDlgItemMessage(hDlg,IDC_TRACE,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hBreakT);

      return TRUE;
  case WM_CLOSE:
      hBreakpoints=NULL;
	  hBreakList=NULL;
	  DestroyWindow(hDlg);
	  CheckMenuItem(hMenu,ID_VIEW_BREAKPOINTS,MF_BYCOMMAND|MF_UNCHECKED);
	  DestroyIcon(hBreakX);
	  DestroyIcon(hBreakR);
	  DestroyIcon(hBreakW);
	  DestroyIcon(hBreakT);
	  return TRUE;
  case WM_SIZE:
	  resize(hDlg,LOWORD(lparam),HIWORD(lparam));
	  return TRUE;
  case WM_COMMAND:
      if (wparam==IDC_ADD)
	  { char str[22]; 
	    uint64_t u;
		int i;
		octa loc;
	    GetDlgItemText(hDlg,IDC_ADDRESS,str,22);
		u=hex_to_uint64(str);
		loc.h=(tetra)((u>>32)&0xFFFFFFFF);
		loc.l=(tetra)(u&0xFFFFFFFF);
        i= find_loc_breakpoint(loc);
		if (i<0)
		{ i = new_breakpoint();
		  breakpoints[i].loc=loc;
		  add_breakpoint(i);
		}
		breakpoints[i].bkpt|=exec_bit;
        mem_set_breakpoint(i);
        return TRUE;
	  }
	  else if (wparam==IDC_REMOVE)
	  { int i, item;
	    item = (int)SendMessage(hBreakList,LB_GETCURSEL,0,0);
        if (item==LB_ERR) return TRUE;
		i = (int)SendMessage(hBreakList,LB_GETITEMDATA,item,0);
		if (!file_unknown(i))
		  ed_set_break(breakpoints[i].file_no,breakpoints[i].line_no,0);
		breakpoints[i].bkpt=0;
        mem_set_breakpoint(i);
		remove_breakpoint(i);
	  }
	  else if (wparam==IDC_EXEC||wparam==IDC_READ||wparam==IDC_WRITE||wparam==IDC_TRACE)
	  {	int i, item;
	    item = (int)SendMessage(hBreakList,LB_GETCURSEL,0,0);
        if (item==LB_ERR) return TRUE;
		i = (int)SendMessage(hBreakList,LB_GETITEMDATA,item,0);
		if (wparam==IDC_EXEC) breakpoints[i].bkpt^=exec_bit; 
		else if (wparam==IDC_READ)breakpoints[i].bkpt^=read_bit; 
		else if (wparam==IDC_WRITE)breakpoints[i].bkpt^=write_bit; 
		else if (wparam==IDC_TRACE)breakpoints[i].bkpt^=trace_bit; 
	    if (!file_unknown(i))
			ed_set_break(breakpoints[i].file_no,breakpoints[i].line_no,breakpoints[i].bkpt);
        mem_set_breakpoint(i);
        InvalidateRect(hBreakList,NULL,FALSE);
	  }
	  else if (LOWORD(wparam)==IDC_LIST_BREAKPOINTS)
	  { if (HIWORD(wparam)== LBN_DBLCLK)
	    { int item, i;
		  item = (int)SendMessage(hBreakList,LB_GETCURSEL,0,0);
	      if (item!=LB_ERR)
		  i = (int)SendMessage(hBreakList,LB_GETITEMDATA,item,0);
		  if (i>=0)
		  { set_edit_file(breakpoints[i].file_no);
		    ed_show_line(breakpoints[i].line_no);
		  }
	    }
	  }
     break;
  case WM_GETMINMAXINFO:
	{ MINMAXINFO *p = (MINMAXINFO *)lparam;
	  p->ptMinTrackSize.x = min_w;
      p->ptMinTrackSize.y = min_h;
	  p->ptMaxTrackSize.x=p->ptMinTrackSize.x;
	  p->ptMaxTrackSize.y=p->ptMinTrackSize.y;
	}
	return 0;
  case WM_MEASUREITEM: 
	  { LPMEASUREITEMSTRUCT lpmis; 
        lpmis = (LPMEASUREITEMSTRUCT) lparam; 
        lpmis->itemHeight = 16+4; 
        return TRUE; 
	  }
  case WM_COMPAREITEM:
	  { LPCOMPAREITEMSTRUCT lpc = (LPCOMPAREITEMSTRUCT) lparam;
	    int i = (int)(lpc->itemData1);
		int j = (int)(lpc->itemData2);
	    if (i==j) return (BOOL)0;
		if (i<0 || i>=blimit) return (BOOL)-1;
		if (j<0 || j>=blimit) return (BOOL)+1;
        if (breakpoints[i].file_no < breakpoints[j].file_no) return (BOOL)-1;
        if (breakpoints[i].file_no > breakpoints[j].file_no) return (BOOL)+1;
		if (breakpoints[i].file_no==-1)
		{ if (breakpoints[i].loc.h < breakpoints[j].loc.h) return (BOOL)-1;
		  if (breakpoints[i].loc.h > breakpoints[j].loc.h) return (BOOL)+1;
		  if (breakpoints[i].loc.l < breakpoints[j].loc.l) return (BOOL)-1;
		  if (breakpoints[i].loc.l > breakpoints[j].loc.l) return (BOOL)+1;
		  return (BOOL)0;
		}
		if (breakpoints[i].line_no < breakpoints[j].line_no) return (BOOL)-1;
		if (breakpoints[i].line_no > breakpoints[j].line_no) return (BOOL)+1;
		return (BOOL)0;
	  }
   case WM_DRAWITEM: 
	   { LPDRAWITEMSTRUCT lpdis; 
		 int i;
		 int x=0;
		 char *name;
		 unsigned char bkpt;
		 COLORREF oldBC;
		 lpdis = (LPDRAWITEMSTRUCT) lparam;
         if (lpdis->itemID == -1) break;
         switch (lpdis->itemAction) 
         { case ODA_SELECT: 
           case ODA_DRAWENTIRE: 
			   		i = (int)SendMessage(hBreakList,LB_GETITEMDATA,lpdis->itemID,0);
					bkpt=breakpoints[i].bkpt;
                    if (lpdis->itemState & ODS_SELECTED) 
					     oldBC=SetBkColor(lpdis->hDC,RGB(0x80,0x80,0xFF)); 
					else
					     oldBC=SetBkColor(lpdis->hDC,RGB(0xFF,0xFF,0xFF)); 
					SetTextAlign(lpdis->hDC,TA_LEFT|TA_TOP|TA_NOUPDATECP);
					x = (bkpt&0x5)+((bkpt&0xA)>>1); /* count bits in bkpt */
					x = (x&0x3)+((x&0xC)>>2);
					x = x*16+4;
					name=break_name(i);
					ExtTextOut(lpdis->hDC,x,lpdis->rcItem.top+2,ETO_OPAQUE,&lpdis->rcItem,name,(int)strlen(name),NULL);
					x=0;
                    if (bkpt&exec_bit)
					{ DrawIconEx(lpdis->hDC,x, lpdis->rcItem.top+2,hBreakX,16,16,0,NULL,DI_NORMAL);
					 x+=16;
					}
					if (bkpt&read_bit)
					{ DrawIconEx(lpdis->hDC,x, lpdis->rcItem.top+2,hBreakR,16,16,0,NULL,DI_NORMAL);
					  x+=16;
					}
					if (bkpt&write_bit)
					{ DrawIconEx(lpdis->hDC,x, lpdis->rcItem.top+2,hBreakW,16,16,0,NULL,DI_NORMAL);
					  x+=16;
					}
					if (bkpt&trace_bit)
					{ DrawIconEx(lpdis->hDC,x, lpdis->rcItem.top+2,hBreakT,16,16,0,NULL,DI_NORMAL);
					  x+=16;
					}

				    SetBkColor(lpdis->hDC,oldBC); 

                    break; 
 
                case ODA_FOCUS: 
 
                    // Do not process focus changes. The focus caret 
                    // (outline rectangle) indicates the selection. 
                    // The IDOK button indicates the final 
                    // selection. 
 
                    break; 
            } 
            return TRUE; 
	   }

  }
  return FALSE;
}


 HWND hBreakpoints=NULL;

 void create_breakpoints(void)
 { if (hBreakpoints!=NULL) return;
   sp_create_options(0,1,0.2,0,NULL);
   hBreakpoints = CreateDialog(hInst,MAKEINTRESOURCE(IDD_BREAKPOINTS),hSplitter,BreakpointsDialogProc); 
 }

