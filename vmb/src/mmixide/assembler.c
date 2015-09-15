#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <setjmp.h> 
#include "resource.h"
#include "winmain.h"
#define _MMIXAL_
#include "mmixlib.h"
#include "info.h"
#include "breakpoints.h"
#include "assembler.h"


extern jmp_buf mmixal_exit;


int x_option = 0;
int b_option = 80;
int l_option = 0;
int auto_assemble=0;
int auto_close_errors=0;

INT_PTR CALLBACK    
OptionAssemblerDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{ 
  switch ( message )
  { case WM_INITDIALOG:
      CheckDlgButton(hDlg,IDC_CHECK_X,x_option?BST_CHECKED:BST_UNCHECKED);
      CheckDlgButton(hDlg,IDC_CHECK_LISTING,l_option?BST_CHECKED:BST_UNCHECKED); 
      CheckDlgButton(hDlg,IDC_CHECK_AUTOASSEMBLE,auto_assemble?BST_CHECKED:BST_UNCHECKED); 
      CheckDlgButton(hDlg,IDC_CHECK_AUTOCLOSE_ERRORS,auto_close_errors?BST_CHECKED:BST_UNCHECKED); 
	  SetDlgItemInt(hDlg,IDC_BUFFERSIZE,b_option,FALSE);
	  SendMessage(GetDlgItem(hDlg,IDC_SPIN_BUFFERSIZE),UDM_SETRANGE,0,(LPARAM) MAKELONG (1000,0));
      return TRUE;
    case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
      { x_option=IsDlgButtonChecked(hDlg,IDC_CHECK_X);
        l_option=IsDlgButtonChecked(hDlg,IDC_CHECK_LISTING);
        auto_assemble=IsDlgButtonChecked(hDlg,IDC_CHECK_AUTOASSEMBLE);
        auto_close_errors=IsDlgButtonChecked(hDlg,IDC_CHECK_AUTOCLOSE_ERRORS);
		b_option=GetDlgItemInt(hDlg,IDC_BUFFERSIZE,NULL,FALSE);
        EndDialog(hDlg, TRUE);
        return TRUE;
      } 
	  else if (wparam==IDCANCEL)
	  { EndDialog(hDlg, TRUE);
        return TRUE;
	  }
     break;
  case WM_HELP:
    ide_help("help\\options\\assembler.html");
    return TRUE;
  }
  return FALSE;
}

/* Assemble MMIX */

static char full_mml_name[MAX_PATH+1]={0};

static char *get_mml_name(char *full_mms_name)
{  if (full_mms_name==NULL || full_mms_name[0]==0) return NULL;
   strncpy_s(full_mml_name,MAX_PATH,full_mms_name,MAX_PATH);
   full_mml_name[strlen(full_mml_name)-1]='l';
   return full_mml_name;
}

void report_error(char*message,int file_no,int line_no)
{ if(message[0]=='!') /* fatal error */
  { ide_add_error(message+1,file_no,line_no);
    ide_status(message+1);
  } 
  else if(message[0]=='*') /* warning */
	ide_add_error(message+1,file_no,line_no);
  else /* error */
	ide_add_error(message,file_no,line_no);
  err_count++;
  if(listing_file){
    if(!line_listed)flush_listing_line("****************** ");
    if(message[0]=='*')fprintf(listing_file,
       "************ warning: %s\n",message+1);
    else if(message[0]=='!')fprintf(listing_file,
       "******** fatal error: %s!\n",message+1);
    else fprintf(listing_file,
       "********** error: %s!\n",message);
  }
  if(message[0]=='!')longjmp(mmixal_exit,-2); /* fatal error */
}



int mmix_assemble(int file_no)
{ int err_count;
  char *source;
  char *listing;

  ide_clear_error_list();
  clear_file_info(file_no);
  source = file2fullname(file_no);
  if (l_option) listing=get_mml_name(source);
  else listing=NULL;
  ide_status("mmixal running ...");

   /* for the FILE instruction to work, the Current Directory 
   should be set to the directory of the file we assemble.
   */
  { static char name[MAX_PATH+2], *tail;
    GetFullPathName(source,MAX_PATH,name,&tail);
    *tail=0;
    SetCurrentDirectory(name);
   }

  err_count = mmixal(source,NULL,listing,x_option,b_option);
  symtab_add_file(file_no,trie_root);
  trie_root=NULL;
  if (err_count!=0) 
  { clear_file_info(file_no);
    ide_status("Errors in mmixal.");
  }
  else
	ide_status("mmixal done.");
  show_breakpoints();
  return err_count;
}
