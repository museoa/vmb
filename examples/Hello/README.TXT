This directory contains a simple hello world program

Source Files:
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
			
Runnning the example:
Start a command shell window and generate hello.mmo, bios.mmo, and bios.img as described above.

Double click the file default.vmb to start the vmb hardware.
Click the power button on the motherboard.

It will start the motherboard, RAM, ROM, keyboard, screen and the MMIX CPU in interactive mode.
The application hello.mmo is already loaded to RAM.

To run the application in continous mode type c at the prompt followed by the <return> key.
The screen shows the "hello world!".
After that you can click on the keyboard to give it the input focus and type. Whatever you
type is echoed on the screen.

Things to do:
- You can rerun the whole program by clicking the Reset button on the mother board.
- You can trace the execution of the program by just typing the <return> key in single step mode.
- You get help by typing h  at the prompt followed by the <return> key.
- You can enable the debugging/tracing inside the BIOS by typing -O at the prompt followed by the <return> key.
- You can terminate the CPU by typing q at the prompt followed by the <return> key.
- You can terminate all devices clicking right on the motherboard and selecting Exit from the contex menu.


