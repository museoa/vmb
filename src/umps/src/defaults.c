/*!
 * \file defaults.c 
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief Default Options for Simulator
 *
 * This File offers defaults and some helper Functions for the variables expected by
 * main.c. It also offers the ::parseArgv function which enables to destinct between
 * cmdline args for the virtual motherboard connection layer or for the simulator itself
 * in it's orginal form. This File was completely written from scratch as the simulator
 * was ported by Martin Hauser.
 *
 * (c) Copyright 2006 Martin Hauser. All Rights Reserved. 
 */
  
#include "h/defaults.h" /*!< include corresponding header file */
#include "../../error.h"
#include <string.h>
#include <stdio.h>

// char *programpath = NULL;                   /*!< Path to the programm, not used here */

char *programname = "uMPS MIPS R3000 Simulator"; /*!< Name of the CPU */

char *host = "localhost";                   /*!< The host to connect to */
int  port = 9002;                           /*!< The port to connect to */

int debugflag = 0; /*!< Whether the simulator is in debug-mode or not */

char version[]="$Revision: 1.2 $ $Date: 2008-05-04 15:46:59 $"; /*!< Version String for the simulator */

char howto[] =
 "\n"
 "The Program simulates an MIPS R3000 CPU."
 "\n"; /*!< howto use this simulator (not written yet) */

unsigned long interrupts = 0xFFFFFFFC; /*!< interrupt register string */
int savArgc; /*!< Save argument counter (ugly but needed) */
char** savArgv = NULL; /*!< Save Argument Vector (ugly but needed) */ 

unsigned int enableCache = 1;

unsigned long ulRamSize = 0;
unsigned int  enableRomFork = 1;

unsigned int enablePerf = 0;

#define ARGCMP(i,str) (strcmp(argv[i],str) == 0)
#define argEnd(i) (i == argc)
/*
 * \fn void printUsage()
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief prints usage, calls ::clean_argv and exits
 *
 * This function prints the usage to stdout and exits
 */

 void printUsage()
 {
     printf("\"%s\" [options]\n",programname);
     printf("[options] could be:"
         "\t-h,   --help               this help.\n"
         "\t-B,   --host               The hostname of the motherboard server.  \n"
         "\t-p,   --port               The port of the motherboard server.      \n"
         "\t-di,  --disable-interrupts Disable the ability for the simulator to \n"
         "\t                           catch any interrupts at all.             \n"
         "\t      --interrupt-mask     sets the mask of interrupts for all      \n"
         "\t                           interrupts the simulator shall be able   \n"
         "\t                           to catch.                                \n"
         "\t      --program-name       set alternate program name               \n"
         "\t-dic, --disable-icache     disables the instruction cache           \n"
         "\t      --defaults           prints the defaults for these settings   \n"
         "\t-d,   --debug              enable some debug messages               \n"
         "\t      --ram-size           set the size of the ram that has been    \n"
         "\t                           installed into the virtual Motherboard   \n"
         "\t                           make sure that you specify this either   \n"
         "\t                           via this switch or via RAMPAGESIZE of    \n"
         "\t                           the .umpsrc files!                       \n"
         "\t-dr, --disable-rom-fork    Disable the BOOTROM and EXECROM settings \n"
         "\t                           and the resulting fork/exec calls. You   \n"
         "\t                           have to take care of the presence of the \n"
         "\t                           rom executeables yourself then!          \n"
         "\t     --enable-perf-tests   Enables performance measuring. It'll     \n"
         "\t                           print the resulting data to stderr with  \n"
         "\t                           a prefix of PERF:.                       \n"
         );
     clean_argv();
     exit(-1);
 }

/*
 * \fn printDefaults()
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief prints default values
 *
 * This function prints most of the default values available to be changed
 * via settings of the command line interface. It then calles ::clean_argv
 * and exits.
 */

void printDefaults()
{
    printf("default settings:\n");
    printf("host           = %s\n"
           "port           = %d\n"
           "ram-size       = %d\n"
           "interrupt-mask = 0x%x\n"
           "program-name   = %s\n"
           "debug-flag     = %d\n"
           "cache-flag     = %d\n"
           "rom-fork-flag  = %d\n"
           "perf-tests     = %d\n",
           host,port,ulRamSize,interrupts,programname,debugflag,enableCache,enableRomFork,enablePerf);
       clean_argv();
       exit(-1);
}

/*!
 * \fn int parseArgv(int argc, char **argv)
 * \author Martin Hauser <info@martin-hauser.net>
 * \param argc The count of the arguments given
 * \param argv The array containing the Commandline arguments
 * \return The number of arguments substracted
 * \brief parses the settings for the virtual Motherboard layer of the simulator
 * \todo Remove assumption that Options for the virtual Motherboard come before the others
 * \warning This Code assumes that the options for the virtual motherboard are given First on the Commandline
 */
int parseArgv(int argc, char **argv)
{
    int i;
    for(i=1; i < argc; i++) //!< loop through all arguments
    {
        if(ARGCMP(i,"--help") || ARGCMP(i,"-h")) //!< printing usage
            printUsage();
        else if(ARGCMP(i,"--host") || ARGCMP(i,"-B")) //!< set hostname
        {
            if(argEnd(i))
            {
                clean_argv();
                fatal_error(__LINE__,"Host argument needs hostname given!\n");
            }
            host = malloc(strlen(argv[i+1]));
            strcpy(host,argv[i+1]);
        }
        else if(ARGCMP(i,"-p") || ARGCMP(i,"--port")) //!< deal with port settings
        {
            if(argEnd(i))
            {
                clean_argv();
                fatal_error(__LINE__,"Port argument needs port given!\n");
            }
            if(sscanf(argv[i+1],"%d",&port) != 1 || port > 65535 || port < 1)
            {
                clean_argv();
                fatal_error(__LINE__,"Failed to parse portnumber from given argument!\n");
            }
        }
        else if(ARGCMP(i,"--disable-interrupts") || ARGCMP(i,"-di")) //!< interrupts?
            interrupts = 0;
        else if(ARGCMP(i,"--interrupt-mask")) //!< interrupt mask setting
        {
            if(argEnd(i))
            {
                clean_argv();
                fatal_error(__LINE__,"Interruptmask argument needs mask given!\n");
            }
            if(sscanf(argv[i+1],"%x",&interrupts) != 1 || interrupts > 0xFFFFFFC || interrupts < 1)
            {
                clean_argv();
                fatal_error(__LINE__,"interrupt mask is out of bounds (1 < interruptmask < 0xFFFFFFFC)\n");
            }
        }
        else if(ARGCMP(i,"--program-name")) //!< change program name
        {
            if(argEnd(i))
            {
                clean_argv();
                fatal_error(__LINE__,"program name argument needs name given!\n");
            }
            programname = malloc(strlen(argv[i+1]));
            strcpy(programname,argv[i+1]);
        }
        else if(ARGCMP(i,"-dic") || ARGCMP(i,"--disable-icache")) //!< instruction cache
            enableCache = 0;
        else if(ARGCMP(i,"--defaults")) //!< print default values
            printDefaults();
        else if(ARGCMP(i,"--debug") || ARGCMP(i,"-d")) //!< debug switch
            debugflag = 1;
        else if (ARGCMP(i,"--disable-rom-fork") || ARGCMP(i,"-dr")) //!< rom fork controll
            enableRomFork = 0;
        else if (ARGCMP(i,"--enable-perf-tests")) //!< performance testing for caches
            enablePerf = 1;
        else if(ARGCMP(i,"--ram-size")) //!< control ram size
        {
            if(argEnd(i))
            {
                clean_argv();
                fatal_error(__LINE__,"ram size argument needs name given!\n");
            } 
            if(sscanf(argv[i+1],"%x",&ulRamSize) != 1 || ulRamSize <= 4)
            {
                clean_argv();
                fatal_error(__LINE__,"Rammsize too small!\n");
            }
        }
    }
    savArgv = argv;
    savArgc = 1;
}

/*
 * \fn void clean_argv()
 * \author Martin Hauser <info@martin-hauser.net>
 * \brief cleans allocated variables
 *
 * This function checks whether variables have been allocated using malloc
 * and if so free's them. This only goes for variables used to config the
 * startup of the system.
 */


void clean_argv()
{
    if(host != NULL && strcmp(host,"localhost") != 0)
        free(host);
    if(programname != NULL && strcmp(programname,"uMPS MIPS R3000 Simulator") != 0)
        free(programname);
}   
