extern HWND hMemory; 

extern INT_PTR CALLBACK   
MemoryDialogProc( HWND hDlg, UINT message, WPARAM wparam, LPARAM lparam );

/* store a function pointer here to get a memory inspector 
   the inspector should copy the required number of byte into buf
   and return the number of byte copied.
*/
extern int (*mem_inspect)(unsigned int offset, int size, unsigned char *buf);

/* call this function to tell the memory inspector that an update is due */
extern void mem_update(unsigned int offset, int size);