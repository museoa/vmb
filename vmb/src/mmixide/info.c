#include <stdio.h>
#include <windows.h>
#include "vmb.h"
#include "error.h"
#include "mmix-internals.h"
#include "mmixlib.h"
#define STATIC_BUILD
#include "../scintilla/include/scintilla.h"
#include "mmixrun.h"
#include "winmain.h"
#include "filelist.h"
#include "symtab.h"
#include "editor.h"
#include "info.h"


/* we maintain files as file numbers in the range 0 to 255
*/



char *fullname[MAX_FILES+1] = {NULL}; /* the full filenames */
char *shortname[MAX_FILES+1] = {NULL}; /* pointers to the tail of the fullname */
static trie_node* symbols[MAX_FILES+1] = {NULL}; /* pointer to pruned symbol table */
void *doc[MAX_FILES+1] = {NULL}; /* pointer to scintilla documents */
char doc_dirty[MAX_FILES+1] ={0};
char has_debug_info[MAX_FILES+1] ={0};
char loading[MAX_FILES+1] ={0};
static int next_file_no=0, count_file_no=0;

#define available(file_no) (fullname[file_no]==NULL&&doc[file_no]==NULL)
#define inuse(file_no) (!available(file_no))


static void free_name(int file_no)
/* free the memory of fullname */
{ if (file_no<0) return;
  if (fullname[file_no]!=NULL)
  { file_list_remove(file_no);
    free(fullname[file_no]);
  }
  fullname[file_no]=shortname[file_no]=NULL;
}


static int alloc_file_no(void)
/* return -1 on failure, otherwise an available file number 
   files without fullname or document are available
*/
{ int file_no;
  if (count_file_no>=MAX_FILES)
  { vmb_error(__LINE__,"Too many files");
    return -1;
  }
  else if (count_file_no==next_file_no)
  { file_no=next_file_no;
    next_file_no++;
  }
  else
  { for(file_no=0;file_no<next_file_no;file_no++)
      if(available(file_no))
		  break;
  }
  count_file_no++;
  return file_no;
}

static void release_file_no(int file_no)
{ free_name(file_no);
  if (doc[file_no]!=NULL) 
  { ed_release_document(doc[file_no]);
    file_list_remove(file_no);
  }
  doc[file_no]=NULL;
  doc_dirty[file_no]=0;
  // symbols[file_no]=NULL; needs freeing 
  count_file_no--;
  while(next_file_no>0 && available(next_file_no-1)) next_file_no--;
}

int unique_shortname(int file_no)
/* return true if the short filename is unique */
{  int i;
   for (i=0; i<next_file_no; i++)
	   if (inuse(file_no) && i!=file_no && 
		   shortname[i]!=NULL && shortname[file_no]!=NULL && 
		   strcmp(shortname[i],shortname[file_no])==0)
		   return 0;
   return 1;
}



static int find_file(char *name)
{ int i;
  for(i=0;i<next_file_no;i++)
    if (fullname[i]!=NULL && strcmp(fullname[i],name)==0) return i;
  return -1;
}

int file_set_name(int file_no, char *filename)
/* function to compute full and short name and set them */
{ static char name[MAX_PATH+1], *tail;
  int n;
  if (filename==NULL)
  { free_name(file_no);
    return file_no;
  } 
  n = GetFullPathName(filename,MAX_PATH,name,&tail);
  if (n<=0)
  { vmb_error(__LINE__,"Illegal file name");
    return -1;
  }
  free_name(file_no);
  fullname[file_no] = malloc(n+1);
  if (fullname[file_no] ==NULL)
  { vmb_error(__LINE__,"Out of memory");
    return -1;
  }
  if (n>MAX_PATH)
    GetFullPathName(filename,MAX_PATH,fullname[file_no],&shortname[file_no]);
  else
  { strncpy(fullname[file_no],name,n+1);
	shortname[file_no]=fullname[file_no]+(tail-name);
  }
  file_list_add(file_no);
  ed_add_tab(file_no);
  return file_no;
}


int filename2file(char *filename)
/* return file_no for this file, allocate fullname as needed */
{ int new_no;
  new_no = alloc_file_no();
  if (new_no<0) return -1;
  if (filename==NULL)
  {	file_list_add(new_no);
    ed_add_tab(new_no);
    return new_no;
  }
  else
  {	int file_no;
    new_no= file_set_name(new_no, filename);
    if (new_no<0) return -1;
	file_no=find_file(fullname[new_no]);
	if (file_no!=new_no)
	  release_file_no(new_no);
	else
	{  file_list_add(file_no);
	   ed_add_tab(file_no);
	}
    return file_no;
  }
}


static void mem_node_clear_breaks(unsigned char file_no,mem_node*p)
{  int i;
	if (p==NULL) return;
   mem_node_clear_breaks(file_no, p->left);
   mem_node_clear_breaks(file_no, p->right);
   for (i=0;i<512;i++)
   { mem_tetra *q=&(p->dat[i]);
	   if(q->file_no==file_no)
		   q->bkpt=0;
   }
}

void mem_clear_breaks(int file_no)
{ mem_node_clear_breaks(file_no,mem_root);
}

static void clear_symbols(int file_no)
{ 
  free_tree(symbols[file_no]);
  symbols[file_no]=NULL;
}




void clear_file_info(int file_no)
/* remove all data about file */
{   mem_clear_breaks(file_no);
    clear_symbols(file_no);
	has_debug_info[file_no]=0;
}

void clear_all_info(void)
/* remove all data */
{ int file_no;
  for (file_no=0;file_no<next_file_no;file_no++)
    clear_file_info(file_no);
}


void mem_iterator(mem_node *p,int file_no, int line_no, void f(octa loc))
{ int j;
  if (p->left) mem_iterator(p->left,file_no,line_no,f);
  for (j=0;j<512;j++) 
	if (p->dat[j].file_no==file_no && p->dat[j].line_no==line_no)
		f(incr(p->loc,4*j));
  if (p->right) mem_iterator(p->right,file_no,line_no,f);

}

void for_all_loc(int file_no, int line_no, void f(octa loc))
/* iterate f over all locations belonging to this file and line */
{ mem_iterator(mem_root,file_no,line_no,f);
}


void for_all_files(void f(int i))
/* set all file names in the file list list box h */
{ int file_no;
  for (file_no=0; file_no<next_file_no;file_no++)
  { if (inuse(file_no))
		f(file_no);
  }
}


void add_line_loc(int file_no, int line_no, octa loc)
/* called from the assembler making relations between lines and locations */
{ if (application_file_no>=0) 
    return; /* there is an application running */
  else
  { mem_tetra *ll = mem_find(loc);
    ll->file_no=file_no;
    ll->line_no=line_no;
    has_debug_info[file_no]=1;
  }
}

int freq_max=-1;

static void get_max_freq(octa loc)
{ int freq = loc2freq(loc);
  if (freq>freq_max) freq_max=freq;
}

int line2freq(int file_no,int line_no)
/* returns the frequency count for this line  or -1 if none found*/
{ freq_max=-1;
	for_all_loc(file_no, line_no, get_max_freq);
  return freq_max;
}



void symtab_add_file(int file_no,trie_node *t)
{ 
  clear_symbols(file_no);
  symbols[file_no]=t;
  has_debug_info[file_no] = 1;
  update_symtab();
}

trie_node *file2symbols(int file_no)
{ if (file_no<0 || file_no>=next_file_no) return NULL;
  return symbols[file_no];
}

void close_file(int file_no)
/* remove a file from the database */
{ release_file_no(file_no);
  clear_symbols(file_no);
  has_debug_info[file_no]=0;
}

int get_inuse_file(void)
/* return a used file number or -1 if none exists */
{ int file_no;
  if (count_file_no>0)
    for(file_no=0;file_no<next_file_no;file_no++)
      if(inuse(file_no)) return file_no;
  return -1;
}


