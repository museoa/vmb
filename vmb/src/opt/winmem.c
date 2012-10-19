#include <windows.h>
#include <commctrl.h>
#include "vmb.h"
#include "resource.h"
#include "winopt.h"
#include "option.h"
#include "param.h"
#include "error.h"
#include "bus-arith.h"
#include "inspect.h"


HWND hMemory=NULL; 
int insp=0;
static uint64_t goto_addr=0x0;

static unsigned char *mem_buf=NULL;
static unsigned int page_range, line_range;

enum mem_fmt mem_format = hex_format;
char *format_names[]={"Hex","Dec","-Dec","Ascii","Float","Double"};

enum chunk_fmt mem_chunk = byte_chunk;
static char *chunk_names[]={"BYTE","WYDE","TETRA","OCTA"};

/* scrolbar management */
static unsigned int sb_base=0; /* offset at base of scrollbar */
static int sb_cur = 0; /* line corresponding to start of page */
static int sb_rng = 8; /* number of lines covered by scrollbar */
static int lines; /* lines per page */
static unsigned int mem_base=0, mem_size=0; /* page currently displayed */
/* for register displays sb_base is always zero */


static void sb_range(void)
/* determine and set scrollbar range as virtual number of lines */
{ SCROLLINFO si;
  /* determine range */
  if (inspector[insp].size==0)
  { mem_base=0;
	mem_size = 0;
	sb_base=0;
	sb_cur=0;
	sb_rng=0;
  }
  else if (inspector[insp].num_regs>0)
  { sb_rng = inspector[insp].num_regs-1;
    sb_base =0;
	if (sb_cur>sb_rng-(lines-1)) sb_cur=sb_rng-(lines-1);
	if (sb_cur<0) sb_cur=0;
    mem_base = inspector[insp].regs[sb_cur].offset;
	mem_size = inspector[insp].size -mem_base;
  }
  else 
  { 
    if (page_range*10<inspector[insp].size)
      sb_rng = page_range*10/line_range-1;
    else if (page_range<inspector[insp].size)
      sb_rng = (inspector[insp].size+line_range-1)/line_range-1;
    else
      sb_rng = (page_range+line_range-1)/line_range-1;
    /* determine address of first line */
	if (sb_cur<0)
	{ if (sb_base > -sb_cur*line_range)
	  { sb_base = sb_base +sb_cur*line_range;
	    sb_cur=0;
	  } 
	  else
	  { sb_base = 0;
	    sb_cur=0;
	  } 
	}
    else if (sb_cur>sb_rng-(lines-1))
	{ int d = sb_cur-sb_rng+(lines-1);
	  if (sb_base+d*line_range + sb_rng*line_range<inspector[insp].size)
	    sb_base = sb_base + d*line_range;
	  else if (inspector[insp].size> sb_rng*line_range)
		sb_base=inspector[insp].size- sb_rng*line_range;
	  else
	    sb_base=0;
	  sb_cur=sb_rng-(lines-1);
	}
    mem_base=sb_base+sb_cur*line_range;
	mem_size = page_range;
	if (mem_size > inspector[insp].size -mem_base)
	  mem_size = inspector[insp].size -mem_base;
  }

  si.cbSize=sizeof(si);
  si.fMask=SIF_PAGE|SIF_POS|SIF_RANGE;
  si.nMin=0;
  si.nMax=sb_rng;
  si.nPage=lines;
  si.nPos=sb_cur;
  SetScrollInfo(GetDlgItem(hMemory,IDC_MEM_SCROLLBAR),SB_CTL,&si,TRUE);
}
static int adjust_mem_display(void);

void sb_move(WPARAM wparam)
{ switch (LOWORD(wparam)) 
	  { case SB_LINEUP:
	      sb_cur--; /* we catch a negative value in sb_range */
		  break;
        case SB_LINEDOWN: 
	      sb_cur++; /* we catch a too big value in sb_range */
		  break;
		case SB_PAGEUP:
		  sb_cur=sb_cur-lines;
		  break;
        case SB_PAGEDOWN: 
		  sb_cur=sb_cur+lines;
		  break;
		case SB_TOP:
		  sb_cur=0;
		  break;
        case SB_BOTTOM: 
		  sb_cur=sb_rng-lines-1;
		  break;
		case SB_THUMBTRACK: /* HIWORD(wparam) is 0 to sb_range */
		   sb_cur = HIWORD(wparam);
          break;
		default:
		case SB_ENDSCROLL:
		case SB_THUMBPOSITION:
		  return;
	  }
	   adjust_mem_display();
}


void adjust_goto_addr(void)
{ if (goto_addr<inspector[insp].address)
    { goto_addr=inspector[insp].address;
    }
    else if (goto_addr>=inspector[insp].address+inspector[insp].size)
    { goto_addr=inspector[insp].address+(inspector[insp].size-1);
    }
    uint64tohex(goto_addr,tmp_option);
    SetDlgItemText(hMemory,IDC_GOTO,tmp_option);
}



static RECT mem_rect;
static BOOL layout_change=TRUE;


void refresh_display()
{  if (inspector[insp].num_regs>0) layout_change=TRUE;
   InvalidateRect(hMemory,&mem_rect,layout_change);
   layout_change=FALSE;
}

static int adjust_mem_display(void)
/* 0 if no update needed, else return 1 */
{ int ret=0;
  static unsigned int old_size=0;
  sb_range();
  if (mem_size>old_size)
  { mem_buf=realloc(mem_buf,mem_size);
    if (mem_buf==NULL)
		vmb_fatal_error(__LINE__,"Out of memory");
    old_size=mem_size;
  }
  if (inspector[insp].get_mem) inspector[insp].get_mem(mem_base,mem_size,mem_buf);
  refresh_display();
  return ret;
}



static int mem_lines, mem_cols;
static int mem_width=0, mem_height=0;
static int line_height, digit_width, address_width, separator_width, separator_height, top_width; 

static void get_font_metrics(HWND hWnd)
{ SIZE size;
  HDC hDC=GetDC(hWnd);
  HFONT hfnt = GetStockObject(ANSI_FIXED_FONT); 
  SelectObject(hDC, hfnt);
  GetTextExtentPoint32(hDC,"0",1,&size);
  separator_height=2;
  digit_width=size.cx;
  line_height=size.cy+separator_height;
  separator_width = digit_width/2;
  GetTextExtentPoint32(hDC,"0x0000000000000000:",19,&size);
  address_width=size.cx;
  top_width=28;
  ReleaseDC(hWnd,hDC);
}

static int chunk_size=1;
static int columns, column_width, column_digits;

int chunk_len(enum mem_fmt f, enum chunk_fmt c)
{ int column_digits;
  int chunk_size=1<<c;
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
	case double_format:
       column_digits=19;
    break;
  }
  return column_digits;
}

static void resize_memory_dialog(void)
{ int sbw;
  RECT sbRect;

  GetWindowRect(GetDlgItem(hMemory,IDC_MEM_SCROLLBAR),&sbRect);
  sbw=sbRect.right-sbRect.left;
  MoveWindow(GetDlgItem(hMemory,IDC_MEM_SCROLLBAR),
	         mem_width-sbw,top_width,sbw,mem_height-top_width,TRUE);

  mem_rect.top=top_width;
  mem_rect.bottom=mem_height;
  mem_rect.left=0;
  mem_rect.right=mem_width-sbw;

  lines = (mem_height-top_width)/line_height;
  chunk_size=1<<mem_chunk;
  
  column_digits=chunk_len(mem_format,mem_chunk);

  column_width=column_digits*digit_width+separator_width;
  columns = (mem_width-sbw-address_width-separator_width)/column_width;
  if (columns<1)
  { columns=1;
    column_width=mem_width-sbw-address_width-separator_width;
	if (column_digits*digit_width>column_width)
      column_digits=column_width/digit_width;
  }
  line_range = columns*chunk_size;
  page_range= lines*line_range;
  layout_change=TRUE;
  adjust_mem_display();
}


void adjust_memory_tab(void)
/* 0 if no update needed, else return 1 */
{ int ret=0;
  
  if (inspector[insp].num_regs==0)
  { adjust_goto_addr();
	ShowWindow(GetDlgItem(hMemory,IDC_GOTO),SW_SHOW);
	ShowWindow(GetDlgItem(hMemory,IDC_GOTO_PROMPT),SW_SHOW);
  }
  else
  { ShowWindow(GetDlgItem(hMemory,IDC_GOTO),SW_HIDE);
  	ShowWindow(GetDlgItem(hMemory,IDC_GOTO_PROMPT),SW_HIDE);
  }
  resize_memory_dialog();
}

/* Color management */

#define COLD RGB(0xFD,0xFD,0xFF)
#define HOT RGB(0xFF,0xA0,0xA0)
static unsigned char* old_mem=NULL;
static unsigned int old_size=0, old_base=0;

void update_old_mem(void)
{ if (old_size<mem_size)
  { old_mem = realloc(old_mem,mem_size);
    if (old_mem==NULL)
		vmb_fatal_error(__LINE__,"Out of memory");
  }
   /* newly displayed memory contente is cold, (copy from mem_buf)
      only previously displayed content that has changed is hot */
   if (old_base<mem_base)
   { /* move overlap of old and new down*/
      unsigned int dist = mem_base-old_base; 
      if (old_size>dist)
	  { int ovlap = old_size-dist;
	    memmove(old_mem, old_mem+dist,ovlap);
	    memmove(old_mem+ovlap,mem_buf+ovlap,mem_size-ovlap);
	  }
	  else
	    memmove(old_mem,mem_buf,mem_size);
   }
   else if (old_base>mem_base)
   { /* move overlap of old and new up*/
      unsigned int dist = old_base-mem_base; 
      if (old_size>dist)
	  { int ovlap = old_size-dist;
	    memmove(old_mem+dist, old_mem,ovlap);
	    memmove(old_mem,mem_buf,dist);
		if (mem_size>old_size)
	      memmove(old_mem+old_size,mem_buf+old_size,mem_size-old_size);
	  }
	  else
	    memmove(old_mem,mem_buf,mem_size);
   }
   else if (mem_size>old_size)
	 memmove(old_mem+old_size,mem_buf+old_size,mem_size-old_size);
   old_base=mem_base;
   old_size=mem_size;
}

static void refresh_old_mem(void)
{ if (old_size<mem_size)
  { old_mem = realloc(old_mem,mem_size);
    if (old_mem==NULL)
		vmb_fatal_error(__LINE__,"Out of memory");
  }
  memmove(old_mem,mem_buf,mem_size);
  old_base=mem_base;
  old_size=mem_size;
}

static int different(int offset, int size)
{ int i;
  for (i=0;i<size;i++)
  { if (old_mem[offset+i]!=mem_buf[offset+i]) return 1;
  }
  return 0;
}


static int chunk_to_str(char *str, unsigned char *buf, enum mem_fmt fmt, int chunk_size, int column_digits)
{ int j;
  int w=column_digits;
  switch (fmt)
  { default:
    case hex_format:
      for (j=0;j<chunk_size;j++)
	    sprintf(str+j*2,"%02X",buf[j]);
	  break;
    case ascii_format:
	  memmove(str,buf,chunk_size);
	  str[chunk_size]=0;
	  break;
	case unsigned_format:
      switch (chunk_size)
      { case 1: sprintf(str,"%*u",column_digits,buf[0]); break;
        case 2: sprintf(str,"%*u",column_digits,GET2(buf)); break;
        case 4: sprintf(str,"%*u",column_digits,GET4(buf)); break;
	    default:
        case 8: w = sprintf(str,"%*llu",column_digits,GET8(buf)); break;
      }
	  break;
	case signed_format:
      switch (chunk_size)
      { case 1: sprintf(str,"%*d",column_digits,(int8_t)buf[0]); break;
        case 2: sprintf(str,"%*d",column_digits,(int16_t)GET2(buf)); break;
        case 4: sprintf(str,"%*d",column_digits,(int32_t)GET4(buf)); break;
	    default:
        case 8: w=sprintf(str,"%*lld",column_digits,(int64_t)GET8(buf)); break;
      }
	  break;
	case float_format: 
	  w=sprintf(str,"%*g",column_digits,(float)GET4(buf)); 
	  break;
	case double_format:
	  w=sprintf(str,"%*g",column_digits,(double)GET4(buf)); 
	  break;
  }
  if (w>column_digits)
  {	memset(str,'*',column_digits);
    str[column_digits]=0;
  }
  else
    str[w]=0;
  return w;
}


void display_registers(HDC hdc)
{ 
  int i,k, nr;
  char str[22]; /* big enough for the largest 8Byte integer */
  RECT rect;
  nr = inspector[insp].num_regs;
  if (nr>lines) nr=lines;
  for (i=0;i<nr;i++)
   { int y, chunk_size, x;
     struct register_def *r;
	 r = &inspector[insp].regs[sb_cur+i];
     y=top_width+i*line_height;
     x=address_width*3/4;
     SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));
     SetBkColor(hdc,GetSysColor(COLOR_BTNFACE));
     SetTextAlign(hdc,TA_RIGHT|TA_NOUPDATECP);
	 rect.top=y;
     rect.left=0;
     rect.right=x;
     rect.bottom=y+line_height-separator_height;
     ExtTextOut(hdc,x,y,
		   ETO_OPAQUE,&rect,r->name,(int)strlen(r->name),NULL);

	 chunk_size=1<<r->chunk;
	 x = x+separator_width;
	 for (k=0;k*chunk_size < r->size;k++)
	 { int l = chunk_len(r->format,r->chunk);
	   chunk_to_str(str, mem_buf+r->offset+k*chunk_size-mem_base, r->format,chunk_size,l);
       if (different(r->offset+k*chunk_size-mem_base,chunk_size))
	     SetBkColor(hdc,HOT);
	   else
	     SetBkColor(hdc,COLD);
	   SetTextAlign(hdc,TA_RIGHT|TA_NOUPDATECP);
	   rect.top=y;
       rect.left=x;
       rect.right=x+l*digit_width;
       rect.bottom=y+line_height-separator_height;
       ExtTextOut(hdc,x+l*digit_width,y,
		   ETO_OPAQUE,&rect,str,(int)strlen(str),NULL);
	   x=x+l*digit_width+separator_width;
	 }
   } 
}
void display_address(HDC hdc)
{ int i; 
  uint64_t addr=inspector[insp].address+mem_base;
  char str[22]; /* big enough for the largest 8Byte integer */
  
  SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));
  SetBkColor(hdc,GetSysColor(COLOR_BTNFACE));
  for (i=0;i<lines && (unsigned int)(i*line_range)<mem_size;i++)
  { uint64tohex(addr+i*line_range,str);
    str[18]=':';
	TextOut(hdc,0,top_width+i*line_height,str,19);
  }
}

void display_memory(HDC hdc)
{ 
  int i,k;
  char str[22]; /* big enough for the largest 8Byte integer */
  RECT r;

  for (i=0;i<lines && (unsigned int)(i*line_range)<mem_size;i++)
   { int y;
     y=top_width+i*line_height;
	 for (k=0;k<columns;k++)
	 { int x,l;
	   if ((unsigned int)((i*columns+k+1)*chunk_size)<= mem_size)
	   { l=chunk_to_str(str, mem_buf+(i*columns+k)*chunk_size, mem_format,chunk_size, column_digits);
         if (different((i*columns+k)*chunk_size,chunk_size))
	       SetBkColor(hdc,HOT);
	     else
	       SetBkColor(hdc,COLD);
	   }
	   else
	   { str[0]=0;
	     l=0;
         SetBkColor(hdc,COLD);
	   }
	   x = address_width+separator_width+k*column_width;
	   SetTextAlign(hdc,TA_RIGHT|TA_NOUPDATECP);
	   r.top=y;
       r.left=x;
       r.right=x+column_width-separator_width;
       r.bottom=y+line_height-separator_height;

       ExtTextOut(hdc,x+column_width-separator_width,y,
		   ETO_OPAQUE,&r,str,l,NULL);
	 }
   } 
}


void display_data(HDC hdc)
{ if (inspector[insp].num_regs>0)
  { update_old_mem();
    display_registers(hdc);
  }
  else
  { display_address(hdc);
    update_old_mem();
    display_memory(hdc);
  }
}


INT_PTR CALLBACK   
MemoryDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ switch ( message )
  { case WM_INITDIALOG :
	  SetDlgItemText(hDlg,IDC_FORMAT,format_names[mem_format]);
	  SetDlgItemText(hDlg,IDC_CHUNK,chunk_names[mem_chunk]);
	  SetFocus(GetDlgItem(hDlg,IDOK));
	  get_font_metrics(hDlg);

      /*SendMessage(hDlg,DM_SETDEFID,(WPARAM)IDOK,0); */
	  return FALSE;
	case WM_COMMAND: 
	  if (wparam ==IDOK)
	  {  /* User has hit the ENTER key.*/
		 if (GetFocus()==GetDlgItem(hDlg,IDC_GOTO))
        { GetDlgItemText(hDlg,IDC_GOTO,tmp_option,MAXTMPOPTION);
    	  goto_addr = strtouint64(tmp_option);
		  adjust_goto_addr();
		  sb_base = (unsigned int)(goto_addr-inspector[insp].address);
		  sb_cur = 0;
		  adjust_mem_display();
		 }
	  }
	  else if (HIWORD(wparam) == BN_CLICKED) 
	  { if ((HWND)lparam==GetDlgItem(hDlg,IDC_FORMAT))
	    { mem_format++; if (mem_format>last_format) mem_format=0;
		  SetDlgItemText(hDlg, IDC_FORMAT, format_names[mem_format]);
		  if (mem_format==float_format)
		  { mem_chunk=tetra_chunk;
            SetDlgItemText(hDlg, IDC_CHUNK, chunk_names[mem_chunk]);
		  } 
		  else if (mem_format==double_format)
		  { mem_chunk=octa_chunk;
            SetDlgItemText(hDlg, IDC_CHUNK, chunk_names[mem_chunk]);
		  } 
     	  resize_memory_dialog();
	    } 
	    else if ((HWND)lparam==GetDlgItem(hDlg,IDC_CHUNK))
	    { if (mem_format==float_format)
		    mem_chunk=tetra_chunk;
		  else if (mem_format==double_format)
		    mem_chunk=octa_chunk;
		  else
		    mem_chunk++; 
		  if (mem_chunk>last_chunk) mem_chunk=0;
		  SetDlgItemText(hDlg, IDC_CHUNK, chunk_names[mem_chunk]);
     	  resize_memory_dialog();
	    }
	  }
	  return FALSE;
	case WM_VSCROLL: 
       sb_move(wparam);
       return TRUE; 
    case SBM_SETRANGE:
		return TRUE;
	case WM_PAINT:
    { PAINTSTRUCT ps;
	  HDC hdc = BeginPaint (hDlg, &ps);
      display_data(hdc);
      EndPaint (hDlg, &ps);
	  return TRUE;
    }    
	case WM_SIZE:
		mem_width=LOWORD(lparam);
		mem_height=HIWORD(lparam);
		resize_memory_dialog();
	  break;
  }
  return FALSE;
}



void mem_update(int i, unsigned int offset, int size)
{ if (hMemory==NULL) return;
  if (i!=insp) return;
  if (inspector[insp].num_regs==0) adjust_goto_addr();
  if (offset>=mem_base+mem_size || offset+size<=mem_base) 
    return;
  else
  { unsigned int from, to;
    refresh_old_mem();
    if (offset<mem_base) from=mem_base; else from=offset;
	if (offset+size<mem_base+mem_size) to = offset+size; else to = mem_base+mem_size;
	if (inspector[insp].get_mem) inspector[insp].get_mem(from, to-from, mem_buf+(from-mem_base));
	refresh_display();
  }
}
