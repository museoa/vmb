
extern void mmix_run(void);
/* runs the mmix simulator */
extern void mmix_debug(void);
/* runs the mmix simulator in debug mode */

extern void mmix_exit(int returncode);
/* called if the mmix simulator needs to call exit(returncode); */

extern void mmix_assemble(void);
/* runs the mmix assembler */

extern char *get_mmo_name(void);

extern int mmix_connect(void);
/* connects to the vmb bus */