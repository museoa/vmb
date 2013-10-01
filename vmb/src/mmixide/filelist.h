
extern void create_filelist(void);
extern void file_list_reset(void);
extern char *file_listname(int file_no);
extern void file_list_remove(int file_no);
extern void file_list_add(int file_no);
extern void file_list_mark(int file_no);
extern int file_list_measureitem(LPMEASUREITEMSTRUCT mi);
extern int file_list_drawitem(LPDRAWITEMSTRUCT di);
