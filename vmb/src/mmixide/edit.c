#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "winmain.h"
#include "edit.h"
#define STATIC_BUILD
#include "../scintilla/include/scilexer.h"
int autosave = 0;
int show_line_no = 0;
int show_profile = 0;
int tabwidth = 4;

void choose_color(HWND hDlg, COLORREF *color)
	  { CHOOSECOLOR cc;                 // common dialog box structure 
        static COLORREF acrCustClr[16]; // array of custom colors 
	    // Initialize CHOOSECOLOR 
        ZeroMemory(&cc, sizeof(cc));
        cc.lStructSize = sizeof(cc);
        cc.hwndOwner = hDlg;
        cc.lpCustColors = (LPDWORD) acrCustClr;
		cc.rgbResult = *color;
        cc.Flags = CC_FULLOPEN | CC_RGBINIT;
        if (ChooseColor(&cc)==TRUE) {
		    *color = cc.rgbResult;
		}
	  }


INT_PTR CALLBACK    
OptionEditorDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ 
  switch ( message )
  { case WM_INITDIALOG:
      CheckDlgButton(hDlg,IDC_CHECK_AUTOSAVE,autosave?BST_CHECKED:BST_UNCHECKED);
	  CheckDlgButton(hDlg,IDC_CHECK_LINENO,show_line_no?BST_CHECKED:BST_UNCHECKED);
	  CheckDlgButton(hDlg,IDC_CHECK_PROFILE,show_profile?BST_CHECKED:BST_UNCHECKED);
	  SendMessage(GetDlgItem(hDlg,IDC_SPIN_TABWIDTH),UDM_SETRANGE,0,(LPARAM) MAKELONG (32,0));
	  SetDlgItemInt(hDlg,IDC_TABWIDTH,tabwidth,FALSE);
      return TRUE;
    case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
	  { autosave =IsDlgButtonChecked(hDlg,IDC_CHECK_AUTOSAVE);
		show_line_no =IsDlgButtonChecked(hDlg,IDC_CHECK_LINENO);
        set_lineno_width();
		show_profile =IsDlgButtonChecked(hDlg,IDC_CHECK_PROFILE);
        set_profile_width();
		update_profile();
		tabwidth=GetDlgItemInt(hDlg,IDC_TABWIDTH,NULL,FALSE);
		set_tabwidth();
		EndDialog(hDlg, TRUE);
        return TRUE;
      } 
	  else if (wparam==IDCANCEL)
	  { EndDialog(hDlg, TRUE);
        return TRUE;
	  }
	  else if (wparam==IDC_CHOOSE_OPCODE)
	  { choose_color(hDlg,&syntax_color[SCE_MMIXAL_OPCODE_VALID]);
	    InvalidateRect(GetDlgItem(hDlg,IDC_STATIC_OPCODE),NULL,FALSE);
		set_text_style();
	  }
 	  else if (wparam==IDC_CHOOSE_OPERROR)
	  { choose_color(hDlg,&syntax_color[SCE_MMIXAL_OPCODE_UNKNOWN]);
	    InvalidateRect(GetDlgItem(hDlg,IDC_STATIC_OPERROR),NULL,FALSE);
		set_text_style();
	  }
 	  else if (wparam==IDC_CHOOSE_REGISTER)
	  { choose_color(hDlg,&syntax_color[SCE_MMIXAL_REGISTER]);
	    InvalidateRect(GetDlgItem(hDlg,IDC_STATIC_REGISTER),NULL,FALSE);
		set_text_style();
	  }
 	  else if (wparam==IDC_CHOOSE_SYMBOL)
	  { choose_color(hDlg,&syntax_color[SCE_MMIXAL_SYMBOL]);
	    InvalidateRect(GetDlgItem(hDlg,IDC_STATIC_SYMBOL),NULL,FALSE);
		set_text_style();
	  }
 	  else if (wparam==IDC_CHOOSE_COMMENT)
	  { choose_color(hDlg,&syntax_color[SCE_MMIXAL_COMMENT]);
	    InvalidateRect(GetDlgItem(hDlg,IDC_STATIC_COMMENT),NULL,FALSE);
		set_text_style();
	  }
     break;
	case WM_DRAWITEM:
	 { LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT) lparam;
	   int c=-1;
       if (lpDrawItem->itemAction!=ODA_DRAWENTIRE)
         return FALSE;
	   if (wparam==IDC_STATIC_OPCODE)
		   c = SCE_MMIXAL_OPCODE_VALID;
	   else	if (wparam==IDC_STATIC_OPERROR)
		   c = SCE_MMIXAL_OPCODE_UNKNOWN;
	   else	if (wparam==IDC_STATIC_REGISTER)
		   c = SCE_MMIXAL_REGISTER;
	   else	if (wparam==IDC_STATIC_SYMBOL)
		   c = SCE_MMIXAL_SYMBOL;
	   else	if (wparam==IDC_STATIC_COMMENT)
		   c = SCE_MMIXAL_COMMENT; 
	   if (c>=0)
	   { HBRUSH hb=CreateSolidBrush(syntax_color[c]);
	     FillRect(lpDrawItem->hDC,&lpDrawItem->rcItem,hb);
		 DeleteObject(hb);
	   }
	   return TRUE;
	}
  }
  return FALSE;
}
