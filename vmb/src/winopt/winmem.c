#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "resource.h"
#include "winopt.h"
//include "float.h"
#include "inspect.h"
#include "dedit.h"
#include "winde.h"

#pragma warning(disable : 4996)

char *format_names[]={"Hex","Ascii","Unsigned","Signed","Float"};
char *chunk_names[]={"BYTE","WYDE","TETRA","OCTA"};


static void sb_range(inspector_def *insp)
/* determine and set scrollbar range as virtual number of insp->lines */
{ SCROLLINFO si;
  unsigned int mem_size; 

  /* determine range */
  if (insp->hWnd==NULL) return;
  if (insp->size==0)
  { insp->mem_base=0;
	mem_size = 0;
	insp->sb_base=0;
	insp->sb_cur=0;
	insp->sb_rng=0;
  }
  else if (insp->regs!=NULL)
  { insp->sb_rng = insp->num_regs-1;
    if (insp->sb_rng<0) insp->sb_rng=0;
    insp->sb_base =0;
	if (insp->sb_cur>insp->sb_rng-(insp->lines-1)) insp->sb_cur=insp->sb_rng-(insp->lines-1);
	if (insp->sb_cur<0) insp->sb_cur=0;
    insp->mem_base = insp->regs[insp->sb_cur].offset;
	mem_size = insp->size -insp->mem_base;
  }
  else 
  { unsigned int page_range;
    page_range= insp->lines*insp->line_range;
    if (page_range*10<insp->size)
      insp->sb_rng = page_range*10/insp->line_range-1;
    else if (page_range<insp->size)
      insp->sb_rng = (insp->size+insp->line_range-1)/insp->line_range-1;
    else
      insp->sb_rng = (page_range+insp->line_range-1)/insp->line_range-1;
    /* determine address of first line */
	if (insp->sb_cur<0)
	{ if (insp->sb_base > -insp->sb_cur*insp->line_range)
	  { insp->sb_base = insp->sb_base +insp->sb_cur*insp->line_range;
	    insp->sb_cur=0;
	  } 
	  else
	  { insp->sb_base = 0;
	    insp->sb_cur=0;
	  } 
	}
    else if (insp->sb_cur>insp->sb_rng-(insp->lines-1))
	{ int d = insp->sb_cur-insp->sb_rng+(insp->lines-1);
	  if (insp->sb_base+d*insp->line_range + insp->sb_rng*insp->line_range<insp->size)
	    insp->sb_base = insp->sb_base + d*insp->line_range;
	  else if (insp->size> insp->sb_rng*insp->line_range)
		insp->sb_base=insp->size- insp->sb_rng*insp->line_range;
	  else
	    insp->sb_base=0;
	  insp->sb_cur=insp->sb_rng-(insp->lines-1);
	}
    insp->mem_base=insp->sb_base+insp->sb_cur*insp->line_range;
	mem_size = page_range;
	if (mem_size > insp->size -insp->mem_base)
	{ mem_size = insp->size -insp->mem_base;
      InvalidateRect(insp->hWnd,NULL,TRUE);
	}
  }

  if (mem_size>insp->mem_size)
  { insp->mem_buf=realloc(insp->mem_buf,mem_size);
    if (insp->mem_buf==NULL)
		win32_fatal_error(__LINE__,"Out of memory");
	memset(insp->mem_buf+insp->mem_size,0,mem_size-insp->mem_size);
  }
  insp->mem_size=mem_size;
  si.cbSize=sizeof(si);
  si.fMask=SIF_PAGE|SIF_POS|SIF_RANGE;
  si.nMin=0;
  si.nMax=insp->sb_rng;
  si.nPage=insp->lines;
  si.nPos=insp->sb_cur;
  SetScrollInfo(GetDlgItem(insp->hWnd,IDC_MEM_SCROLLBAR),SB_CTL,&si,TRUE);
}



static void sb_move(HWND hMemory,inspector_def *insp,WPARAM wparam)
{ switch (LOWORD(wparam)) 
	  { case SB_LINEUP:
	      insp->sb_cur--; /* we catch a negative value in sb_range */
		  break;
        case SB_LINEDOWN: 
	      insp->sb_cur++; /* we catch a too big value in sb_range */
		  break;
		case SB_PAGEUP:
		  insp->sb_cur=insp->sb_cur-insp->lines;
		  break;
        case SB_PAGEDOWN: 
		  insp->sb_cur=insp->sb_cur+insp->lines;
		  break;
		case SB_TOP:
		  insp->sb_cur=0;
		  break;
        case SB_BOTTOM: 
		  insp->sb_cur=insp->sb_rng-insp->lines-1;
		  break;
		case SB_THUMBTRACK: /* HIWORD(wparam) is 0 to sb_range */
		   insp->sb_cur = HIWORD(wparam);
          break;
		default:
		case SB_ENDSCROLL:
		case SB_THUMBPOSITION:
		  return;
	  }
	   adjust_mem_display(insp);
}


static uint64_t adjust_goto_addr(HWND hMemory,inspector_def *insp, uint64_t goto_addr)
{  char hexstr[20];
	if (!insp->change_address)
    { if (goto_addr<insp->address)
      { goto_addr=insp->address;
      }
      else if (goto_addr>=insp->address+insp->size)
      { goto_addr=insp->address+(insp->size-1);
      }
    }
    uint64tohex(goto_addr,hexstr);
    SetDlgItemText(hMemory,IDC_GOTO,hexstr);
	return goto_addr;
}




static void refresh_old_mem(inspector_def *insp)
{ if (insp->old_size<insp->mem_size)
  { insp->old_mem = realloc(insp->old_mem,insp->mem_size);
    if (insp->old_mem==NULL)
		win32_fatal_error(__LINE__,"Out of memory");
  }
  memmove(insp->old_mem,insp->mem_buf,insp->mem_size);
  insp->old_base=insp->mem_base;
  insp->old_size=insp->mem_size;
}

  

void set_edit_rect(inspector_def *insp);

void adjust_mem_display(inspector_def *insp)
{ 
  if (insp->hWnd==NULL) return;
  sb_range(insp);
  refresh_old_mem(insp);
  if (insp->get_mem) 
	insp->get_mem(insp->mem_base,insp->mem_size,insp->mem_buf);
  else if (insp->mem_size>0 && insp->mem_buf!=NULL)
    memset(insp->mem_buf,0,insp->mem_size);
  set_edit_rect(insp);
  InvalidateRect(insp->hWnd,NULL,insp->regs!=NULL);
}





/* these depend only on the font, which is the same for all inspectors */
static int line_height=0, digit_width, address_width, separator_width, separator_height, top_width; 
int mem_min_width=0,mem_min_height=0;
void set_mem_font_metrics(void)
{ SIZE size;
  HDC hDC=GetDC(NULL);
  HFONT hfnt = GetStockObject(ANSI_FIXED_FONT); 
  HFONT holdfnt=SelectObject(hDC, hfnt);
  GetTextExtentPoint32(hDC,"0",1,&size);
  separator_height=2;
  digit_width=size.cx;
  line_height=size.cy+separator_height;
  separator_width = digit_width/2;
  GetTextExtentPoint32(hDC,"0x0000000000000000:",19,&size);
  address_width=size.cx;
  top_width=28;
  SelectObject(hDC,holdfnt);
  ReleaseDC(NULL,hDC);
  mem_min_width=25*digit_width+separator_width;
  mem_min_height= top_width+separator_height+line_height;
}


int chunk_len(enum mem_fmt f, int chunk_size)
{ int column_digits;
  switch (f)
  { case hex_format: column_digits=2*chunk_size; break;
    case ascii_format: column_digits=1*chunk_size; break;
	case signed_format:
      if (chunk_size==1) column_digits=4;
      else column_digits=1+5*chunk_size/2;
	  break;
	case unsigned_format:
      if (chunk_size==1) column_digits=3;
      else column_digits=5*chunk_size/2;
	  break;
	default:
	case float_format:
       column_digits=19;
    break;
  }
  return column_digits;
}


static void resize_memory_dialog(HWND hMemory,inspector_def *insp)
{ int sbw;
  RECT sbRect;

  GetWindowRect(GetDlgItem(hMemory,IDC_MEM_SCROLLBAR),&sbRect);
  sbw=sbRect.right-sbRect.left;
  MoveWindow(GetDlgItem(hMemory,IDC_MEM_SCROLLBAR),
	         insp->width-sbw,top_width,sbw,insp->height-top_width,TRUE);


  insp->lines = (insp->height-top_width)/line_height;
  if (insp->lines<1) insp->lines=1;
  
  insp->column_digits=chunk_len(insp->format,1<<insp->chunk);

  insp->column_width=insp->column_digits*digit_width+separator_width;
  insp->columns = (insp->width-sbw-address_width-separator_width)/insp->column_width;
  if (insp->columns<1)
  { insp->columns=1;
    insp->column_width=insp->width-sbw-address_width-separator_width;
	if (insp->column_digits*digit_width>insp->column_width)
      insp->column_digits=insp->column_width/digit_width;
  }
  insp->line_range = insp->columns*(1<<insp->chunk);
  InvalidateRect(hMemory,NULL,TRUE); /* can possibly be removed */
  adjust_mem_display(insp);
}





/* Color management */

#define COLD RGB(0xFD,0xFD,0xFF)
#define HOT RGB(0xFF,0xA0,0xA0)

void update_old_mem(inspector_def *insp)
{ if (insp->old_size<insp->mem_size)
  { insp->old_mem = realloc(insp->old_mem,insp->mem_size);
    if (insp->old_mem==NULL)
		win32_fatal_error(__LINE__,"Out of memory");
  }
   /* newly displayed memory contents is cold, (copy from insp->mem_buf)
      only previously displayed content that has changed is hot */
   if (insp->old_base<insp->mem_base)
   { /* move overlap of old and new down*/
      unsigned int dist = insp->mem_base-insp->old_base; 
      if (insp->old_size>dist)
	  { int ovlap = insp->old_size-dist;
	    memmove(insp->old_mem, insp->old_mem+dist,ovlap);
	    memmove(insp->old_mem+ovlap,insp->mem_buf+ovlap,insp->mem_size-ovlap);
	  }
	  else
	    memmove(insp->old_mem,insp->mem_buf,insp->mem_size);
   }
   else if (insp->old_base>insp->mem_base)
   { /* move overlap of old and new up*/
      unsigned int dist = insp->old_base-insp->mem_base; 
      if (insp->old_size>dist)
	  { int ovlap = insp->old_size-dist;
	    memmove(insp->old_mem+dist, insp->old_mem,ovlap);
	    memmove(insp->old_mem,insp->mem_buf,dist);
		if (insp->mem_size>insp->old_size)
	      memmove(insp->old_mem+insp->old_size,insp->mem_buf+insp->old_size,insp->mem_size-insp->old_size);
	  }
	  else
	    memmove(insp->old_mem,insp->mem_buf,insp->mem_size);
   }
   else if (insp->mem_size>insp->old_size)
	 memmove(insp->old_mem+insp->old_size,insp->mem_buf+insp->old_size,insp->mem_size-insp->old_size);
   insp->old_base=insp->mem_base;
   insp->old_size=insp->mem_size;
}


static int different(inspector_def *insp,int offset, int size)
{ int i;
  for (i=0;i<size;i++)
  { unsigned int index=offset+i;
    if (index>=insp->old_size || index>=insp->mem_size ||
	    insp->old_mem[offset+i]!=insp->mem_buf[offset+i]) return 1;
  }
  return 0;
}

#define GET2(a)   ((unsigned int)(((a)[0]<<8)+(a)[1]))
#define GET4(a)   ((unsigned int)(((a)[0]<<24)+((a)[1]<<16)+((a)[2]<<8)+(a)[3]))
#define GET8(a)   ((uint64_t)((((uint64_t)GET4(a))<<32)+GET4((a)+4)))

int chunk_to_str(char *str, unsigned char *buf, enum mem_fmt fmt, 
						int chunk_size, int column_digits)
/* prints the data from buf to str using format and chunk size 
   tries to use column_digits characters if column_digits>0. 
   returns the number of characters needed 
*/
{ int j;
  int w;
  switch (fmt)
  { default:
    case hex_format:
      for (j=0;j<chunk_size;j++)
	    sprintf(str+j*2,"%02X",buf[j]);
	    w=chunk_size*2;
	  break;
    case ascii_format:
	  memmove(str,buf,chunk_size);
	  str[chunk_size]=0;
	  w=chunk_size;
	  break;
	case unsigned_format:
      switch (chunk_size)
      { case 1: w=sprintf(str,"%*u",column_digits,buf[0]); break;
        case 2: w=sprintf(str,"%*u",column_digits,GET2(buf)); break;
        case 4: w=sprintf(str,"%*u",column_digits,GET4(buf)); break;
	    default:
        case 8: w = sprintf(str,"%*llu",column_digits,GET8(buf)); break;
      }
	  break;
	case signed_format:
      switch (chunk_size)
      { case 1: w=sprintf(str,"%*d",column_digits,(int8_t)buf[0]); break;
        case 2: w=sprintf(str,"%*d",column_digits,(int16_t)GET2(buf)); break;
        case 4: w=sprintf(str,"%*d",column_digits,(int32_t)GET4(buf)); break;
	    default:
        case 8: w=sprintf(str,"%*lld",column_digits,(int64_t)GET8(buf)); break;
      }
	  break;
	case float_format:
	  if (chunk_size<8)
	    w=f64_to_str(str,f64_from_f32(GET4(buf)),column_digits);
	  else
	    w=f64_to_str(str,GET8(buf),column_digits); 
	  break;
  }
  if (column_digits>0 && w>column_digits)
  {	memset(str,'*',column_digits);
	w=column_digits;
  }
  str[w]=0;
  return w;
}


void update_max_regnames(inspector_def *insp)
{ int i;
  if (insp->regs==NULL) return;
  insp->max_regname=4; /* the basic minimum */
  for (i=0;i<insp->num_regs;i++)
  { int n;
    n = (int)strlen(insp->regs[i].name);
	if (n> insp->max_regname) insp->max_regname=n;
  }
}

void display_registers(inspector_def *insp,HDC hdc)
{ 
  int i,k, nr;
  char str[22]; /* big enough for the largest 8-Byte integer */
  RECT rect;
  nr = insp->num_regs;
  if (nr>insp->lines) nr=insp->lines;
  for (i=0;i<nr;i++)
   { enum mem_fmt format;
	 enum chunk_fmt chunk;
	 int y, chunk_size, x;
     struct register_def *r;
	 r = &insp->regs[insp->sb_cur+i];
     y=top_width+i*line_height;
     x=digit_width*(insp->max_regname+1);
     SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));
     SetBkColor(hdc,GetSysColor(COLOR_BTNFACE));
	 if (r->options&REG_OPT_DISABLED)
	   SetTextColor(hdc,GetSysColor(COLOR_GRAYTEXT));
	 else
 	   SetTextColor(hdc,GetSysColor(COLOR_BTNTEXT));
     SetTextAlign(hdc,TA_RIGHT|TA_NOUPDATECP);
	 rect.top=y;
     rect.left=0;
     rect.right=x;
     rect.bottom=y+line_height-separator_height;
     ExtTextOut(hdc,x,y,
		   ETO_OPAQUE,&rect,r->name,(int)strlen(r->name),NULL);
	 if (r->format==user_format) format=insp->format; else format=r->format;
	 if (r->chunk==user_chunk) chunk=insp->chunk; else chunk=r->chunk;
	 chunk_size=1<<chunk;
	 if (chunk_size>r->size) chunk_size=r->size;
	 x = x+separator_width;
	 for (k=0;k*chunk_size < r->size;k++)
	 { int len;
	   len = chunk_len(format,chunk_size);
	   chunk_to_str(str, insp->mem_buf+r->offset+k*chunk_size-insp->mem_base, format,chunk_size,len);
       if (different(insp,r->offset+k*chunk_size-insp->mem_base,chunk_size))
	     SetBkColor(hdc,HOT);
	   else
	     SetBkColor(hdc,COLD);
	   SetTextAlign(hdc,TA_RIGHT|TA_NOUPDATECP);
	   rect.top=y;
       rect.left=x;
       rect.right=x+len*digit_width;
       rect.bottom=y+line_height-separator_height;
       ExtTextOut(hdc,x+len*digit_width,y,
		   ETO_OPAQUE,&rect,str,(int)strlen(str),NULL);
	   x=x+len*digit_width+separator_width;
	 }
   } 
}
void display_address(inspector_def *insp,HDC hdc)
{ int i; 
  uint64_t addr=insp->address+insp->mem_base;
  char str[22]; /* big enough for the largest 8Byte integer */
  
  SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));
  SetBkColor(hdc,GetSysColor(COLOR_BTNFACE));
  for (i=0;i<insp->lines && (unsigned int)(i*insp->line_range)<insp->mem_size;i++)
  { uint64tohex(addr+i*insp->line_range,str);
    str[18]=':';
	TextOut(hdc,0,top_width+i*line_height,str,19);
  }
}

void display_memory(inspector_def *insp,HDC hdc)
{ 
  int i,k;
  char str[22]; /* big enough for the largest 8Byte integer */
  RECT r;
  int chunk_size=1<<insp->chunk;
  int columns= insp->columns;
  for (i=0;i<insp->lines && (unsigned int)(i*insp->line_range)<insp->mem_size;i++)
   { int y;
     y=top_width+i*line_height;
	 for (k=0;k<columns;k++)
	 { int x,l;
	   if ((unsigned int)((i*columns+k+1)*chunk_size)<= insp->mem_size)
	   { l=chunk_to_str(str, insp->mem_buf+(i*columns+k)*chunk_size, insp->format,chunk_size, insp->column_digits);
         if (different(insp,(i*columns+k)*chunk_size,chunk_size))
	       SetBkColor(hdc,HOT);
	     else
	       SetBkColor(hdc,COLD);
	   }
	   else
	   { str[0]=0;
	     l=0;
         SetBkColor(hdc,COLD);
	   }
	   x = address_width+separator_width+k*insp->column_width;
	   SetTextAlign(hdc,TA_RIGHT|TA_NOUPDATECP);
	   r.top=y;
       r.left=x;
       r.right=x+insp->column_width-separator_width;
       r.bottom=y+line_height-separator_height;

       ExtTextOut(hdc,x+insp->column_width-separator_width,y,
		   ETO_OPAQUE,&r,str,l,NULL);
	 }
   } 
}


void display_data(inspector_def *insp,HDC hdc)
{ if (insp->regs!=NULL)
  { update_old_mem(insp);
    display_registers(insp,hdc);
  }
  else
  { display_address(insp,hdc);
    update_old_mem(insp);
    display_memory(insp,hdc);
  }
}



void set_edit_rect(struct inspector_def *insp)
/* determine the edit rectangle from the edit_offset. */
{ int page_offset;
  int line_offset;
  int edit_line;
  int edit_width;
  SetRectEmpty(&insp->edit_rect);
  if (insp->de_offset<0) return;
  if (insp->regs!=NULL)
  { enum mem_fmt fmt;
    enum mem_chunk chunk;
    fmt = insp->regs[insp->de_offset].format;
	if (fmt==user_format) fmt=insp->format;
	chunk = insp->regs[insp->de_offset].chunk;
	if (chunk == user_chunk) chunk = insp->chunk;
    edit_line=insp->de_offset-insp->sb_cur;
	insp->edit_rect.left=digit_width*(insp->max_regname+1);
	line_offset=0;
	edit_width=chunk_len(fmt,1<<chunk)*digit_width+separator_width;
  }
  else {
	page_offset= insp->de_offset-insp->mem_base;
    if (page_offset<0 || page_offset>=(int)insp->mem_size) return;
    edit_line= page_offset/insp->line_range;
    line_offset= page_offset-edit_line*insp->line_range;
	insp->edit_rect.left=address_width;
	edit_width=insp->column_width;
  }
  insp->edit_rect.top=top_width+edit_line*line_height;
  insp->edit_rect.bottom=insp->edit_rect.top+line_height-separator_height;
  insp->edit_rect.left+=separator_width+(line_offset/(1<<insp->chunk))*edit_width;
  insp->edit_rect.right=insp->edit_rect.left+edit_width-separator_width;
  InflateRect(&insp->edit_rect,separator_height,separator_height);
}

void  set_edit_offset(inspector_def *insp,int x, int y)
/* determine edit_offset from the position */
{ int i = (y-top_width)/line_height;
  insp->de_offset=-1;
  if (i<0 || i >= insp->lines) return;
  if (insp->regs!=NULL)
  { if(i+insp->sb_cur>=insp->num_regs) return;
      insp->de_offset=i+insp->sb_cur;
  }
  else
  { unsigned int offset = insp->mem_base+i*insp->line_range;
	if (x<=address_width) return;
    offset+=(1<<insp->chunk)*((x-address_width)/insp->column_width);
	if (offset>=insp->size) return;
    insp->de_offset=offset;
  }
}

void SetInspector(HWND hMemory, inspector_def * insp)
{ RECT r;
  SetWindowLongPtr(hMemory,DWLP_USER,(LONG)(LONG_PTR)insp);
  insp->hWnd=hMemory;
  GetClientRect(hMemory,&r);
  insp->width=r.right-r.left;
  insp->height=r.bottom-r.top;
  update_max_regnames(insp);
  resize_memory_dialog(hMemory,insp);
  if (insp->regs==NULL)
  { RECT r;
	POINT p;
    int xButton;
    adjust_goto_addr(hMemory,insp,insp->address+insp->mem_base);
	ShowWindow(GetDlgItem(hMemory,IDC_GOTO_PROMPT),SW_SHOW);
	ShowWindow(GetDlgItem(hMemory,IDC_GOTO),SW_SHOW);
	GetWindowRect(GetDlgItem(hMemory,IDC_GOTO),&r);
	p.x=r.right;
	p.y=r.top;
    ScreenToClient(hMemory,&p);
	xButton=p.x+digit_width;
	GetWindowRect(GetDlgItem(hMemory,IDC_FORMAT),&r);
	MoveWindow(GetDlgItem(hMemory,IDC_FORMAT),xButton,separator_height,r.right-r.left,r.bottom-r.top,TRUE);
	MoveWindow(GetDlgItem(hMemory,IDC_CHUNK),xButton+digit_width+r.right-r.left,separator_height,r.right-r.left,r.bottom-r.top,TRUE);
  }
  else
  { RECT r;
	ShowWindow(GetDlgItem(hMemory,IDC_GOTO),SW_HIDE);
  	ShowWindow(GetDlgItem(hMemory,IDC_GOTO_PROMPT),SW_HIDE);
	GetWindowRect(GetDlgItem(hMemory,IDC_FORMAT),&r);
	MoveWindow(GetDlgItem(hMemory,IDC_FORMAT),digit_width,separator_height,r.right-r.left,r.bottom-r.top,TRUE);
	MoveWindow(GetDlgItem(hMemory,IDC_CHUNK),2*digit_width+r.right-r.left,separator_height,r.right-r.left,r.bottom-r.top,TRUE);

  }
  SetDlgItemText(hMemory,IDC_FORMAT,format_names[insp->format]);
  SetDlgItemText(hMemory,IDC_CHUNK,chunk_names[insp->chunk]);
}

static INT_PTR CALLBACK   
MemoryDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ switch ( message )
  { case WM_INITDIALOG :
	  SetDlgItemText(hDlg,IDC_FORMAT,format_names[0]);
	  SetDlgItemText(hDlg,IDC_CHUNK,chunk_names[0]);
	  SetFocus(GetDlgItem(hDlg,IDOK));
	  hDataEditInstance=hInst;
	  register_subwindow(hDlg);
	  return FALSE;
	case WM_CLOSE:
		DestroyWindow(hDlg);
		return 0;
	case WM_DESTROY:
		{  inspector_def *insp=(inspector_def *)(LONG_PTR)GetWindowLongPtr(hDlg,DWLP_USER);
		   unregister_subwindow(hDlg);
		   DestroyDataEdit(0);
		   if (insp!=NULL) insp->hWnd=NULL;
		}
	  return FALSE;
	case WM_LBUTTONDBLCLK:
	{ HWND hde;
	  inspector_def *insp=(inspector_def *)(LONG_PTR)GetWindowLongPtr(hDlg,DWLP_USER);
	  if (insp==NULL) return FALSE;
	  if (insp->hWnd!=NULL)
	    InvalidateRect(insp->hWnd,&insp->edit_rect,TRUE);
	  hde=GetDataEdit(0,hDlg);
	  de_connect(hde,insp);
      set_edit_offset(insp,LOWORD(lparam),HIWORD(lparam));
	  set_edit_rect(insp);
	  InvalidateRect(insp->hWnd,&insp->edit_rect,FALSE);
	  de_update(hde);
	}
    return FALSE;
	case WM_COMMAND: 
	  if (wparam ==IDOK)
	  {  /* User has hit the ENTER key.*/
		 inspector_def *insp=(inspector_def *)(LONG_PTR)GetWindowLongPtr(hDlg,DWLP_USER);
		if (insp==NULL) return FALSE;
	    if (GetFocus()==GetDlgItem(hDlg,IDC_GOTO))
        { uint64_t goto_addr;
#define   MAXVALUE 32
		  char value[MAXVALUE];
          GetDlgItemText(hDlg,IDC_GOTO,value,MAXVALUE);
    	  goto_addr = strtouint64(value);
		  goto_addr=adjust_goto_addr(hDlg,insp,goto_addr);
		  if (!insp->change_address || (goto_addr> insp->address && goto_addr-insp->address<INT_MAX))
		    insp->sb_base = (unsigned int)(goto_addr-insp->address);
		  else /* move base address of inspector */
		  { insp->address=goto_addr;
		    insp->sb_base=0;
			insp->mem_size=0;
		    insp->de_offset=-1;
			set_edit_rect(insp);
		  }
		  insp->sb_cur = 0;
		  adjust_mem_display(insp);
		 }
	  }
	  else if (HIWORD(wparam) == BN_CLICKED) 
	  { inspector_def *insp=(inspector_def *)(LONG_PTR)GetWindowLongPtr(hDlg,DWLP_USER);
	    if (insp==NULL) return FALSE;
        if ((HWND)lparam==GetDlgItem(hDlg,IDC_FORMAT))
	    { insp->format++; if (insp->format>last_format) insp->format=0;
		  SetDlgItemText(hDlg, IDC_FORMAT, format_names[insp->format]);
		  if (insp->format==float_format && insp->chunk<tetra_chunk)
		  { insp->chunk=tetra_chunk;
            SetDlgItemText(hDlg, IDC_CHUNK, chunk_names[insp->chunk]);
		  } 
     	  resize_memory_dialog(hDlg,insp);
	    } 
	    else if ((HWND)lparam==GetDlgItem(hDlg,IDC_CHUNK))
	    { if (insp->format==float_format && insp->chunk==octa_chunk)
		    insp->chunk=tetra_chunk;
		  else if (insp->format==float_format && insp->chunk!=octa_chunk)
		    insp->chunk=octa_chunk;
		  else
		    insp->chunk++; 
		  if (insp->chunk>last_chunk) insp->chunk=0;
		  SetDlgItemText(hDlg, IDC_CHUNK, chunk_names[insp->chunk]);
     	  resize_memory_dialog(hDlg,insp);
	    }
	  }
	  return FALSE;
	case WM_VSCROLL: 
	  { inspector_def *insp=(inspector_def *)(LONG_PTR)GetWindowLongPtr(hDlg,DWLP_USER);
	    if (insp==NULL) return FALSE;
        sb_move(hDlg,insp,wparam);
	  }
       return TRUE; 
    case SBM_SETRANGE:
		return TRUE;
	case WM_PAINT:
    { inspector_def *insp=(inspector_def *)(LONG_PTR)GetWindowLongPtr(hDlg,DWLP_USER);
	  PAINTSTRUCT ps;
	  HDC hdc = BeginPaint (hDlg, &ps);
	  if (insp!=NULL)
	  { display_data(insp,hdc);
	    if (!IsRectEmpty(&insp->edit_rect))
	    { HBRUSH hb = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
	      LOGPEN lp = {PS_SOLID,{separator_height,0},RGB(0,0,0)};
	      HPEN hpnew =  CreatePen(PS_SOLID|PS_INSIDEFRAME,separator_height,RGB(0,0,0)); //CreatePenIndirect(&lp);
		  HPEN hpold = SelectObject(hdc,hpnew);
		  Rectangle(hdc,insp->edit_rect.left,insp->edit_rect.top,insp->edit_rect.right,insp->edit_rect.bottom);
		  SelectObject(hdc, hpold);
		  DeleteObject(hpnew);
          SelectObject(hdc, hb);
	    }
	  }
      EndPaint (hDlg, &ps);
	  return TRUE;
    }    
	case WM_SIZE:
    {   inspector_def *insp=(inspector_def *)(LONG_PTR)GetWindowLongPtr(hDlg,DWLP_USER);
		if (insp!=NULL) 
		{ insp->width=LOWORD(lparam);
		  insp->height=HIWORD(lparam);
		  resize_memory_dialog(hDlg,insp);
		}
	}
	  break;
	case WM_GETMINMAXINFO:
	{ MINMAXINFO *p = (MINMAXINFO *)lparam;
	  p->ptMinTrackSize.x = mem_min_width;
      p->ptMinTrackSize.y = mem_min_height;
	  p->ptMaxTrackSize.x=p->ptMinTrackSize.x;
	  p->ptMaxTrackSize.y=p->ptMinTrackSize.y;
	}
	return 0;
  }
  return FALSE;
}

HWND CreateMemoryDialog(HINSTANCE hInst,HWND hParent)
{ HWND h;
  if (line_height==0) set_mem_font_metrics();
  h= CreateDialog(hInst,MAKEINTRESOURCE(IDD_MEMORY),hParent,MemoryDialogProc);
  hDataEditParent=hParent;
  return h;
}


void MemoryDialogUpdate(HWND hMemory,inspector_def *insp, unsigned int offset, int size)
/* called if size byte in memory at offset have changed */
{ if (hMemory==NULL) return;
  if (insp->regs==NULL) adjust_goto_addr(hMemory,insp,insp->address+insp->mem_base);
  if (offset>=insp->mem_base+insp->mem_size || offset+size<=insp->mem_base) 
    return;
  else
  { unsigned int from, to;
    refresh_old_mem(insp);
    if (offset<insp->mem_base) from=insp->mem_base; else from=offset;
	if (offset+size<insp->mem_base+insp->mem_size) to = offset+size; else to = insp->mem_base+insp->mem_size;
	if (insp->get_mem) insp->get_mem(from, to-from, insp->mem_buf+(from-insp->mem_base));
    InvalidateRect(hMemory,NULL,insp->regs!=NULL);
  }
}



void de_disconnect(inspector_def *insp)
/* called if a dataedit window disconects from the inspector */
{ if (insp->hWnd!=NULL)
    InvalidateRect(insp->hWnd,&insp->edit_rect,TRUE);
  insp->de_offset=-1;
  set_edit_rect(insp);
//  refresh_display(hMemory, insp);
}




