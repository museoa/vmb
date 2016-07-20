
#include <windows.h>
#include "winopt.h"
#ifdef VMB
#include "error.h"
#include "option.h"
#endif

void win32_param_init(void)
{ int argc;
  char *argv[MAXARG];
  int i;
#ifdef VMB
  option_defaults();
#endif
  argc=mk_argv(argv,GetCommandLine(),TRUE);
  do_program(argv[0]);
  if (do_define(argv[1])) i=2; else i=1;
  read_regtab(defined);
  parse_commandline(argc-i, argv+i);
  get_xypos();
#ifdef VMB
  if (vmb_verbose_flag) vmb_debug_mask=0;
#endif
  SetWindowText(hMainWnd,defined);
}

