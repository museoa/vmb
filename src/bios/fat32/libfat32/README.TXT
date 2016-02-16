Documentation


The primitive access to the physical disk is provided in File
FAT32_Disk.h and FAT32_Disk.c
here functions are provided to initialize the disk, and read and write
sectors 
these functions use the external functions disk_init, disk_read and disk_write.


The other end of the interface is provided in Files
FAT32_FileLib.h and FAT32_FileLib.c