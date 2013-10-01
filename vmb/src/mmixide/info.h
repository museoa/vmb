#include "mmix-internals.h"
extern int edit_file_no; /* the file currently in the editor */
#define MAX_FILES 0x100
extern char *fullname[MAX_FILES+1]; /* the full filenames */
extern char *shortname[MAX_FILES+1]; /* pointers to the tail of the full name */
extern char has_debug_info[MAX_FILES+1];
extern void *doc[257]; /* pointer to scintilla documents */

#define file2shortname(file_no) (shortname[file_no])
#define file2fullname(file_no)  (fullname[file_no])
#define file2debuginfo(file_no) (has_debug_info[file_no])


int unique_shortname(int file_no);
/* find out wheter the shortname is unique */

extern void *file2document(int file_no); 
/* return document, open if needed, return NULL if file not found  */

extern int filename2file(char *filename);
/* return file_no for this file, allocate fullname as needed */

extern int file_change_name(int file_no, char *filename);
/* give the file a new name */

extern void set_file(int file_no, char *filename);
/* rearrange data to associate file_no with filename, 
   allocate fullname if needed, 
   */

extern void clear_file_info(int file_no);
/* remove all data about file */

extern void clear_all_info(void);
/* remove all data */

extern void for_all_loc(int file_no, int line_no, void f(octa loc));
/* iterate f over all locations belonging to this file and line */

/* retrieve information about locations */
#define loc2bkpt(loc) (mem_find(loc)->bkpt)
#define loc2freq(loc) (mem_find(loc)->freq)
#define loc2file(loc) (mem_find(loc)->file_no)
#define loc2line(loc) (mem_find(loc)->line_no)

extern void add_line_loc(int file_no, int line_no, octa loc);
/* associate this location with the given file and line */
extern void fill_file_list(void);
/* set all file names in the listbox h */
extern void fill_symtab(void);
/* set all symbols in the symbol table*/
extern void symtab_add_file(int file_no,trie_node *t);
/* add a symbol table for this file */
extern sym_node * symbol2sym_node(char *symbol);
/* find a symbol in one of the symbol tables */
extern void close_file(int file_no);
/* remove a file from the database */
extern int get_inuse_file(void);
/* return a file that is in use */
extern int line2freq(int file_no,int line_no);
/* returns the frequency count for this line  or -1 if none found*/

