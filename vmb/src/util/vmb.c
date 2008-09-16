
#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "vmb.h"
#include "error.h"
#include "message.h"
#include "bus-arith.h"
#include "cache.h"


static struct
{ unsigned char address[8];
  unsigned int size;
  unsigned int lo_mask;
  unsigned int hi_mask;
} device_info;


static int vmb_fd;
unsigned int vmb_connected = 0;
unsigned int vmb_power = 0;
unsigned int vmb_reset_flag = 0;

static unsigned int vmb_interrupt_lo=0;
static unsigned int vmb_interrupt_hi=0;

void vmb_raise_reset(void)
     /* trigger a hard reset on the bus */
{ send_msg(vmb_fd, TYPE_BUS, 0, 0, ID_RESET, 0, 0, 0);
}


#ifdef WIN32
HANDLE hevent;
CRITICAL_SECTION   event_section;
#else
static pthread_mutex_t event_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t event_cond = PTHREAD_COND_INITIALIZER;

static void clean_up_event_mutex(void *_dummy)
{ pthread_mutex_unlock(&event_mutex); /* needed if canceled waiting */
}

#endif

int vmb_get_interrupt(unsigned int *hi, unsigned int *lo)
     /* return 0 if there was no recent interrupt 
        return 1 if there was, and add the interrupt bits to lo and hi
        return -1 if the bus disconnected
     */
{
  if (vmb_interrupt_lo == 0 && vmb_interrupt_hi ==0) return 0;
#ifdef WIN32
  EnterCriticalSection (&event_section);
#else
  { int rc = pthread_mutex_lock(&event_mutex);
    if (rc) 
    { vmb_error(__LINE__,"Locking event mutex failed");
      pthread_exit(NULL);
    }
  }
#endif
  *hi |= vmb_interrupt_hi; 
  *lo |= vmb_interrupt_lo; 
  vmb_interrupt_hi = 0;
  vmb_interrupt_lo = 0;
#ifdef WIN32
  LeaveCriticalSection (&event_section);
#else
  { int rc = pthread_mutex_unlock(&event_mutex);
    if (rc) 
    { vmb_error(__LINE__,"Unlocking event mutex failed");
      pthread_exit(NULL);
    }
  }
#endif
  if (vmb_connected) return 1;
  else return -1;
}

void vmb_raise_interrupt(unsigned char i)
     /* raise interrupt i to the bus */
{  if ( i >= 64)
    return;
   send_msg(vmb_fd, TYPE_BUS, 0, (unsigned char)i, ID_INTERRUPT, 0, 0, 0);
}

static unsigned int cancel_wait_for_event = 0;

void vmb_wait_for_event(void)
/* waits for a  power off, reset, disconnect, or an interrupt
*/
{ 
#ifndef WIN32
  { int rc = pthread_mutex_lock(&event_mutex);
    if (rc) 
    { vmb_error(__LINE__,"Locking event mutex failed");
      pthread_exit(NULL);
    }
  }
  pthread_cleanup_push(clean_up_event_mutex,NULL);
#endif
  /* in the meantime the event might have happend */
  while (vmb_power &&
         !vmb_reset_flag && 
         vmb_connected &&
	 vmb_interrupt_lo == 0 && vmb_interrupt_hi ==0 &&
         !cancel_wait_for_event)
#ifdef WIN32
     WaitForSingleObject(hevent,INFINITE);
#else
     pthread_cond_wait(&event_cond,&event_mutex);
  pthread_cleanup_pop(1);
#endif
  cancel_wait_for_event=0;
}



void vmb_wait_for_power(void)
/* waits for a  power on or for a disconnect */
{ 
#ifndef WIN32
  { int rc = pthread_mutex_lock(&event_mutex);
    if (rc) 
    { vmb_error(__LINE__,"Locking event mutex failed");
      pthread_exit(NULL);
    }
  }
  pthread_cleanup_push(clean_up_event_mutex,NULL);
#endif
  /* in the meantime power might be on */
  while (vmb_connected && !vmb_power)
#ifdef WIN32
     WaitForSingleObject(hevent,INFINITE);
#else
     pthread_cond_wait(&event_cond,&event_mutex);
  pthread_cleanup_pop(1);
#endif  
}

void vmb_wait_for_disconnect(void)
/* waits for a disconnect */
{ 
 #ifndef WIN32
  { int rc;rc = pthread_mutex_lock(&event_mutex);
    if (rc) 
    { vmb_error(__LINE__,"Locking event mutex failed");
      pthread_exit(NULL);
    }
    pthread_cleanup_push(clean_up_event_mutex,NULL);
#endif
  /* in the meantime power might be on */
  while (vmb_connected)
#ifdef WIN32
    WaitForSingleObject(hevent,INFINITE);
#else
    rc = pthread_cond_wait(&event_cond,&event_mutex);
  pthread_cleanup_pop(1);
  }
#endif 
}

static void change_event(unsigned int *event, unsigned int value)
/* change one of the variables protected by the event mutex */
{ 
 #ifdef WIN32
  EnterCriticalSection (&event_section);
#else
  int rc = pthread_mutex_lock(&event_mutex);
  if (rc) 
    { vmb_error(__LINE__,"Locking event mutex failed");
      pthread_exit(NULL);
    }
#endif 
  *event = value;
#ifdef WIN32
  LeaveCriticalSection (&event_section);
  SetEvent (hevent);
#else
  rc = pthread_cond_signal(&event_cond);
  rc = pthread_mutex_unlock(&event_mutex);
  if (rc) 
  { vmb_error(__LINE__,"Unlocking event mutex failed");
    pthread_exit(NULL);
  }
#endif
}

extern void vmb_cancel_wait_for_event(void)
{ change_event(&cancel_wait_for_event,1);
}

#ifdef WIN32
static DWORD dwReadThreadId;
static HANDLE hReadThread;
#else
static int read_tid;
static pthread_t read_thr;
#endif

/* a circular buffer acting as a queue for pending read requests.
   the write tread manupilates pending_last
   and all elements in the buffer starting with pending_last
   up to but not including pending_first putting things in
   the read thread manipulates pending_first 
   and all elements in the buffer starting with pending_first
   up to but not including pending_last taking things out
   In order to be able to tell a full from an empty queueu,
   there is always one empty field and hence pending_first==pending_last
   means an empty queue not a full one.
*/

#define PENDINGREADMAX (1<<8)
#define PENDINGREADMASK (PENDINGREADMAX-1)
static data_address *pending_read[PENDINGREADMAX];
static unsigned int pending_first=0; /* points the the first pending read */
static unsigned int pending_last=0; /* points past the last pending read */
#ifdef WIN32
HANDLE hnot_full_pending;
#else
static pthread_mutex_t pending_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t pending_cond = PTHREAD_COND_INITIALIZER;
static void clean_up_pending_mutex(void *_dummy)
{ pthread_mutex_unlock(&pending_mutex); /* needed if canceled waiting */
}
#endif

#define pending_size ((pending_last-pending_first) &  PENDINGREADMASK)

static int full_pending_read(void)
{ return pending_size == (PENDINGREADMAX-1);
}
static int empty_pending_read(void)
{ return pending_size == 0;
}

static void wait_not_full_pending_read(void)
{ 
#ifndef WIN32
  int rc;
  rc = pthread_mutex_lock(&pending_mutex);
  if (rc) 
    { vmb_error(__LINE__,"Locking pending mutex failed");
      pthread_exit(NULL);
    }
  pthread_cleanup_push(clean_up_pending_mutex,NULL);
#endif
  /* in the meantime the queue might no longer be full */
  while (full_pending_read())
#ifdef WIN32
	     WaitForSingleObject(hnot_full_pending,INFINITE);
#else
    rc = pthread_cond_wait(&pending_cond,&pending_mutex);
  pthread_cleanup_pop(1);
#endif
}

static void enqueue_read_request(data_address *da)
{ if (full_pending_read()) wait_not_full_pending_read();
  pending_read[pending_last] = da;
  pending_last = (pending_last +1) & PENDINGREADMASK;
}

static void remove_queue_entry(int i)
/* remove entry i from the pending read queue. */
{ while(i!=pending_first)
  { int k;
    k = (i-1)& PENDINGREADMASK;
    pending_read[i] = pending_read[k];
    i = k;
   }
  pending_first = (pending_first +1) & PENDINGREADMASK;
  if (pending_size == (PENDINGREADMAX-2)) /* full -> not full */
#ifdef WIN32
  SetEvent(hnot_full_pending);
#else
  { int rc;
    rc = pthread_mutex_lock(&pending_mutex);
    if (rc) 
    { vmb_error(__LINE__,"Locking pending mutex failed");
      pthread_exit(NULL);
    }
    rc = pthread_cond_signal(&pending_cond);
    rc = pthread_mutex_unlock(&pending_mutex);
    if (rc) 
    { vmb_error(__LINE__,"Unlocking pending mutex failed");
      pthread_exit(NULL);
    }
  }
#endif
}

static data_address *dequeue_read_request(unsigned int address_hi, unsigned int address_lo)
{ int i;
  data_address *da;
  if (empty_pending_read()) return NULL;
  for (i = pending_first; i!= pending_last; i = (i+1)&PENDINGREADMASK)
  { da = pending_read[i];
    if (da->status != STATUS_READING) /* should not be here remove it */
      remove_queue_entry(i); 
    else if (da->address_hi == address_hi && da->address_lo == address_lo)
    { remove_queue_entry(i); 
      return da;
    }
  }
  /* no matching request found */
  return NULL;   
}

#ifdef WIN32
HANDLE hvalid;
CRITICAL_SECTION   valid_section;
#else
static pthread_mutex_t valid_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t valid_cond = PTHREAD_COND_INITIALIZER;

static void clean_up_valid_mutex(void *_dummy)
{ pthread_mutex_unlock(&valid_mutex); /* needed if canceled waiting */
}
#endif

static void deliver_answer(data_address *da, int size, unsigned char *payload)
{  
#ifdef WIN32
  EnterCriticalSection (&valid_section);
#else
  int rc;
  rc = pthread_mutex_lock(&valid_mutex);
  if (rc) 
  { vmb_error(__LINE__,"Locking valid mutex failed");
    pthread_exit(NULL);
  }
#endif
  if (da->status == STATUS_READING || da->status == STATUS_INVALID)
  { if (size>da->size) size = da->size;
    if (size>0)
    { if (payload != NULL) 
        memmove(da->data,payload, size);
      else
        memset(da->data,0,size);
    }
    da->status = STATUS_VALID;
  }
#ifdef WIN32
  LeaveCriticalSection (&valid_section);
  SetEvent(hvalid);
#else
  rc = pthread_cond_signal(&valid_cond);
  rc = pthread_mutex_unlock(&valid_mutex);
  if (rc) 
  { vmb_error(__LINE__,"Unlocking valid mutex failed");
    pthread_exit(NULL);
  }
#endif
}
static void flush_pending_read_queue(void)
{  data_address *da;
   while (!empty_pending_read())
    { da = pending_read[pending_first];
      remove_queue_entry(pending_first); 
      deliver_answer(da,0,NULL);
    }
}


void vmb_cancel_all_loads(void)
{ flush_pending_read_queue();
}

static void reply_payload(unsigned char address[8],int size, unsigned char *payload)
{ unsigned int address_hi = chartoint(address);
  unsigned int address_lo = chartoint(address+4);
  data_address *da;
  vmb_debugx(0, "Searching for read request matching %s",address,8);
  da = dequeue_read_request(address_hi,address_lo);
  if (da!=NULL)
  { vmb_debug(0, "Matching read request found");
    deliver_answer(da,size,payload);
  }
  else if (size == 0) /* this was a dummy answer, we drop all pending requests */
  { vmb_debug(0, "No matching request for dummy answer");
    flush_pending_read_queue();
  }
  else
     vmb_debug(0, "No matching read request found");
}

void vmb_wait_for_valid(data_address *da)
{ 
#ifndef WIN32
  int rc;
  rc = pthread_mutex_lock(&valid_mutex);
  if (rc) 
    { vmb_error(__LINE__,"Locking valid mutex failed");
      pthread_exit(NULL);
    }
  pthread_cleanup_push(clean_up_valid_mutex,NULL);
#endif
  /* in the meantime the da might be valid */
  if (vmb_connected)
    while (da->status != STATUS_VALID)
#ifdef WIN32
      WaitForSingleObject(hvalid,INFINITE);
#else
      rc = pthread_cond_wait(&valid_cond,&valid_mutex);
  pthread_cleanup_pop(1);
#endif
}

static void answer_readrequest(unsigned char slot,
   			   unsigned char address[8], int size, unsigned char *data)
{ unsigned char type, id;
  if (size == 0 || data == NULL)
    { type = TYPE_ADDRESS|TYPE_ROUTE;
      id = ID_NOREPLY; 
      size = 0;}
  else if (size == 1)
    { type = TYPE_ADDRESS|TYPE_ROUTE|TYPE_PAYLOAD;
      id = ID_BYTEREPLY; 
      size = 0;}
   else if (size == 2)
    { type = TYPE_ADDRESS|TYPE_ROUTE|TYPE_PAYLOAD;
      id = ID_WYDEREPLY; 
      size = 0;}
   else if (size == 4)
    { type = TYPE_ADDRESS|TYPE_ROUTE|TYPE_PAYLOAD;
      id = ID_TETRAREPLY; 
      size = 0;}
  else /* if (size>4) */
    { type = TYPE_ADDRESS|TYPE_ROUTE|TYPE_PAYLOAD;
      id = ID_READREPLY; 
      size = (size+7)/8-1;}
  send_msg(vmb_fd, type,(unsigned char)size,slot,id,0,address,data);
}


static void read_request( unsigned char a[8], int s, unsigned char slot, unsigned char p[])
{ unsigned int offset;
  unsigned char *data;

  offset = get_offset(device_info.address,a);
  if (hi_offset || overflow_offset || offset + s > device_info.size)
  { char hex[17]={0};
    chartohex(a,hex,8);
    vmb_debugs(0, "Read request out of range %s",hex);
    vmb_debug(0, "Sending empty answer");
    answer_readrequest(slot, a,0,NULL);
    vmb_debug(0, "raising interrupt");
    vmb_raise_interrupt(INT_NOMEM);
    return;
  }
  vmb_debug(0, "sending answer");
  data = vmb_get_payload(offset,s);
  answer_readrequest(slot,a,s,data);
}

static void write_request(unsigned char a[8], int s, unsigned char p[])
{ unsigned int offset;
  offset = get_offset(device_info.address,a);
  if (hi_offset || overflow_offset || offset + s > device_info.size)
  { char hex[17]={0};
    chartohex(a,hex,8);
    vmb_debugs(0, "Write request out of range %s",hex);
    vmb_debugx(0, "Address: %s",device_info.address,8);
    vmb_debugi(0, "Size:    %d",device_info.size);
    vmb_debugi(0, "Offset:  %ud", offset);
    vmb_debug(0, "raising interrupt");
    vmb_raise_interrupt(INT_NOMEM);
    return;
  }
  vmb_debug(0, "Writing");
  vmb_put_payload(offset,s,p);
}


static void dispatch_message(unsigned char type, 
                             unsigned char size, 
                             unsigned char slot, 
                             unsigned char id, 
                             unsigned char address[8], 
                             unsigned char *payload)
{ switch (id)
    { case ID_IGNORE: 
        return;
      case ID_READ:
	read_request(address, (size+1)*8, slot, payload);
        return;
      case ID_READBYTE:
	read_request(address, 1, slot, payload);
        return;
      case ID_READWYDE:
	read_request(address, 2, slot, payload);
        return;
      case ID_READTETRA:
	read_request(address, 4, slot, payload);
        return;
      case ID_WRITE:
	write_request(address, (size+1)*8, payload);
        return;
      case ID_WRITEBYTE:
	write_request(address, 1, payload);
        return;
      case ID_WRITEWYDE:
	write_request(address, 2, payload);
        return;
      case ID_WRITETETRA:
	write_request(address, 4, payload);
        return;
      case ID_READREPLY:
	reply_payload(address,(size+1)*8,payload);
        return;
      case ID_BYTEREPLY:
	reply_payload(address,1,payload);
        return;
      case ID_WYDEREPLY:
	reply_payload(address,2,payload);
        return;
      case ID_TETRAREPLY:
	reply_payload(address,4,payload);
        return;
      case ID_NOREPLY:
	reply_payload(address,0,payload);
        return;
	  case ID_TERMINATE:
		  vmb_terminate();
		  return;
      case ID_RESET:
        change_event(&vmb_reset_flag, 1);
	vmb_reset();
        return;
      case ID_POWEROFF:
        vmb_reset_flag = 0;
        change_event(&vmb_power,0);
	vmb_poweroff();
        return;
      case ID_POWERON:
        vmb_reset_flag = 0;
        change_event(&vmb_power, 1);
	vmb_poweron();
        return;
      case ID_INTERRUPT:
        if (slot<32)
           change_event(&vmb_interrupt_lo, vmb_interrupt_lo| BIT(slot));
        else 
           change_event(&vmb_interrupt_hi, vmb_interrupt_hi| BIT(slot-32));
	vmb_interrupt(slot);
        return;
      default:
        vmb_unknown(type,size,slot,id,get_offset(device_info.address,address),payload);
	return;
      }
}

static int get_request(unsigned char *type, 
                        unsigned char *size, 
                        unsigned char *slot,
                        unsigned char *id,
   			unsigned char address[8],
                        unsigned char *payload)
{ unsigned int  time;

  int i;
  i=receive_msg(vmb_fd,type,size,slot,id,&time,address,payload);
  if (i<0) 
	return 0;
  vmb_debugm(0, *type,*size,*slot,*id,address,payload);
  return 1;
}


static void clean_up_read_thread(void *_dummy)
{ if (vmb_fd>=0)
  { bus_unregister(vmb_fd);
    bus_disconnect(vmb_fd); 
    vmb_fd = INVALID_SOCKET;
  }
  change_event(&vmb_connected, 0);
  flush_pending_read_queue();
  vmb_disconnected();
  #ifdef WIN32
   CloseHandle(hevent); hevent = NULL;
   CloseHandle(hnot_full_pending); hnot_full_pending=NULL;
   CloseHandle(hvalid); hvalid=NULL;
   DeleteCriticalSection(&event_section);
   DeleteCriticalSection(&valid_section);
#endif
}

#ifdef WIN32
static DWORD WINAPI read_loop(LPVOID dummy)
#else
static void *read_loop(void *_dummy)
#endif
{ unsigned char type,size,slot,id;
  unsigned char address[8];
  unsigned char payload[MAXPAYLOAD];

  /* the loop will exit if the bus disconnects. Then clean up */
#ifndef WIN32
  pthread_cleanup_push(clean_up_read_thread,NULL);
#endif
  while (get_request(&type,&size,&slot,&id,address,payload))
    dispatch_message(type,size,slot,id,address,payload);
#ifdef WIN32
  clean_up_read_thread(NULL);
#else
  pthread_cleanup_pop(1);
#endif
  return 0;
}



void vmb_disconnect(void)
     /* call this to terminate the reading thread properly before
        terminating the main program.
     */
{ 
#ifndef WIN32
  void *exitcode;
  int rc = pthread_mutex_lock(&event_mutex);
  if (rc) 
    { vmb_error(__LINE__,"Locking event mutex failed");
      pthread_exit(NULL);
    }
#endif 
  if (vmb_connected)
#ifdef WIN32
	  bus_disconnect(vmb_fd);
#else
    /* should not be used. Kills the thread immediately without cleaning up. */
    pthread_cancel(read_thr);
  rc = pthread_mutex_unlock(&event_mutex);
  if (rc) 
  { vmb_error(__LINE__,"Unlocking event mutex failed");
    pthread_exit(NULL);
  }
  pthread_join(read_thr,&exitcode);
#endif
 
}

/* Functions called by the CPU thread */

void vmb_connect(char *host, int port)
/* call to initialize the interface to the virtual bus 
   must be called before any of the other functions
*/
{  vmb_fd= bus_connect(host,port);
   if (vmb_fd<0) vmb_fatal_error(__LINE__,"Unable to connect to motherboard");

  vmb_connected = 1;
#ifdef WIN32
  { DWORD dwReadThreadId;
    hevent =CreateEvent(NULL,FALSE,FALSE,NULL);
    hnot_full_pending = CreateEvent(NULL,FALSE,FALSE,NULL);
    hvalid =CreateEvent(NULL,FALSE,FALSE,NULL);
    InitializeCriticalSection (&event_section);
    InitializeCriticalSection (&valid_section);
    hReadThread = CreateThread( 
            NULL,              // default security attributes
            0,                 // use default stack size  
            read_loop,        // thread function 
            NULL,             // argument to thread function 
            0,                 // use default creation flags 
            &dwReadThreadId);   // returns the thread identifier 
        // Check the return value for success. 
    if (hReadThread == NULL) 
      vmb_fatal_error(__LINE__, "Creation of bus thread failed");
/* in the moment, I really dont use the handle */
    CloseHandle(hReadThread);
  }
#else
   read_tid = pthread_create(&read_thr,NULL,read_loop,NULL);
#endif
}

void vmb_register(unsigned int address_hi, unsigned int address_lo,
		  unsigned int size,
                  unsigned int lo_mask, unsigned int hi_mask,
                  char *name)
/* call after initialize to register with the  virtual bus 
   must be called before any of the other funcions
*/
{  unsigned char limit[8];
   inttochar(address_hi,device_info.address);
   inttochar(address_lo,device_info.address+4);
   device_info.size = size;
   add_offset(device_info.address,size,limit);
   device_info.lo_mask = lo_mask;
   device_info.hi_mask = hi_mask;
   if (bus_register(vmb_fd,device_info.address,limit,lo_mask,hi_mask,name)<0)
      vmb_fatal_error(__LINE__,"Unable to register with motherboard");
}

void vmb_init_data_address(data_address *da, int size)
     /* initialize a data_address structure */
{  da->size=size;
   size = (size+7)&~0x7; /* round size up to a multiple of 8 */
   da->data=malloc(size);
   if (da->data == NULL)
     vmb_fatal_error(__LINE__,"Out of memory initializing data address pair");
    da->address_hi = 0;
   da->address_lo = 0;
   da->status = STATUS_INVALID;  
}

void vmb_store(data_address *da)
{ unsigned char id;
  unsigned char address[8];
  unsigned char size;
  if (da->size==1)
    { id = ID_WRITEBYTE; size = 0;}
  else   if (da->size==2)
    { id = ID_WRITEWYDE; size = 0;}
  else  if (da->size<=4)
    { id = ID_WRITETETRA; size = 0;}
  else
    { id = ID_WRITE; size = (da->size+7)/8-1; }
 
  inttochar(da->address_hi,address);
  inttochar(da->address_lo,address+4);
  send_msg(vmb_fd,
           (unsigned char)(TYPE_ADDRESS|TYPE_PAYLOAD),
           size,0,id,0,address,da->data);
  da->status = STATUS_VALID;
}

void vmb_load(data_address *da)
{ unsigned char id;
  unsigned char address[8];
  unsigned char size;
  da->status = STATUS_READING;
  if (da->size==1)
    { id = ID_READBYTE; size = 0;}
  else   if (da->size==2)
    { id = ID_READWYDE; size = 0;}
  else  if (da->size<=4)
    { id = ID_READTETRA; size = 0;}
  else
    { id = ID_READ; size = (da->size+7)/8-1; }
  inttochar(da->address_hi,address);
  inttochar(da->address_lo,address+4);
  enqueue_read_request(da);
  vmb_debugx(0, "Pending read request added for address %s",address,8);
  send_msg(vmb_fd,
           (unsigned char)(TYPE_ADDRESS|TYPE_REQUEST),
           size,0,id,0,address,NULL);
}
