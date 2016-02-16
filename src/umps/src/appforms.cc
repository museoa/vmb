/* File: $Id: appforms.cc,v 1.2 2008-05-04 15:46:59 mbbh Exp $ */

/****************************************************************************
 *
 * This module contains full definition for user interface windows. 
 * All windows are called "forms" since they are built using a X interface
 * library called XFORMS.  
 * Forms may be split in two families: application windows (for specific
 * purposes), which use a common AppForms interface; and forms used for
 * range insertion of breakpoints/suspects/traces; they use RangeForm
 * interface.  
 * AppForm and RangeForm are interface (i.e.  empty) classes for specific
 * forms (or groups of forms). Each form is tagged with a specific index,
 * and all forms with the same interface are organized into a table. 
 * XFORMS describes user actions activating some "callback functions" in
 * response to user action: they will perform some operations on objects and
 * force refresh of user interface contents when needed.  Because of XFORMS
 * library peculiarities, methods cannot be used as "callback functions":
 * they must be regular C functions.  For this reason, to each conceptual
 * "form object" a static structure is defined, and hidden here. 
 * This way, all modules may interface this one without knowing there are no
 * full-fledged objects inside. See h/formdefs.h for object contents.
 *
 * See XFORMS documentation for details on library functions.
 *
 ****************************************************************************/

/****************************************************************************/
/* Inclusion of header files.                                               */
/****************************************************************************/
#include <h/const.h>
#include <h/types.h>

#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <ctype.h>

#include <forms.h>

#include <h/formdefs.h>

/****************************************************************************/
/* Inclusion of imported declarations.                                      */
/****************************************************************************/

#include <e/setup.e>
#include <e/watch.e>
#include <e/xinterface.e>
#include <e/utility.e>


/****************************************************************************/
/* Declarations strictly local to the module.                               */
/****************************************************************************/

// Static buffer sizes and definitions
#define STRBUFSIZE	256
#define SMALLSTRBUF	64
HIDDEN char strbuf[STRBUFSIZE];

// memory change operation code
#define MEMMOD	3

// number of memory words to be dumped in memDumpBrowser: maximum is 4
#define DUMPWORDS	2

// pointers to other objects
HIDDEN SetupInfo * setup;
HIDDEN XInterface * xIntf;
HIDDEN Watch * watch;

//
// Callback operations may be easily understood by looking at names and
// checking to which window items they are connected to
//

// MainForm static structure and callback functions
HIDDEN FD_MainForm * mFp;
HIDDEN void sStepCB(FL_OBJECT *dummyob, SWord dummy);
HIDDEN void stopCB(FL_OBJECT *dummyob, SWord dummy);
HIDDEN void runCB(FL_OBJECT *dummyob, SWord dummy);
HIDDEN void stepCB(FL_OBJECT *dummyob, SWord dummy);
HIDDEN void excCB(FL_OBJECT *ob, SWord dummy);
HIDDEN void suspCB(FL_OBJECT *ob, SWord dummy);
HIDDEN void utlbk0CB(FL_OBJECT *ob, SWord dummy);
HIDDEN void utlbk1CB(FL_OBJECT *ob, SWord dummy);
HIDDEN void brkptCB(FL_OBJECT *ob, SWord dummy);
HIDDEN void updSpeedCB(FL_OBJECT *ob, SWord dummy);
HIDDEN void regHandleCB(FL_OBJECT *ob, SWord which);
HIDDEN void setupHandleCB(FL_OBJECT *dummyob, SWord dummy);
HIDDEN void winHandleCB(FL_OBJECT *dummyob, SWord dummy);
HIDDEN void exitHandleCB(FL_OBJECT *dummyob, SWord dummy);
HIDDEN void resetHandleCB(FL_OBJECT *dummyob, SWord dummy);
HIDDEN int exitCB(FL_FORM * dummyform, void * dummy);


// SetupForm static structure and callback functions
HIDDEN FD_SetupForm * sFp;
HIDDEN void setMemCB(FL_OBJECT *ob, SWord dummy);
HIDDEN void setSpeedCB(FL_OBJECT *ob, SWord dummy);
HIDDEN void romFileCB(FL_OBJECT *dummyob, SWord which);
HIDDEN void bootChoiceCB(FL_OBJECT *ob, SWord dummy);

// MemBrowser static structure and callback functions
HIDDEN FD_MemBrowser * mBp;		
HIDDEN void rangeAddCB(FL_OBJECT *dummyob, SWord which);
HIDDEN void rangeDelCB(FL_OBJECT *dummyob, SWord which);
HIDDEN void traceToCB(FL_OBJECT *dummyob, SWord dummy);
HIDDEN int memBFormClose(FL_FORM * form, void * dummy);


// DevStatus forms static structures and callback functions
// terminals have one window each: so dSp don't cover all the interrupts


// TLBDisplay form static structure and callback functions
HIDDEN FD_TLBDisplay * tlbDp;		
HIDDEN void tlbModCB(FL_OBJECT *ob, SWord dummy);

// SymbForm static structure and callbacks
HIDDEN FD_SymbForm * symbFp;		
HIDDEN void symbolSelectCB(FL_OBJECT * ob, SWord dummy);

// Terminal forms static structures and callback functions
// terminals have one window each


// VRangeForm form static structure and callback functions
HIDDEN FD_VRangeForm * vRFp;
HIDDEN void symSelCB(FL_OBJECT * ob, SWord dummy);
HIDDEN void vAsidCB(FL_OBJECT * ob, SWord dummy);
HIDDEN void vStartCB(FL_OBJECT * ob, SWord dummy);
HIDDEN void vEndCB(FL_OBJECT * ob, SWord dummy);
HIDDEN void vCancCB(FL_OBJECT * dummyob, SWord dummy);
HIDDEN void vOkCB(FL_OBJECT * dummyob, SWord dummy);
HIDDEN void vClearCB(FL_OBJECT * dummyob, SWord dummy);
HIDDEN void vRefresh(void);

// PRangeForm form static structure and callback functions 
HIDDEN FD_PRangeForm * pRFp;
HIDDEN void pStartCB(FL_OBJECT * ob, SWord dummy);
HIDDEN void pEndCB(FL_OBJECT * ob, SWord dummy);
HIDDEN void pCancCB(FL_OBJECT * dummyob, SWord dummy);
HIDDEN void pOkCB(FL_OBJECT * dummyob, SWord dummy);
HIDDEN void pClearCB(FL_OBJECT * dummyob, SWord dummy);
HIDDEN void pRefresh(void);


//
// general utility functions
// 

// This function closes any open window which calls it
HIDDEN int formClose(FL_FORM * dummyform, void * dummy);

// This function is a callback for window gadgets which should do nothing
HIDDEN void dummyCB(FL_OBJECT *dummyob, SWord dummy);

// This function computes the top line for a browser given a line which must
// be absolutely shown
HIDDEN unsigned int computeTopLine(unsigned int lsel, unsigned int maxl, unsigned int screenl);

// This function returns a string describing the Processor register name and
// contents
HIDDEN const char * regLineStr(unsigned int num);

// This function returns a string describing memory contents, given a
// starting physical address and a number of words to be read and displayed
HIDDEN const char * memDumpStr(Word paStart, unsigned int wNum);

// This function sets the title of a range form given the range type which
// should be inserted
HIDDEN void rangeTitle(unsigned int rType, FL_OBJECT * title);

// This function closes a range form window
HIDDEN int rFormClose(FL_FORM * rform, void * dummy);


/****************************************************************************/
/* Definitions to be exported.                                              */
/****************************************************************************/

//
// These functions initialize some pointers used by callbacks to
// access other objects in the simulator.
//

void InitAppForms(XInterface * xi)
{
	xIntf = xi;
}


void ResetAppForms(SetupInfo * stp, Watch * wch)
{
	setup = stp;
	watch = wch;
}


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


MainForm::MainForm(unsigned int dummy)
{
	mFp = create_form_MainForm();
}


MainForm::~MainForm(void)
{
	if (fl_form_is_visible(mFp->mainForm))
		fl_hide_form(mFp->mainForm);
	fl_free_form(mFp->mainForm);
	delete mFp;
}


void MainForm::RefreshV(Boolean fullRef)
{
	Boolean onLD, onBD, onVM;
	
	unsigned int i;
	
	if (fl_form_is_visible(mFp->mainForm))
	{
		fl_freeze_form(mFp->mainForm);
		fl_set_object_label(mFp->prevSLine, watch->getPrevCPUStatusStr());
		fl_set_object_label(mFp->currSLine, watch->getCPUStatusStr(&onLD, &onBD, &onVM));
		fl_set_object_label(mFp->currStatus, watch->getStatusStr());
		fl_set_button(mFp->buttBD, onBD);
		fl_set_button(mFp->buttLD, onLD);
		fl_set_button(mFp->buttVM, onVM);
		if (fullRef)
		{
			// buttons repositioning
			fl_set_button(mFp->stopButt, TRUE);
			fl_set_button(mFp->excButt, watch->getExcStop());
			fl_set_button(mFp->suspButt, watch->getSuspStop());
			fl_set_button(mFp->utlbk0Butt, watch->getUTLBStopK0());
			fl_set_button(mFp->utlbk1Butt, watch->getUTLBStopK1());
			fl_set_button(mFp->brkptButt, watch->getBrkptStop());
			fl_set_slider_value(mFp->updSlider, (double) watch->getUpdSpeed());
			fl_set_object_label(mFp->updSlider, watch->getUpdSpeedStr());
			// browser clearing
			fl_clear_browser(mFp->cpuRegBrowser);
			fl_clear_browser(mFp->miscRegBrowser);
		}

		// registers update
		
		for (i = 0; i < CPUREGNUM; i++)
			if (watch->UpdateReg(i) || fullRef)
			{
				if (fullRef)
					fl_add_browser_line(mFp->cpuRegBrowser, regLineStr(i));
				else
					fl_replace_browser_line(mFp->cpuRegBrowser, i + 1, regLineStr(i));
			}
		for (i = CPUREGNUM; i < WATCHREGNUM; i++)
			if (watch->UpdateReg(i) || fullRef)
			{
				if (fullRef)
					fl_add_browser_line(mFp->miscRegBrowser, regLineStr(i));
				else
					fl_replace_browser_line(mFp->miscRegBrowser, ((i + 1) - CPUREGNUM), regLineStr(i));
			}

		fl_unfreeze_form(mFp->mainForm);
	}
}


void MainForm::FreezeV(void)
{
	if (fl_form_is_visible(mFp->mainForm))
		fl_deactivate_form(mFp->mainForm);
}


void MainForm::UnFreezeV(void)
{
	if (fl_form_is_visible(mFp->mainForm))
		fl_activate_form(mFp->mainForm);
}


void MainForm::Show(void)
{
	if (!fl_form_is_visible(mFp->mainForm))
	{
		fl_show_form(mFp->mainForm, FL_PLACE_ASPECT, FL_FULLBORDER, "uMPS - Main Window");
		RefreshV(TRUE);		
	}
	else
	{
		// let's try to make it more visible
		fl_raise_form(mFp->mainForm);
		fl_set_form_position(mFp->mainForm, -((mFp->mainForm)->w),  -((mFp->mainForm)->h));
	}

}


void MainForm::RunSetup(void)
{
	fl_deactivate_object(mFp->sStepButt);
	fl_deactivate_object(mFp->runButt);
	fl_deactivate_object(mFp->stepButt);
	fl_deactivate_object(mFp->stepSlide);
	fl_deactivate_object(mFp->setupMenu);
	fl_deactivate_object(mFp->resetButt);
	fl_set_browser_dblclick_callback(mFp->cpuRegBrowser,  dummyCB, 0);
	fl_set_browser_dblclick_callback(mFp->miscRegBrowser, dummyCB, 0);
}


void MainForm::StopSetup(void)
{
	fl_activate_object(mFp->sStepButt);
	fl_activate_object(mFp->runButt);
	fl_activate_object(mFp->stepButt);
	fl_activate_object(mFp->stepSlide);
	fl_activate_object(mFp->setupMenu);
	fl_activate_object(mFp->resetButt);	
	fl_set_browser_dblclick_callback(mFp->miscRegBrowser, regHandleCB, 0);
	fl_set_browser_dblclick_callback(mFp->cpuRegBrowser, regHandleCB, 1);
	fl_set_object_label(mFp->currStatus, watch->getStatusStr());

	// this in case stop cause is not button press  
	fl_set_button(mFp->stopButt, TRUE);
}

// This method builds the MainForm static structure
// It is automatically generated by XFORMS fdesign tool
FD_MainForm * MainForm::create_form_MainForm(void)
{
  FL_OBJECT *obj;
  FD_MainForm *fdui = new FD_MainForm();

  fdui->mainForm = fl_bgn_form(FL_NO_BOX, 400, 550);
  obj = fl_add_box(FL_UP_BOX,0,0,400,550,EMPTYSTR);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME,10,200,210,75,"Stop on:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 270, 200, 110, 75, "Sim Speed:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
  obj = fl_add_frame(FL_UP_FRAME,10,10,380,20,EMPTYSTR);
  fdui->setupMenu = obj = fl_add_menu(FL_PULLDOWN_MENU,10,10,70,20,"Setup");
    fl_set_object_shortcut(obj,"#S",1);
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_callback(obj,setupHandleCB,0);
    fl_set_menu(obj, "Setup Window|Reset to Setup...|Reset to Defaults...");
  fdui->winMenu = obj = fl_add_menu(FL_PULLDOWN_MENU,90,10,70,20,"Windows");
    fl_set_object_shortcut(obj,"#W",1);
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_callback(obj,winHandleCB,0);
    fl_set_menu(obj, "Symbol Table|Memory Browser|TLB Display");
  fdui->miscRegBrowser = obj = fl_add_browser(FL_SELECT_BROWSER,205,305,185,235,"CP0 & other Registers:");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
	fl_set_browser_fontsize(obj, FL_NORMAL_SIZE);
    fl_set_browser_fontstyle(obj, FL_FIXED_STYLE);
    fl_set_object_callback(obj, dummyCB, 0); 
    fl_set_browser_dblclick_callback(obj, regHandleCB, 0);
  fdui->prevSLine = obj = fl_add_text(FL_NORMAL_TEXT,10,40,380,20,EMPTYSTR);
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_FIXED_STYLE);
  fdui->currStatus = obj = fl_add_text(FL_NORMAL_TEXT,10,90,280,20,EMPTYSTR);
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_FIXED_STYLE);
  fdui->currSLine = obj = fl_add_text(FL_NORMAL_TEXT,10,65,380,20,EMPTYSTR);
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_FIXED_STYLE);
  fdui->buttBD = obj = fl_add_round3dbutton(FL_PUSH_BUTTON,295,85,30,30,"BD");
    fl_set_object_color(obj,FL_COL1,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_deactivate_object(obj);
  fdui->buttLD = obj = fl_add_round3dbutton(FL_PUSH_BUTTON,345,85,30,30,"LD");
    fl_set_object_color(obj,FL_COL1,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_deactivate_object(obj);
  fdui->buttVM = obj = fl_add_round3dbutton(FL_PUSH_BUTTON,230,240,30,30,"VM");
    fl_set_object_color(obj,FL_COL1,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_deactivate_object(obj);    
  fdui->sStepButt = obj = fl_add_button(FL_TOUCH_BUTTON,10,120,45,45,"@8=");
    fl_set_object_callback(obj,sStepCB,0);
  fdui->stopButt = obj = fl_add_button(FL_RADIO_BUTTON,330,115,55,55,"@9+");
    fl_set_button_shortcut(obj,"^C",1);
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_callback(obj, stopCB,0);
  fdui->runButt = obj = fl_add_button(FL_RADIO_BUTTON,275,120,45,45,"@>");
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_callback(obj,runCB,0);
  fdui->stepButt = obj = fl_add_button(FL_RADIO_BUTTON,225,120,45,45,"@>|");
    fl_set_object_lcolor(obj,FL_RIGHT_BCOL);
    fl_set_object_callback(obj,stepCB,0);
  fdui->stepSlide = obj = fl_add_counter(FL_NORMAL_COUNTER,60,120,160,45,"Stepping");
    fl_set_object_color(obj,FL_COL1,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,dummyCB,0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, MINSTEP, MAXSTEP);
    fl_set_counter_value(obj, STARTSTEP);
    fl_set_counter_step(obj, MINSTEP, STARTSTEP);
  obj = fl_add_text(FL_NORMAL_TEXT,5,165,55,20,"SStep");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,220,165,55,20,"Step");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,270,165,55,20,"Run");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,330,170,55,20,"Stop");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
  fdui->excButt = obj = fl_add_round3dbutton(FL_PUSH_BUTTON,110,210,30,30,"Exceptions");
    fl_set_object_color(obj,FL_COL1,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,excCB,0);
  fdui->suspButt = obj = fl_add_round3dbutton(FL_PUSH_BUTTON,15,240,30,30,"Suspects");
    fl_set_object_color(obj,FL_COL1,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,suspCB,0);
  fdui->utlbk0Butt = obj = fl_add_round3dbutton(FL_PUSH_BUTTON,110,240,30,30,"Krnl TLB Usr");
	fl_set_object_color(obj,FL_COL1,FL_RIGHT_BCOL);
	fl_set_object_lsize(obj,FL_SMALL_SIZE);
	fl_set_object_lstyle(obj,FL_NORMAL_STYLE);
	fl_set_object_callback(obj,utlbk0CB,0);
  fdui->utlbk1Butt = obj = fl_add_round3dbutton(FL_PUSH_BUTTON,195,240,30,30,"");
	fl_set_object_color(obj,FL_COL1,FL_RIGHT_BCOL);
	fl_set_object_lsize(obj,FL_NORMAL_SIZE);
	fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
	fl_set_object_callback(obj,utlbk1CB,0);	
  fdui->brkptButt = obj = fl_add_round3dbutton(FL_PUSH_BUTTON,15,210,30,30,"Breakpoints");
    fl_set_object_color(obj,FL_COL1,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,brkptCB,0);
  fdui->updSlider = obj = fl_add_slider(FL_HOR_SLIDER, 280, 220, 90, 30, EMPTYSTR);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj, updSpeedCB,0);
    fl_set_slider_bounds(obj, SLOWEST, FASTEST);
    fl_set_slider_step(obj, 1);
	fl_set_slider_value(obj, SLOWEST);
  fdui->cpuRegBrowser = obj = fl_add_browser(FL_SELECT_BROWSER,10,305,185,235,"CPU Registers:");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_browser_fontsize(obj, FL_NORMAL_SIZE);
    fl_set_browser_fontstyle(obj, FL_FIXED_STYLE); 
    fl_set_object_callback(obj, dummyCB, 0);
    fl_set_browser_dblclick_callback(obj,regHandleCB,1);
  fdui->exitButt = obj = fl_add_button(FL_NORMAL_BUTTON,340,10,50,20,"Exit");
    fl_set_object_shortcut(obj,"#E",1);
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_callback(obj, exitHandleCB,0);
  fdui->resetButt = obj = fl_add_button(FL_NORMAL_BUTTON,290,10,50,20,"Reset");
    fl_set_object_shortcut(obj,"#R",1);
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
    fl_set_object_callback(obj, resetHandleCB,0);    
  fl_end_form();

  fl_set_form_atclose(fdui->mainForm, exitCB, NULL);
  fl_adjust_form_size(fdui->mainForm);
  fdui->mainForm->fdui = fdui;

  return (fdui);
}


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


SetupForm::SetupForm(unsigned int dummy)
{
	sFp = create_form_SetupForm();
}


SetupForm::~SetupForm(void)
{
	if (fl_form_is_visible(sFp->setupForm))
		fl_hide_form(sFp->setupForm);
	fl_free_form(sFp->setupForm);
	delete sFp;
}


void SetupForm::RefreshV(Boolean fullRef)
{
	unsigned int i, j;
	
	if (fl_form_is_visible(sFp->setupForm))
	{
		if (fullRef)
		{
			fl_freeze_form(sFp->setupForm);
			fl_set_choice(sFp->bootChoice, setup->getBootType() + 1);
			fl_set_counter_value(sFp->memSlide, (double) (setup->getRamSize() * FRAMEKB));
			fl_set_counter_value(sFp->speedSlide, (double) setup->getSpeed());
			fl_set_object_label(sFp->kernFileName, setup->getROMFileName(COREINDEX));
			fl_set_object_label(sFp->stabFileName, setup->getROMFileName(STABINDEX));
					
			fl_unfreeze_form(sFp->setupForm);
		}
		// else device status during run (fullRef == FALSE) cannot be altered
	}
}


void SetupForm::FreezeV(void)
{
	if (fl_form_is_visible(sFp->setupForm))
		fl_deactivate_form(sFp->setupForm);
}


void SetupForm::UnFreezeV(void)
{
	if (fl_form_is_visible(sFp->setupForm))
		fl_activate_form(sFp->setupForm);
}


void SetupForm::Show(void)
{
	if (!fl_form_is_visible(sFp->setupForm))
	{
		fl_show_form(sFp->setupForm, FL_PLACE_ASPECT, FL_FULLBORDER, "uMPS - Setup Screen");
		RefreshV(TRUE);
	}
	else
	{
		// let's try to make it more visible
		fl_raise_form(sFp->setupForm);
		fl_set_form_position(sFp->setupForm, -((sFp->setupForm)->w),  -((sFp->setupForm)->h));
	}
	
}


void SetupForm::RunSetup(void)
{
	fl_deactivate_object(sFp->kernButt);
	fl_deactivate_object(sFp->stabButt);
	fl_deactivate_object(sFp->memSlide);
	fl_deactivate_object(sFp->speedSlide);
	fl_deactivate_object(sFp->bootChoice);
}


void SetupForm::StopSetup(void)
{
	fl_activate_object(sFp->memSlide);
	fl_activate_object(sFp->speedSlide);
	fl_activate_object(sFp->kernButt);
	fl_activate_object(sFp->stabButt);
	fl_activate_object(sFp->bootChoice);
}


// This method builds the SetupForm static structure
// It is automatically generated by XFORMS fdesign tool
FD_SetupForm * SetupForm::create_form_SetupForm()
{
  FL_OBJECT *obj;
  FD_SetupForm *fdui = new FD_SetupForm();
  
  fdui->setupForm = fl_bgn_form(FL_NO_BOX, 400, 550);
  obj = fl_add_box(FL_UP_BOX,0,0,400,550,EMPTYSTR);
  fdui->bootChoice = obj = fl_add_choice(FL_NORMAL_CHOICE,325,332,60,25,"Load CORE file:");
    fl_set_object_boxtype(obj, FL_UP_BOX);
    fl_clear_choice(obj);
    fl_addto_choice(obj, "No|Yes");
    fl_set_choice_fontsize(obj, FL_NORMAL_SIZE);
    fl_set_choice_fontstyle(obj, FL_BOLDITALIC_STYLE);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);  
    fl_set_object_callback(obj, bootChoiceCB, 0);
  fdui->kernFileName = obj = fl_add_text(FL_NORMAL_TEXT,95,360,290,25,EMPTYSTR);
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_FIXED_STYLE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
  fdui->kernButt = obj = fl_add_button(FL_HIDDEN_BUTTON,95,360,290,25,EMPTYSTR);
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
    fl_set_object_callback(obj,romFileCB, COREINDEX);
  fdui->stabFileName = obj = fl_add_text(FL_NORMAL_TEXT,95,465,290,25,EMPTYSTR);
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_color(obj,FL_TOP_BCOL,FL_MCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_FIXED_STYLE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
  fdui->stabButt = obj = fl_add_button(FL_HIDDEN_BUTTON,95,465,290,25,EMPTYSTR);
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
    fl_set_object_callback(obj,romFileCB,STABINDEX);   
  fdui->memSlide = obj = fl_add_counter(FL_NORMAL_COUNTER,30,498,160,27,"Memory Available (KB)");
    fl_set_object_color(obj,FL_COL1,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,setMemCB,0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, MINRAM * FRAMEKB, MAXRAM * FRAMEKB);
    fl_set_counter_value(obj, 0);
    fl_set_counter_step(obj, STEPRAM * FRAMEKB, 4 * STEPRAM * FRAMEKB);
  fdui->speedSlide = obj = fl_add_counter(FL_NORMAL_COUNTER,210,498,160,27,"Processor Speed (MHz)");
    fl_set_object_color(obj,FL_COL1,FL_RIGHT_BCOL);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,setSpeedCB,0);
    fl_set_counter_precision(obj, 0);
    fl_set_counter_bounds(obj, MINSPEED, MAXSPEED);
    fl_set_counter_value(obj, MINSPEED);
    fl_set_counter_step(obj, MINSPEED, 5 * MINSPEED);
    obj = fl_add_text(FL_NORMAL_TEXT,15,360,80,25,"CORE file:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    obj = fl_add_text(FL_NORMAL_TEXT,15,395,80,25,"BOOT ROM:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    obj = fl_add_text(FL_NORMAL_TEXT,15,430,80,25,"EXEC ROM:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    obj = fl_add_text(FL_NORMAL_TEXT,15,465,80,25,"Sym Table:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
  fl_end_form();

  fl_set_form_atclose(fdui->setupForm, formClose, NULL);
  fl_adjust_form_size(fdui->setupForm);
  fdui->setupForm->fdui = fdui;

  return fdui;
}


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


MemBrowser::MemBrowser(unsigned int formNum)
{
	mBp = create_form_MemBrowser(formNum);
}


MemBrowser::~MemBrowser(void)
{
	if (fl_form_is_visible(mBp->memBrowser))
		fl_hide_form(mBp->memBrowser);
	fl_free_form(mBp->memBrowser);
	delete mBp;
}


void MemBrowser::RefreshV(Boolean fullRef)
{
	unsigned int i, brwNum, tracelines;
	int linesel;
	Word dummyasid, dummyacc, paStart, paEnd;
	
	if (fl_form_is_visible(mBp->memBrowser))
	{
		fl_freeze_form(mBp->memBrowser);
		if (fullRef)
		{
			// request is done on full refresh, to avoid that reset
			// deactives this facility; full refresh is done at least on
			// window opening and on reset
			watch->setDirtyMemReq(TRUE);
		
			// browsers' contents is refreshed
			for (brwNum = BRKPT; brwNum < RTABNUM; brwNum++)
			{
				fl_clear_browser(mBp->rBrowser[brwNum]);
				for (i = 1, tracelines = watch->getRangeNum(brwNum); i <= tracelines; i++)
					fl_add_browser_line(mBp->rBrowser[brwNum], watch->getRangeStr(brwNum, i));
			}
		}			  
		
		if ((linesel = watch->getBrkptLine()) > 0)
		{
			// center and highlight breakpoint line
			fl_set_browser_topline(mBp->rBrowser[BRKPT], computeTopLine(linesel,\
			fl_get_browser_maxline(mBp->rBrowser[BRKPT]), fl_get_browser_screenlines(mBp->rBrowser[BRKPT])));
			if (linesel <= fl_get_browser_maxline(mBp->rBrowser[BRKPT]))
				fl_select_browser_line(mBp->rBrowser[BRKPT], linesel);
		}
		else
			// deselects line, if any
			fl_deselect_browser(mBp->rBrowser[BRKPT]);
			
		if ((linesel = watch->getSuspLine()) > 0)
		{
			// center and highlight suspect line
			fl_set_browser_topline(mBp->rBrowser[SUSP], computeTopLine(linesel,\
			fl_get_browser_maxline(mBp->rBrowser[SUSP]), fl_get_browser_screenlines(mBp->rBrowser[SUSP])));
			if (linesel <= fl_get_browser_maxline(mBp->rBrowser[SUSP]))
				fl_select_browser_line(mBp->rBrowser[SUSP], linesel);
		}
		else
			// deselects line, if any
			fl_deselect_browser(mBp->rBrowser[SUSP]);
			
		if (watch->testResetDirtyMem() || fullRef)
		{
			// memory dump area must be refreshed
			linesel = fl_get_browser_topline(mBp->memDumpBrowser);
			fl_clear_browser(mBp->memDumpBrowser);
			tracelines = watch->getRangeNum(TRACE);
			for (i = 1; i <= tracelines; i++)
			{
				watch->getRange(TRACE, i, &dummyasid, &paStart, &paEnd, &dummyacc);
	
				for(; paStart <= paEnd; paStart += (DUMPWORDS * WORDLEN))
					fl_add_browser_line(mBp->memDumpBrowser, memDumpStr(paStart, DUMPWORDS));
	
				fl_add_browser_line(mBp->memDumpBrowser, "\n");
			}
			if (linesel > 0 && linesel <= fl_get_browser_maxline(mBp->memDumpBrowser))
				fl_set_browser_topline(mBp->memDumpBrowser, linesel);
		}
		fl_unfreeze_form(mBp->memBrowser);
	}
}


void MemBrowser::FreezeV(void)
{
	if (fl_form_is_visible(mBp->memBrowser))
		fl_deactivate_form(mBp->memBrowser);
}


void MemBrowser::UnFreezeV(void)
{
	if (fl_form_is_visible(mBp->memBrowser))
		fl_activate_form(mBp->memBrowser);
}


void MemBrowser::Show(void)
{
	if (!fl_form_is_visible(mBp->memBrowser))
	{
		fl_show_form(mBp->memBrowser, FL_PLACE_ASPECT, FL_FULLBORDER, "uMPS - Memory Browser");
		RefreshV(TRUE);
	}
	else
	{
		// let's try to make it more visible
		fl_raise_form(mBp->memBrowser);
		fl_set_form_position(mBp->memBrowser, -((mBp->memBrowser)->w),  -((mBp->memBrowser)->h));
	}
}


void MemBrowser::RunSetup(void)
{
	fl_deactivate_object(mBp->brkptAddButt);
	fl_deactivate_object(mBp->brkptDelButt);	
	fl_deactivate_object(mBp->suspAddButt);
	fl_deactivate_object(mBp->suspDelButt);
	fl_deactivate_object(mBp->traceAddButt);
	fl_deactivate_object(mBp->traceDelButt);
	fl_set_browser_dblclick_callback(mBp->rBrowser[BRKPT], dummyCB, 0);
	fl_set_browser_dblclick_callback(mBp->rBrowser[SUSP], dummyCB, 0);
	fl_set_browser_dblclick_callback(mBp->rBrowser[TRACE], dummyCB, 0);
	fl_deactivate_object(mBp->traceToButt);
	fl_deactivate_object(mBp->memModButt);
}


void MemBrowser::StopSetup(void)
{
	fl_activate_object(mBp->brkptAddButt);
	fl_activate_object(mBp->brkptDelButt);	
	fl_activate_object(mBp->suspAddButt);
	fl_activate_object(mBp->suspDelButt);
	fl_activate_object(mBp->traceAddButt);
	fl_activate_object(mBp->traceDelButt);
	fl_set_browser_dblclick_callback(mBp->rBrowser[BRKPT], rangeDelCB, BRKPT);
	fl_set_browser_dblclick_callback(mBp->rBrowser[SUSP], rangeDelCB, SUSP);
	fl_set_browser_dblclick_callback(mBp->rBrowser[TRACE], rangeDelCB, TRACE);
	fl_activate_object(mBp->traceToButt);
	fl_activate_object(mBp->memModButt);
}


// This method builds the MemBrowser static structure
// It is automatically generated by XFORMS fdesign tool
FD_MemBrowser * MemBrowser::create_form_MemBrowser(unsigned int fNum)
{
  FL_OBJECT *obj;
  FD_MemBrowser *fdui = new FD_MemBrowser();
  
  fdui->formNum = fNum;
  
  fdui->memBrowser = fl_bgn_form(FL_NO_BOX, 400, 550);
  obj = fl_add_box(FL_UP_BOX,0,0,400,550,EMPTYSTR);
  fdui->brkptAddButt = obj = fl_add_button(FL_NORMAL_BUTTON,280,45,110,30,"Add Breakpoint");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,rangeAddCB,BRKPT);
  fdui->brkptDelButt = obj = fl_add_button(FL_NORMAL_BUTTON,280,80,110,30,"Delete Breakpoint");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,rangeDelCB,BRKPT);
  fdui->suspAddButt = obj = fl_add_button(FL_NORMAL_BUTTON,280,150,110,30,"Add Suspect");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,rangeAddCB,SUSP);
  fdui->suspDelButt = obj = fl_add_button(FL_NORMAL_BUTTON,280,185,110,30,"Delete Suspect");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,rangeDelCB,SUSP);
  fdui->traceAddButt = obj = fl_add_button(FL_NORMAL_BUTTON,280,280,110,30,"Add Trace");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,rangeAddCB,TRACE);
  fdui->traceDelButt = obj = fl_add_button(FL_NORMAL_BUTTON,280,315,110,30,"Delete Trace");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,rangeDelCB,TRACE);
  fdui->rBrowser[BRKPT] = obj = fl_add_browser(FL_HOLD_BROWSER,10,30,260,95,"Breakpoint ranges:");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_browser_fontsize(obj, FL_NORMAL_SIZE);
    fl_set_browser_fontstyle(obj, FL_FIXED_STYLE); 
    fl_set_object_callback(obj,dummyCB,0);
    fl_set_browser_dblclick_callback(obj,rangeDelCB, BRKPT);
  fdui->memDumpBrowser = obj = fl_add_browser(FL_NORMAL_BROWSER,10,395,380,150,EMPTYSTR);
    fl_set_object_color(obj,FL_TOP_BCOL,FL_COL1);
    fl_set_browser_fontsize(obj, FL_NORMAL_SIZE);
    fl_set_browser_fontstyle(obj, FL_FIXED_STYLE); 
    fl_set_object_callback(obj,dummyCB,0);
  fdui->rBrowser[SUSP] = obj = fl_add_browser(FL_HOLD_BROWSER,10,155,260,95,"Suspect ranges:");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_browser_fontsize(obj, FL_NORMAL_SIZE);
    fl_set_browser_fontstyle(obj, FL_FIXED_STYLE); 
    fl_set_browser_dblclick_callback(obj, rangeDelCB, SUSP); 
    fl_set_object_callback(obj,dummyCB,0);
  fdui->rBrowser[TRACE] = obj = fl_add_browser(FL_HOLD_BROWSER,10,280,260,95,"Traced ranges:");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_browser_fontsize(obj, FL_NORMAL_SIZE);
    fl_set_browser_fontstyle(obj, FL_FIXED_STYLE); 
    fl_set_browser_dblclick_callback(obj, rangeDelCB, TRACE);
    fl_set_object_callback(obj,dummyCB,0);
  fdui->traceToButt = obj = fl_add_button(FL_NORMAL_BUTTON,280,220,110,30,"Trace Suspect");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,traceToCB,0);
  fdui->memModButt = obj = fl_add_button(FL_NORMAL_BUTTON,280,360,110,30,"Modify Memory");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,rangeAddCB,MEMMOD);
  fl_end_form();

  fl_set_form_atclose(fdui->memBrowser, memBFormClose, NULL);
  fl_adjust_form_size(fdui->memBrowser);
  fdui->memBrowser->fdui = fdui;

  return fdui;
}


/****************************************************************************/


// DevStatus windows report device status. There is one window for each
// interrupt line, terminals excluded. It displays current device status and
// allow user making device fail on hardware operations.
//
// See AppForm class for method descriptions.




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


TLBDisplay::TLBDisplay(unsigned int formNum)
{
	tlbDp = create_form_TLBDisplay(formNum);
}


TLBDisplay::~TLBDisplay(void)
{
	if (fl_form_is_visible(tlbDp->tlbDisplay))
		fl_hide_form(tlbDp->tlbDisplay);
	fl_free_form(tlbDp->tlbDisplay);
	delete tlbDp;
}


void TLBDisplay::RefreshV(Boolean fullRef)
{
	unsigned int i;
	
	if (fl_form_is_visible(tlbDp->tlbDisplay))
	{
		fl_freeze_form(tlbDp->tlbDisplay);
		
		if (fullRef)
			fl_clear_browser(tlbDp->tlbBrowser);
			
		for (i = 0; i < setup->getTLBSize(); i++)
			if (watch->UpdateTLB(i) || fullRef)
			{
				sprintf(strbuf, "TLB[%.2u] : 0x%.8lX : 0x%.8lX", i, watch->getTLBHI(i), watch->getTLBLO(i));
				if (fullRef)
					fl_add_browser_line(tlbDp->tlbBrowser, strbuf);
				else
					fl_replace_browser_line(tlbDp->tlbBrowser, i + 1, strbuf);
			}
		fl_unfreeze_form(tlbDp->tlbDisplay);
	}
}


void TLBDisplay::FreezeV(void)
{
	if (fl_form_is_visible(tlbDp->tlbDisplay))
		fl_deactivate_form(tlbDp->tlbDisplay);
}


void TLBDisplay::UnFreezeV(void)
{
	if (fl_form_is_visible(tlbDp->tlbDisplay))
		fl_activate_form(tlbDp->tlbDisplay);
}


void TLBDisplay::Show(void)
{
	if (!fl_form_is_visible(tlbDp->tlbDisplay))
	{
		fl_show_form(tlbDp->tlbDisplay, FL_PLACE_ASPECT, FL_FULLBORDER, "uMPS - TLB Display");
		RefreshV(TRUE);
	}
	else
	{
		// let's try to make it more visible
		fl_raise_form(tlbDp->tlbDisplay);
		fl_set_form_position(tlbDp->tlbDisplay, -((tlbDp->tlbDisplay)->w),  -((tlbDp->tlbDisplay)->h));
	}
	
}


void TLBDisplay::RunSetup(void)
{
	fl_set_object_callback(tlbDp->tlbBrowser, dummyCB, 0);
}


void TLBDisplay::StopSetup(void)
{
	fl_set_object_callback(tlbDp->tlbBrowser, tlbModCB, 0);
}


// This method builds the TLBDisplay static structure
// It is automatically generated by XFORMS fdesign tool
FD_TLBDisplay * TLBDisplay::create_form_TLBDisplay(unsigned int fNum)
{
  FL_OBJECT *obj;
  FD_TLBDisplay *fdui = new FD_TLBDisplay();
  
  fdui->formNum = fNum;
  
  fdui->tlbDisplay = fl_bgn_form(FL_NO_BOX, 360, 550);
  obj = fl_add_box(FL_UP_BOX,0,0,360,550,EMPTYSTR);
  fdui->tlbBrowser = obj = fl_add_browser(FL_SELECT_BROWSER,20,30,320,490,"TLB Status:");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_browser_fontsize(obj, FL_NORMAL_SIZE);
    fl_set_browser_fontstyle(obj, FL_FIXED_STYLE); 
    fl_set_object_callback(obj,tlbModCB, 0);
  fl_end_form();

  fl_set_form_atclose(fdui->tlbDisplay, formClose, NULL);
  fl_adjust_form_size(fdui->tlbDisplay);
  fdui->tlbDisplay->fdui = fdui;

  return fdui;
}


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


SymbForm::SymbForm(unsigned int formNum)
{
	symbFp = create_form_SymbForm(formNum);
}


SymbForm::~SymbForm(void)
{
	if (fl_form_is_visible(symbFp->symbForm))
		fl_hide_form(symbFp->symbForm);
	fl_free_form(symbFp->symbForm);
	delete symbFp;
}


void SymbForm::RefreshV(Boolean fullRef)
{
	unsigned int i, tracelines, asid;
	
	if (fl_form_is_visible(symbFp->symbForm))
	{
		fl_freeze_form(symbFp->symbForm);
		
		if (fullRef)
		{
			asid = setup->getSymbolTableASID();
			if (asid == MAXASID)
				sprintf(strbuf, "ASID: PHYS");
			else
				sprintf(strbuf, "ASID: 0x%X",asid);
				
			fl_set_object_label(symbFp->symbBrowser, strbuf);
			fl_clear_browser(symbFp->symbBrowser);
			for (i = 1, tracelines = watch->getSymbolNum(); i <= tracelines; i++)
				fl_add_browser_line(symbFp->symbBrowser, watch->getSymbolStr(i));	
		}
		fl_unfreeze_form(symbFp->symbForm);
	}
}


void SymbForm::FreezeV(void)
{
	if (fl_form_is_visible(symbFp->symbForm))
		fl_deactivate_form(symbFp->symbForm);
}


void SymbForm::UnFreezeV(void)
{
	if (fl_form_is_visible(symbFp->symbForm))
		fl_activate_form(symbFp->symbForm);
}


void SymbForm::Show(void)
{
	if (!fl_form_is_visible(symbFp->symbForm))
	{
		fl_show_form(symbFp->symbForm, FL_PLACE_ASPECT, FL_FULLBORDER, "uMPS - Symbol Table Contents");
		RefreshV(TRUE);
	}
	else
	{
		// let's try to make it more visible
		fl_raise_form(symbFp->symbForm);
		fl_set_form_position(symbFp->symbForm, -((symbFp->symbForm)->w),  -((symbFp->symbForm)->h));
	}
	
}

void SymbForm::RunSetup(void)
{
	fl_set_browser_dblclick_callback(symbFp->symbBrowser, dummyCB, 0);
}

void SymbForm::StopSetup(void)
{
	fl_set_browser_dblclick_callback(symbFp->symbBrowser,symbolSelectCB,0);
}


// This method builds the SymbForm static structure
// It is automatically generated by XFORMS fdesign tool
FD_SymbForm * SymbForm::create_form_SymbForm(unsigned int fNum)
{
  FL_OBJECT *obj;
  FD_SymbForm *fdui = new FD_SymbForm();
  
  fdui->formNum = fNum;
  
  fdui->symbForm = fl_bgn_form(FL_NO_BOX, 400, 550);
  obj = fl_add_box(FL_UP_BOX,0,0,400,550,EMPTYSTR);
  fdui->symbBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,20,30,360,490,EMPTYSTR);
    fl_set_object_color(obj,FL_TOP_BCOL,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_browser_fontsize(obj, FL_NORMAL_SIZE);
    fl_set_browser_fontstyle(obj, FL_FIXED_STYLE); 
    fl_set_object_callback(obj,dummyCB, 0);
	fl_set_browser_dblclick_callback(obj,symbolSelectCB,0);
  fl_end_form();

  fl_set_form_atclose(fdui->symbForm, formClose, NULL);
  fl_adjust_form_size(fdui->symbForm);
  fdui->symbForm->fdui = fdui;

  return fdui;
}


/****************************************************************************/




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


VRangeForm::VRangeForm(void)
{
	vRFp = create_form_VRangeForm();

	// initializes internal structures describing the range
	vRFp->asid = 0UL;
	vRFp->startAddr = 0UL;
	vRFp->endAddr = 0UL;
	vRFp->rBrowser = NULL;
	vRFp->rType = BRKPT;
}


VRangeForm::~VRangeForm(void)
{
	if (fl_form_is_visible(vRFp->vRangeForm))
		fl_hide_form(vRFp->vRangeForm);
	fl_free_form(vRFp->vRangeForm);
	delete vRFp;
}


void VRangeForm::InsertRange(unsigned int rType, FL_OBJECT * rBrowser)
{
	unsigned int i, tracelines;
	
	xIntf->FreezeAllVForms();
	if (rType == BRKPT || rType == SUSP)
		vRFp->rType = rType;
	else
		Panic("Invalid range type in VRangeForm::InsertRange()");

	vRFp->rBrowser = rBrowser;
	vRFp->asid = setup->getSymbolTableASID();
	
	rangeTitle(rType, vRFp->title);
	vRefresh();
	
	// load symbol table
	fl_clear_browser(vRFp->symBrowser);
	for (i = 1, tracelines = watch->getSymbolNum(); i <= tracelines; i++)
		fl_add_browser_line(vRFp->symBrowser, watch->getSymbolStr(i));
	
	fl_show_form(vRFp->vRangeForm, FL_PLACE_ASPECT, FL_FULLBORDER, "uMPS - Symbolic/Virtual Range Insert");
}


// This method builds the VRangeForm static structure
// It is automatically generated by XFORMS fdesign tool
FD_VRangeForm * VRangeForm::create_form_VRangeForm(void)
{
  FL_OBJECT *obj;
  FD_VRangeForm *fdui = new FD_VRangeForm();

  fdui->vRangeForm = fl_bgn_form(FL_NO_BOX, 400, 540);
  obj = fl_add_box(FL_UP_BOX,0,0,400,540,"");
  fdui->title = obj = fl_add_labelframe(FL_ENGRAVED_FRAME,15,345,370,130,"Enter Breakpoint:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
  fdui->symBrowser = obj = fl_add_browser(FL_HOLD_BROWSER,15,30,370,290,"Symbol Table:");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_COL1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
	fl_set_browser_fontsize(obj, FL_NORMAL_SIZE);
    fl_set_browser_fontstyle(obj, FL_FIXED_STYLE);    
    fl_set_browser_dblclick_callback(obj,symSelCB,0);
    fl_set_object_callback(obj,dummyCB,0);
  fdui->asidInput = obj = fl_add_input(FL_NORMAL_INPUT,25,380,165,30,"ASID field:  (use 0x40 for PHYS range)");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,vAsidCB,0);
    fl_set_input_return(obj, FL_RETURN_END);
  fdui->startInput = obj = fl_add_input(FL_NORMAL_INPUT,25,435,165,30,"Starting address:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,vStartCB,0);
    fl_set_input_return(obj, FL_RETURN_END);
  fdui->endInput = obj = fl_add_input(FL_NORMAL_INPUT,210,435,165,30,"Ending address:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,vEndCB,0);
    fl_set_input_return(obj, FL_RETURN_END);
  fdui->cancelButt = obj = fl_add_button(FL_NORMAL_BUTTON,25,490,80,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,vCancCB,0);
  fdui->okButt = obj = fl_add_button(FL_NORMAL_BUTTON,160,490,80,30,"OK");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,vOkCB,0);
  fdui->clearButt = obj = fl_add_button(FL_NORMAL_BUTTON,295,490,80,30,"Clear");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,vClearCB,0);
  fl_end_form();

  fl_set_form_atclose(fdui->vRangeForm, rFormClose, NULL);	
  fl_adjust_form_size(fdui->vRangeForm);
  fdui->vRangeForm->fdui = fdui;

  return fdui;
}


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


PRangeForm::PRangeForm(void)
{
	pRFp = create_form_PRangeForm();

	// initializes internal structures describing the range to be inserted
	pRFp->startAddr = 0UL;
	pRFp->endAddr = 0UL;
	pRFp->rBrowser = NULL;
	pRFp->rType = BRKPT;
}


PRangeForm::~PRangeForm(void)
{
	if (fl_form_is_visible(pRFp->pRangeForm))
		fl_hide_form(pRFp->pRangeForm);
	fl_free_form(pRFp->pRangeForm);
	delete pRFp;
}


void PRangeForm::InsertRange(unsigned int rType, FL_OBJECT * rBrowser)
{
	xIntf->FreezeAllVForms();
	pRFp->rBrowser = rBrowser;
	pRFp->rType = rType;
	if (rType == MEMMOD)
	{
		fl_set_object_label(pRFp->startInput, "Memory address (PHYS):");
		fl_set_object_label(pRFp->endInput, "New value:");

	}
	else
	{
		fl_set_object_label(pRFp->startInput, "Start address (PHYS):");
		fl_set_object_label(pRFp->endInput, "End address (PHYS):");
	}
	
	rangeTitle(rType, pRFp->title);
	pRFp->endAddr = 0UL;
	pRefresh();
		
	if (rType == MEMMOD)
        sprintf(strbuf, "uMPS - Memory Location Change");
	else 
		sprintf(strbuf, "uMPS - Physical Range Insert");

	fl_show_form(pRFp->pRangeForm, FL_PLACE_ASPECT, FL_FULLBORDER, strbuf);
}


// This method builds the PRangeForm static structure
// It is automatically generated by XFORMS fdesign tool
FD_PRangeForm * PRangeForm::create_form_PRangeForm(void)
{
  FL_OBJECT *obj;
  FD_PRangeForm *fdui = new FD_PRangeForm();

  fdui->pRangeForm = fl_bgn_form(FL_NO_BOX, 400, 175);
  obj = fl_add_box(FL_UP_BOX,0,0,400,175,"");
  fdui->title = obj = fl_add_labelframe(FL_ENGRAVED_FRAME,15,25,370,80,EMPTYSTR);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
  fdui->startInput = obj = fl_add_input(FL_NORMAL_INPUT,25,65,165,30,EMPTYSTR);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,pStartCB,0);
    fl_set_input_return(obj, FL_RETURN_END);
  fdui->endInput = obj = fl_add_input(FL_NORMAL_INPUT,210,65,165,30,EMPTYSTR);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_ENGRAVED_STYLE);
    fl_set_object_callback(obj,pEndCB,0);
    fl_set_input_return(obj, FL_RETURN_END);
  fdui->cancelButt = obj = fl_add_button(FL_NORMAL_BUTTON,25,125,80,30,"Cancel");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,pCancCB,0);
  fdui->okButt = obj = fl_add_button(FL_NORMAL_BUTTON,160,125,80,30,"OK");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,pOkCB,0);
  fdui->clearButt = obj = fl_add_button(FL_NORMAL_BUTTON,295,125,80,30,"Clear");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,pClearCB,0);
  fl_end_form();
  
  fl_set_form_atclose(fdui->pRangeForm, rFormClose, NULL);	
  fl_adjust_form_size(fdui->pRangeForm);
  fdui->pRangeForm->fdui = fdui;

  return fdui;
}


/****************************************************************************/
/* Definitions strictly local to the module.                                */
/****************************************************************************/

//
// callbacks for MainForm
//


HIDDEN void sStepCB(FL_OBJECT *dummyob, SWord dummy)
{
	xIntf->PrepareRun();
	watch->Step(1);
	xIntf->PrepareStop();
}


HIDDEN void stopCB(FL_OBJECT *dummyob, SWord dummy)
{
	watch->SignalUserStop();
}


HIDDEN void runCB(FL_OBJECT *dummyob, SWord dummy)
{
	xIntf->PrepareRun();
	watch->Run();
	// this to make stop more "visible"
	fl_ringbell(100);
	xIntf->PrepareStop();
}


HIDDEN void stepCB(FL_OBJECT *dummyob, SWord dummy)
{
	xIntf->PrepareRun();
	watch->Step((unsigned int)fl_get_counter_value(mFp->stepSlide));
	// this to make stop more "visible"
	fl_ringbell(100);
	xIntf->PrepareStop();
}


HIDDEN void excCB(FL_OBJECT *ob, SWord dummy)
{
	watch->setExcStop(fl_get_button(ob));
}


HIDDEN void suspCB(FL_OBJECT *ob, SWord dummy)
{
	watch->setSuspStop(fl_get_button(ob));
}


HIDDEN void utlbk0CB(FL_OBJECT *ob, SWord dummy)
{
	watch->setUTLBStopK0(fl_get_button(ob));
}


HIDDEN void utlbk1CB(FL_OBJECT *ob, SWord dummy)
{
	watch->setUTLBStopK1(fl_get_button(ob));
}

        
HIDDEN void brkptCB(FL_OBJECT *ob, SWord dummy)
{
	watch->setBrkptStop(fl_get_button(ob));
}


HIDDEN void updSpeedCB(FL_OBJECT *ob, SWord dummy)
{
	watch->setUpdSpeed((unsigned int) fl_get_slider_value(ob));
	fl_set_object_label(ob, watch->getUpdSpeedStr());
}


HIDDEN void regHandleCB(FL_OBJECT *ob, SWord which)
{
	unsigned int i = fl_get_browser(ob);
	unsigned int regnum;
	char sbuf[SMALLSTRBUF];
	const char * istrp, * namep;
	Word temp;

	if (which == 1)
		// CPU reg selected
		regnum =  i - 1;
	else
		// misc reg selected
		regnum = i + CPUREGNUM - 1;
	
	// skipping blanks in register name head
	namep = watch->getRegName(regnum);
	sprintf(strbuf, "Enter a new value for register %s:", namep);
	sprintf(sbuf,"0x%.8lX",watch->getReg(regnum));
	while ((istrp =	fl_show_input(strbuf, sbuf)) != NULL && !StrToWord(istrp, &temp))
		fl_ringbell(100);
	if (istrp == NULL || temp == watch->getReg(regnum))
		// do nothing
		;
	else
		// a valid input has occurred and register value has to be changed
		if (watch->setReg(regnum, temp))
		{
			// reg is not alterable
			sprintf(strbuf, "Register %s may not be changed", namep);
			fl_ringbell(100);
			fl_show_alert(strbuf, EMPTYSTR, EMPTYSTR, TRUE);		
		}
		else
			// update the register value on screen 
			fl_replace_browser_line(ob, i, regLineStr(regnum));
}
				

HIDDEN void setupHandleCB(FL_OBJECT *dummyob, SWord dummy)
{
 	int choice = fl_get_menu(mFp->setupMenu);
 	
 	switch(choice)
 	{
 		case 1:
 			xIntf->ShowSetupForm();
 			break;
 		
 		case 2:
 			fl_ringbell(100);
 			if (xIntf->Reset(FALSE, "Resetting the system to Setup... \nwill reload all files in current configuration.\nAre you sure?") &&  \
 				(watch->getRangeNum(BRKPT) || watch->getRangeNum(SUSP) || watch->getRangeNum(TRACE)))
 			{
	 			fl_ringbell(100);
	 			fl_show_alert("Symbol table file has been reloaded:", \
					"some memory browser entries could now point to unwanted or", "invalid areas; you have to delete them by hand", TRUE);	
			}
 			break;
 				
 		case 3:
 			fl_ringbell(100);
			if (xIntf->Reset(TRUE, "The configuration will be reset to default\nand all files will be reloaded.\nAre you sure?") && \
	 			(watch->getRangeNum(BRKPT) || watch->getRangeNum(SUSP) || watch->getRangeNum(TRACE)))
			{
				fl_ringbell(100);
				fl_show_alert("Symbol table file has been reloaded:", \
					"some memory browser entries could now point to unwanted or", "invalid areas; you have to delete them by hand", TRUE);	
			}
 			break;
 		
 		case -1:
 		default:
 			break;
 	}	
 			
}


HIDDEN void winHandleCB(FL_OBJECT *dummyob, SWord dummy)
{
	// redefine this function for different menu order...
	
	int choice = fl_get_menu(mFp->winMenu);
	
	if (choice != -1)
		xIntf->ShowForm(choice); 
}


// This function is called when to "close application" border button is pressed
HIDDEN int exitCB(FL_FORM * dummyform, void * dummy)
{
	fl_ringbell(100);
	if (setup->getExpertMode() || fl_show_question("Are you sure you want to quit?", FALSE))
	        xIntf->Quit();
	return(FL_IGNORE);
}


HIDDEN void resetHandleCB(FL_OBJECT *dummyob, SWord dummy)
{
	fl_ringbell(100);
	if (xIntf->Reset(FALSE, "Resetting the system to Setup... \nwill reload all files in current configuration.\nAre you sure?") &&  \
		(watch->getRangeNum(BRKPT) || watch->getRangeNum(SUSP) || watch->getRangeNum(TRACE)))
	{
		fl_ringbell(100);
		fl_show_alert("Symbol table file has been reloaded:", \
		"some memory browser entries could now point to unwanted or", "invalid areas; you have to delete them by hand", TRUE);	
	}
}


HIDDEN void exitHandleCB(FL_OBJECT *dummyob, SWord dummy)
{
	fl_ringbell(100);
	if (setup->getExpertMode() || fl_show_question("Are you sure you want to quit?", FALSE))
  		xIntf->Quit();
}


HIDDEN const char * regLineStr(unsigned int num)
{
	if (num < CPUGPRNUM)
		sprintf(strbuf, "$%.2u(%s) : 0x%.8lX", num, watch->getRegName(num), watch->getReg(num));	 
	else
		if (num >= CPUREGNUM)
		// other regs
			sprintf(strbuf, "%10s : 0x%.8lX", watch->getRegName(num), watch->getReg(num));	 
		else
			// HI and LO regs
			sprintf(strbuf, "   (%s) : 0x%.8lX", watch->getRegName(num), watch->getReg(num));	 
	
	return(strbuf);
}


/****************************************************************************/

//
// SetupForm form callbacks
//



HIDDEN void setMemCB(FL_OBJECT *ob, SWord dummy)
{
	Word mem = (Word) fl_get_counter_value(ob); 

	fl_ringbell(100);	
	if (setup->getExpertMode() || fl_show_question("Can't change memory size without resetting\nthe system to Setup...\n\
Do you want to reset?", FALSE))
		{
			setup->setRamSize(mem / FRAMEKB);
			xIntf->Reset(FALSE);
		}
		else
			// resets the counter to original value		
			fl_set_counter_value(ob, (double) (setup->getRamSize() * FRAMEKB));
}


HIDDEN void setSpeedCB(FL_OBJECT *ob, SWord dummy)
{
	unsigned int spd = (unsigned int) fl_get_counter_value(ob); 
	
	fl_ringbell(100);
	if (setup->getExpertMode() || fl_show_question("Can't change system speed without resetting to Setup...\n\
Do you want to reset?", FALSE))
	{
		setup->setSpeed(spd);
		xIntf->Reset(FALSE);
	}

	else
		// resets the counter to original value
		fl_set_counter_value(ob, (double)  setup->getSpeed());
}


HIDDEN void romFileCB(FL_OBJECT *dummyob, SWord which)
{
	const char * fileName;
	unsigned int index = (unsigned int) which;
	Word asid;
	const char * inputp;	
	
	getcwd(strbuf, STRBUFSIZE);
	fileName = fl_show_fselector("Choose a new file:", strbuf, \
		MPSFILEPAT, NULL);
		
	if (fileName == NULL || SAMESTRING(fileName, EMPTYSTR))
		// do nothing
		;
	else
		if (index == STABINDEX)
		{
			// an ASID is needed to decode accesses correctly
			sprintf(strbuf,"0x%lX", setup->getSymbolTableASID());
			while ((inputp = fl_show_input("You must assign an ASID to symbol table:", strbuf)) != NULL && \
			 	(!StrToWord(inputp, &asid) || asid > MAXASID))
			 	// some error occurred
			 	fl_ringbell(100);
			 
			// this file name can be changed without resetting the system
			// but it modifies virtual symbol table list, so it gives a warning
			if (inputp != NULL && setup->setROMFileName(index, fileName, FALSE))
			{
				setup->setSymbolTableASID(asid);	
				watch->LoadSymbolTable();	
				xIntf->RefreshAllVForms(TRUE);
				if (watch->getRangeNum(BRKPT) || watch->getRangeNum(SUSP) || watch->getRangeNum(TRACE))
				{
					fl_ringbell(100);
					fl_show_alert("Symbol table file has been reloaded:", \
						"some memory browser entries could now point to unwanted or", "invalid areas; you have to delete them by hand", TRUE);	
				}
			}
			// else Cancel button was selected or bad file format: in the
			// latter case, setup printed a warning message, and watch did
			// not load a new symbol table nor change current symbol table
			// ASID
		}
		else 
			if (index == COREINDEX)
			{
				// this file name can be changed without resetting the system
				// but kernel will be loaded only at reset
				if (setup->setROMFileName(index, fileName, FALSE))
				{
					// file was valid
					fl_ringbell(100);
					fl_show_alert("Kernel image file for ROM boot has been changed:", \
						"it will be loaded only at system reset",  "(with ROM boot activated)", TRUE);
					xIntf->RefreshAllVForms(TRUE);				
				}
				// else setup will emit a warning message
			}
			else
			{
				fl_ringbell(100);
				// other file names change forces system reset
				if (setup->getExpertMode() || fl_show_question("Can't change system ROM without resetting to Setup...\n\
Do you want to reset?", FALSE))
					if (setup->setROMFileName(index, fileName, FALSE))
						xIntf->Reset(FALSE);
					// else setup will emit a warning message
			}
}


HIDDEN void bootChoiceCB(FL_OBJECT * ob, SWord dummy)
{
	unsigned int bType = fl_get_choice(ob);

	if (bType > 0 && bType != (setup->getBootType() + 1))
	{
		// valid choice (bType is one more than real one)
		bType--;
			// to switch to ROM boot, a core file must be selected
		if (bType == COREBOOT && SAMESTRING(setup->getROMFileName(COREINDEX), EMPTYSTR))
		{
			fl_ringbell(100);	
			fl_show_alert("CORE Boot may not be activated:", "select a kernel file first", EMPTYSTR, TRUE);
			fl_set_choice(ob, setup->getBootType() + 1);
		}		
		else	
		{
			fl_ringbell(100);
			if (setup->getExpertMode() || fl_show_question("Can't change boot method without resetting\n\
the system to Setup...\nDo you want to reset?", FALSE))
			{
				setup->setBootType(bType);
				xIntf->Reset(FALSE);
			}		
			else
				// resets the choice to current value
				fl_set_choice(ob, setup->getBootType() + 1);
		}
	}
	// else do nothing
}


/****************************************************************************/

//
// MemBrowser form callbacks
//

HIDDEN void rangeAddCB(FL_OBJECT *dummyob, SWord which)
{
	unsigned int choice;
	FL_OBJECT * rBrowp;
	
	switch (which)
	{
		case BRKPT:
			rBrowp = mBp->rBrowser[BRKPT];
			break;
			
		case SUSP:
			rBrowp = mBp->rBrowser[SUSP];
			break;
				
		case TRACE:
			rBrowp = mBp->rBrowser[TRACE];
			break;
			
		case MEMMOD:
		default:
			rBrowp = NULL; 
			break;
			
	} 
	if (which == TRACE || which == MEMMOD || (choice = fl_show_choice("Which kind of range do you want to add?", EMPTYSTR, EMPTYSTR,\
		3, "Symbolic/Virtual", "Physical", "Cancel", 3)) == 2)	
		xIntf->ShowRangeForm(FALSE, (unsigned int) which, rBrowp);
	else
		if (choice == 1)
			xIntf->ShowRangeForm(TRUE, (unsigned int) which, rBrowp); 
		// else cancel was pressed
}		


HIDDEN void rangeDelCB(FL_OBJECT *dummyob, SWord which)
{
	unsigned int i = 0;
	FL_OBJECT * brwsr = NULL;

	if (which >= BRKPT && which < RTABNUM)
		brwsr = mBp->rBrowser[which];
	else
		Panic("Unknown range type in appforms.cc module rangeDelCB() function");

	if ((i = fl_get_browser(brwsr)) == 0)
	{
		fl_ringbell(100);
		fl_show_alert("No range selected", "Click with mouse over one to select it", EMPTYSTR, TRUE);
	}
	else
		if (setup->getExpertMode() || fl_show_question("Are you sure you want to delete this range?", TRUE))
		{
			watch->RangeDelete(which, i);
			fl_delete_browser_line(brwsr, i);

			// refresh memory display area if needed
			if (which == TRACE)
				xIntf->RefreshVForm(mBp->formNum, TRUE);

		}
}


HIDDEN void traceToCB(FL_OBJECT *dummyob, SWord dummy)
{
	unsigned int i = fl_get_browser(mBp->rBrowser[SUSP]);
	
	Word asid, start, end, access;
	
  	if (i == 0)
  	{
  		fl_ringbell(100);
  		fl_show_alert("No range selected", "Click with mouse over one to select it", EMPTYSTR, TRUE);
  	}
	else
	{
		watch->getRange(SUSP, i, &asid, &start, &end, &access);
		if (asid != MAXASID)
		{	
			// only physical suspect ranges are traceable
			fl_ringbell(100);	
			fl_show_alert("Virtual range selected", "Virtual ranges may not be traced", EMPTYSTR, TRUE);
		}
		else	
			if ((setup->getExpertMode() || fl_show_question("Are you sure you want to trace this suspect range?", TRUE)) && \
				!watch->RangeInsert(TRACE, asid, start, end, WRITE))
			{	
				fl_ringbell(100);
				fl_show_alert("Trace range has not been inserted:", "it (partially) overlaps with another", EMPTYSTR, TRUE);				
			}
	// in any case
	xIntf->RefreshVForm(mBp->formNum, TRUE);
	}
}


HIDDEN int memBFormClose(FL_FORM * dummyform, void * dummy)
{
	watch->setDirtyMemReq(FALSE);
	return(FL_OK);
}


/****************************************************************************/

//
// callback for DevStatus forms 
//
  

/****************************************************************************/

//
// callback for TLBDisplay form
//

HIDDEN void tlbModCB(FL_OBJECT *ob, SWord dummy)
{
	unsigned int i = fl_get_browser(ob);
	const char * istrp;
	Word tlbHI, tlbLO;
	
	sprintf(strbuf,"0x%.8lX", watch->getTLBHI(i - 1));
	while ((istrp =	fl_show_input("Enter TLB EntryHi value:", strbuf)) != NULL && !StrToWord(istrp, &tlbHI))
		fl_ringbell(100);
	if (istrp != NULL)	
	{
		sprintf(strbuf, "0x%.8lX", watch->getTLBLO(i - 1));
		while ((istrp =	fl_show_input("Enter TLB EntryLo value:", strbuf)) != NULL && !StrToWord(istrp, &tlbLO))
			fl_ringbell(100);
		if (istrp != NULL)
		{
			watch->setTLB(i - 1, tlbHI, tlbLO);
			xIntf->RefreshVForm(tlbDp->formNum, TRUE);
		}
	}
}

/****************************************************************************/


/****************************************************************************/

//
// callbacks for SymbForm
//

HIDDEN void symbolSelectCB(FL_OBJECT *ob, SWord dummy)
{
	
	unsigned int i = fl_get_browser(ob);
	Boolean isFun;
	Word startaddr, endaddr, asid, access;
	unsigned int trType;
	
	if (i > 0)
	{
		xIntf->FreezeAllVForms();
		watch->getSymbolData(i, &startaddr, &endaddr, &isFun);
		asid = setup->getSymbolTableASID();	
		if 	((asid > MAXASID) || BADADDR(startaddr) || BADADDR(endaddr) || (startaddr > endaddr))
  		{
  			// wrong data input
  			fl_ringbell(100);
  			fl_show_alert("Invalid field format or alignment", EMPTYSTR, EMPTYSTR, TRUE);			
  		}
  		else
  		{
		 	if (isFun)
		 	{
		 		// insert BRKPT
				if (setup->getExpertMode() || fl_show_question("Are you sure you want to add this breakpoint?", FALSE))
				{
					if (!watch->RangeInsert(BRKPT, asid, startaddr, startaddr, EXEC))
					{
						fl_ringbell(100);
						fl_show_alert("Breakpoint has not been inserted:", "it (partially) overlaps with another", EMPTYSTR, TRUE);
					}
					else
					{
						xIntf->RefreshAllVForms(TRUE);
					}
				}
			}
			else	  
			{	
				if (setup->getExpertMode() || fl_show_question("Are you sure you want to add this suspect?", FALSE))
				{	
	 	 			// insert SUSP
					trType = fl_show_choice("Which kind of suspect do you want to add?", EMPTYSTR, EMPTYSTR,\
				 			3, "Write", "Read", "Read/Write", 2);
					access = trType << 1;
					if (!watch->RangeInsert(SUSP, asid, startaddr, endaddr, access))
					{
						fl_ringbell(100);
						fl_show_alert("Suspect has not been inserted:", "it (partially) overlaps with another", EMPTYSTR, TRUE);
					}	
					else
					{ 
						if (asid == MAXASID)
						{
							// insert trace range if PHYS range
							if (!watch->RangeInsert(TRACE, asid, startaddr, endaddr, WRITE))
							{
								fl_ringbell(100);
								fl_show_alert("Trace range has not been inserted:", "it (partially) overlaps with another", EMPTYSTR, TRUE);
							}
						}
						// trace range or not, the suspect has been inserted
						xIntf->RefreshAllVForms(TRUE);
					}
				}
			}
		}
		xIntf->UnFreezeAllVForms();  
	}
	// else no line selected: do nothing
}


/****************************************************************************/

//
// callbacks for VRangeForm
//

HIDDEN void symSelCB(FL_OBJECT *ob, SWord dummy)
{
	
	unsigned int i = fl_get_browser(ob);
	Boolean dummyisFun;
		
	if (i > 0)
	{
		watch->getSymbolData(i, &(vRFp->startAddr), &(vRFp->endAddr), &dummyisFun);
		fl_ringbell(50);
		vRefresh();
	}
	// else no line selected: do nothing
}


HIDDEN void vAsidCB(FL_OBJECT *ob, SWord dummy)
{
	const char * inputstr = fl_get_input(ob);
	Word temp;
	
	if (!StrToWord(inputstr, &temp) || temp > MAXASID)
	{
		fl_ringbell(100);
		fl_show_alert("Invalid ASID field format or value out of range:", "re-type it", EMPTYSTR, TRUE);
	}
	else
		vRFp->asid = temp;
		
	vRefresh();  
}


HIDDEN void vStartCB(FL_OBJECT *ob, SWord dummy)
{
	const char * inputstr = fl_get_input(ob);
	Word temp;
	
	if (!StrToWord(inputstr, &temp) || BADADDR(temp))
	{
		fl_ringbell(100);
		fl_show_alert("Invalid field format or alignment:", "re-type it", EMPTYSTR, TRUE);			
	}
	else
	{
		vRFp->startAddr = temp;
		// to help range insertion at beginning of input
		if (vRFp->endAddr == 0)
			vRFp->endAddr = temp;
	}
	vRefresh();  
}


HIDDEN void vEndCB(FL_OBJECT *ob, SWord dummy)
{
	const char * inputstr = fl_get_input(ob);
	Word temp;
	
	if (!StrToWord(inputstr, &temp) || BADADDR(temp))
	{
		fl_ringbell(100);
		fl_show_alert("Invalid field format or alignment:", "re-type it", EMPTYSTR, TRUE);
	}
	else
		vRFp->endAddr = temp;
		
	vRefresh();  
}


HIDDEN void vCancCB(FL_OBJECT *dummyob, SWord dummy)
{
	fl_hide_form(vRFp->vRangeForm);
	xIntf->UnFreezeAllVForms();  
}


HIDDEN void vOkCB(FL_OBJECT *dummyob, SWord dummy)
{
	Word access;
 	unsigned int trType;
  	const char * startp, * endp, * asidp;
  	Word startt, endt, asidt;
  	
  	// re-conversion is needed again to assure interface vs. internal values
	// coherency (since input objects invoke callbacks only when TAB or ENTER is
	// pressed) 
  	startp = fl_get_input(vRFp->startInput);
  	endp = fl_get_input(vRFp->endInput);
  	asidp = fl_get_input(vRFp->asidInput);
  	
  	if (!StrToWord(asidp, &asidt) || !StrToWord(startp, &startt) || !StrToWord(endp, &endt) || \
 		(asidt > MAXASID) || BADADDR(startt) || BADADDR(endt))
  	{
  		// wrong data input
  		fl_ringbell(100);
  		fl_show_alert("Invalid field format or alignment:", "re-type it", EMPTYSTR, TRUE);			
  		vRefresh();
  	}
  	else
  	{
  		vRFp->startAddr = startt;
  		vRFp->endAddr = endt;
  		vRFp->asid = asidt;
	
	  	if (vRFp->startAddr > vRFp->endAddr)
	  	{
	  		// wrong range
	  		fl_ringbell(100);
  			fl_show_alert("Wrong range:", "end address must be higher than start address", EMPTYSTR, TRUE);
  			vRefresh();
	  	}
	  	else
		 	switch (vRFp->rType)
			{
				case BRKPT:
					if (!watch->RangeInsert(vRFp->rType, vRFp->asid, vRFp->startAddr, vRFp->endAddr, EXEC))
					{
						fl_ringbell(100);
						fl_show_alert("Breakpoint has not been inserted:", "it (partially) overlaps with another", EMPTYSTR, TRUE);
						vRefresh();
					}
					else
					{
						// now it may close request window
						fl_hide_form(vRFp->vRangeForm);
						xIntf->RefreshAllVForms(TRUE);
						xIntf->UnFreezeAllVForms();  
					}
	 				break;
	  		
	 	 		case SUSP:
					trType = fl_show_choice("Which kind of suspect do you want to add?", EMPTYSTR, EMPTYSTR,\
				 			3, "Write", "Read", "Read/Write", 2);
					access = trType << 1;
					if (!watch->RangeInsert(vRFp->rType, vRFp->asid, vRFp->startAddr, vRFp->endAddr, access))
					{
						fl_ringbell(100);
						fl_show_alert("Suspect has not been inserted:", "it (partially) overlaps with another", EMPTYSTR, TRUE);
						vRefresh();
					}	
					else
					{
						// now it may close request window
						fl_hide_form(vRFp->vRangeForm);
						xIntf->RefreshAllVForms(TRUE);
						xIntf->UnFreezeAllVForms();  
					}
  					break;
  					
  				default:
  					Panic("Invalid range type in VRangeForm::vOkCB() callback function");
  					break;
 		 	}	
	}
}


HIDDEN void vClearCB(FL_OBJECT *dummyob, SWord dummy)
{
 	vRFp->startAddr = 0UL;
 	vRFp->endAddr = 0UL;
 	vRFp->asid = setup->getSymbolTableASID();
 	vRefresh();   
}


HIDDEN void vRefresh(void)
{
	sprintf(strbuf, "0x%lX", vRFp->asid);
	fl_set_input(vRFp->asidInput, strbuf);
	sprintf(strbuf, "0x%.8lX", vRFp->startAddr);
	fl_set_input(vRFp->startInput, strbuf);
	sprintf(strbuf, "0x%.8lX", vRFp->endAddr);
	fl_set_input(vRFp->endInput, strbuf);
}
  
/****************************************************************************/

//
// callbacks  and utility functions for PRangeForm
//

HIDDEN void pOkCB(FL_OBJECT *dummyob, SWord dummy)
{
	Word access;
 	unsigned int trType;
	const char * startp, * endp;
	Word startt, endt;
	// re-conversion is needed again to assure interface vs. internal values
	// coherency (since input objects invoke callbacks only when TAB or ENTER is
	// pressed) 
  	startp = fl_get_input(pRFp->startInput);
  	endp = fl_get_input(pRFp->endInput);
 	if (!StrToWord(startp, &startt) || !StrToWord(endp, &endt) || \
  		BADADDR(startt) || (pRFp->rType != MEMMOD && BADADDR(endt)))
  	{
  		// wrong data input
  		fl_ringbell(100);
  		fl_show_alert("Invalid field format or alignment:", "re-type it", EMPTYSTR, TRUE);			
  		pRefresh();
  	}
  	else
  	{
  		pRFp->startAddr = startt;
  		pRFp->endAddr = endt;
	  	if (pRFp->rType != MEMMOD && pRFp->startAddr > pRFp->endAddr)
  		{
  			// wrong range
  			fl_ringbell(100);
  			fl_show_alert("Wrong range:", "end address must be higher than start address", EMPTYSTR, TRUE);
  			pRefresh();
  		}
  		else
  			if (pRFp->rType == TRACE && pRFp->endAddr >= (pRFp->startAddr + (MAXTRACESIZE * WORDLEN)))
  			{
  				// trace range too large for insertion
		  		sprintf(strbuf, "maximum is %d words", MAXTRACESIZE);
  				fl_ringbell(100);
				fl_show_alert("Trace range too large for insertion:", strbuf, EMPTYSTR, TRUE);
				pRefresh();
  			}
 	 		else
			  	switch (pRFp->rType)
  				{
  					case MEMMOD:
  						if (watch->MemWrite(pRFp->startAddr, pRFp->endAddr))
  						{
  							fl_ringbell(100);
							fl_show_alert("Read-only location", "No change", EMPTYSTR, TRUE);		
						}
						// else
						// if memory location is traced it surely will be refreshed
						// by RefreshAllVForms in Memory Browser and 
						// Device Status windows 
						// now it may close request window
						fl_hide_form(pRFp->pRangeForm);
						xIntf->RefreshAllVForms(TRUE);
						xIntf->UnFreezeAllVForms();
  						break;
  					
  					case BRKPT:
  						if (!watch->RangeInsert(pRFp->rType, MAXASID, pRFp->startAddr, pRFp->endAddr, EXEC))
  						{
  							fl_ringbell(100);
							fl_show_alert("Breakpoint range has not been inserted:", "it (partially) overlaps with another", EMPTYSTR, TRUE);
							pRefresh();
						}
						else
						{
							// now it may close request window
							fl_hide_form(pRFp->pRangeForm);
							xIntf->RefreshAllVForms(TRUE);
							xIntf->UnFreezeAllVForms();
						}
  						break;
  					
  					case SUSP:
  						trType = fl_show_choice("Which kind of suspect do you want to add?", EMPTYSTR, EMPTYSTR,\
						 			3, "Write", "Read", "Read/Write", 2);
						access = trType << 1;
						if (!watch->RangeInsert(pRFp->rType, MAXASID, pRFp->startAddr, pRFp->endAddr, access))
						{	
							fl_ringbell(100);
							fl_show_alert("Suspect range has not been inserted:", "it (partially) overlaps with another", EMPTYSTR, TRUE);
							pRefresh();
						}
						else
						{	
							// now it may close request window
							fl_hide_form(pRFp->pRangeForm);
							xIntf->RefreshAllVForms(TRUE);
							xIntf->UnFreezeAllVForms();
						}
  						break;
  					
  					case TRACE:
  						if (!watch->RangeInsert(pRFp->rType, MAXASID, pRFp->startAddr, pRFp->endAddr, WRITE))
						{	
							fl_ringbell(100);
							fl_show_alert("Trace range has not been inserted:", "it (partially) overlaps with another", EMPTYSTR, TRUE);
							pRefresh();
						}
						else
						{	
							// now it may close request window
							fl_hide_form(pRFp->pRangeForm);
							xIntf->RefreshAllVForms(TRUE);
							xIntf->UnFreezeAllVForms();
						}
  						break;
  		
  					default:
  						Panic("Unknown range type in PRangeForm::pOkCB() callback function");
  						break;
  				}
	}
}


HIDDEN void pStartCB(FL_OBJECT *ob, SWord dummy)
{
	const char * inputstr = fl_get_input(ob);
	Word temp;
	
	if (!StrToWord(inputstr, &temp) || BADADDR(temp))
	{
		fl_ringbell(100);
		fl_show_alert("Invalid field format or alignment:", "re-type it", EMPTYSTR, TRUE);			
	}
	else
	{
		pRFp->startAddr = temp;
		// to help range insertion at beginning of input
		if (pRFp->rType != MEMMOD && pRFp->endAddr == 0)
			pRFp->endAddr = temp;
	}		
	pRefresh();
}


HIDDEN void pEndCB(FL_OBJECT *ob, SWord dummy)
{
	const char * inputstr = fl_get_input(ob);
	Word temp;
	
	if (!StrToWord(inputstr, &temp) || (pRFp->rType != MEMMOD && BADADDR(temp)))
	{
		fl_ringbell(100);
		fl_show_alert("Invalid field format or alignment:", "re-type it", EMPTYSTR, TRUE);
	}
	else
		pRFp->endAddr = temp;
		
	pRefresh();  
}


HIDDEN void pCancCB(FL_OBJECT *dummyob, SWord dummy)
{
	fl_hide_form(pRFp->pRangeForm);
	xIntf->UnFreezeAllVForms();
}


HIDDEN void pClearCB(FL_OBJECT *dummyob, SWord dummy)
{
 	pRFp->startAddr = 0UL;
 	pRFp->endAddr = 0UL;
 	pRefresh(); 
}


HIDDEN void pRefresh(void)
{
	if (pRFp->rType == MEMMOD)
		sprintf(strbuf, "0x%lX", pRFp->endAddr);
	else
		sprintf(strbuf, "0x%.8lX", pRFp->endAddr);

	fl_set_input(pRFp->endInput, strbuf);

	sprintf(strbuf, "0x%.8lX", pRFp->startAddr);
	fl_set_input(pRFp->startInput, strbuf);
}


/****************************************************************************/

//
// utility callbacks and functions
//

// This function closes any open window which calls it
HIDDEN int formClose(FL_FORM * dummyform, void * dummy)
{
	return(FL_OK);
}


// This function closes a range form window
HIDDEN int rFormClose(FL_FORM * rform, void * dummy)
{
	// like pressing the cancel button
	fl_hide_form(rform);
	xIntf->UnFreezeAllVForms();
	return(FL_IGNORE);
}


// This function is a callback for window gadgets which should do nothing
HIDDEN void dummyCB(FL_OBJECT *dummyob, SWord dummy)
{
	// no operations
}

// This function computes the top line for a browser given a line which must
// be absolutely shown; it computes a line number (min = 1) which allow the
// display of a selected line near the middle of a screenful of text
HIDDEN unsigned int computeTopLine(unsigned int lsel, unsigned int maxl, unsigned int screenl)
{
	unsigned int topl = lsel - (screenl / 2);
	
	if ((topl + screenl) > maxl)
		topl = (maxl - screenl) + 1;
		
	if (topl < 1)
		topl = 1;
	
	return(topl);
}	

// This function returns a string describing memory contents, given a
// starting physical address and a number of words to be read and displayed
HIDDEN const char * memDumpStr(Word paStart, unsigned int wNum)
{

	Word memval, byteval, pAddr;
	Word paEnd = paStart + (wNum * WORDLEN);
	char * strp, * chp;
	char chbuf[SMALLSTRBUF];
	unsigned int i;
	
	sprintf(strbuf, "0x%.8lX  ", paStart);
	strp = strbuf + strlen(strbuf);
	chp = chbuf;
	sprintf(chp," |  ");
	chp += 4;
	
	for (pAddr = paStart; pAddr < paEnd; pAddr += WORDLEN)
	{	
		if (watch->MemRead(pAddr, &memval))
		{
			// non-existent memory location
			sprintf(strp, "NN NN NN NN ");
			sprintf(chp, "NNNN");
			strp += 12;
			chp += 4;
		}
		else
			for (i = 0; i < WORDLEN; i++)
			{
				// byte ordering counts
				if (BIGENDIANCPU)
					byteval = (memval >> (i * BYTELEN)) & BYTEMASK;
				else
					byteval = (memval >> (((WORDLEN - 1) * BYTELEN) - (i * BYTELEN))) & BYTEMASK;	
					
				sprintf(strp, "%.2lX ", byteval);
				
				// ASCII decoding goes the other way
				if (BIGENDIANCPU)
					byteval = (memval >> (((WORDLEN - 1) * BYTELEN) - (i * BYTELEN))) & BYTEMASK;	
				else
					byteval = (memval >> (i * BYTELEN)) & BYTEMASK;				
				
				if (isprint((unsigned char) byteval))
					*chp = (unsigned char) byteval;
				else
					*chp = DOT;
				strp += 3;
				chp++;
			}
		
	}
	// adding the address to ASCII part
	sprintf (chp, "  0x%.8lX", paStart);
	chp += 12;
	// marking end of chbuf
	*chp = EOS;
	// and copying it at strbuf end
	strcpy(strp, chbuf);

	return(strbuf);
}

// This function sets the title of a range form given the range type which
// should be inserted
HIDDEN void rangeTitle(unsigned int rType, FL_OBJECT * title)
{
	switch (rType)
	{
		case BRKPT:		
			fl_set_object_label(title, "Insert Breakpoint range:");				
			break;
	
		case SUSP:
			fl_set_object_label(title, "Insert Suspect range:");
			break;
			
		case TRACE:
			fl_set_object_label(title, "Insert Trace range:");
			break;

		case MEMMOD:
			fl_set_object_label(title, "");		
			break;
			
		default:
			Panic("Unknown range type in appform.cc module rangeTitle() function");			
			break;
	}
}	

