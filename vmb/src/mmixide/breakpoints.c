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

/* the breakpoint window */
static HWND hBreakList=NULL;
/* functions to update the breakpoint window */
static void add_breakpoint(int i);
static void del_breakpoint(int i);
static void change_breakpoint(int i);

/* The datastructure representing all breakpoints */
#define MAX_BREAKPOINTS 256
static
struct {
	int file_no;
	int line_no;
	octa loc, loc_last; /* loc <= location <= loc_last */
	unsigned char bkpt;
	unsigned char mark; /* used for mark and sweep */
} breakpoints[MAX_BREAKPOINTS];

/* currently we do not support loc ranges that cross 4GByte boundaries
   so loc.h == loc_last.h for all breakpoints 
   (actualy, we dont maintain loc_last.h)
*/

/* there are two kinds of breakpoinst with a file!=-1 and with a loc!=-1 */
#define loc_unknown(i) (breakpoints[i].loc.h==-1 && breakpoints[i].loc.l==-1)
#define file_unknown(i) (breakpoints[i].file_no<0)
/* the other breakpoints are unused */
#define unused(i) (loc_unknown(i)&& file_unknown(i))

/* all used breakpoints are between 0 and blimit-1,
   the number of used breakpoints id bcount. */
static int blimit = 0, bcount=0;


/* there is are important invariants:
   - for each file and line there is at most one breakpoint entry in the list.
     (entries may have overlapping location ranges, but then all but one
	  must not have a file_no).

   We ensure this by never changing the file,line entry and we call new_breakpoint
   and we call new_breakpoint only if the file, line entry does not exist
   or with an unkonwn file.

   - if the mem_tetra at loc has a nonzero bkpt field,
     then there must be an entry in the breakpoints list with that (valid) loc
	 within its range.

   We ensure this in three ways:
   1) we have one function that sets the bkpt field
      in a mem_tetra using the loc range from the breakpoints list.
	  no other function sets the bkpt field to anything but zero.
   2) before we remove an entry in the breakpoints list,
      we set the bkpt field in the mem_tetra to zero for all locs in its range. 
   3) if we never shrink the loc range in the breakpoint list.
*/

static void mem_set_breakpoint(int i)
{ if (!loc_unknown(i))
  { octa loc;
    for (loc=breakpoints[i].loc;loc.l<=breakpoints[i].loc_last.l;loc.l+=4)
      loc2bkpt(loc)=breakpoints[i].bkpt;
  }
}

static void zero_breakpoint(int i)
{  if (!loc_unknown(i))
  { octa loc;
    for (loc=breakpoints[i].loc;loc.l<=breakpoints[i].loc_last.l;loc.l+=4)
      loc2bkpt(loc)=0;
  }
}

/* destructing an old breakpoint */
static void remove_breakpoint(int i)
{ 
  if (i<0 ||i>=MAX_BREAKPOINTS) return;
  zero_breakpoint(i);
  del_breakpoint(i);
  if (blimit==i+1) 
    blimit=i;
  else
  { breakpoints[i].file_no=-1; /* file unknown */
    breakpoints[i].loc=neg_one; /* loc unknown */
  }
  bcount--;
}

/* constructing a new breakpoint */
static int new_breakpoint(int file_no, int line_no,	octa loc, unsigned char bkpt)
{ int n;
  if (bcount>=MAX_BREAKPOINTS)
  { win32_error(__LINE__,"Too many breakpoints");
	remove_breakpoint(bcount-1);/* reuse last breakpoint */
  }
  n=blimit;
  if (blimit==bcount) 
	  blimit++;
  else
	  do n--; while(!unused(n));
  breakpoints[n].file_no=file_no;
  breakpoints[n].line_no=line_no;
  breakpoints[n].loc=loc;
  breakpoints[n].bkpt=bkpt;
  breakpoints[n].mark=0;
  if (!unused(n))
  {	bcount++;
    add_breakpoint(n);
  }
  return n;
}

#if 0
/* probably not used */


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

static void mem_file_line_breakpoints(int file_no, int line_no, int bits)
/* set breakpoints for whole line */
{ if (!mmix_active()) return;
  break_bits=bits;
  break_file_no=file_no; 
  break_line_no=line_no;
  mem_iterator(file_line_break);
}	

static void mem_file_breakpoints(int file_no, int bits)
/* set breakpoint for whole file */
{ if (!mmix_active()) return;
  break_bits=bits;
  break_file_no=file_no; 
  mem_iterator(file_break);
}	

#endif

void add_line_loc(int file_no,int line_no, octa loc)
/* update breakpoints with locations and
   set mem_tetras with bkpt's
*/
{ int i;

  /* we iterate over the breakpoints, this could be done more efficiently
     by sorting them by file and line */
  mem_tetra* ll=mem_find(loc);
  ll->file_no=file_no;
  ll->line_no=line_no;
  for (i=0; i<blimit;i++)
    if (breakpoints[i].file_no==file_no && 
		breakpoints[i].line_no==line_no)
    { 
	  if (loc_unknown(i))
	  { breakpoints[i].loc=loc;
        breakpoints[i].loc_last.l=loc.l;
		ll->bkpt=breakpoints[i].bkpt;
      }
      else if (breakpoints[i].loc_last.l < loc.l && 
	         breakpoints[i].loc.h == loc.h) 
	  {  breakpoints[i].loc_last.l=loc.l;
	     ll->bkpt=breakpoints[i].bkpt;
	  }
    }
}


/* auxiliar variable and function used in set_loc_breakpoint */
static int  the_i;
static void file_line_loc(octa loc, mem_tetra *dat)
{ if (dat->line_no==breakpoints[the_i].line_no &&
	  dat->file_no==breakpoints[the_i].file_no) 
  { if (loc_unknown(the_i))
    { breakpoints[the_i].loc=loc;
      breakpoints[the_i].loc_last.l=loc.l;
    }
    else if (breakpoints[the_i].loc_last.l < loc.l && 
             breakpoints[the_i].loc.h == loc.h)
      breakpoints[the_i].loc_last.l=loc.l;
  }
}

int set_loc_breakpoint(int i)
/* iterate over the memory and set the location for
   breakpoint i based on its file and line.
   return 1 if successufull, 0 if not.
   This can be an expensive function use only when necessary 
*/
{   if (!loc_unknown(i)) return 1;
    if (file_unknown(i)) return 0;
    the_i=i;
    mem_iterator(file_line_loc);
	return !loc_unknown(i);
}

/* interfacing the breakpoint list with the editor and
   functions to call if a break marker in the editor changes */

/* removing all loc information refering to a file */
void remove_loc_breakpoints(int file_no)
{ int i=0;
  if (file_no<0) return;
  for (i=0;i<blimit;i++)
	if (breakpoints[i].file_no==file_no)
	{ zero_breakpoint(i);
	  breakpoints[i].loc=neg_one;
	}
}


void mark_all_file_breakpoints(int file_no)
{ int i=0;
  if (file_no<0) return;
  for (i=0;i<blimit;i++)
	if (breakpoints[i].file_no==file_no)
	  breakpoints[i].mark = 1;
}

void sweep_all_file_breakpoints(int file_no)
{ int i=0;
  if (file_no<0) return;
  for (i=0;i<blimit;i++)
	if (breakpoints[i].file_no==file_no && breakpoints[i].mark)
	  remove_breakpoint(i);
}


/* removing all breakpoints refering to a file */
void remove_file_breakpoints(int file_no)
{ int i=0;
  if (file_no<0) return;
  for (i=0;i<blimit;i++)
	if (breakpoints[i].file_no==file_no)
	  remove_breakpoint(i);
}


/* setting a breakpoint in a file
   call with bits, 0 to set bits
   call with bits,bits, to selectively add bits 
*/
void set_file_breakpoint(int file_no, int line_no,  int bits, int mask)
{ int i;
  for (i=0; i<blimit;i++)
    if ( breakpoints[i].file_no==file_no && 
		 breakpoints[i].line_no==line_no)
    {  breakpoints[i].bkpt=(breakpoints[i].bkpt & ~mask)|bits;
       breakpoints[i].mark=0; /* prepare for sweep */
       change_breakpoint(i);
	   if (mmix_active())
         mem_set_breakpoint(i); /* for an existing breakpoint, we assume known loc */ 
	   return;
    }
  /* no breakpoint for this file and line exists. */
  i = new_breakpoint(file_no, line_no,neg_one,bits);
  if (mmix_active() && set_loc_breakpoint(i))
    mem_set_breakpoint(i);
}	

/* deleting a breakpoint in a file */
void del_file_breakpoint(int file_no, int line_no, int mask)
{ int i;
  for (i=0; i<blimit;i++)
    if ( breakpoints[i].file_no==file_no && 
		 breakpoints[i].line_no==line_no)
	{ breakpoints[i].bkpt&=~mask;
      if (breakpoints[i].bkpt==0) 
	  { remove_breakpoint(i);
	  }
	  else 
	  { if (mmix_active())
          mem_set_breakpoint(i); /* for an existing breakpoint, we assume known loc */
        change_breakpoint(i);
	  }
	  return;
    }
}

/* interfacing the symbol table with breakpoints */
int break_at_symbol(int file_no,char *symbol)
{ sym_node *sym=find_symbol(symbol,file_no);
  if (sym!=NULL&& sym->link==DEFINED)
  { set_file_breakpoint(sym->file_no,sym->line_no,exec_bit,exec_bit);
	ed_set_break(sym->file_no,sym->line_no,exec_bit);
	return 1;
  }
  else
    return 0;
}


/* auxiliar function for update_breakpoints */
static void break_main(int file_no)
{ if (file2loading(file_no))
	break_at_symbol(file_no,":Main");
}

/* call before running/debugging  */
void update_breakpoints(void)
{ if (break_at_Main) 
	  for_all_files(break_main);
  ed_refresh_breaks(); /* syncronize Markers in editor with breakpoints */
}

void sync_breakpoints(void)
/* syncronizing all breakpoints with the loaded memory */
{  int i;
   for (i=0;i<blimit;i++)
	   mem_set_breakpoint(i);
}


/* function to determine the name for a breakpoint as displayed in the breakpoint window */

#define MAX_BREAK_NAME 64

char *break_name(int i)
{ static char name[MAX_BREAK_NAME];
  char *str=name;
  int n =MAX_BREAK_NAME-1;
  int k;
  if (!file_unknown(i))
  { k = sprintf_s(str,n,"%s(%d)",unique_name(breakpoints[i].file_no),breakpoints[i].line_no);
    str +=k;
	n -=k;
  }
  if (!loc_unknown(i))
  { unsigned int hhb, hlb, mh, ml,l;
    if (n<MAX_BREAK_NAME-1)
	{ k=sprintf_s(str,n,": "); str+=k; n -=k; }
    hhb = (breakpoints[i].loc.h>>24)&0xFF;
    hlb = (breakpoints[i].loc.h>>16)&0xFF;
    mh  =   breakpoints[i].loc.h&0xFFFF;
    ml  =   (breakpoints[i].loc.l>>16)&0xFFFF;
    l   =   breakpoints[i].loc.l&0xFFFF;

	k=sprintf_s(str,n,"#%02X",hhb); str+=k; n -=k; 
	if (hlb)
	{ k=sprintf_s(str,n,"%02X",hhb); str+=k; n -=k; }
	if (mh)
	{ k=sprintf_s(str,n,"%04X",mh); str+=k; n -=k; }
	if (ml)
	{ k=sprintf_s(str,n,"%04X",ml); str+=k; n -=k; }
    if (!(hlb&&mh&&ml)) 
	{ k=sprintf_s(str,n,"..."); str+=k; n -=k; }
	k=sprintf_s(str,n,"%04X",l); str+=k; n -=k;
	if (breakpoints[i].loc.l<breakpoints[i].loc_last.l)
	{ k=sprintf_s(str,n,"->...%04X",breakpoints[i].loc_last.l); str+=k; n -=k; }
  } 
  if (n==MAX_BREAK_NAME-1)
	  return "ERROR";
  return name;
}

/* functions to update the breakpoint window */

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

static void change_breakpoint(int i)
{  if (hBreakList==NULL) return;
     InvalidateRect(hBreakList,NULL,FALSE);
}
  
static POINT list={0,0}; /* top left coordinates of list box */
static void resize(HWND hDlg, int w, int h)
{ 
  MoveWindow(hBreakList,list.x,list.y,w-2*list.x,h-list.y-list.x,TRUE);
}

static int find_loc_breakpoint(octa loc)
/* find a breakpoint with the given location as its range */
{ int i;
  for (i=0; i<blimit;i++)
    if (breakpoints[i].loc.h==loc.h &&
		breakpoints[i].loc.l==loc.l && breakpoints[i].loc_last.l==loc.l  
		)
	 return i;
  return -1;
}

/* the window procedure for the breakpoints window */

INT_PTR CALLBACK    
BreakpointsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ static int min_w, min_h;
  static HANDLE hBreakX,hBreakR,hBreakW,hBreakT;

  switch ( message )
  { case WM_INITDIALOG:
	    hBreakX = LoadImage(hInst, MAKEINTRESOURCE(IDI_BREAKX),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	    hBreakR = LoadImage(hInst, MAKEINTRESOURCE(IDI_BREAKR),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	    hBreakW = LoadImage(hInst, MAKEINTRESOURCE(IDI_BREAKW),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	    hBreakT = LoadImage(hInst, MAKEINTRESOURCE(IDI_BREAKT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
        SendDlgItemMessage(hDlg,IDC_EXEC,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hBreakX);
        SendDlgItemMessage(hDlg,IDC_READ,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hBreakR);
        SendDlgItemMessage(hDlg,IDC_WRITE,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hBreakW);
        SendDlgItemMessage(hDlg,IDC_TRACE,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hBreakT);
      { RECT r,d;
		hBreakList=GetDlgItem(hDlg,IDC_LIST_BREAKPOINTS);
	    GetWindowRect(hBreakList,&d);
		list.x=d.left; list.y=d.top;
		ScreenToClient(hDlg,&list);
	  	GetWindowRect(hDlg,&r);
	    min_w = r.right-r.left;
	    min_h = r.bottom-r.top;
	  }
	    resize(hDlg,min_w,min_h);
	  { int i;
        for (i=0; i<blimit;i++) add_breakpoint(i);
	  }
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
		  i = new_breakpoint(-1,0,loc,exec_bit);
		else
		{ breakpoints[i].bkpt|=exec_bit;
		  if (!file_unknown(i))
		    ed_set_break(breakpoints[i].file_no,breakpoints[i].line_no,breakpoints[i].bkpt);
		  change_breakpoint(i);
		}
		if(mmix_active())
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
        if(mmix_active())
			mem_set_breakpoint(i);
		change_breakpoint(i);
	  }
	  else if (LOWORD(wparam)==IDC_LIST_BREAKPOINTS)
	  { if (HIWORD(wparam)== LBN_DBLCLK)
	    { int item, i;
		  item = (int)SendMessage(hBreakList,LB_GETCURSEL,0,0);
	      if (item!=LB_ERR)
		  i = (int)SendMessage(hBreakList,LB_GETITEMDATA,item,0);
		  if (i>=0 && !file_unknown(i))
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
		if (breakpoints[i].file_no!=-1)
		{ if (breakpoints[i].line_no < breakpoints[j].line_no) return (BOOL)-1;
		  if (breakpoints[i].line_no > breakpoints[j].line_no) return (BOOL)+1;
		}
		if (breakpoints[i].loc.h < breakpoints[j].loc.h) return (BOOL)-1;
		if (breakpoints[i].loc.h > breakpoints[j].loc.h) return (BOOL)+1;
		if (breakpoints[i].loc.l < breakpoints[j].loc.l) return (BOOL)-1;
		if (breakpoints[i].loc.l > breakpoints[j].loc.l) return (BOOL)+1;
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

/* creating the breakpoints window */
 HWND hBreakpoints=NULL;

 void create_breakpoints(void)
 { if (hBreakpoints!=NULL) return;
   sp_create_options(0,1,0.2,0,NULL);
   hBreakpoints = CreateDialog(hInst,MAKEINTRESOURCE(IDD_BREAKPOINTS),hSplitter,BreakpointsDialogProc); 
 }

