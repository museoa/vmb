#if 0
#include <stdlib.h> 
#include <ctype.h> 
#include <string.h> 
#include <signal.h> 
#include "libconfig.h"
#include <time.h>
#endif
#include <stdio.h> 
#include <setjmp.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include "vmb.h"
#include "mmixlib/libtype.h"
#include "mmixlib/libglobals.h"
#include "mmixlib/mmixlib.h"
#include "mmix-bus.h"

char localhost[]="localhost";
char *host=localhost;
int port=9002;
device_info vmb;

bool exit_on_halt=false;
tetra init_rI=0;

/* mmixcpu adds a few extra options:
   -X to exit the simulator when a TRAP 0,Halt,0 instruction is executed
   -T<n> set rI to <n> to interrupt after n cycles 
 */

/* see line 2947 of  "mmixware/mmix-sim.w" */
void scan_option(char*arg,bool usage)
{ register int k;
  switch(*arg){
	case 'X':
		exit_on_halt=true;
		return;
	case 'T':
		if(sscanf(arg+1,"%u",(int*)&init_rI)!=1) 
		{ fprintf(stderr,"Unable to get rI value\n"); longjmp(mmix_exit,-2) ;}
		return;
	case't': 
		if(strlen(arg)> 10)trace_threshold= 0xffffffff;
		else if(sscanf(arg+1,"%d",(int*)&trace_threshold)!=1)trace_threshold= 0;
        return;
    case'e':
		if(!*(arg+1))tracing_exceptions= 0xff;
		else if(sscanf(arg+1,"%x",&tracing_exceptions)!=1)tracing_exceptions= 0;
		return;
	case'r':
		stack_tracing= true;
		return;
	case's':
		showing_stats= true;
		return;
	case'l':
		if(!*(arg+1))gap= 3;
		else if(sscanf(arg+1,"%d",&gap)!=1)gap= 0;
		showing_source= true;
		return;
	case'L':
		if(!*(arg+1))profile_gap= 3;
		else if(sscanf(arg+1,"%d",&profile_gap)!=1)profile_gap= 0;
		profile_showing_source= true;
	case'P':
		profiling= true;
		return;
	case'v':
		trace_threshold= 0xffffffff;tracing_exceptions= 0xff;
		stack_tracing= true;showing_stats= true;
		gap= 10,showing_source= true;
		profile_gap= 10,profile_showing_source= true,profiling= true;
		return;
	case'q':
		trace_threshold= tracing_exceptions= 0;
		stack_tracing= showing_stats= showing_source= false;
		profiling= profile_showing_source= false;
		return;
	case'i':
		interacting= true;
		return;
	case'I':
		interact_after_break= true;
		return;
	case'b':
		if(sscanf(arg+1,"%d",&buf_size)!=1)buf_size= 0;
		return;
	case'O':
		show_operating_system= true;
		return;
	case'o':
		show_operating_system= false;
		return;
	case 'B': 
	{	char *p; 
		p = strchr(arg+1,':'); 
		if (p==NULL) 
		{ host=localhost; 
			port = atoi(arg+1); 
		}    
	    else 
		{ port = atoi(p+1); 
		  host = malloc(p+1-arg+1); 
		  if (host==NULL) { fprintf(stderr,"No room for hostname\n"); longjmp(mmix_exit,-2) ;}
          strncpy(host,arg+1,p-arg-1); 
          host[p-arg-1]=0; 
        } 
	} 
		return; 
	case'c':
		if(sscanf(arg+1,"%d",&lring_size)!=1)lring_size= 0;
		return;
	case'f':
		if(fake_stdin)fclose(fake_stdin);
		fake_stdin= fopen(arg+1,"r");
		if(!fake_stdin)fprintf(stderr,"Sorry, I can't open file %s!\n",arg+1);
		else mmix_fake_stdin(fake_stdin);
		return;
	case'D':
		dump_file= fopen(arg+1,"wb");
		if(!dump_file)fprintf(stderr,"Sorry, I can't open file %s!\n",arg+1);
		return;
	default:
		if(usage){
			fprintf(stderr, "Usage: %s <options> progfile command line-args...\n",myself);
			for(k= 0;usage_help[k][0];k++)fprintf(stderr,"%s",usage_help[k]);
			fprintf(stderr,"-X    exit when executing TRAP 0,Halt,0\n");
			fprintf(stderr,"-T<n>    Set rI Register to n\n");
			longjmp(mmix_exit,-1);
		} else
			for(k= 0;usage_help[k][1]!='b';k++)printf("%s",usage_help[k]);
		return;
	}
}




int main(int argc,char *argv[])
{
	char**boot_argv;
	int boot_argc;
	mmix_lib_initialize();
	g[255].h= 0;
	g[255].l= setjmp(mmix_exit);
	if(g[255].l!=0)
		goto end_simulation;

	myself= argv[0];
	for(cur_arg= argv+1;*cur_arg&&(*cur_arg)[0]=='-';cur_arg++)
		scan_option(*cur_arg+1,true);
	argc-= (int)(cur_arg-argv);

	init_mmix_bus(host,port,"MMIX CPU");

	if (!vmb.connected) {fprintf(stderr,"Not connected\n"); return 0;}
	mmix_initialize();

	boot_argv= cur_arg;
	boot_argc= argc;
boot:
	argc= boot_argc;
	cur_arg= boot_argv;
	vmb_clear_reset(&vmb);
	printf("Power...");
	while (!vmb.power)
	{  vmb_wait_for_power(&vmb);
	if (!vmb.connected){ fprintf(stderr,"Power but not connected"); return 0;}
	}
	printf("ON\n");

	mmix_boot();
    if (init_rI!=0) 
		g[rI].h=0,g[rI].l=init_rI;

	if (cur_arg!=NULL && cur_arg[0]!=0) 
	{ vmb_raise_reset(&vmb);  
#ifdef WIN32
	Sleep(50); /* give all devices some time to power up before loading the application */
#else
	usleep(50000);
#endif
	  vmb_clear_reset(&vmb);
      mmix_load_file(*cur_arg);
	  mmix_commandline(argc,cur_arg);
	}

	while(vmb.connected){
		if(interrupt&&!breakpoint)breakpoint= interacting= true,interrupt= false;
		else if(!(inst_ptr.h&sign_bit)||show_operating_system||
			(inst_ptr.h==0x80000000&&inst_ptr.l==0))
		{breakpoint= false;
		if(interacting){
			if(!mmix_interact())goto end_simulation;
		}
		}
		if(halted)break;
		do
		{if(!resuming)
			mmix_fetch_instruction();
		if (exit_on_halt && inst==0)  
		{ fprintf(stderr,"TRAP 0,Halt,0 at location #%08x%08x\n",loc.h,loc.l);
		  goto end_simulation;
		}
		mmix_perform_instruction();
		mmix_trace();
		mmix_dynamic_trap();
		if(resuming&&op!=RESUME)resuming= false;
		}while((vmb.connected && vmb.power && !vmb.reset_flag &&
			!interrupt && !breakpoint) || 
			resuming);
		if(interact_after_break)
			interacting= true,interact_after_break= false;
		if(!vmb.power || vmb.reset_flag || g[rQ].l&g[rK].l&RE_BIT)
		{breakpoint= true;
		goto boot;
		}
	}
end_simulation:if(profiling)mmix_profile();
	if(interacting||profiling||showing_stats)show_stats(true);
	mmix_finalize();
	Sleep(50); /* let the messages settle down */
	return g[255].l;
}
