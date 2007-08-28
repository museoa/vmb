/*
    Copyright 2005 Martin Ruckert
    
    ruckertm@acm.org

    This file is part of the MMIX Motherboard project

    This file is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this software; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#include "resource.h"
#include "win32main.h"
#include <io.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include "message.h"
#include "bus-arith.h"
#include "bus-util.h"
#include "option.h"
#include "param.h"
#include "error.h"
#include "main.h"

#ifdef WIN32

static void open_file(void);

void InitControlls(HINSTANCE hInst,HWND hWnd)
{
}

void PositionControlls(HWND hWnd,int width, int height)
{  
   power_led_position(2,8);
}

void process_focus(int on)
{
}
BOOL APIENTRY   
SettingsDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam )
{

  switch ( message )
  { case WM_INITDIALOG:
      SetDlgItemText(hDlg,IDC_ADDRESS,hexaddress);
      SetDlgItemText(hDlg,IDC_FILE,filename);
      return TRUE;
   case WM_SYSCOMMAND:
      if( wparam == SC_CLOSE ) 
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
    case WM_COMMAND:
      if( wparam == IDOK )
      { GetDlgItemText(hDlg,IDC_ADDRESS,tmp_option,MAXTMPOPTION);
	    set_option(&hexaddress,tmp_option);
		hextochar(hexaddress,address,8);
        GetDlgItemText(hDlg,IDC_FILE,tmp_option,MAXTMPOPTION);
	    set_option(&filename,tmp_option);
		open_file();
      }
	  else if (HIWORD(wparam) == BN_CLICKED  && LOWORD(wparam) == IDC_BROWSE) 
	  { OPENFILENAME ofn;       /* common dialog box structure */
         /* Initialize OPENFILENAME */
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hMainWnd;
        ofn.lpstrFile = tmp_option;
        ofn.nMaxFile = MAXTMPOPTION;
        ofn.lpstrFilter = "All\0*.*\0Rom\0*.rom\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        /* Display the Open dialog box. */
        if (GetOpenFileName(&ofn)==TRUE) 
		   SetDlgItemText(hDlg,IDC_FILE,tmp_option);
	  }
     if (wparam == IDOK || wparam == IDCANCEL)
      { EndDialog(hDlg, TRUE);
        return TRUE;
      }
      break;
  }
  return FALSE;
}


void get_settings(void)
 {
   DialogBox(hInst,MAKEINTRESOURCE(IDD_SETTINGS),hMainWnd,SettingsDialogProc);
 }
 
#endif

char version[]="$Revision: 1.1 $ $Date: 2007-08-28 12:21:03 $";

char howto[] =
"\n"
"The program will contact the motherboard at host:port\n"
"and register itself with the given satrt address.\n"
"Then, the program will read the file and answer read requests from the bus.\n"
"\n"
;

static unsigned char *rom=NULL;
typedef unsigned long Word;

#define PAGESIZE (1<<10) /* one kbyte */
#define BIOSFILEID	0x0253504D
#define WORDLEN 4

/*!
 * \fn void processUMPSFile(Word* biosBuf,long lSize)
 * \author Martin Hauser <info@martin-hauser.net>
 * \param biosBuf Buff filled with command words
 * \param lSize the size of the buffer
 * \brief read words into rom specificed by biosBuf
 * \warning Make sure that biosBuf is a malloc'ed structure, nothing local!
 *
 * This function will essentially walk through all the words of biosBuf and
 * attempt to convert them one by one into the char structure of ::rom. Then
 * it'll calculate the size to be passed on to the rom file and finally free
 * the buffer turned over.
 */

void processUMPSFile(Word* biosBuf,long lSize)
{
	int i,k;
	int j = 0;
	
	rom = malloc(lSize * WORDLEN); /*!< allocate real rom buffer (is of char) */
	
	for(i = 0; i < lSize; i++) /*!< walk through biosBuf */
	{  
	    for(k=0;k<4;k++)    
		    rom[j + k] = biosBuf[i] >> (32 - (k+1)*8); /*!< do an int to char conversion */
		
		j += WORDLEN;
	}
	free(biosBuf);
	size = lSize * WORDLEN; /*!< calc real size of rom */
}

/*
 * \fn void readUMPSFile(File* bFile)
 * \author Martin Hauser <info@martin-hauser.net>
 * \param bFile The file to read from
 * \brief does some safty checks on the file and turns the results over to ::processUMPSFile
 *
 * This function will take any File descriptor as argument, read the BIOS tag from it and if that's 
 * okay it'll turn over to read the size. If both are okay the rest of the file will be read into
 * a freshly allocated buffer, which will then be turned over to ::processUMPSFile where it will
 * be freed.
 */

void readUMPSFile(FILE* bFile)
{
    Word tag;
	Word * biosBuf;
    
    unsigned long lSize;
	unsigned long* sizep = &lSize;
	
	/*! check whether tag and size can be read from the file */
    if (fread((void *) &tag, WORDLEN, 1, bFile) != 1 || tag != BIOSFILEID || \
         fread((void *) sizep, WORDLEN, 1, bFile) != 1)
	{
	    fatal_error(__LINE__,"couldn't read umps file.\n");
	}
	
	/*! alloc buffer and read data into it */
	biosBuf = malloc(lSize * WORDLEN); 
	if (fread((void *) biosBuf, WORDLEN, *sizep, bFile) != *sizep)
	{
		// wrong file format
		free(biosBuf);
        fatal_error(__LINE__,"wrong UMPS file format.\n");
	}
	/*! turn data over */
	processUMPSFile(biosBuf,lSize);
	
}

static void open_file(void)
{ 
    FILE *f;
    char *c;
    struct stat fs;
    
    if (filename==NULL || strcmp(filename,"") == 0 || (f = fopen(filename, "rb")) == NULL)
        fatal_error(__LINE__,"No filename or file");
        
    c = strrchr(filename,'.');
    if(strcmp(c,".umps") == 0)
    {
        readUMPSFile(f);
    }
    else
    {
       
        if (fstat(fileno(f),&fs)<0)
            fatal_error(__LINE__,"Unable to get file size");
    
        size = ((fs.st_size+7)/8)*8; /* make it a multiple of octas */
        if (rom!=NULL) free(rom);
        rom = malloc(size);
        if (rom==NULL) fatal_error(__LINE__,"Out of memory");
        size=fread(rom,1,size,f);
        if (size < 0) fatal_error(__LINE__,"Unable to read file");
        if (size == 0) fatal_error(__LINE__,"Empty file");
        size = (size+PAGESIZE-1)&~(PAGESIZE-1); /*round up to whole pages*/
       
    }
    fclose(f);
}




unsigned char *get_payload(unsigned int offset,int size)
{
    return rom+offset;
}

int reply_payload(unsigned char address[8], int size,unsigned char *payload)
{ return 1;
}

void put_payload(unsigned int offset,int size, unsigned char *payload)
{	/* cannot write to rom -> raise interrupt */
	debug("Write request received");
	debug("raising interrupt");
	set_interrupt(bus_fd, INT_WRITE+32);
}


void init_device(void)
{  debugs("address: %s",hexaddress);
   open_file();
   debugi("size: %d",size);
   close(0);
}

void process_input(unsigned char c) 
{
}

int process_interrupt(unsigned char interrupt)
{ return 0;
}	

int process_poweron(void)
{ 
#ifdef WIN32
	SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hon); 
#endif
	return 0;
}

int process_poweroff(void)
{  
#ifdef WIN32
  SendMessage(hpower,STM_SETIMAGE,(WPARAM) IMAGE_BITMAP,(LPARAM)hconnect);
#endif
  return 0;
}

int process_reset(void)
{ return 0;
}

