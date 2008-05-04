/* File: $Id: formdefs.h,v 1.2 2008-05-04 15:46:59 mbbh Exp $ */

/**************************************************************************** 
 *
 * This file contains a C++ converted version of type definitions built by
 * "fdesign" utility from XFORMS toolkit.
 * Each type represents a FORM object, containing other objects (buttons,
 * sliders, text fields, etc.); they point to the real objects
 * implemented by XFORMS library.  
 * This file is needed since fdesign automated C procedure building for 
 * interfaces relies on its contents. 
 * 
 ****************************************************************************/

// Main form window
class FD_MainForm
{
	public:

		// pointer to form itself	
		FL_FORM *mainForm;

		// pointers to objects belonging to form 
		FL_OBJECT *setupMenu;
		FL_OBJECT *winMenu;
		FL_OBJECT *miscRegBrowser;
		FL_OBJECT *prevSLine;
		FL_OBJECT *currStatus;
		FL_OBJECT *currSLine;
		FL_OBJECT *buttBD;
		FL_OBJECT *buttLD;
 		FL_OBJECT *buttVM;
		FL_OBJECT *sStepButt;
		FL_OBJECT *stopButt;
		FL_OBJECT *runButt;
		FL_OBJECT *stepButt;
		FL_OBJECT *stepSlide;
		FL_OBJECT *excButt;
		FL_OBJECT *suspButt;
		FL_OBJECT *utlbk0Butt;
		FL_OBJECT *utlbk1Butt;
		FL_OBJECT *brkptButt;
		FL_OBJECT *updSlider;
		FL_OBJECT *cpuRegBrowser;
		FL_OBJECT *exitButt;
		FL_OBJECT *resetButt;
};


// Setup form window
class FD_SetupForm
{
	public:
	
		// pointer to form itself
		FL_FORM *setupForm;

		// pointers to objects belonging to form		
		FL_OBJECT *devBrowser;
		FL_OBJECT *memSlide;
		FL_OBJECT *bootChoice;
		FL_OBJECT *speedSlide;
		FL_OBJECT *kernFileName;
		FL_OBJECT *kernButt;
		FL_OBJECT *bootFileName;
		FL_OBJECT *bootButt;
		FL_OBJECT *biosFileName;
		FL_OBJECT *biosButt;
		FL_OBJECT *stabFileName;
		FL_OBJECT *stabButt;
};


// Memory browser form window
class FD_MemBrowser
{
	public:
		// pointer to form itself
		FL_FORM *memBrowser;
		
		// XInterface tag for form activation/manipulation		
		unsigned int formNum;
		
		// pointers to objects belonging to form
		FL_OBJECT *brkptAddButt;
		FL_OBJECT *brkptDelButt;
		FL_OBJECT *suspAddButt;
		FL_OBJECT *suspDelButt;
		FL_OBJECT *traceAddButt;
		FL_OBJECT *traceDelButt;
		FL_OBJECT * rBrowser[RTABNUM];
		FL_OBJECT *memDumpBrowser;
		FL_OBJECT *traceToButt;
		FL_OBJECT *memModButt;
};

// Device status form windows (one for interrupt line)
class FD_DevStatus
{
	public:

		// pointer to form itself
		FL_FORM *devStatus;
		
		// XInterface tag for form activation/manipulation	
		unsigned int intLine;
		
		// pointers to objects belonging to form
		FL_OBJECT *devName[DEVPERINT];	
		FL_OBJECT *devStatLine[DEVPERINT];
		FL_OBJECT *failButt[DEVPERINT];
		FL_OBJECT *devCompLine[DEVPERINT];
};

// TLB display form window
class FD_TLBDisplay 
{
	public:
		
		// pointer to form itself
		FL_FORM *tlbDisplay;
		
		// XInterface tag for form activation/manipulation
		unsigned int formNum;
		
		// pointer to objects belonging to form
		FL_OBJECT *tlbBrowser;
};

// Symbol table form window
class FD_SymbForm 
{
	public:
			
		// pointer to form itself
		FL_FORM *symbForm;

		// XInterface tag for form activation/manipulation
		unsigned int formNum;

		// pointer to objects belonging to form
		FL_OBJECT *symbBrowser;
};

// Terminal form windows (one for terminal)
class FD_Terminal
{
	public:
		
		// pointer to form itself
		FL_FORM *terminal;
		
		// pointer to objects belonging to form
		FL_OBJECT *termName;
		FL_OBJECT *termBrowser;
		FL_OBJECT *inputButt;
		FL_OBJECT *termRecvLine;
		FL_OBJECT *termRecvCompLine;
		FL_OBJECT *termTranLine;
		FL_OBJECT *termTranCompLine;
		FL_OBJECT *failButt;
};

// Virtual range insertion form window
class FD_VRangeForm
{
	public:
		
		// pointer to form itself
		FL_FORM *vRangeForm;
		
		// latest input data (ASID, start/end addresses, insertion type=brpkt/susp)
		Word asid;
		Word startAddr;
		Word endAddr;
		unsigned int rType;

		// MemBrowser form window object where range inserted should be put
		FL_OBJECT * rBrowser;
		
		// pointer to objects belonging to form
		FL_OBJECT *title;
		FL_OBJECT *symBrowser;
		FL_OBJECT *asidInput;
		FL_OBJECT *startInput;
		FL_OBJECT *endInput;
		FL_OBJECT *cancelButt;
		FL_OBJECT *okButt;
		FL_OBJECT *clearButt;
};

// Physical range insertion form window
class FD_PRangeForm
{
	public:

		// pointer to form itself
		FL_FORM *pRangeForm;
		
		// latest input data (start/end addresses, insertion type=brkpt/susp/trace)
		Word startAddr;
		Word endAddr;
		unsigned int rType;

		// MemBrowser form window object where range inserted should be put
		FL_OBJECT * rBrowser;

		// pointer to objects belonging to form
		FL_OBJECT *title;
		FL_OBJECT *startInput;
		FL_OBJECT *endInput;
		FL_OBJECT *cancelButt;
		FL_OBJECT *okButt;
		FL_OBJECT *clearButt;
};
