#include <windows.h>
#include "mmixlib.h"
#include "winmain.h"
#include "dedit.h"
#include "mmixwinde.h"
#include "splitter.h"



#define MAXDATAEDIT 4
static HWND hDataEdit[MAXDATAEDIT] = {0};
HINSTANCE hDataEditInstance=0;
HWND hDataEditParent=0;

int is_dataedit(HWND h)
{ int i;
  while (h!=NULL)
  { for(i=0; i<MAXDATAEDIT; i++) if (hDataEdit[i]==h) return 1;
	h=GetParent(h);
  }
  return 0;
}

HWND GetDataEdit(int id, HWND hMemory)
{ if (id <0 || id>=MAXDATAEDIT) 
  { win32_error(__LINE__,"ID out of range in GetDataEdit");
    id = 0;
  }
  if (hDataEdit[id]!=NULL)
    return hDataEdit[id];
  sp_create_options(0, 0, 0.2, mem_min_height,hMemory);
  hDataEdit[id] = CreateDataEdit(hInst,hSplitter);
  if (hDataEdit[id]==NULL)
    win32_error(__LINE__,"Unable to create Data Editor");
  return hDataEdit[id];
}

void DestroyDataEdit(int id)
{   if (id <0 || id>=MAXDATAEDIT) 
  { win32_error(__LINE__,"ID out of range in GetDataEdit");
    return;
  }	  
  if (hDataEdit[id]==NULL)
	return;
  de_connect(hDataEdit[id],NULL);
  DestroyWindow(hDataEdit[id]);
  hDataEdit[id]=NULL;
}


