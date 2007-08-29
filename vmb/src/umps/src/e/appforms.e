/* File: $Id: appforms.e,v 1.1 2007-08-29 09:19:37 ruckert Exp $ */

/****************************************************************************
 *
 * External declarations from the corresponding module .cc
 *
 ****************************************************************************/

class SetupInfo;
class XInterface;
class Watch;
class FD_MainForm;
class FD_SetupForm;
class FD_MemBrowser;
class FD_DevStatus;
class FD_TLBDisplay;
class FD_SymbForm;
class FD_Terminal;		
class FD_VRangeForm;
class FD_PRangeForm;


//
// These functions initialize some pointers used by callbacks to
// access other objects in the simulator.
//

void InitAppForms(XInterface * xi);

void ResetAppForms(SetupInfo * stp, Watch * wch);


/**************************************************************************/


// This interface class allows to display range insertion window as needed,
// and let the user insert a address range of wanted type, putting a
// description for it in a browser gadget at closing.

class RangeForm
{
	public:
		// This method will open a RangeForm window, ready for insertion of
		// desired rType range of addresses (breakpoint, suspects, traces)
		virtual void InsertRange(unsigned int rType, FL_OBJECT * rBrowser) = 0;
};		


/****************************************************************************/


// This interface class give standard methods used by XInterface and
// callback functions to control window displaying and update.

class AppForm
{
	public:
		// This method will refresh the displayed contents of the selected
		// window; fullRef tells if double-buffering by Watch should be
		// overridden, to force display update to actual simulation state
		virtual void RefreshV(Boolean fullRef) = 0;

		// This method will "freeze" window contents to actual ones, thus
		// permitting display update without flicker
		virtual void FreezeV(void) = 0;
		
		// This method will unfreeze window contents to force refreshing it
		virtual void UnFreezeV(void) = 0;
		
		// This method will open the window and show it
		virtual void Show(void) = 0;
		
		// This method deactivates some window gadgets as needed, when
		// simulation starts running, to forbid user interaction with
		// controls active when simuation is not running
		virtual void RunSetup(void) = 0;
		
		// This method reactivates window gadget when simulation stops
		virtual void StopSetup(void) = 0;
};


/****************************************************************************/

		
// MainForm window is the main window of the simulator. It allows to start
// and stop simulation, and to monitor an change processor internal status.
// It also provides some menus which allow to open other windows and
// terminate the application.
// 
// See AppForm class for method descriptions.

class MainForm : public AppForm
{
	public:

		MainForm(unsigned int dummy = 0);
		virtual ~MainForm(void);
		void RefreshV(Boolean fullRef);
		void FreezeV(void);
		void UnFreezeV(void);
		void Show(void);
		void RunSetup(void);
		void StopSetup(void);

	private: 

		// This method builds the MainForm static structure
		FD_MainForm * create_form_MainForm(void);
};


/****************************************************************************/


// SetupForm window is the configuration window for the simulator. It allows
// to change device configuration, memory installed size, processor speed
// and ROM and kernel files.
//
// See AppForm class for method descriptions.

class SetupForm : public AppForm
{
	public:

		SetupForm(unsigned int dummy = 0);
		virtual ~SetupForm(void);
		void RefreshV(Boolean fullRef);
		void FreezeV(void);
		void UnFreezeV(void);
		void Show(void);
		void RunSetup(void);
		void StopSetup(void);

	private: 

		// This method builds the static structure
		FD_SetupForm * create_form_SetupForm(void);
};


/****************************************************************************/


// MemBrowser window is the debugging tool window of the simulator.  It
// allows to define and delete breakpoint, suspect and trace ranges, browse
// memory and change it.
//
// See AppForm class for method descriptions.


class MemBrowser : public AppForm
{
	public:
	
		// formNum is XInterface form identification number
		MemBrowser(unsigned int formNum);
		virtual ~MemBrowser(void);
		void RefreshV(Boolean fullRef);
		void FreezeV(void);
		void UnFreezeV(void);
		void Show(void);
		void RunSetup(void);
		void StopSetup(void);

	private: 
	
		// This method builds the static structure
		FD_MemBrowser * create_form_MemBrowser(unsigned int fNum);
};


/****************************************************************************/


// DevStatus windows report device status. There is one window for each
// interrupt line, terminals excluded. It displays current device status and
// allow user making device fail on hardware operations.
//
// See AppForm class for method descriptions.


class DevStatus : public AppForm
{
	public:
	
		// intLine is used to recognize devStatus window
		DevStatus(unsigned int intLine);
		virtual ~DevStatus(void);
		void RefreshV(Boolean fullRef);
		void FreezeV(void);
		void UnFreezeV(void);
		void Show(void);
		void RunSetup(void);
		void StopSetup(void);

	private: 

		// intLine passed before as argument	
		unsigned int dSpIndex;
		
		// This method builds a static structure
		FD_DevStatus * create_form_DevStatus(unsigned int intL);
};


/****************************************************************************/


// TLBDisplay window shows TLB contents of CP0 coprocessor, and allows user
// change it.
//
// See AppForm class for method descriptions.


class TLBDisplay : public AppForm
{
	public:
	
		// formNum is XInterface form identification number
		TLBDisplay(unsigned int formNum);
		virtual ~TLBDisplay(void);
		void RefreshV(Boolean fullRef);
		void FreezeV(void);
		void UnFreezeV(void);
		void Show(void);
		void RunSetup(void);
		void StopSetup(void);

	private: 
	
		// This method builds the static structure
		FD_TLBDisplay * create_form_TLBDisplay(unsigned int fNum);
};


/****************************************************************************/


// SymbForm window is used to display the symbol table loaded by the simulator.
//
// See AppForm class for method descriptions.


class SymbForm : public AppForm
{
	public:

		SymbForm(unsigned int formNum);
		virtual ~SymbForm(void);
		void RefreshV(Boolean fullRef);
		void FreezeV(void);
		void UnFreezeV(void);
		void Show(void);
		void RunSetup(void);
		void StopSetup(void);

	private: 

		// This method builds the static structure
		FD_SymbForm * create_form_SymbForm(unsigned int fNum);
};


/****************************************************************************/


// Terminal windows display terminal screen, and transmitter/receiver device
// status. They allow the user to input a line on the terminal, and to make
// the device fail hardware operations.
// There is one terminal window for each terminal installed.
//
// See AppForm class for method descriptions.


class Terminal : public AppForm
{
	public:
	
		// tNum is terminal number 
		Terminal(unsigned int tNum);
		virtual ~Terminal(void);
		void RefreshV(Boolean fullRef);
		void FreezeV(void);
		void UnFreezeV(void);
		void Show(void);
		void RunSetup(void);
		void StopSetup(void);

	private: 
	
		// terminal number
		unsigned int termNum;
		
		// This method builds the static structure
		FD_Terminal * create_form_Terminal(unsigned int tNum);
};


/****************************************************************************/


// VRangeForm window lets the user insert a virtual address range of wanted
// type, putting a description for it in a browser gadget at closing.
//
// See AppForm class for method descriptions.

class VRangeForm : public RangeForm
{
	public:
	
		VRangeForm(void);
	
		virtual ~VRangeForm(void);
	
		virtual void InsertRange(unsigned int rType, FL_OBJECT * rBrowser);
		
	private: 
	
		// This method builds the static structure
		FD_VRangeForm * create_form_VRangeForm(void);
};


/****************************************************************************/


// PRangeForm window lets the user insert a physical address range of wanted
// type, putting a description for it in a browser gadget at closing.
//
// See AppForm class for method descriptions

class PRangeForm : public RangeForm
{
	public:

		PRangeForm(void);

		virtual ~PRangeForm(void);

		virtual void InsertRange(unsigned int rType, FL_OBJECT * rBrowser);
		
	private: 
	
		// This method builds the static structure
		FD_PRangeForm * create_form_PRangeForm(void);
};
