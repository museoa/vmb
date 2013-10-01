#include <windows.h>
#include <winuser.h>
#include "error.h"
#include "splitter.h"

static HINSTANCE hSplitterInst;
static const char  sp_bar_class_name[] = "SplitterBarClass";
static const char  sp_class_name[] = "SplitterClass";
static ATOM SplitterBarClass, SplitterClass;
/* Layout */
#define SPLITWIDTH (8+6)
/* Colors for the spliterbar and its border */
#define COLOR_SPLIT COLOR_MENUBAR
#define COLOR_SPLIT_HI RGB(255,255,255))
#define COLOR_SPLIT_MH RGB(GetRVakue(COLOR_SPLIT)+5,GetGVakue(COLOR_SPLIT)+6,GetBVakue(COLOR_SPLIT)+10)
#define COLOR_SPLIT_ML RGB(GetRVakue(COLOR_SPLIT)-64,GetGVakue(COLOR_SPLIT)-65,GetBVakue(COLOR_SPLIT)-63)
#define COLOR_SPLIT_LO RGB(GetRVakue(COLOR_SPLIT)-123,GetGVakue(COLOR_SPLIT)-122,GetBVakue(COLOR_SPLIT)-116)
#define ARROWWIDTH 8
#define ARROWHEIGHT 10
#define MAXMID (ARROWHEIGHT+CLOSEHEIGHT+ 2*SPLITWIDTH)

/* darkness to produce the arrow bitmaps */
static const double Arrow[ARROWHEIGHT][ARROWWIDTH] = { 
1.0,0.75,1.0,1.0,1.0,1.0,1.0,1.0,
0.75,0.0,0.25,1.0,1.0,1.0,1.0,1.0,
1.0,0.0,0.0,0.0,0.5,1.0,1.0,1.0,
1.0,0.0,0.0,0.0,0.0,0.25,0.75,1.0,
1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.75,
1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.75,
1.0,0.0,0.0,0.0,0.0,0.25,0.75,1.0,
1.0,0.0,0.0,0.0,0.5,1.0,1.0,1.0,
0.75,0.0,0.25,1.0,1.0,1.0,1.0,1.0,
1.0,0.75,1.0,1.0,1.0,1.0,1.0,1.0
};
#define CLOSEWIDTH 8
#define CLOSEHEIGHT 8
/* darkness to produce the close bitmap */
static const double Close[CLOSEHEIGHT][CLOSEWIDTH] = { 
1.0,0.5,1.0,1.0,1.0,1.0,0.5,1.0,0.5,0.0,
0.25,1.0,1.0,0.25,0.0,0.5,1.0,0.25,0.0,0.25,
0.25,0.0,0.25,1.0,1.0,1.0,0.25,0.0,0.0,0.25,
1.0,1.0,1.0,1.0,0.25,0.0,0.0,0.25,1.0,1.0,
1.0,0.25,0.0,0.25,0.25,0.0,0.25,1.0,0.5,0.0,
0.25,1.0,1.0,0.25,0.0,0.5,1.0,0.5,1.0,1.0,
1.0,1.0,0.5,1.0
};
/* the arrow bitmaps and the restore bitmaps */
static HDC TopDC, BottomDC, LeftDC, RightDC, CloseDC, DragDC;
static HBITMAP hOldTopBmp,hOldBottomBmp,hOldLeftBmp,hOldRightBmp, hOldCloseBmp;
static HBRUSH hSplitterBrush;
static HWND hSpliterBase;
static HCURSOR hVCursor = NULL;
static HCURSOR hHCursor = NULL;
static HCURSOR hXCursor = NULL;
static HCURSOR hCCursor = NULL;
static HBRUSH hDragBrush, holdDragBrush; 
static HBRUSH hDragPen, holdDragPen;  

static void init_layout(HWND hSpliterBase)
{ HBITMAP  hBmp;
  HDC imgDC;
  int x,y;
  COLORREF SplitColor=GetSysColor(COLOR_SPLIT);
  BYTE SplitR, SplitG, SplitB;
  SplitR = GetRValue(SplitColor);
  SplitG = GetGValue(SplitColor);
  SplitB = GetBValue(SplitColor);

  hSplitterBrush=GetSysColorBrush(COLOR_SPLIT);
  hVCursor = LoadCursor(NULL, IDC_SIZEWE); 
  hHCursor = LoadCursor(NULL, IDC_SIZENS); 
  hXCursor = LoadCursor(NULL, IDC_SIZEALL); 
  hCCursor = LoadCursor(NULL, IDC_ARROW);


  imgDC = GetDC(hSpliterBase);
  TopDC = CreateCompatibleDC(imgDC);
  hBmp = CreateCompatibleBitmap(imgDC,ARROWWIDTH,ARROWHEIGHT);
  hOldTopBmp = (HBITMAP)SelectObject(TopDC, hBmp);
  BottomDC = CreateCompatibleDC(imgDC);
  hBmp = CreateCompatibleBitmap(imgDC,ARROWWIDTH,ARROWHEIGHT);
  hOldBottomBmp = (HBITMAP)SelectObject(BottomDC, hBmp);
  LeftDC = CreateCompatibleDC(imgDC);
  hBmp = CreateCompatibleBitmap(imgDC,ARROWHEIGHT,ARROWWIDTH);
  hOldLeftBmp = (HBITMAP)SelectObject(LeftDC, hBmp);
  RightDC = CreateCompatibleDC(imgDC);
  hBmp = CreateCompatibleBitmap(imgDC,ARROWHEIGHT,ARROWWIDTH);
  hOldRightBmp = (HBITMAP)SelectObject(RightDC, hBmp);
  CloseDC = CreateCompatibleDC(imgDC);
  hBmp = CreateCompatibleBitmap(imgDC,CLOSEHEIGHT,CLOSEWIDTH);
  hOldCloseBmp = (HBITMAP)SelectObject(CloseDC, hBmp);
  ReleaseDC(hSpliterBase,imgDC);

  for(y=0;y<ARROWHEIGHT;y++)
    for(x=0;x<ARROWWIDTH;x++)
	{ COLORREF rgb;
	  BYTE r,g,b;
	  r = (BYTE)(SplitR*Arrow[y][x]);
	  g = (BYTE)(SplitG*Arrow[y][x]);
	  b = (BYTE)(SplitB*Arrow[y][x]);
	  rgb=RGB(r,g,b);
      SetPixel(TopDC,x,y,rgb);
      SetPixel(BottomDC,ARROWWIDTH-x-1,y,rgb);
      SetPixel(LeftDC,y,x,rgb);
      SetPixel(RightDC,y,ARROWWIDTH-x-1,rgb);
	}
    for(y=0;y<CLOSEHEIGHT;y++)
    for(x=0;x<CLOSEWIDTH;x++)
	{ COLORREF rgb;
	  BYTE r,g,b;
	  r = (BYTE)(SplitR*Close[y][x]);
	  g = (BYTE)(SplitG*Close[y][x]);
	  b = (BYTE)(SplitB*Close[y][x]);
	  rgb=RGB(r,g,b);
      SetPixel(CloseDC,x,y,rgb);
	}
	DragDC= GetDC(hSpliterBase);
    hDragBrush=GetStockObject(LTGRAY_BRUSH);
    hDragPen=GetStockObject(WHITE_PEN);
    holdDragBrush=SelectObject(DragDC,hDragBrush);
    holdDragPen=SelectObject(DragDC,hDragPen);
    SetROP2(DragDC, R2_NOTXORPEN);
}


static void release_layout(void)
{ HBITMAP hBmp;	
  hBmp=SelectObject(TopDC, hOldTopBmp);
  DeleteObject(hBmp);
  DeleteDC(TopDC);
  hBmp=SelectObject(BottomDC, hOldBottomBmp);
  DeleteObject(hBmp);
  DeleteDC(BottomDC);
  hBmp=SelectObject(LeftDC, hOldLeftBmp);
  DeleteObject(hBmp);
  DeleteDC(LeftDC);
  hBmp=SelectObject(RightDC, hOldRightBmp);
  DeleteObject(hBmp);
  DeleteDC(RightDC);
  hBmp=SelectObject(CloseDC, hOldCloseBmp);
  DeleteObject(hBmp);
  DeleteDC(CloseDC);
  SelectObject(DragDC,holdDragPen);
  SelectObject(DragDC,holdDragBrush);
  ReleaseDC(hSpliterBase, DragDC); 
}




/* The splitter data structure */
/* unit of a fill stretch */
#define FILL (1<<((sizeof(int)*8)/4))
typedef struct split {
  int x, y,w,h;
  int min_w,min_h,stretch_w,stretch_h;
  struct split *Parent;
  enum {leaf=0, branch=1} tag;
  union {
	  struct { /* used if tag == branch */
        double ratio;
        int vertical;
        int mid_max;
        HWND hSplit; /* contains backlink to the split. do not copy or move */
		struct split *TopLeft; 
	    struct split *BottomRight; 
	  } sp;
	  HWND hWnd;  /* used if tag == leaf */
  } o;
} split;
split *Root=NULL;

static split *new_split(void)
{ split *sp;
  sp = (split*)malloc(sizeof(split));
  if (sp==NULL) fatal_error("Out of Memory");
  return sp;
}

static void sp_resize(split *sp);

#define get_split(ratio, vertical, w, h, sep) ((vertical)?((int)(((w)-(sep))*ratio)):((int)(((h)-(sep))*ratio)))


static int get_split_width(split *sp)
/* return the size of the top/left part */
{ return get_split(sp->o.sp.ratio, sp->o.sp.vertical,sp->w,sp->h,SPLITWIDTH);
#if 0
 if (sp->o.sp.vertical) /* return left part */
  { int d = sp->w-sp->min_w;
	if (sp->o.sp.BottomRight->stretch_w!=0)
	  d = (d * sp->o.sp.TopLeft->stretch_w)/(sp->o.sp.TopLeft->stretch_w+sp->o.sp.BottomRight->stretch_w);
    return sp->o.sp.TopLeft->min_w+d;
  }
  else
  { int d = sp->h-sp->min_h;
	if (sp->o.sp.BottomRight->stretch_h!=0)
	  d = (d * sp->o.sp.TopLeft->stretch_h)/(sp->o.sp.TopLeft->stretch_h+sp->o.sp.BottomRight->stretch_h);
    return sp->o.sp.TopLeft->min_h+d;
  }
#endif
}

static void set_split_ratio(split *sp, int w, int h)
/* determine ration from width of top or left split window */
{  if (sp->o.sp.vertical)
     sp->o.sp.ratio =  (double)w/(sp->w-SPLITWIDTH);
   else 
	 sp->o.sp.ratio = (double)h/(sp->h-SPLITWIDTH);
   sp_resize(sp);
}



static void sp_setsize(split *sp, int x, int y, int w, int h)
/* call if outer dimensions of split window have changed */
{ if (sp==NULL) sp=Root;
  if (sp==NULL) return;
  sp->x=x;
  sp->y=y;
  sp->w=w;
  sp->h=h;
  sp_resize(sp);
}

static void sp_propagate_layout(split *sp)
/* call to determine minimum sizes and strechability at and below sp 
   should be called only by sp_layout
*/
{ if (sp->tag==leaf)
{ MINMAXINFO mmi={0};
  mmi.ptMaxTrackSize.x=mmi.ptMaxTrackSize.y=mmi.ptMaxSize.x=mmi.ptMaxSize.y=FILL;
  if (SendMessage(sp->o.hWnd,WM_GETMINMAXINFO,0,(LPARAM)&mmi)==0)
	{ sp->min_w=mmi.ptMinTrackSize.x;
	  sp->min_h=mmi.ptMinTrackSize.y;
	  if (mmi.ptMaxTrackSize.x>mmi.ptMinTrackSize.x) 
		sp->stretch_w=1*FILL;
	  else
		sp->stretch_w=0;
	  if (mmi.ptMaxTrackSize.x>mmi.ptMinTrackSize.x) 
		sp->stretch_h=1*FILL;
	  else
		sp->stretch_h=0;
	}
	else
	{ sp->min_h=sp->min_w=0;
	  sp->stretch_h=sp->stretch_w=1*FILL;
	}
  }
  else if (sp->tag==branch)
  { sp_propagate_layout(sp->o.sp.TopLeft);
    sp_propagate_layout(sp->o.sp.BottomRight);
	if (sp->o.sp.vertical)
	{ sp->min_w= sp->o.sp.TopLeft->min_w + sp->o.sp.BottomRight->min_w+SPLITWIDTH;
	  sp->min_h= max(sp->o.sp.TopLeft->min_h, sp->o.sp.BottomRight->min_h);
      sp->stretch_w= sp->o.sp.TopLeft->stretch_w + sp->o.sp.BottomRight->stretch_w;
	  sp->stretch_h= min(sp->o.sp.TopLeft->stretch_h, sp->o.sp.BottomRight->stretch_h);
	}
	else
	{ sp->min_h= sp->o.sp.TopLeft->min_h + sp->o.sp.BottomRight->min_h+SPLITWIDTH;
	  sp->min_w= max(sp->o.sp.TopLeft->min_w, sp->o.sp.BottomRight->min_w);
      sp->stretch_h= sp->o.sp.TopLeft->stretch_h+ sp->o.sp.BottomRight->stretch_h;
	  sp->stretch_w= min(sp->o.sp.TopLeft->stretch_w, sp->o.sp.BottomRight->stretch_w);
	}
  }
}

static void sp_layout(void)
/* use this to recompute minimum sizes and strechability */
{ if (Root!=NULL) sp_propagate_layout(Root);
}

static void sp_resize(split *sp)
/* call if internal composition of splitter has changed */
{ if (sp==NULL) sp=Root;
  if (sp->tag==leaf)
    MoveWindow(sp->o.hWnd,sp->x,sp->y,sp->w,sp->h,TRUE);
  else if (sp->tag==branch)
  { if (sp->o.sp.vertical)
    { int lw, rw;
	  lw = get_split_width(sp);
	  rw = sp->w-lw-SPLITWIDTH;
	  sp_setsize(sp->o.sp.TopLeft,sp->x,sp->y,lw,sp->h);
      MoveWindow(sp->o.sp.hSplit,sp->x+lw,sp->y,SPLITWIDTH,sp->h,FALSE);
	  sp_setsize(sp->o.sp.BottomRight,sp->x+lw+SPLITWIDTH,sp->y,rw,sp->h);
	  if (sp->h<3*MAXMID)
	    sp->o.sp.mid_max = sp->h/6;
	  else
	    sp->o.sp.mid_max = sp->h/2 - MAXMID;
    }
    else
    { int th, bh;
	  th = get_split_width(sp);
      bh = sp->h-th-SPLITWIDTH;
	  sp_setsize(sp->o.sp.TopLeft,sp->x,sp->y,sp->w,th);
      MoveWindow(sp->o.sp.hSplit,sp->x,sp->y+th,sp->w,SPLITWIDTH,FALSE);
	  sp_setsize(sp->o.sp.BottomRight,sp->x,sp->y+th+SPLITWIDTH,sp->w,bh);
	  if (sp->w<3*MAXMID)
	    sp->o.sp.mid_max = sp->w/6;
	  else
	    sp->o.sp.mid_max = sp->w/2-MAXMID;
    }
  }
}


static split *sp_unlink(split *me)
/* unlink a leaf node and its parent from the tree returns NULL if either is missing.*/
{ split * parent, *grandparent, *sibling;
  if (me->tag!=leaf) return NULL;
  parent=me->Parent;
  if (parent==NULL)
    return NULL;
  if (parent->o.sp.TopLeft==me)
  {  sibling = parent->o.sp.BottomRight;
     parent->o.sp.BottomRight = NULL;
  }
  else
  {	  sibling = parent->o.sp.TopLeft;
    parent->o.sp.TopLeft=NULL;
  }
  grandparent=parent->Parent;
  sibling->Parent=grandparent;

  if (grandparent==NULL)
  { sibling->x = Root->x;
    sibling->y = Root->y;
    sibling->w = Root->w;
    sibling->h = Root->h;
    Root = sibling;
  }
  else
  { if (grandparent->o.sp.TopLeft==parent)
	  grandparent->o.sp.TopLeft=sibling;
    else
  	  grandparent->o.sp.BottomRight=sibling;
  }
  sp_resize(grandparent);
  return parent;
}

static void sp_delete_leaf(split *sp)
{ if (sp!=NULL && sp->tag==leaf)
  { sp = sp_unlink(sp);
    if (sp==NULL) /* this was the Root */
	{ free(Root); 
	  Root=NULL;
	}
	else
	{ if (sp->o.sp.TopLeft!=NULL) free(sp->o.sp.TopLeft);
	  if (sp->o.sp.BottomRight!=NULL) free(sp->o.sp.BottomRight);
      DestroyWindow(sp->o.sp.hSplit);
	  free(sp);
	  sp_resize(Root);
	}
  }
}

static split *sp_add_leaf(HWND hWnd, double ratio, int vertical, int left,split *top)
{ split *sp;
  if (top==NULL)
  {	RECT r;
	sp= new_split();
    sp->Parent=NULL;
    Root = top = sp;
	GetWindowRect(hSpliterBase,&r);
	sp->x=0;
	sp->y=0;
	sp->w=r.right-r.left;
	sp->h=r.bottom-r.top;
  }
  else 
  {  split *sibling;
	 sp = new_split();
     sibling = new_split();
	 sp->Parent=top;
	 sibling->Parent=top;
	 sibling->tag=top->tag;
     if (sibling->tag==leaf)
	 { sibling->o.hWnd = top->o.hWnd;
	   top->o.sp.hSplit=CreateWindow(sp_bar_class_name,"",WS_CHILD|WS_VISIBLE|WS_DLGFRAME 
//|WS_EX_NOPARENTNOTIFY 
		   ,0,0,0,0,hSpliterBase,NULL,hSplitterInst,top);
	   top->tag=branch;
	 }
	 else /* if (sibling->tag == branch) */
	 { sibling->o.sp.BottomRight= top->o.sp.BottomRight;
	   sibling->o.sp.TopLeft= top->o.sp.TopLeft;
	   sibling->o.sp.BottomRight->Parent=sibling->o.sp.TopLeft->Parent=sibling;
   	   sibling->o.sp.vertical= top->o.sp.vertical;
	   sibling->o.sp.ratio= top->o.sp.ratio;
	   sibling->o.sp.hSplit=CreateWindow(sp_bar_class_name,"Split",WS_CHILD|WS_VISIBLE|WS_DLGFRAME 
		   ,0,0,0,0,hSpliterBase,NULL,hSplitterInst,sibling);
	 }
	 top->o.sp.vertical=vertical;
	 top->o.sp.ratio=ratio;
	 if (left)
	 { top->o.sp.TopLeft=sp;
	   top->o.sp.BottomRight=sibling;
	 }
	 else
	 { top->o.sp.TopLeft=sibling;
	   top->o.sp.BottomRight=sp;
	 }
  }
  sp->tag=leaf;
  sp->o.hWnd=hWnd;
  sp_resize(top);
  return sp;
}
static void release_tree(split *sp)
{ if (sp==NULL) return;
  if (sp->tag==branch)
  { release_tree(sp->o.sp.TopLeft);
    release_tree(sp->o.sp.BottomRight);
  }
  free(sp);
}

static void split_terminate(void)
{ 
  release_layout();	
  release_tree(Root);
  Root = NULL;
}

static void split_by_position(split *sp, split * tree, RECT *rect, int x, int y)
/* find in the tree a node to split based on x and y.
   start with the rectangle of cursor positions,
   set x,y,w,h,vertical, TopLeft and BottomRight in sp.
*/
{ int vertical;
  int minx, left;
  int miny, top;
  /* determine distances to the sides */
  int dleft= x-rect->left;
  int dright=rect->right-x;
  int dtop = y-rect->top;
  int dbottom=rect->bottom-y;
  /* determine vertical and left based on the side that is closest */
  if (dleft<dright)
  { minx = dleft;
	left=1;
  }
  else
  { minx = dright;
	left = 0;
  }
  if (dtop<dbottom)
  { miny = dtop;
    top=1;
  }
  else
  { miny = dbottom;
	top = 0;
  }
  if (minx<miny)
	vertical = 1;
  else
  { vertical = 0;
	left=top;
	minx = miny;
  }
  if ((tree->tag==leaf) || (minx < 2*SPLITWIDTH) ) /* split current node based on vertical/left */
  { sp->o.sp.vertical=vertical;
    sp->x = tree->x;
	sp->y = tree->y;
	sp->w = tree->w;
	sp->h = tree->h;
    if (left)
	{   sp->o.sp.TopLeft=NULL;
	    sp->o.sp.BottomRight=tree;
	}
	else
	{	sp->o.sp.TopLeft=tree;
	    sp->o.sp.BottomRight=NULL;
	}
  }
  else /* update rectangle and recurse to subnode of tree */
  { int a = get_split(tree->o.sp.ratio,tree->o.sp.vertical,tree->w,tree->h,0);
    if (tree->o.sp.vertical)
	{ rect->top+=2*SPLITWIDTH;
      rect->bottom-=2*SPLITWIDTH;
	  a = a+tree->x;
      if (x< a)
	  {  rect->left+=2*SPLITWIDTH;
	     rect->right = a;
	     split_by_position(sp, tree->o.sp.TopLeft, rect, x, y);
	  }
	  else
	  {  rect->left = a;
	     rect->right -= 2*SPLITWIDTH;
	     split_by_position(sp, tree->o.sp.BottomRight, rect, x, y);
	  }
    }
	else
	{ rect->left+=2*SPLITWIDTH;
      rect->right-=2*SPLITWIDTH;
	  a = a+tree->y;
      if (y< a)
	  {  rect->top+=2*SPLITWIDTH;
	     rect->bottom = a;
	     split_by_position(sp, tree->o.sp.TopLeft, rect, x, y);
	  }
	  else
	  {  rect->top = a;
	     rect->bottom -= 2*SPLITWIDTH;
	     split_by_position(sp, tree->o.sp.BottomRight, rect, x, y);
	  }
    }
  }
}


static split *leaf_by_arrow(split *sp,LPARAM lParam)
/* determine subwindow by position for closing or moving */
{ int dist;
  split *d;
  if (sp->o.sp.vertical)
    dist = HIWORD(lParam);
  else
    dist = LOWORD(lParam);
  if (dist < CLOSEHEIGHT+ARROWHEIGHT+2*SPLITWIDTH) /*bootom or right */
  { d = sp->o.sp.BottomRight;
	while (d->tag==branch) d=d->o.sp.TopLeft;
  }
  else
  { d = sp->o.sp.TopLeft;
	while (d->tag==branch) d=d->o.sp.BottomRight;
  }
  return d;
}

static split *leaf_by_window(split *sp, HWND hWnd)
{ if (sp->tag==leaf && sp->o.hWnd==hWnd) 
   return sp;
  else if (sp->tag==branch)
  { split *child;
    child = leaf_by_window(sp->o.sp.TopLeft,hWnd);
	if (child != NULL) return child;
	child = leaf_by_window(sp->o.sp.BottomRight,hWnd);
	if (child != NULL) return child;
  }
  return NULL;
}

static void sp_drag_win(RECT *rect,split *sp, int x, int y)
{ int left, a;
  double ratio;
	
  rect->left=Root->x;
  rect->top=Root->y;
  rect->right=rect->left+Root->w;
  rect->bottom=rect->top+Root->h;
  split_by_position(sp, Root, rect, x, y);
  rect->left=sp->x;
  rect->top=sp->y;
  rect->right=rect->left+sp->w;
  rect->bottom=rect->top+sp->h;
  left = (sp->o.sp.TopLeft==NULL);
  ratio = left?sp->o.sp.ratio:1.0-sp->o.sp.ratio;
  a = get_split(ratio,sp->o.sp.vertical,rect->right-rect->left,rect->bottom-rect->top,SPLITWIDTH);
  if (sp->o.sp.vertical)
  { if (left)
	  rect->right=rect->left+a;
	else
	  rect->left=rect->left+a+SPLITWIDTH;	
  }
  else
  { if (left)
	  rect->bottom=rect->top+a;
	else
	  rect->top= rect->top+a+SPLITWIDTH;
  }
}
static void sp_drag_line(RECT *rect, int w, int h, split *sp)
{ 
  if (sp->o.sp.vertical)
  {  rect->left=sp->x+w;
	 rect->right=rect->left+SPLITWIDTH;
	 rect->top=sp->y;
	 rect->bottom=sp->y+sp->h;
  }
  else
  {  rect->top=sp->y+h;
	 rect->bottom=rect->top+SPLITWIDTH;
	 rect->left=sp->x;
	 rect->right=sp->x+sp->w;
  }
}
/* optinons to guide the creation of subwindows */
static int create_left = 1;
static int create_vertical = 1;
static int create_min=-1;
static double create_ratio = 0.681;
static HWND create_wnd = NULL;

void sp_create_options(int left, int vertical, double ratio, int min_wh, HWND hWnd)
{ if (left==0 || left==1) create_left=left; else create_left=1;
  if (vertical==0 || vertical==1) create_vertical=vertical; else create_vertical=!create_vertical;
  if (0.0<ratio && ratio < 1.0)
    create_ratio= ratio;
  else
    create_ratio = 0.618;
  if (min_wh>0) create_min= min_wh; else create_min=0;
  if (create_left==0) create_ratio=1-create_ratio;
  create_wnd=hWnd; 
}

UINT rec[1000]={0};
static int recptr=-1;


static LRESULT CALLBACK SplitterProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (recptr>=0) rec[recptr++]=message;
	switch (message)
	{
	case WM_CREATE:
		hSpliterBase=hWnd;
        init_layout(hSpliterBase);
		return 0;
	case WM_SIZE:
		if (wParam==SIZE_RESTORED ||wParam==SIZE_MAXIMIZED)
		  sp_setsize(NULL,0,0,LOWORD(lParam),HIWORD(lParam));
		return 1;
	case WM_ERASEBKGND:
		return 1;
	case WM_PARENTNOTIFY:
		if (LOWORD(wParam)==WM_DESTROY || LOWORD(wParam)==WM_CREATE)
		{ HWND hChildWnd = (HWND)lParam;
		  ATOM hClass;
		  hClass = (ATOM)GetClassLongPtr(hChildWnd,GCW_ATOM);
		  if (hClass== SplitterBarClass) return 0;
          SendMessage(GetParent(hWnd),WM_PARENTNOTIFY,wParam,lParam);
		  if (LOWORD(wParam)==WM_DESTROY&& Root!=NULL)
		    sp_delete_leaf(leaf_by_window(Root,hChildWnd));
		  else if (LOWORD(wParam)==WM_CREATE)
		  { split *entry=NULL;
		    if (create_wnd!=NULL)  entry = leaf_by_window(Root,create_wnd);
			if (entry==NULL) entry=Root;
			if (create_min&& entry!=NULL)
			{ if (create_vertical)
				  create_ratio = (double)create_min/entry->w;
			  else
				  create_ratio = (double)create_min/entry->h;
			  if (!create_left) create_ratio=1.0-create_ratio;
     		}
			sp_add_leaf(hChildWnd,create_ratio,create_vertical,create_left,entry);
		  }
     	  InvalidateRect(hSpliterBase,NULL,FALSE);
		}
		return 0;
	case WM_COMMAND:
	case WM_NOTIFY:
    case WM_MEASUREITEM:
	case WM_DRAWITEM: 
		return SendMessage(GetParent(hWnd),message,wParam,lParam);
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:
		split_terminate();
        break;
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}




static LRESULT CALLBACK SplitterBarProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   static BOOL split_draging = FALSE;
    static BOOL drag_painted = FALSE;
    static RECT drag_rect;
	static BOOL mouse_mid = TRUE, mouse_move= FALSE, mouse_close = FALSE;
    static LPARAM ptsOld;  
	static int drag_x, drag_y;
	static split *move_branch, *move_leaf;
	split *sp;
	sp = (split *)(LONG_PTR)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (message)
	{
	case WM_CREATE:
		SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG)(LONG_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);
		break;
	case WM_LBUTTONDOWN:
		if (mouse_mid || mouse_move)
		{	RECT rcSplit; 
		    POINT ptSplitUL;              // client upper left corner 
            POINT ptSplitLR;              // client lower right corner 

            ptsOld = lParam; 
            if (mouse_mid)
			{ ptSplitUL.x = sp->x; 
              ptSplitUL.y = sp->y; 
              ptSplitLR.x = sp->x+sp->w + 1; 
              ptSplitLR.y = sp->y+sp->h + 1; 
              drag_x = drag_y = get_split_width(sp);
			  sp_drag_line(&drag_rect,drag_x,drag_y,sp);

			}
			else if (mouse_move)
			{ 
              drag_x = sp->x+LOWORD(lParam);
			  drag_y = sp->y+HIWORD(lParam);
			  if (sp->o.sp.vertical)
			    drag_x += get_split_width(sp);
			  else
			    drag_y += get_split_width(sp);

			  ptSplitUL.x = Root->x; 
              ptSplitUL.y = Root->y; 
              ptSplitLR.x = Root->x+Root->w + 1; 
              ptSplitLR.y = Root->y+Root->h + 1; 
              move_leaf = leaf_by_arrow(sp,lParam);
			  move_branch=sp_unlink(move_leaf);
			  if (move_branch==NULL) return 0;
			  ShowWindow(move_leaf->o.hWnd,SW_HIDE);
			  ShowWindow(move_branch->o.sp.hSplit,SW_HIDE);

			  if (move_leaf==move_branch->o.sp.BottomRight) 
			    move_branch->o.sp.ratio=1.0-move_branch->o.sp.ratio;
			  move_branch->o.sp.TopLeft=NULL;
			  move_branch->o.sp.BottomRight=NULL;
			  sp_drag_win(&drag_rect,move_branch,drag_x, drag_y);
			}
			ClientToScreen(hSpliterBase, &ptSplitUL); 
            ClientToScreen(hSpliterBase, &ptSplitLR); 
            SetRect(&rcSplit, ptSplitUL.x, ptSplitUL.y, ptSplitLR.x, ptSplitLR.y); 
            ClipCursor(&rcSplit);
			SetCapture(sp->o.sp.hSplit); 
			split_draging =TRUE;
			InvalidateRect(hSpliterBase,NULL,FALSE);
			UpdateWindow(hSpliterBase);
			Rectangle(DragDC,drag_rect.left,drag_rect.top,drag_rect.right,drag_rect.bottom);
			drag_painted=TRUE;
		}
        return 0; 
	case WM_MOUSEMOVE:
		if ((wParam & MK_LBUTTON) && split_draging) 
        {   RECT rect;
			int dx, dy;
			dx = ((MAKEPOINTS(lParam)).x - (MAKEPOINTS(ptsOld)).x);	
            dy = ((MAKEPOINTS(lParam)).y - (MAKEPOINTS(ptsOld)).y);	 
			if (mouse_mid)
              sp_drag_line(&rect, drag_x+dx,drag_y+dy,sp);
		    else if (mouse_move)
			{				  move_branch->o.sp.TopLeft=NULL;
			  move_branch->o.sp.BottomRight=NULL;

  			  sp_drag_win(&rect,move_branch, drag_x+dx, drag_y+dy);
			}
            if (drag_rect.top!=rect.top||drag_rect.bottom!=rect.bottom
				|| drag_rect.left!=rect.left|| drag_rect.right!=rect.right|| !drag_painted)
			{ 
              if (drag_painted)
			  { Rectangle(DragDC,drag_rect.left,drag_rect.top,drag_rect.right,drag_rect.bottom);
			    drag_painted=FALSE;
			  }
			  drag_rect=rect;
			  Rectangle(DragDC,drag_rect.left,drag_rect.top,drag_rect.right,drag_rect.bottom);
			  drag_painted=TRUE;
			}
		}
		else 
		{ int dist; /*distance from center */
		  if (sp->o.sp.vertical)
		    dist = abs(HIWORD(lParam) - sp->h/2);
		  else
		    dist = abs(LOWORD(lParam) - sp->w/2);
          mouse_mid = dist< sp->o.sp.mid_max;
		  mouse_move = (!mouse_mid) && (dist< sp->o.sp.mid_max+ARROWHEIGHT+SPLITWIDTH);
		  mouse_close = (!mouse_mid) && (!mouse_move);
		}
        return 0;
	case WM_LBUTTONUP:
	    if (split_draging)
		{  ClipCursor(NULL); 
          ReleaseCapture(); 
          split_draging=FALSE;
		  if (drag_painted)
		  { Rectangle(DragDC,drag_rect.left,drag_rect.top,drag_rect.right,drag_rect.bottom);
		    drag_painted=FALSE;
		  }

	      if (mouse_mid)
		  { int dx, dy;
		    RECT rect;
			dx = ((MAKEPOINTS(lParam)).x - (MAKEPOINTS(ptsOld)).x);	
            dy = ((MAKEPOINTS(lParam)).y - (MAKEPOINTS(ptsOld)).y);	 
		    set_split_ratio(sp,drag_x+dx,drag_y+dy);
			rect.left=sp->x; rect.top=sp->y; rect.right=rect.left+sp->w; rect.bottom=rect.top+sp->h;
		    InvalidateRect(hSpliterBase,&rect,TRUE);
		  }
		  else if (mouse_move)
		  { 
		    if (move_branch->o.sp.TopLeft==NULL) 
			{ move_branch->o.sp.TopLeft= move_leaf;
			  move_branch->Parent=move_branch->o.sp.BottomRight->Parent;
			  if (move_branch->Parent!=NULL)
			  { if (move_branch->Parent->o.sp.TopLeft==move_branch->o.sp.BottomRight)
			      move_branch->Parent->o.sp.TopLeft=move_branch;
			    else
			      move_branch->Parent->o.sp.BottomRight=move_branch;
			  }
			  else
				  Root=move_branch;
			}
			else 
			{ move_branch->o.sp.BottomRight= move_leaf;
	          move_branch->o.sp.ratio=1.0-move_branch->o.sp.ratio;
			  move_branch->Parent=move_branch->o.sp.TopLeft->Parent;
			  if (move_branch->Parent!=NULL)
			  { if (move_branch->Parent->o.sp.TopLeft==move_branch->o.sp.TopLeft)
			      move_branch->Parent->o.sp.TopLeft=move_branch;
			    else
			      move_branch->Parent->o.sp.BottomRight=move_branch;
			  }
			  else
				  Root=move_branch;
			}
			move_branch->o.sp.TopLeft->Parent=move_branch;
			move_branch->o.sp.BottomRight->Parent=move_branch;
			ShowWindow(move_leaf->o.hWnd,SW_SHOW);
			ShowWindow(move_branch->o.sp.hSplit,SW_SHOW);
			sp_resize(NULL);
			InvalidateRect(hSpliterBase,NULL,FALSE);
		  }
		}
        return 0; 		
	case WM_LBUTTONDBLCLK:
		if (mouse_mid)
		{ RECT rect;
		  if (sp->o.sp.vertical)
		  { split *aux;
			aux = sp->o.sp.TopLeft;
			sp->o.sp.TopLeft=sp->o.sp.BottomRight;
			sp->o.sp.BottomRight= aux;
			sp->o.sp.vertical = 0;
            sp->o.sp.ratio = 1.0 -sp->o.sp.ratio;
		  }
		  else
		  { sp->o.sp.vertical = 1;
		  }
		  sp_resize(sp);
		  rect.left=sp->x; rect.top=sp->y; rect.right=rect.left+sp->w; rect.bottom=rect.top+sp->h;
		  InvalidateRect(hSpliterBase,&rect,FALSE);
		} 
		else if (mouse_close)
		{ split *child =leaf_by_arrow(sp,lParam);
		  if (child!=NULL && child->tag==leaf)  
			  SendMessage(child->o.hWnd,WM_CLOSE,0,0);
		}
		return 0;
	case WM_SETCURSOR: 
		if (mouse_mid)
		{ if (sp->o.sp.vertical)
		    SetCursor(hVCursor); 
		  else
            SetCursor(hHCursor); 
		}
		else if (mouse_move)
			SetCursor(hXCursor);
		else
			SetCursor(hCCursor);
	    return 0;
	case WM_PAINT:
		{ 	PAINTSTRUCT ps;
		    HDC hdc;
            hdc = BeginPaint(hWnd, &ps);
			if (sp->o.sp.vertical)
			{ BitBlt(hdc, 0, SPLITWIDTH/2, CLOSEWIDTH, CLOSEHEIGHT, CloseDC, 0, 0, SRCCOPY);
              BitBlt(hdc, 0, 3*SPLITWIDTH/2+CLOSEHEIGHT, ARROWWIDTH, ARROWHEIGHT, TopDC, 0, 0, SRCCOPY);
              BitBlt(hdc, 0, sp->h/2-ARROWHEIGHT, ARROWWIDTH, ARROWHEIGHT, BottomDC, 0, 0, SRCCOPY);
			  BitBlt(hdc, 0, sp->h/2, ARROWWIDTH, ARROWHEIGHT, TopDC, 0, 0, SRCCOPY);
              BitBlt(hdc, 0, sp->h-3*SPLITWIDTH/2-CLOSEHEIGHT-ARROWHEIGHT, ARROWWIDTH, ARROWHEIGHT, BottomDC, 0, 0, SRCCOPY);
              BitBlt(hdc, 0, sp->h-CLOSEHEIGHT-SPLITWIDTH/2, CLOSEWIDTH, CLOSEHEIGHT, CloseDC, 0, 0, SRCCOPY);
			}
			else
			{ BitBlt(hdc, SPLITWIDTH/2, 0, CLOSEWIDTH, CLOSEHEIGHT, CloseDC, 0, 0, SRCCOPY);
              BitBlt(hdc, 3*SPLITWIDTH/2+CLOSEHEIGHT, 0, ARROWHEIGHT, ARROWWIDTH, LeftDC, 0, 0, SRCCOPY);
              BitBlt(hdc, sp->w/2-ARROWHEIGHT, 0, ARROWHEIGHT, ARROWWIDTH, RightDC, 0, 0, SRCCOPY);
			  BitBlt(hdc, sp->w/2, 0, ARROWHEIGHT, ARROWWIDTH, LeftDC, 0, 0, SRCCOPY);
              BitBlt(hdc, sp->w-3*SPLITWIDTH/2-CLOSEHEIGHT-ARROWHEIGHT, 0, ARROWHEIGHT, ARROWWIDTH, RightDC, 0, 0, SRCCOPY);
              BitBlt(hdc, sp->w-CLOSEHEIGHT-SPLITWIDTH/2, 0, CLOSEWIDTH, CLOSEHEIGHT, CloseDC, 0, 0, SRCCOPY);
			}
		    EndPaint(hWnd, &ps);
		}
		return 0;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY: /* Do not call PostQuitMessage */
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

static ATOM RegisterSplitterBarClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= SplitterBarProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= sizeof(LONG_PTR);
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= (HBRUSH)(COLOR_SPLIT+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= sp_bar_class_name;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

static ATOM RegisterSplitterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= SplitterProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_MENUBAR+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= sp_class_name;
	wcex.hIconSm		= NULL;
	return RegisterClassEx(&wcex);
}



void sp_init(HINSTANCE hInstance)
/* returns pointer to the toplevel splitter */
{ if (Root!=NULL) fatal_error("sp_init called twice");
  hSplitterInst = hInstance;
  SplitterClass = RegisterSplitterClass(hInstance);
  SplitterBarClass = RegisterSplitterBarClass(hInstance);
}




HWND sp_CreateSplitter(LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight,
    HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)

{ HWND hWnd;
   hWnd = CreateWindow(sp_class_name, lpWindowName, dwStyle,x,y,nWidth, nHeight,
					hWndParent, hMenu, hInstance, lpParam);

   return hWnd;
}

