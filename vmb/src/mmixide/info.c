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
#include "info.h"


/* we maintain files as file numbers in the range 0 to 255
*/



char *fullname[MAX_FILES+1] = {NULL}; /* the full filenames */
char *shortname[MAX_FILES+1] = {NULL}; /* pointers to the tail of the fullname */
static trie_node* symbols[MAX_FILES+1] = {NULL}; /* pointer to pruned symbol table */
void *doc[MAX_FILES+1] = {NULL}; /* pointer to scintilla documents */
char has_debug_info[MAX_FILES+1] ={0};
static int next_file_no=0, count_file_no=0;

#define available(file_no) (fullname[file_no]==NULL&&doc[file_no]==NULL)
#define inuse(file_no) (!available(file_no))


static void free_name(int file_no)
/* free the memory of fullname */
{ if (fullname[file_no]!=NULL)
   free(fullname[file_no]);
  fullname[file_no]=shortname[file_no]=NULL;
}


static int new_file_no(void)
/* return -1 on failure, otherwise an available file number 
   files without fullname and document are available
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
  if (doc[file_no]!=NULL) ed_send(SCI_RELEASEDOCUMENT,0,(LONG_PTR)doc[file_no]);
  doc[file_no]=NULL;
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


int edit_file_no = -1; /* the file currently in the editor */

void *file2document(int file_no)
/* return document, open if needed, return NULL if file not found  */
{ if (file_no>=next_file_no) return NULL;
  if (doc[file_no]==NULL)
  { doc[file_no]=(void *)ed_send(SCI_CREATEDOCUMENT,0,0);
    if (fullname[file_no]==NULL) file_list_add(file_no);
  }
  return doc[file_no];
}

static int find_file(char *name)
{ int i;
  for(i=0;i<next_file_no;i++)
    if (fullname[i]!=NULL && strcmp(fullname[i],name)==0) return i;
  return -1;
}

static int file_set_name(int file_no, char *filename)
/* auxiliar function to compute full and short name and set them */
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
  return file_no;
}

int file_change_name(int file_no, char *filename)
{ file_list_remove(file_no);
  file_no = file_set_name(file_no,filename);
  file_list_add(file_no);
  return file_no;
}

int filename2file(char *filename)
/* return file_no for this file, allocate fullname as needed */
{ int file_no, no = new_file_no();
	
  if (filename==NULL)
	 file_no = no;
  else
  {	if (file_set_name(no, filename)<0) return -1;
	file_no=find_file(fullname[no]);
	if (file_no!=no)
	  release_file_no(no);
	else
	  file_list_add(file_no);
  }
  return file_no;
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

static void mem_clear_breaks(int file_no)
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


void fill_file_list(void)
/* set all file names in the file list list box h */
{ int file_no;
  file_list_reset();
  for (file_no=0; file_no<next_file_no;file_no++)
  { if (inuse(file_no))
		file_list_add(file_no);
  }
  file_list_mark(edit_file_no);
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

static char symtab_buf[1000];

void enumerate_symtab(trie_node *t, char *sym_ptr)
{

  if (t->left) enumerate_symtab(t->left,sym_ptr);
  *sym_ptr=(char)t->ch;
  if (t->sym && t->sym->link==DEFINED)
  { *(sym_ptr+1)=0;
     symtab_add(symtab_buf+1,t->sym); /* skip leading : */
  }
  if (t->mid) enumerate_symtab(t->mid,sym_ptr+1);
  if (t->right) enumerate_symtab(t->right,sym_ptr);
}

static symtab_add_file_no(int file_no)
{ 
   if (symbols[file_no]!=NULL)
     enumerate_symtab(symbols[file_no], symtab_buf);
}


void fill_symtab(void)
{ int file_no;
  symtab_reset();
  for (file_no=0; file_no<next_file_no;file_no++)
  { if (inuse(file_no))
		symtab_add_file_no(file_no);
  }
}

void symtab_add_file(int file_no,trie_node *t)
{ 
  clear_symbols(file_no);
  symbols[file_no]=t;
  has_debug_info[file_no] = 1;
  fill_symtab();
}


sym_node * symbol2sym_node(char *symbol)
{ int file_no;
  sym_node *p;
  for (file_no=0; file_no<next_file_no;file_no++)
  { if (inuse(file_no))
    { p = find_symbol(symbol,symbols[file_no]);
      if (p!=NULL && p->link==DEFINED) return p;
    }
  }
  return NULL;
}

void close_file(int file_no)
/* remove a file from the database */
{ file_list_remove(file_no);
  release_file_no(file_no);
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


