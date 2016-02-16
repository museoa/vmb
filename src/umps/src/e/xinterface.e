/* File: $Id: xinterface.e,v 1.2 2008-05-04 15:46:59 mbbh Exp $ */

/****************************************************************************
 *
 * External declarations from the corresponding module .cc
 *
 ****************************************************************************/

class SetupInfo;
class Watch;
class AppForm;
class RangeForm;


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
