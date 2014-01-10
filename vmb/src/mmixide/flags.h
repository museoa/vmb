extern DWORD DFlags;

#define FLAG_AUTOSAVE (1<<0)
#define FLAG_SHOW_LINESNO (1<<1)
#define FLAG_SHOW_PROFILE (1<<2)


extern void set_flags(void);

extern void get_flags(void);