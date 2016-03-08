This directory contains a simple hello world program

Source Files:
-------------
README.TXT	This file
hello.mms	The hello world application
bios.mms	The BIOS necessary to provide screen output (and keyboard input)
default.vmb	The configuration file that defines the vmb devices that make up the hardware

Generated Files:
hello.mmo	The hello world application object file
			generate from hello.mms by the command
			mmixal	hello.mms
			
bios.mmo	The BIOS object file
			generate from bios.mms by the command
			mmixal	bios.mms

bios.img	The BIOS image file that gets loaded into the ROM device
			generate from bios.mmo by the command
			mmoimg	bios.mmo
	
Configuration:
--------------    
ROM, RAM, Keyboard, Screen, MMIX CPU, Virtual Motherboard
The configuration is described in detail in the file default.vmb,
the configuration file read by all components (except 
the MMIX CPU).

	
Starting the example (Windows):
-------------------------------
Start a command shell window and generate hello.mmo, bios.mmo, and bios.img as described above.

Double click the file default.vmb to start the vmb hardware.
Click the power button on the motherboard.

It will start the motherboard, RAM, ROM, keyboard, screen and the MMIX CPU in interactive mode.
The application hello.mmo is already loaded to RAM.

To run the application in continous mode type c at the prompt followed by the <return> key.
The screen shows the "hello world!".
After that you can click on the keyboard to give it the input focus and type. Whatever you
type is echoed on the screen.

Starting the example (Unix):
-------------------------------
In a terminal window, type make to generate bios.img and hello.mmo,
then start the program mother. It will read the deafouls.vmb file
and start the necessary devices. 
These programms will in turn read again default.vmb
and connect to the motherboard.
The rom program will in addition load the file bios.img
containing the rom image for the boot rom.

Before the components become operational you should switch on
the virtual power source by typing "on" in the motherboard
window folloed by hitting the return key.

Running the program:
--------------------
The most interesting device is the MMIX CPU.
It will load the MMIX object file hello.mmo into RAM
and start in interactive mode.
You get simple help by typing h as a response to the
mmix> prompt. If you are not interested in mmix,
just type c (continue) and "enter" and the program will run without
further bothering you.
The MMIX CPU boots by executing instructions from
virtual address 0x8000000000000000 which is mapped
to pysical address 0, thats just where the rom is mapped
on the bus. It will then jump to the user program at virtual address #100
(in the text segement mapped to physical address 0x0000000100002100)
 there it will load the 
virtual address 0x2000000000000000, where the Text "Hello World!"
is located, it will use an operating system call (implemented in the
rom), to write this text to the screen. Upon return from this
call the user program will call TRAP 0,Halt,0 terminating
the user program and returning the the operating system.

The operating system will run into an idle loop, waiting for events.
If you type into the keyboard window,
interupts comming from the keyboard will be served by the MMIX CPU
as directed in the bios and input
characters from the keyboard will be copied to the screen.
This is a kind of echo funtion.

Things to do:
-------------
- You can interrupt the running MMIX by typing Ctrl-C and single step through the program by hitting the "enter" key.
- You can rerun the whole program 
- - by clicking the Reset button on the mother board (Windows)
- - by typing reset in the motherboard window (Unix).
- You can trace the execution of the program by just typing the <return> key in single step mode.
- You get help by typing h  at the prompt followed by the <return> key.
- You can enable the debugging/tracing inside the BIOS by typing -O at the prompt followed by the <return> key.
- You can terminate the CPU by typing q at the prompt followed by the <return> key.
- You can terminate all devices 
- - by clicking right on the motherboard and selecting Exit from the contex menu (Windows).
- - by typing quit in the motherboard window (Unix).
- You can modify the source programs hello.mms and bios.mms and start over (dont forget to remake
  bios.img and hello.mmo before restarting the motherboard.

