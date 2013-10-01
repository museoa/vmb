#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include "vmb.h"
#include "mmix-internals.h"
#include "mmixlib.h"

jmp_buf error_exit;
device_info vmb;

#define panic(m) vmb_fatal_error(__LINE__,m)
#define sign_bit ((unsigned)0x80000000)



void mmix_exit(int returncode)
{ longjmp(error_exit, returncode);
}

/* this is the plain vmb version of mmix main() */

int mmix_main(int argc, char *argv[],char *mmo_name)
{ g[255].h=0;
  g[255].l=setjmp(error_exit);
  if (g[255].l!=0)
   goto end_simulation;
 
  if (!vmb.connected) panic("Not connected");
  if (vmb.power)  
    vmb_raise_reset(&vmb);
  mmix_initialize();
boot:
  mmix_boot(); 
  fprintf(stderr,"Power...");
  while (!vmb.power)
  {  vmb_wait_for_power(&vmb);
     if (!vmb.connected) goto end_simulation;
  }
  fprintf(stderr,"ON\n");
  mmix_load_file(mmo_name);
  mmix_commandline(argc, argv);
  while (vmb.connected) {
    if (interrupt && !breakpoint) breakpoint=interacting=true, interrupt=false;
    else if (!(inst_ptr.h&sign_bit) || show_operating_system || 
          (inst_ptr.h==0x80000000 && inst_ptr.l==0))
    { breakpoint=false;
      if (interacting) { 
		if (!mmix_interact()) goto end_simulation;
      }
    }
    if (halted) break;
    do   
    { if (!resuming)
        mmix_fetch_instruction();
      mmix_perform_instruction(); 
	  mmix_trace();
	  mmix_dynamic_trap();
      if (resuming && op!=RESUME) resuming=false; 
    } while ((vmb.connected && vmb.power && !vmb.reset_flag &&
              !interrupt && !breakpoint) || 
              resuming);
    if (interact_after_break) 
       interacting=true, interact_after_break=false;
    if (!vmb.power|| vmb.reset_flag)
    { breakpoint=true; 
      vmb.reset_flag=0; 
      goto boot;
    }
  }
  end_simulation:
  if (interacting || profiling || showing_stats) show_stats(true);
  mmix_finalize();
  return g[255].l;
}
