#ifdef WIN32
  #include <windows.h>
#else
  #include <pthread.h>
  #include <time.h>

/* unshure which .h file to get this constant from */
  #ifndef ETIMEDOUT
  #define ETIMEDOUT 145
  #endif

  extern void clean_up_event_mutex(void *vmb);

#endif

#include "vmb.h"
#include "error.h"

void vmb_wait_for_event_timed(device_info *vmb, int ms)
/* waits for a  power off, reset, disconnect, or an interrupt
   or untim the Time in ms expires.
*/
{ 
#ifndef WIN32
  int w = 0;
#define WAIT_TIMEOUT ETIMEDOUT
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += ms/1000;
  ts.tv_nsec += (ms%1000)*1000;
  { int rc = pthread_mutex_lock(&vmb->event_mutex);
    if (rc) 
    { vmb_error(__LINE__,"Locking event mutex failed");
      pthread_exit(NULL);
    }
  }
  pthread_cleanup_push(clean_up_event_mutex,vmb);
#else
  DWORD w = 0;
#endif
  /* in the meantime the event might have happend */
  while (vmb->power &&
         !vmb->reset_flag && 
         vmb->connected &&
	     vmb->interrupt_lo == 0 &&
		 vmb->interrupt_hi ==0 &&
         !vmb->cancel_wait_for_event &&
		 w != WAIT_TIMEOUT
         )
#ifdef WIN32
     w = WaitForSingleObject(vmb->hevent,ms);
#else
     pthread_cond_timedwait(&vmb->event_cond,&vmb->event_mutex, &ts);
  pthread_cleanup_pop(1);
#endif
  vmb->cancel_wait_for_event=0;
}
