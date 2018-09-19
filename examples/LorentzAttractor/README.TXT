This directory contains a complete and selfcontained example
of a working configuration.

Configuration:
--------------    
ROM, RAM, vram Graphics card, MMIX CPU, Virtual Motherboard
The configuration is described in detail in the file default.vmb,
the configuration file read by all components (except 
the MMIX CPU).

Execution:
----------
Start the example by changing into the example directory
and start the program "mother".

mother will read deault.vmb and will start
rom, ram, vram and mmix.
These programms will in turn read again default.vmb
and connect to the motherboard.
The rom program will in addition load the file bios.img
containing the rom image for the boot rom.

Before the components become operational you should switch on
the virtual power source by typing "on" in the motherboard
window folloed by hitting the return key.
This will the vram device cause popping up its output window.

The only active component is the MMIX CPU.
It will load the MMIX object file hello.mmo into RAM
It will boot from the boot image bios.img loaded into rom
and the jump to the user program hello.mmo contained in ram.
It will then use the PutPixel Operating system call
to paint on the graphics card. The card itself
is mapped at physical address 0x0002 0000 0000 0000
which is outside the range that can be mapped
inside the virtual address range of any user program
according to the address translation rules of MMIX.
Hence it can be only addrressed through the operating system.
One coud choose to place the vram physically below
0x0001 0000 0000 0000, which is inside the mappable range
and then set up page tables to give the user program
access to it for instance as part of the pool segement.
The MMIX rules of mapping and caching then however
will access this memeory through the data cache, which
gives strange results. Check it out. You can see the
cache performing on the video ram.

You can interrupt the running MMIX by typing Ctrl-C and
single step through the program by hitting the "enter" key.
You terminate the program by switching off power
(type "off" in the motherboard window). 
and then terminate the motehrboard typing "quit".

You can modify the source programs
   hello.mms and bios.mms
to produce object files use the mmix assembler mmixal
 run: mmixal bios.mms 
 and: mmixal hello,mms  
this will produce hello.mmo and bios.mmo
To produce the boot image bios.img
you have to run mmoboot on bios.mmo 
 run: mmoboot bios.mmo
producing bios.img

If you have make (you should), you can remake all files
just by typing make.




    