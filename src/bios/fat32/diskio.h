


extern bool disk_init(void);
extern bool disk_read(unsigned int sector, unsigned int count, BYTE * buffer);
extern bool disk_write(unsigned int sector, unsigned int count, BYTE * buffer);
