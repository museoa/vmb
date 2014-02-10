


@x
@d test_store_bkpt(ll) if ((ll)->bkpt&write_bit) breakpoint=tracing=true
@y
@d do_store_bkpt breakpoint=tracing=true,gdb_signal=TARGET_SIGNAL_TRAP
@d test_store_bkpt(ll) if ((ll)->bkpt&write_bit) do_store_bkpt
@z

@x
@d test_load_bkpt(ll) if ((ll)->bkpt&read_bit) breakpoint=tracing=true
@y
@d do_load_bkpt breakpoint=tracing=true,gdb_signal=TARGET_SIGNAL_TRAP
@d test_load_bkpt(ll) if ((ll)->bkpt&read_bit) do_load_bkpt
@z


@x
     breakpoint=true;
@y
     breakpoint=true;
     gdb_signal=yz;
@z

@x
break_inst: breakpoint=tracing=true;
@y
break_inst: breakpoint=tracing=true;
  gdb_signal=TARGET_SIGNAL_ILL;
@z

@x
  for (p=buf,m=0,a=addr; m<size;) {
@y
  if (addr.h==0xFFFFFFFF || !valid_address(addr) ) /* gdb goes over segment boundaries */
  { memset(buf,0,size);
    return size;
  }
  for (p=buf,m=0,a=addr; m<size;) {
@z

@x
  for (p=buf,m=0,a=addr; m<size;) {
@y
  if (addr.h==0xFFFFFFFF || !valid_address(addr) ) /* gdb goes over segment boundaries */
    return;
  for (p=buf,m=0,a=addr; m<size;) {
@z

@x
  if (interact_after_resume)
  { breakpoint = true;
@y
  if (interact_after_resume)
  { breakpoint = true;
    gdb_signal=TARGET_SIGNAL_TRAP;
@z


@x
jmp_buf error_exit;
@y
#include "gdb.h"
jmp_buf error_exit;
@z

@x
  init_mmix_bus(host,port,"MMIX CPU");
@y
  init_mmix_bus(host,port,"MMIX CPU");
  if (interacting && gdb_init(gdbport)) 
  { breakpoint = true;
    gdb_signal=TARGET_SIGNAL_TRAP;
  }
@z


@x
  mmix_load_file(*cur_arg);
  mmix_commandline(argc, argv);
@y
@z

@x
    if (interrupt && !breakpoint) breakpoint=interacting=true, interrupt=false;
@y
    if (interrupt && !breakpoint) 
    { breakpoint=interacting=true;
      interrupt=false;
      gdb_signal=TARGET_SIGNAL_INT;
    }
@z

@x
		if (!mmix_interact()) goto end_simulation;
@y
        if (!interact_with_gdb())
        { interacting=false;
          goto end_simulation;
        }
@z

@x
    if (interact_after_break)
       interacting=true, interact_after_break=false;
@y
    if (interact_after_break)
       interacting=true, interact_after_break=false;
    if (stepping) 
    { breakpoint=true;
      gdb_signal=TARGET_SIGNAL_TRAP;
      stepping=false;
    }
@z

@x
    { breakpoint=true;
@y
    { breakpoint=true; 
      gdb_signal=TARGET_SIGNAL_PWR;
@z

@x
  end_simulation:@+if (profiling) mmix_profile();
@y
  end_simulation:
  if (interacting) { gdb_signal=-1; interact_with_gdb(); }
@z



@x
@<Glob...@>=
char command_buf[command_buf_size];
FILE *incl_file; /* file of commands included by `\.i' */
char cur_disp_mode='l'; /* |'l'| or |'g'| or |'$'| or |'M'| */
char cur_disp_type='!'; /* |'!'| or |'.'| or |'#'| or |'"'| */
bool cur_disp_set; /* was the last \.{<t>} of the form \.{=<val>}? */
octa cur_disp_addr; /* the |h| half is relevant only in mode |'M'| */
octa cur_seg; /* current segment offset */
char spec_reg_code[]={rA,rB,rC,rD,rE,rF,rG,rH,rI,rJ,rK,rL,rM,
      rN,rO,rP,rQ,rR,rS,rT,rU,rV,rW,rX,rY,rZ};
char spec_regg_code[]={0,rBB,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,rTT,0,0,rWW,rXX,rYY,rZZ};
@y
@z
