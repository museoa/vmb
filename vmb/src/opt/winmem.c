#include <windows.h>
#include <commctrl.h>
#include <string.h>
#include "vmb.h"
#include "resource.h"
#include "winopt.h"
#include "option.h"
#include "param.h"
#include "error.h"
#include "bus-arith.h"
#include "float.h"
#include "inspect.h"


HWND hMemory=NULL; 
static int insp=0;
static uint64_t goto_addr=0x0;

static unsigned char *mem_buf=NULL;
static unsigned int line_range;

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
  { unsigned int page_range;
    page_range= lines*line_range;
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

static void sb_move(WPARAM wparam)
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


static void adjust_goto_addr(void)
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

void set_edit_rect(void);

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
  layout_change=TRUE;
  adjust_mem_display();
  set_edit_rect();
}



void adjust_memory_tab(int i)
/* 0 if no update needed, else return 1 */
{ int ret=0;
  insp = i;
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
   /* newly displayed memory contents is cold, (copy from mem_buf)
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


static int chunk_to_str(char *str, unsigned char *buf, enum mem_fmt fmt, 
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
	  w=f64_to_str(str,f64_from_f32(GET4(buf)),column_digits);
	  break;
	case double_format:
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


int edit_offset=-1; /* offset of the chunk currently in the edit window or negative */
int edit_register=0; /* the register containing edit offset */
RECT edit_rect={0,0,0,0};

void set_edit_rect(void)
/* determine the edit rectangle from the edit_offset. */
{ int page_offset;
  int line_offset;
  int edit_line;
  int edit_width;
  SetRectEmpty(&edit_rect);
  if (edit_offset<0) return;
  page_offset= edit_offset-mem_base;
  if (page_offset<0 || page_offset>=(int)mem_size) return;

  if (inspector[insp].num_regs>0)
  { int loop_count=0;
    while (1) 
	{ line_offset= edit_offset - inspector[insp].regs[edit_register].offset;
	  if (line_offset>=0 && line_offset < inspector[insp].regs[edit_register].size)
		  break;
      edit_register--;
	  if (edit_register<0) 
	  { edit_register=inspector[insp].num_regs-1;
	    if (++loop_count>1) return;
	  }
	}
    edit_line=edit_register-sb_cur;
	edit_rect.left=address_width*3/4;
	edit_width=chunk_len(inspector[insp].regs[edit_register].format,inspector[insp].regs[edit_register].chunk)*digit_width+separator_width;
  }
  else {
    edit_line= page_offset/line_range;
    line_offset= page_offset-edit_line*line_range;
	edit_rect.left=address_width;
	edit_width=column_width;
  }
  edit_rect.top=top_width-separator_height/2+edit_line*line_height;
  edit_rect.bottom=edit_rect.top+line_height;
  edit_rect.left+=separator_width/2+(line_offset/chunk_size)*edit_width;
  edit_rect.right=edit_rect.left+edit_width;
}

void  set_edit_offset(int x, int y)
/* determine edit_offset from the position */
{ int i = (y-top_width)/line_height;
  edit_offset=-1;
  if (i<0 || i >= lines) return;
  if (inspector[insp].num_regs>0)
  { if(i+sb_cur>=inspector[insp].num_regs) return;
    edit_offset=inspector[insp].regs[i+sb_cur].offset;
  }
  else
  { if (x<=address_width) return;
	edit_offset=mem_base+i*line_range;
    edit_offset=edit_offset+chunk_size*((x-address_width)/column_width);
  }
}


HWND CreateDataEdit(HINSTANCE hInst,HWND hDlg);
void set_edit_region(HWND hDlg, int offset, enum chunk_fmt chunk, char *name);
HWND hDataEdit = NULL;

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
	case WM_LBUTTONDBLCLK:
      if (hDataEdit==NULL)
        CreateDataEdit(hInst,hDlg);
      set_edit_offset(LOWORD(lparam),HIWORD(lparam));
	  InvalidateRect(hDlg,&edit_rect,TRUE);
	  set_edit_rect();
      if (inspector[insp].num_regs>0)
		set_edit_region(hDataEdit,edit_offset,inspector[insp].regs[edit_register].chunk,
		inspector[insp].regs[edit_register].name);
	  else
		set_edit_region(hDataEdit,edit_offset,mem_chunk,NULL);
	  InvalidateRect(hDlg,&edit_rect,FALSE);
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
	  if (!IsRectEmpty(&edit_rect))
	  { HBRUSH hb = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
	    HPEN hp= SelectObject(hdc, GetStockObject(BLACK_PEN));
		Rectangle(hdc,edit_rect.left,edit_rect.top,edit_rect.right,edit_rect.bottom);
		SelectObject(hdc, hp);
        SelectObject(hdc, hb);
	  }
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



void mem_update(unsigned int offset, int size)
{ if (hMemory==NULL) return;
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

void mem_update_i(int i, unsigned int offset, int size)
{ if (i!=insp) return;
  mem_update(offset, size);
}

/* DataEditor */


typedef struct dataedit
{  HWND hDlg;
   char * name; /* display if not NULL */
   uint64_t address; /* display instead of name if name==NULL */
   int offset; /* offset to devices base address */
   unsigned char mem[8]; /* up to one octa */
   int size; /* 1,2,4, or 8  number of byte to edit */
   unsigned char *(*load)(unsigned int offset,int size); /* function to simulate load */
   void (*store)(unsigned int offset,int size, unsigned char *payload); /* same for store */
   enum mem_fmt format;
   enum chunk_fmt chunk;
} dataedit;

void get_edit_mem(dataedit *de);
void put_edit_mem(dataedit *de);

static void show_edit_windows(dataedit *de)
{ HWND h = de->hDlg;
  enum chunk_fmt chunk=de->chunk;
  int size=de->size;
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE0),chunk==byte_chunk?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE1),chunk==byte_chunk&&size>1?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE2),chunk==byte_chunk&&size>2?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE3),chunk==byte_chunk&&size>3?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE4),chunk==byte_chunk&&size>4?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE5),chunk==byte_chunk&&size>5?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE6),chunk==byte_chunk&&size>6?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITBYTE7),chunk==byte_chunk&&size>7?SW_SHOW:SW_HIDE);

  ShowWindow(GetDlgItem(h,IDC_EDITWYDE0),chunk==wyde_chunk?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITWYDE1),chunk==wyde_chunk&&size>2?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITWYDE2),chunk==wyde_chunk&&size>4?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITWYDE3),chunk==wyde_chunk&&size>6?SW_SHOW:SW_HIDE);

  ShowWindow(GetDlgItem(h,IDC_EDITTETRA0),chunk==tetra_chunk?SW_SHOW:SW_HIDE);
  ShowWindow(GetDlgItem(h,IDC_EDITTETRA1),chunk==tetra_chunk&&size>4?SW_SHOW:SW_HIDE);

  ShowWindow(GetDlgItem(h,IDC_EDITOCTA0),chunk==octa_chunk?SW_SHOW:SW_HIDE);
}

set_edit_format(dataedit *de, enum mem_fmt format)
{ 
  if (format==ascii_format && de->chunk!=byte_chunk) format++;
  if (format>last_format) format=hex_format;
  if (format==float_format && de->size<4) format=hex_format;
  if (format==float_format && de->chunk<tetra_chunk) format=hex_format;
  if (format==float_format && de->chunk==octa_chunk) format=double_format;
  if (format==double_format && de->size<8) format=hex_format;
  if (format==double_format && de->chunk<octa_chunk) format=hex_format;
  put_edit_mem(de);
  de->format=format;
  get_edit_mem(de);
  show_edit_windows(de);
  SetDlgItemText(de->hDlg, IDC_FORMAT, format_names[format]);
}
set_edit_chunk(dataedit *de, enum chunk_fmt chunk)
{ if (chunk>last_chunk) chunk=byte_chunk;
  if (1<<chunk>de->size) chunk=byte_chunk;
  if (de->format==ascii_format)
   chunk=byte_chunk;
  else if (de->format==float_format)
   chunk=tetra_chunk;
  else if (de->format==double_format)
   chunk=octa_chunk;
  put_edit_mem(de);
  de->chunk=chunk;
  get_edit_mem(de);
  show_edit_windows(de);
  SetDlgItemText(de->hDlg, IDC_CHUNK, chunk_names[chunk]);
}

uint64_t str_to_u64(char *str)
{ uint64_t u=0;
  while (isspace(*str)) str++;
  while (*str!=0 && isdigit(*str))
  { u=u*10+((*str)-'0');
    str++;
  }
  return u;
}

uint64_t hex_to_u64(char *str)
{ uint64_t u=0;
  while (isspace(*str)) str++;
  while (*str!=0)
  { if (isdigit(*str))
      u=u*16+((*str)-'0');
    else if ('a'<=*str && *str<='f')
      u=u*16+((*str)-'a'+10);
    else if ('A'<=*str && *str<='F')
      u=u*16+((*str)-'A'+10);
    else
	  break;
    str++;
  }
  return u;
}


static void str_to_chunk(char *str, unsigned char *buf, enum mem_fmt fmt, int chunk_size)
/* reads the data from str using format and chunk size.
   puts results into buffer.
*/
{ uint64_t u;
         
  if (fmt==float_format)
	u=f32_from_f64(f64_from_str(str));
  else if (fmt==double_format)
    u=f64_from_str(str);
  else if (fmt==signed_format)
	u= (uint64_t)_atoi64(str);
  else if (fmt==unsigned_format)
    u= str_to_u64(str);
  else if (fmt==hex_format)
	u= hex_to_u64(str);
  else if (fmt==ascii_format)
    u=(*str)&0xFF;

  while (chunk_size>0)
  { chunk_size--;
	*buf = (unsigned char)((u>>(8*chunk_size))&0xFF);
	buf++;
  }
}

static void get_item(dataedit *de,int IDC,int offset)
{ char str[22]; 
  if (offset>=de->size) return;
  chunk_to_str(str, de->mem+offset,de->format,1<<de->chunk,0);
  SetDlgItemText(de->hDlg,IDC,str);
  SendMessage(GetDlgItem(de->hDlg,IDC),EM_SETMODIFY,0,0); 
}

void get_edit_mem(dataedit *de)
{ if (de->chunk==byte_chunk)
  {	get_item(de,IDC_EDITBYTE0,0);
    get_item(de,IDC_EDITBYTE1,1);
    get_item(de,IDC_EDITBYTE2,2);
    get_item(de,IDC_EDITBYTE3,3);
    get_item(de,IDC_EDITBYTE4,4);
    get_item(de,IDC_EDITBYTE5,5);
    get_item(de,IDC_EDITBYTE6,6);
    get_item(de,IDC_EDITBYTE7,7);
  } else if (de->chunk==wyde_chunk)
  {	get_item(de,IDC_EDITWYDE0,0);
    get_item(de,IDC_EDITWYDE1,2);
    get_item(de,IDC_EDITWYDE2,4);
    get_item(de,IDC_EDITWYDE3,6);
 } else if (de->chunk==tetra_chunk)
  {	get_item(de,IDC_EDITTETRA0,0);
    get_item(de,IDC_EDITTETRA1,4);
  } else if (de->chunk==octa_chunk)
    get_item(de,IDC_EDITOCTA0,0);
}

static void put_item(dataedit *de, int IDC, int offset, int size)
{ char str[22]; 
  if (de->size>offset && SendMessage(GetDlgItem(de->hDlg,IDC),EM_GETMODIFY,0,0)) 
  { GetDlgItemText(de->hDlg,IDC,str,22);
	str_to_chunk(str, de->mem+offset,de->format,size);
  }
}

void put_edit_mem(dataedit *de)
{ if (de->chunk==byte_chunk)
  {	put_item(de,IDC_EDITBYTE0,0,1);
	put_item(de,IDC_EDITBYTE1,1,1);
	put_item(de,IDC_EDITBYTE2,2,1);
	put_item(de,IDC_EDITBYTE3,3,1);
	put_item(de,IDC_EDITBYTE4,4,1);
	put_item(de,IDC_EDITBYTE5,5,1);
	put_item(de,IDC_EDITBYTE6,6,1);
	put_item(de,IDC_EDITBYTE7,7,1);
  }
  else if (de->chunk==wyde_chunk)
  {	put_item(de,IDC_EDITWYDE0,0,2);
	put_item(de,IDC_EDITWYDE1,2,2);
	put_item(de,IDC_EDITWYDE2,4,2);
	put_item(de,IDC_EDITWYDE3,6,2);
  }
  else if (de->chunk==tetra_chunk)
  {	put_item(de,IDC_EDITTETRA0,0,4);
	put_item(de,IDC_EDITTETRA1,4,4);
  }
  else if (de->chunk==octa_chunk)
	put_item(de,IDC_EDITOCTA0,0,8);
}
	
void set_edit_name(dataedit *de)
{ if (de->name!=NULL)
	SetDlgItemText(de->hDlg,IDC_NAME,de->name);
  else
  { char str[22]; /* big enough for the largest 8Byte integer */
    uint64tohex(de->address+de->offset,str);
    SetDlgItemText(de->hDlg,IDC_NAME,str);
  }
}

INT_PTR CALLBACK   
DataEditDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ switch ( message )
  { case WM_INITDIALOG :
      { dataedit *de = (dataedit *)lparam;
	    de->hDlg=hDlg;
	    SetWindowLongPtr(hDlg,DWLP_USER,(LONG)lparam);
	    SetDlgItemText(hDlg,IDC_FORMAT,format_names[de->format]);
	    SetDlgItemText(hDlg,IDC_CHUNK,chunk_names[de->chunk]);

		get_edit_mem(de);
		show_edit_windows(de);
	    SetFocus(GetDlgItem(hDlg,IDC_LOAD));
	    //get_font_metrics(hDlg);
	  }
	  return FALSE;
	case WM_COMMAND: 
      if (HIWORD(wparam) == BN_CLICKED) 
	  { dataedit *de = (dataedit*)(LONG_PTR)GetWindowLongPtr(hDlg,DWLP_USER);
		if (LOWORD(wparam) ==IDC_LOAD)   /* User has hit the Load key.*/
		{ if (de->load!=NULL) 
		  { memmove(de->mem,de->load(de->offset,de->size),de->size); 
		    get_edit_mem(de);
		  }
		}
		else if (LOWORD(wparam) ==IDC_STORE)   /* User has hit the Load key.*/
		{ if (de->store !=NULL)
		  { put_edit_mem(de);
		    de->store(de->offset,de->size,de->mem); 
		  }
		}
	    else if (LOWORD(wparam) == IDC_FORMAT)
		   set_edit_format(de,de->format+1);
	    else if (LOWORD(wparam) == IDC_CHUNK)
		   set_edit_chunk(de,de->chunk+1);
	  }
	  return FALSE;
	case WM_CLOSE:
	  DestroyWindow(hDlg);
	  hDataEdit=NULL;
	  return FALSE;
 }
  return FALSE;
}

dataedit de = {0};

HWND CreateDataEdit(HINSTANCE hInst,HWND hParent)
{ 
  de.format=mem_format;
  de.address=inspector[insp].address;
  de.load=vmb.get_payload;
  de.store=vmb.put_payload;
  hDataEdit = CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_DATAEDIT),hParent,DataEditDialogProc,(LPARAM)&de);
  de.hDlg=hDataEdit;
  return hDataEdit;
}

void set_edit_region(HWND hDlg, int offset, enum chunk_fmt chunk, char *name)
{ 
  dataedit *de = (dataedit*)(LONG_PTR)GetWindowLongPtr(hDlg,DWLP_USER);	
  de->name=name;
  de->offset=offset;
  de->size=1<<chunk;
  set_edit_chunk(de,chunk);
  memmove(de->mem,mem_buf+offset,de->size);
  get_edit_mem(de);
  set_edit_name(de);
}

