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
#include "symtab.h"
#include "editor.h"
#include "info.h"


/* we maintain files as file numbers in the range 0 to 255
*/



char *fullname[MAX_FILES+1] = {NULL};	/* the full filenames */
char *shortname[MAX_FILES+1] = {NULL};	/* pointers to the tail of the fullname */
static trie_node* symbols[MAX_FILES+1] = {NULL}; /* pointer to pruned symbol table */
void *doc[MAX_FILES+1] = {NULL};		/* pointer to scintilla documents */
char doc_dirty[MAX_FILES+1] ={0};		/* records whether the doc is dirty, but not for the edit_file */
char has_debug_info[MAX_FILES+1] ={0};	/* is debug information available */
char needs_assembly[MAX_FILES+1] ={0};			/* does this file need needs_assembly */
char needs_reading[MAX_FILES+1] ={0};   /* reading a file with a full filename can be delayed until displayed for the first time */
static int next_file_no=0;				/* all used file numbers are below next_file_no */
static int count_file_no=0;				/* number of used file numbers */
int application_file_no = -1; /* the main application for debugging and running */


/* file numbers get allocated together with documents 
   setting a filename must be followed by reading the file
   the dirty flag is valid only if the file is currently not in the editor
   the fullname and the short name are set and freed together, they are not required.
   unique_name provides a uniqe short name for all files
   file_no must always point to a valid file number 0<=file_no<=255
*/
#define available(file_no) (doc[file_no]==NULL)
#define inuse(file_no) (!available(file_no))



int file_dirty(int file_no)
{ if (file_no==edit_file_no)
    return (int)ed_operation(SCI_GETMODIFY);
  else
	return file2dirty(file_no);
}

void set_application(int file_no)
{ application_file_no = file_no;
  if (application_file_no<0) 
	  SetWindowText(hMainWnd,"VMB MMIX IDE");
  else
	  SetWindowText(hMainWnd,unique_name(file_no));
}

static int alloc_file_no(void)
/* return -1 on failure, otherwise an available file number 
   files without fullname or document are available
*/
{ int file_no;
  if (count_file_no<MAX_FILES)
  { for(file_no=0;file_no<MAX_FILES;file_no++)
      if(available(file_no))
	  { count_file_no++;
        doc[file_no]=ed_create_document();
		fullname[file_no]=shortname[file_no]=NULL;
		needs_reading[file_no]=0;
		symbols[file_no]=NULL;
        doc_dirty[file_no]=0;
        has_debug_info[file_no]=0;
        needs_assembly[file_no]=0;
		ed_add_tab(file_no);
		if (file_no>=next_file_no) next_file_no = file_no+1;
        return file_no;
      }
  }
  vmb_error(__LINE__,"Too many files");
  return 0; /* we should never get here */
}

static void release_file_no(int file_no)
{ if (file_no<0) return;
  ed_remove_tab(file_no);
  if (fullname[file_no]!=NULL)
    free(fullname[file_no]);
  fullname[file_no]=shortname[file_no]=NULL;
  needs_reading[file_no]=0;
  ed_release_document(doc[file_no]);
  doc[file_no]=NULL;
  // symbols[file_no]=NULL; needs freeing 
  count_file_no--;
  while(next_file_no>0 && available(next_file_no-1)) next_file_no--;
}

static int is_unique_shortname(int file_no)
/* return true if the short filename is unique */
{  int i;
   for (i=0; i<next_file_no; i++)
	   if (inuse(i) && i!=file_no && 
		    ( (shortname[i]==NULL && shortname[file_no]==NULL) ||
		      (shortname[i]!=NULL && shortname[file_no]!=NULL && strcmp(shortname[i],shortname[file_no])==0)
		    )
		  )
		  return 0;
   return 1;
}

char *unique_name(int file_no)
{ 
  static char noname[]="Unnamed";
  char *name = file2shortname(file_no);
  if (name==NULL) name=noname;
  if(!is_unique_shortname(file_no) && file_no>0)
  { static char str[64+7];
    sprintf_s(str,64,"%.64s-%d",name,file_no);
	name = str;
  }
  return name;
}

static char *full_filename(char *filename, char **tail)
/* compute and allocate the full filename for the given filename set tail to the shortname*/
{ static char name[MAX_PATH+1], *head;
  int n;
  if (filename==NULL) return NULL;
  n = GetFullPathName(filename,MAX_PATH,name,tail);
  if (n<=0)
  { vmb_error(__LINE__,"Illegal file name");
    return NULL;
  }
  head =malloc(n+1);
  if (head ==NULL)
  { vmb_error(__LINE__,"Out of memory");
    return NULL;
  }
  if (n>MAX_PATH)
    GetFullPathName(filename,n,head,tail);
  else
  { strncpy(head,name,n+1);
    *tail = head +(*tail-name);
  }
  return head;
}


static int find_file(char *name)
{ int i;
  for(i=0;i<next_file_no;i++)
    if (fullname[i]!=NULL && strcmp(fullname[i],name)==0) return i;
  return -1;
}

int filename2file(char *filename,char c)
/* return file_no for this file, allocate file_no if needed */
{ int file_no;
  char *head, *tail;
  head = full_filename(filename, &tail);
  if (head==NULL)
  { file_no = alloc_file_no();
    return file_no;
  }
  file_no=find_file(head);
  if (file_no>=0) 
  { free(head);
	return file_no;
  }
  /* at this point we might reuse file number 0 if it is unnamed, in use, and not dirty */
  if (fullname[0]==NULL && doc[0]!=NULL && !file_dirty(0))
	  file_no=0;
  else
      file_no = alloc_file_no();
  fullname[file_no]=head;
  shortname[file_no]=tail;
  needs_reading[file_no]=1;
  ed_add_tab(file_no); /* name has changed */
  if (application_file_no<0||application_file_no==file_no) set_application(file_no); /* the first filename is the application */
  return file_no;
}


void file_set_name(int file_no, char *filename)
/* function to compute full and short name and set them */
{ char *head, *tail;
  head = full_filename(filename, &tail);
  if (fullname[file_no]!=NULL) free(fullname[file_no]);
  fullname[file_no]=NULL;
  shortname[file_no]=NULL;
  needs_reading[file_no]=0;
  if (head!=NULL)
  { fullname[file_no]=head;
    shortname[file_no]=tail;
    needs_reading[file_no]=1;
	ed_add_tab(file_no);
	if (application_file_no<0||application_file_no==file_no) set_application(file_no); /* the first filename is the application */
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
{ if (mmix_active()) 
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


