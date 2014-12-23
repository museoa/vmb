#include <windows.h>
#include <string.h>

#include "winopt.h"


/* these depend only on the font, which is the same for all inspectors */

HFONT hFixedFont=NULL;
HFONT hVarFont=NULL;
int fixed_line_height=0;
int fixed_char_width=0;
int fixed_char_height=0;
int version_width=0; /* length of the version string */

void init_layout(int interactive)
{ SIZE size;
  HFONT holdfnt;
  HDC hdc;                  // display device context of owner window
  CHOOSEFONT cf;            // common dialog box structure
  LOGFONT lf;        // logical font structure
  TEXTMETRIC tm;
  hdc=GetDC(NULL);


  ZeroMemory(&cf, sizeof(cf));
  cf.lStructSize = sizeof (cf);
  ZeroMemory(&lf, sizeof(lf));
  ZeroMemory(&tm, sizeof(tm));
  cf.hwndOwner = hMainWnd;
  cf.lpLogFont = &lf;
  lf.lfCharSet=ANSI_CHARSET;
  lf.lfHeight = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
  lf.lfWeight = FW_NORMAL;
  lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
  lf.lfQuality =  0x05 /*CLEARTYPE_QUALITY */;
  strcpy_s(lf.lfFaceName,sizeof(lf.lfFaceName),"Courier New");
  cf.rgbColors = RGB(0,0,0);
  cf.Flags = CF_SCREENFONTS  
	       | CF_TTONLY  
 	       | CF_FIXEDPITCHONLY
	       | CF_FORCEFONTEXIST 
		   | CF_NOVERTFONTS
           | CF_INITTOLOGFONTSTRUCT 
           | CF_SELECTSCRIPT
	  ;

  if (interactive && ChooseFont(&cf)==TRUE)
    hFixedFont = CreateFontIndirect(cf.lpLogFont);
  else 
	hFixedFont = CreateFontIndirect(cf.lpLogFont);
  if (hFixedFont==NULL)
    hFixedFont = GetStockObject(ANSI_FIXED_FONT); 

  holdfnt=SelectObject(hdc, hFixedFont);
  GetTextMetrics(hdc,&tm);
  SelectObject(hdc,holdfnt);
  fixed_char_width=tm.tmAveCharWidth;
  fixed_char_height=tm.tmHeight; 
  fixed_line_height= (fixed_char_height*12+9)/10; /*add 20% baselineskip */
  hVarFont=GetStockObject(DEFAULT_GUI_FONT);
  holdfnt=SelectObject(hdc, hVarFont);
  GetTextExtentPoint32(hdc,version,(int)strlen(version),&size);
  version_width=size.cx;
  SelectObject(hdc,holdfnt);
  ReleaseDC(NULL,hdc);
}

#if 0
BOOL GetTextMetrics(
  HDC hdc,            // handle to DC
  LPTEXTMETRIC lptm   // text metrics
);
int GetTextFace(
  HDC hdc,            // handle to DC
  int nCount,         // length of typeface name buffer
  LPTSTR lpFaceName   // typeface name buffer
);

(WPARAM)GetStockObject(DEFAULT_GUI_FONT)
{ int nHeight;
      HDC hdc;
      int ydpi;
	   hdc =GetDC(NULL);
	   ydpi = GetDeviceCaps(hdc, LOGPIXELSY);
	   ReleaseDC(NULL,hdc);
	   nHeight = MulDiv(8,ydpi , 72);
	   hLogFont = CreateFont(-nHeight,0,0,0,FW_REGULAR,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY,FIXED_PITCH|FF_MODERN,"MS Sans Serif");
	}
#endif