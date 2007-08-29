/* File: $Id: xinterface.cc,v 1.1 2007-08-29 09:19:37 ruckert Exp $ */

/****************************************************************************
 *
 * This module defines the XInterface class. A single object of this class
 * controls all windows opening, closing and syncronization in simulator
 * user interface.  
 * All windows are called "forms" since they are built using a X interface
 * library called XFORMS.  
 * Forms may be split in two families: application windows (for specific
 * purposes), which use a common AppForms interface; and forms used for
 * range insertion of breakpoints/suspects/traces; they use RangeForm
 * interface.  
 * AppForm and RangeForm are interface (i.e.  empty) classes for specific
 * forms (or groups of forms). Each form is tagged with a specific index,
 * and all forms with the same interface are organized into a table.
 * All forms are defined in appforms.cc module.
 *
 ****************************************************************************/


/****************************************************************************/
/* Inclusion of header files.                                               */
/****************************************************************************/
#include <h/const.h>
#include <h/types.h>

#include <stdlib.h>
#include <strings.h>
#include <stdio.h>

#include <forms.h>



/****************************************************************************/
/* Inclusion of imported declarations.                                      */
/****************************************************************************/

#include <e/setup.e>
#include <e/watch.e>
#include <e/appforms.e>
#include <e/utility.e>


/****************************************************************************/
/* Declarations strictly local to the module.                               */
/****************************************************************************/

// XInterface forms index
#define MAINFORM	0
#define SYMBFORM	1
#define MEMBFORM	2
#define TLBDFORM	3
#define DEVFORMBASE	4
#define SETPFORM (DEVFORMBASE)

#define PRANGEFORM	0
#define VRANGEFORM	1


/****************************************************************************/
/* Definitions to be exported.                                              */
/****************************************************************************/

// A single object of XIinterface class
// controls all windows opening, closing and syncronization in simulator
// user interface.  
// All windows are called "forms" since they are built using a X interface
// library called XFORMS.  
// Forms may be split in two families: application windows (for specific
// purposes), which use a common AppForms interface; and forms used for
// range insertion of breakpoints/suspects/traces; they use RangeForm
// interface.  
// AppForm and RangeForm are interface (i.e.  empty) classes for specific
// forms (or groups of forms). Each form is tagged with a specific index,
// and all forms with the same interface are organized into a table.

class XInterface
{
	public:
		// This method decodes command line arguments, creates a XInterface
		// object and related structures
		XInterface(int * argcp, char * argv[]);
		
		// This method disposes of all structures contained in a XInterface
		// object and deletes it
		~XInterface(void);

		// This method resets simulator _completely_.  Returns TRUE if reset
		// has been done, FALSE otherwise: if message == NULL, the reset is
		// always done; if message != NULL, a question is shown and reset is
		// done only if answer is affirmative. toDefaults tells if even
		// current configuration (in SetupInfo object) need to be restored
		// to defaults
		Boolean Reset(Boolean toDefaults, const char * message = NULL);

		// This method is called whenever user wants to shutdown the
		// simulator 
		void Quit(void);

		// This method forces refresh for all displayed forms; fullRef tells
		// if refresh should be complete or not, for performance reasons 
		void RefreshAllVForms(Boolean fullRef);

		// This method forces refresh for a single form (indexed by num);
		// fullRef tells if refresh should be complete or not, for
		// performance reasons
		void RefreshVForm(unsigned int num, Boolean fullRef);

		// This method "freezes" (stops visual display update of) the form
		// indexed by num: it is done to reduce flicker and speed up update
		void FreezeVForm(unsigned int num);

		// This method "freezes" all forms
		void FreezeAllVForms(void);

 		// This method "unfreezes" (forces display update of) the form
 		// indexed by num
		void UnFreezeVForm(unsigned int num);

		// This method "unfreezes" all forms
		void UnFreezeAllVForms(void);

		// This method shows main user interface window
		void ShowMainForm(void);

		// This method shows simulator setup interface window
		void ShowSetupForm(void);

		// This method shows the generic application form indexed by num
		void ShowForm(unsigned int num);

		// This method shows the range insertion form for virtual or
		// physical ranges (told apart by isVirtual flag); the range
		// inserted will be of BRKPT(SUSP/TRACE rType, and will be inserted
		// in rBrowp browser item
		void ShowRangeForm(Boolean isVirtual, unsigned int rType, FL_OBJECT * rBrowp);

		// This method starts simulation running
		void PrepareRun(void);

		// This method checks X event queue for user interaction when
		// simulation is running; if specific actions are performed, (like
		// pressing on "Stop" button) simulation itself is stopped
		void CheckForStop(void);

		// This method stops simulation running
		void PrepareStop(void);

		// This method runs user interface when simulation is not running
		void MainLoop(void);

	private:
		
		// configuration information object
		SetupInfo * setup;

		// watch interface object
		Watch * watch;

		// form windows table
		AppForm * formTable[FORMSNUM];

		// range insertion form windows table
		RangeForm * rfTable[RFORMSNUM];

		// simulation boot type ("core", from disk, from tape)
		unsigned int bootType;

		// .core image file for "core" boot
		char * bootFile;
		
		// This method decodes command line arguments (see showHelp() for
		// details): returns TRUE on successful decode, FALSE otherwise
		Boolean decodeArgs(int argc, char * argv[]);

		// This function prints a warning/help message on standard error
		void showHelp(const char * prg);
};


// This method decodes command line arguments, creates a XInterface object
// and related structures
XInterface::XInterface(int *argcp, char * argv[])
{
	int i;
	
	setup = NULL;
	watch = NULL;
	bootType = COREBOOT;
	bootFile = NULL;
	
	fl_initialize(argcp, argv, "MPS", NULL, 0);

// no more arguments decoding since .umpsrc introduction	
//	if (!decodeArgs(*argcp, argv))
//	{
//		// unknown line arguments: abort
//		showHelp(argv[0]);
//		fl_finish();
//		exit(EXIT_FAILURE);
//	}
//	else
//	{	
		// build all window forms
		fl_set_goodies_font(FL_NORMAL_STYLE, FL_NORMAL_SIZE);

		InitAppForms(this);
		
		formTable[MAINFORM] = new MainForm();
		
		formTable[SYMBFORM] = new SymbForm(SYMBFORM);
		
		formTable[MEMBFORM] = new MemBrowser(MEMBFORM);
		
		formTable[TLBDFORM] = new TLBDisplay(TLBDFORM);
		
		formTable[SETPFORM] = new SetupForm();
	
		
		rfTable[PRANGEFORM] = new PRangeForm();
		rfTable[VRANGEFORM] = new VRangeForm();
		
		// reset simulator
		
		Reset(TRUE);
//	}
}


// This method disposes of all structures contained in a XInterface object
// and deletes it 
XInterface::~XInterface(void) 
{
	unsigned int i;
	
	delete setup;
	watch->DeleteHTable();
	delete watch;
	for (i = 0; i < FORMSNUM; i++)
		delete formTable[i];
	
	if (bootFile != NULL)
		delete bootFile;		
}

// This method resets simulator _completely_.  Returns TRUE if reset has
// been done, FALSE otherwise: if message == NULL, the reset is always done;
// if message != NULL, a question is shown and reset is done only if answer
// is affirmative. toDefaults tells if even current configuration (in
// SetupInfo object) need to be restored to defaults. Reset of the simulator
// is easily obtained deleting and re-creating all objects contained: in a
// chain of destructions and constructions, all new objects are reset to
// startup state
Boolean XInterface::Reset(Boolean toDefaults, const char * message)
{
	Boolean ret;
	
	FreezeAllVForms();
	
	if ((setup  != NULL && setup->getExpertMode()) || message == NULL || SAMESTRING(message, EMPTYSTR) || \
		fl_show_question(message, FALSE)) 
	{
		ret = TRUE;
		// deletes setup only when a reset to defaults is requested
		if (toDefaults && setup != NULL)
		{	
			delete setup;
			setup = NULL;
		}

		if (setup == NULL)
			// boot-up of simulator or reset toDefaults requested
			setup = new SetupInfo(bootType, bootFile);
	
		if (watch != NULL)
		{
			// deletes watch object 
			delete watch;

			if (toDefaults)
				// rebuilds the watch object and the hash table
				watch = new Watch(setup, this, TRUE);
			else
				// reset to setup: rebuilds watch but keeps the old hash table
				watch = new Watch(setup, this, FALSE);
		}
		else
			// watch == NULL means first activation: must build the hash table
			watch = new Watch(setup, this, TRUE);

		ResetAppForms(setup, watch); 

		RefreshAllVForms(TRUE);
	}
	else
		ret = FALSE;
			
	UnFreezeAllVForms();
	return(ret);
}


// This method is called whenever user wants to shutdown the simulator
void XInterface::Quit(void)
{
	delete this;
	fl_finish();
	exit(EXIT_SUCCESS);
}


// This method forces refresh for all displayed forms; fullRef tells if
// refresh should be complete or not, for performance reasons 
void XInterface::RefreshAllVForms(Boolean fullRef)
{
	unsigned int i;

	for (i = 0; i < FORMSNUM; i++)
		(formTable[i])->RefreshV(fullRef);
}


// This method forces refresh for a single form (indexed by num); fullRef
// tells if refresh should be complete or not, for performance reasons
void XInterface::RefreshVForm(unsigned int num, Boolean fullRef)
{
	(formTable[num])->RefreshV(fullRef);
}


// This method starts simulation running
void XInterface::PrepareRun(void)
{
	unsigned int i;
	
	for (i = 0; i < FORMSNUM; i++)
		(formTable[i])->RunSetup();
}	
	
// This method checks X event queue for user interaction; if specific
// actions are performed, (like pressing on Stop button) simulation is
// stopped.
// This is done using a single specific XFORMS library function
void XInterface::CheckForStop(void)
{
	if (fl_check_forms() != NULL)
		Panic("Unknown form object detected in XInterface::CheckForStop()");
}


// This method stops simulation running 
void XInterface::PrepareStop(void)
{
	unsigned int i;
	
	for (i = 0; i < FORMSNUM; i++)
		(formTable[i])->StopSetup();
}


// This method "freezes" (stops visual display update of) the form
// indexed by num: it is done to reduce flicker and speed up update
void XInterface::FreezeVForm(unsigned int num)
{
	(formTable[num])->FreezeV();
}

// This method "unfreezes" the form indexed by num
void XInterface::UnFreezeVForm(unsigned int num)
{
	(formTable[num])->UnFreezeV();
}


// This method "freezes" the display of all forms
void XInterface::FreezeAllVForms(void)
{
	unsigned int i;
	
	for (i = 0; i < FORMSNUM; i++)
		(formTable[i])->FreezeV();
}


// This method "unfeezes" the display of all forms
void XInterface::UnFreezeAllVForms(void)
{
	unsigned int i;
	
	for (i = 0; i < FORMSNUM; i++)
		(formTable[i])->UnFreezeV();
}


// This method shows main user interface window
void XInterface::ShowMainForm(void)
{
	(formTable[MAINFORM])->Show();
}


// This method shows simulator setup interface window
void XInterface::ShowSetupForm(void)
{
	(formTable[SETPFORM])->Show();
}


// This method shows the generic application form indexed by num
void XInterface::ShowForm(unsigned int num)
{
	(formTable[num])->Show();
}	


// This method shows the range insertion form for virtual or physical ranges
// (told apart by isVirtual flag); the range inserted will be of
// BRKPT(SUSP/TRACE rType, and will be inserted in rBrowp browser item
void XInterface::ShowRangeForm(Boolean isVirtual, unsigned int rType, FL_OBJECT * rBrowp)
{
	if (isVirtual)
		(rfTable[VRANGEFORM])->InsertRange(rType, rBrowp);
	else
		(rfTable[PRANGEFORM])->InsertRange(rType, rBrowp);
}


// This method runs user interface when simulation is not running
void XInterface::MainLoop(void)
{
	if (fl_do_forms() != NULL)
		Panic("Undetected form object in XInterface::MainLoop()");
}

// This function prints a warning/help message on standard error
void XInterface::showHelp(const char * prg)
{
	fprintf(stderr, "%s : Wrong/unknown argument(s)\n\n", prg);
	fprintf(stderr, "%s syntax : %s [-d | -t | <file>]\n\n", prg, prg);
	fprintf(stderr, "<file>\tboot using core file <file>\n\n");	
}

 