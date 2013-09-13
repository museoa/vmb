#include <windows.h>
#include <stdio.h>
#include "winmain.h"
#include "print.h"

#define STATIC_BUILD
#include "../scintilla/include/scintilla.h"
extern sptr_t ed_send(unsigned int msg,uptr_t wparam,sptr_t lparam);



int page_setup(void)
{ PAGESETUPDLG psd;    // common dialog box structure
           // owner window

// Initialize PAGESETUPDLG
ZeroMemory(&psd, sizeof(psd));
psd.lStructSize = sizeof(psd);
psd.hwndOwner   = hMainWnd;
psd.hDevMode    = NULL; // Don't forget to free or store hDevMode
psd.hDevNames   = NULL; // Don't forget to free or store hDevNames
psd.Flags       = PSD_INTHOUSANDTHSOFINCHES | PSD_MARGINS;
psd.rtMargin.top = 1000;
psd.rtMargin.left = 1250;
psd.rtMargin.right = 1250;
psd.rtMargin.bottom = 1000;
psd.lpfnPagePaintHook = NULL;

if (PageSetupDlg(&psd)==TRUE) {
    // check paper size and margin values here
}
return 1;
}


int print(void)
{ PRINTDLG pd;

  // Initialize PRINTDLG
  ZeroMemory(&pd, sizeof(pd));
  pd.lStructSize = sizeof(pd);
  pd.hwndOwner   = hMainWnd;
  pd.hDevMode    = NULL;     // Don't forget to free or store hDevMode
  pd.hDevNames   = NULL;     // Don't forget to free or store hDevNames
  pd.Flags       = PD_USEDEVMODECOPIESANDCOLLATE | PD_RETURNDC; 
  pd.nCopies     = 1;
  pd.nFromPage   = 0xFFFF; 
  pd.nToPage     = 0xFFFF; 
  pd.nMinPage    = 1; 
  pd.nMaxPage    = 0xFFFF; 

  if (PrintDlg(&pd)!=TRUE) return 0;
  else
  { /* taken from Printing a Document */
 
    DOCINFO di;
	SIZE szMetric;
	int nError;
	int cWidthPels, xLeft,yTop;
	struct Sci_RangeToFormat srf;

    memset( &di, 0, sizeof(DOCINFO) );
    di.cbSize = sizeof(DOCINFO); 
    di.lpszDocName = fullname; 
    di.lpszOutput = (LPTSTR) NULL; 
    di.lpszDatatype = (LPTSTR) NULL; 
    di.fwType = 0; 
 
    // Begin a print job by calling the StartDoc function. 
 
    nError = StartDoc(pd.hDC, &di); 
    if (nError == SP_ERROR) 
    { 
        // errhandler("StartDoc", hwnd); 
        goto Error; 
    } 
 
    // Inform the driver that the application is about to begin 
    // sending data. 
 
    nError = StartPage(pd.hDC); 
    if (nError <= 0) 
    { 
        // errhandler("StartPage", hwnd); 
        goto Error; 
    } 
 
 
 
    // Retrieve the width of the string that specifies the full path 
    // and filename for the file that contains the bitmap. 
 
    GetTextExtentPoint32(pd.hDC,fullname, (int)strlen(fullname),&szMetric); 
 
    // Compute the starting point for the text-output operation. The 
    // string will be centered horizontally and positioned three lines 
    // down from the top of the page. 
    cWidthPels = GetDeviceCaps(pd.hDC, HORZRES); 

    xLeft = ((cWidthPels / 2) - (szMetric.cx / 2)); 
    yTop = (szMetric.cy * 3); 
 
    // Print the path and filename for the bitmap, centered at the top 
    // of the page. 
 
    TextOut(pd.hDC, xLeft, yTop, fullname, (int)strlen(fullname)); 
 

	ed_send(SCI_SETPRINTCOLOURMODE,SC_PRINT_BLACKONWHITE,0);
 
	srf.chrg.cpMin=0;
	srf.chrg.cpMax=100;
    srf.hdc=srf.hdcTarget=pd.hDC;
	srf.rcPage.top=0;
	srf.rcPage.bottom = GetDeviceCaps(pd.hDC, VERTRES); 
	srf.rcPage.left=0;
	srf.rcPage.right = GetDeviceCaps(pd.hDC, HORZRES); 
	srf.rc.top=srf.rcPage.top+100;
	srf.rc.bottom=srf.rcPage.bottom-100;
	srf.rc.left=srf.rcPage.left+100;
	srf.rc.right=srf.rcPage.right-100;

	ed_send(SCI_FORMATRANGE,1,(sptr_t)&srf);

	

    // Determine whether the user has pressed the Cancel button in the 
    // AbortPrintJob dialog box; if the button has been pressed, call 
    // the AbortDoc function. Otherwise, inform the spooler that the 
    // page is complete. 
 
    nError = EndPage(pd.hDC); 
 
    if (nError <= 0) 
    { 
        //errhandler("EndPage", hwnd); 
        goto Error; 
    } 
 
    // Inform the driver that document has ended. 
 
    nError = EndDoc(pd.hDC); 
    //if (nError <= 0) 
      //  errhandler("EndDoc", hwnd); 
 
Error: 
    // Enable the application's window. 
 
    EnableWindow(hMainWnd, TRUE); 
 
    // Remove the AbortPrintJob dialog box. 
 
   // DestroyWindow(hdlgCancel); 


    // Delete DC when done.
    DeleteDC(pd.hDC);
  return 1;
  }
}
