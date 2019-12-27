#include <windows.h>
#include <string.h>
#include <wingdi.h>
#include "winopt.h"


/* these depend only on the font, which is the same for all inspectors */

HFONT hFixedFont=NULL;
HFONT hNarrowFont=NULL;
HFONT hVarFont=NULL;
HFONT hGUIFont=NULL;
int fixed_line_height=10;
int var_line_height=20;
int fixed_char_width=0;
int fixed_char_height=0;
int var_char_height=0;
int border_size=0;
int edge_size=0;
int separator_width=0;
int separator_height=0; 
int version_width=0; /* length of the version string */
int fontsize=10;
int screen_width=640;
int screen_height=480;

static int dpi;

static LOGFONT varlf={0};
void set_var_font(int fontsize)
{ if (hVarFont!=NULL) { DeleteObject(hVarFont); hVarFont=NULL; }
  varlf.lfHeight = -(((fontsize*dpi)+36)/72);  
  hVarFont = CreateFontIndirect(&varlf);
  if (hVarFont==NULL)
	  hVarFont=hGUIFont;
}

static LOGFONT fixedlf={0};
void set_fixed_font(int fontsize)
{ if (hFixedFont!=NULL) { DeleteObject(hFixedFont); hFixedFont=NULL; }
  fixedlf.lfHeight = -(((fontsize*dpi)+36)/72);  
  hFixedFont = CreateFontIndirect(&fixedlf);
  if (hFixedFont==NULL)
    hFixedFont = GetStockObject(ANSI_FIXED_FONT); 
}


static LOGFONT narrowlf={0};
void set_narrow_font(int width, int height)
{   int i;
	TEXTMETRIC tm;
   HDC hdc; 
   HFONT holdfnt;
     hdc=GetDC(NULL);

   if (hNarrowFont!=NULL) { DeleteObject(hNarrowFont); hNarrowFont=NULL; }
   for (i=0;i<3;i++)
   { narrowlf.lfHeight = height;
     narrowlf.lfWidth = width-i;
     hNarrowFont = CreateFontIndirect(&narrowlf); 
     holdfnt=SelectObject(hdc, hNarrowFont);
     ZeroMemory(&tm, sizeof(tm));
     GetTextMetrics(hdc,&tm);
     SelectObject(hdc,holdfnt);
     if (tm.tmAveCharWidth<= width) break;
   }
   if (hNarrowFont==NULL)
     hNarrowFont = GetStockObject(ANSI_FIXED_FONT); 
}


void set_font_size(int size)
{  TEXTMETRIC tm;
   HDC hdc; 
   HFONT holdfnt;

   if (size<2 || size>32) size = DEFAULT_FONT_SIZE;
   fontsize=size;
   set_var_font(fontsize);
   set_fixed_font(fontsize);
   hdc=GetDC(NULL);

   holdfnt=SelectObject(hdc, hFixedFont);
   ZeroMemory(&tm, sizeof(tm));
   GetTextMetrics(hdc,&tm);
   SelectObject(hdc,holdfnt);
   fixed_char_width=tm.tmAveCharWidth;
   fixed_char_height=tm.tmHeight; 
   fixed_line_height= (fixed_char_height*12+9)/10; /*add 20% baselineskip */
   separator_height=fixed_line_height-fixed_char_height;
   if (separator_height<2) separator_height=2;
   separator_width = fixed_char_width/2;
   if (separator_width<2) separator_width=2;
   set_narrow_font(fixed_char_width/2,fixed_char_height-separator_height);

   holdfnt=SelectObject(hdc, hVarFont);
   ZeroMemory(&tm, sizeof(tm));
   GetTextMetrics(hdc,&tm);
   SelectObject(hdc,holdfnt);
   var_char_height=tm.tmHeight; 
   var_line_height= tm.tmHeight+tm.tmExternalLeading;
   /* var_line_height= (var_char_height*12+9)/10; /*add 20% baselineskip */

   ReleaseDC(NULL,hdc);
}

void change_font_size(int scale)
/* scale==0 default size, scale==+1 increase size, scale ==-1 decrease size */
{  if (scale==0) fontsize=DEFAULT_FONT_SIZE;
   else if (scale>0) fontsize++;
   else fontsize--;
   if (fontsize<2) fontsize=2;
   else if (fontsize>32) fontsize=32;
   set_font_size(fontsize);
#if 0
   /* logscaling of font sizes */
   static double factor=1.0;

   /* the fourth root of 2 doubles in four steps */
#define  root4_2 1.1892071150027210667174999705605
#define  root6_2 1.1224620483093729814335330496792
   if (scale==0) factor=1.0;
   else if (scale>0) factor=factor*root6_2;
   else factor=factor/root6_2;
   fontsize= (int)(10*factor+0.5);
#endif
}

void init_layout(int interactive)
{ SIZE size;
  HFONT holdfnt;
  HDC hdc;                  // display device context of owner window

  screen_width = GetSystemMetrics(SM_CXSCREEN);
  screen_height = GetSystemMetrics(SM_CYSCREEN);
  border_size = GetSystemMetrics(SM_CXFIXEDFRAME);
  edge_size = GetSystemMetrics(SM_CXEDGE);
  hdc=GetDC(NULL);
  dpi = GetDeviceCaps(hdc, LOGPIXELSY);
  hGUIFont=GetStockObject(DEFAULT_GUI_FONT);
  holdfnt=SelectObject(hdc, hGUIFont);
  GetTextExtentPoint32(hdc,version,(int)strlen(version),&size);
  version_width=size.cx;
  SelectObject(hdc,holdfnt);
  ReleaseDC(NULL,hdc);
  
  varlf.lfCharSet=ANSI_CHARSET; 
  varlf.lfWeight = FW_NORMAL;
  varlf.lfPitchAndFamily = VARIABLE_PITCH  | FF_SWISS;
  varlf.lfQuality =  PROOF_QUALITY;
  varlf.lfOutPrecision=OUT_TT_ONLY_PRECIS;
  varlf.lfItalic=FALSE;
  strcpy_s(varlf.lfFaceName,sizeof(varlf.lfFaceName),"Microsoft Sans Serif");

  fixedlf.lfCharSet=ANSI_CHARSET;
  fixedlf.lfWeight = FW_NORMAL;
  fixedlf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
  fixedlf.lfQuality =  PROOF_QUALITY;
  fixedlf.lfOutPrecision=OUT_TT_ONLY_PRECIS;
  fixedlf.lfItalic=FALSE;
  strcpy_s(fixedlf.lfFaceName,sizeof(fixedlf.lfFaceName),"Courier New");

  narrowlf.lfCharSet=ANSI_CHARSET;
  narrowlf.lfWeight = FW_SEMIBOLD;
  narrowlf.lfPitchAndFamily = FIXED_PITCH  | FF_MODERN;
  narrowlf.lfQuality =  PROOF_QUALITY;
  narrowlf.lfOutPrecision=OUT_TT_ONLY_PRECIS;
  narrowlf.lfItalic=FALSE;

  change_font_size(0);

  if (interactive)
  { CHOOSEFONT cf;            
    ZeroMemory(&cf, sizeof(cf));
    cf.lStructSize = sizeof(cf);
    cf.rgbColors = RGB(0,0,0);
    cf.Flags = CF_SCREENFONTS  
	       | CF_TTONLY  
 	       | CF_FIXEDPITCHONLY
	       | CF_FORCEFONTEXIST 
		   | CF_NOVERTFONTS
           | CF_INITTOLOGFONTSTRUCT 
           | CF_SELECTSCRIPT
	  ;
    cf.hwndOwner = hMainWnd;
    cf.lpLogFont = &fixedlf;
    ChooseFont(&cf);
	change_font_size(0);
  }
}
