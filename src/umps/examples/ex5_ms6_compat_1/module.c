/****************************************************************************
 * This simple module exports termprint function, which print a string on a 
 * terminal with busy waiting on device status
 *
 ****************************************************************************/


/****************************************************************************/
/* Inclusion of header files.                                               */
/****************************************************************************/

#include "h/const.h"
#include "../../support/h/types.h"

/****************************************************************************/
/* Inclusion of imported declarations.                                      */
/****************************************************************************/


/****************************************************************************/
/* Declarations strictly local to the module.                               */
/****************************************************************************/

#define TERM0BASE	0x10000250
#define DEVLENGTH	16
#define WORDLENGTH	4
#define TRANSMCOMMAND	3
#define TRANSMSTATUS	2

#define READY	1
#define BUSY	3
#define TRANSMITTED	5

#define ACK	1
#define PRINTCHR	2
#define CHAROFFSET	8

#define STATUSMASK	0xFF

#define MAXTERM 	8

HIDDEN unsigned int termstat(terminal_t * tp);
 
/****************************************************************************/
/* Definitions to be exported.                                              */
/****************************************************************************/

/* This function prints a string on specified terminal and returns TRUE if 
 * print was successful, FALSE if not
 */
 
unsigned int termprint(char * str, unsigned int term)
{
	terminal_t * termp;
	unsigned int stat;
	unsigned int cmd;
	unsigned int error = FALSE;
	
	if (term < MAXTERM)
	{
		/* terminal is correct */
		/* compute device register field addresses */
		termp = (terminal_t *) (TERM0BASE + (term * DEVLENGTH));
		
		/* test device status */
		stat = termstat(termp);
		if (stat == READY || stat == TRANSMITTED)
		{
			/* device is available */
			
			/* print cycle */
			while (*str != EOS && !error)		
			{
				cmd = (*str << CHAROFFSET) | PRINTCHR;
				termp->transm_command = cmd;
				

				/* busy waiting */
				stat = termstat(termp);
				while (stat == BUSY)
					 stat = termstat(termp);
				
				/* end of wait */
				if (stat != TRANSMITTED)
					error = TRUE;
				else
					/* move to next char */
					str++;
			} 
		}
		else
			/* device is not available */
			error = TRUE;
	}
	else
		/* wrong terminal device number */
		error = TRUE;

	return (!error);		
}


/****************************************************************************/
/* Definitions strictly local to the module.                                */
/****************************************************************************/

/* This function returns the terminal transmitter status value given its address 
 */ 
HIDDEN unsigned int termstat(terminal_t * tp)
{
	return((tp->transm_status) & STATUSMASK);
}
