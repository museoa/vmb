/* File: $Id: processor.e,v 1.1 2007-08-29 09:19:37 ruckert Exp $ */

/****************************************************************************
 *
 * External declarations from the corresponding module .cc
 *
 ****************************************************************************/

class Watch;
class SystemBus;
class TLBEntry;


// A Processor object emulates MIPS R2/3000 processor features almost
// perfectly. It is linked to a SystemBus object for physical memory
// accesses and is controlled by a Watch object for simulation control. 
// MIPS processor features are too complex to be fully descripted here:
// refer to external documentation.

class Processor
{
	public:
	
		// This method creates a new Processor object, links to other
		// components of the system, and initializes it
		Processor(Watch * wch, SystemBus * bs);
		
		// This method deletes a Processor object and all its internal
		// structures
		~Processor(void);

		// This method makes Processor execute a single instruction.  For
		// simulation purposes, it differs from traditional processor cycle:
		// the first instruction after a reset is pre-loaded, and cycle is
		// execute - fetch instead of fetch - execute.  This way, it is
		// possible to know what instruction will be executed before its
		// execution happens
		void Cycle(void);
		
		// This method allows SystemBus and Processor itself to signal
		// Processor when an exception happens. SystemBus signal IBE/DBE
		// exceptions; Processor itself signal all other kinds of exception.
		void SignalExc(unsigned int exc, Word cpuNum = 0UL);
		
		// the following methods allow inspection of Processor internal
		// status (typically invoked by Watch object). Name & parameters are
		// self-explanatory: remember that all addresses are _virtual_ when
		// not marked Phys/P/phys (for physical ones), and all index
		// checking must be performed from caller

		void getCurrStatus(Word * asid, Word * pc, Word * instr, Boolean * inLD, Boolean * inBD, Boolean * inVM);
		void getPrevStatus(Word * pc, Word * instr);
		const char * getExcCauseStr(void);
		Word getNextPC(void); 
		Word getSuccPC(void);
		Word getPrevPPC(void);
		Word getCurrPPC(void);
		SWord getGPR(unsigned int num);
		Word getCP0Reg(unsigned int num);
		void getTLB(unsigned int index, Word * hi, Word * lo);


		// the following methods allow to change Processor internal status
		// (typically invoked by Watch object). Name & parameters are almost
		// self-explanatory: remember that all addresses are _virtual_ when
		// not marked Phys/P/phys (for physical ones), and all index
		// checking must be performed from caller.  Processor status
		// modification is allowed during debugging inside the simulation

		void setGPR(unsigned int num, SWord val);
		void setCP0Reg(unsigned int num, Word val);
		void setNextPC(Word npc);
		void setSuccPC(Word spc);
		void setTLB(unsigned int index, Word hi, Word lo);

	private:
	
		// object references for memory access (bus) and for virtual address
		// accessing (watch)
		Watch * watch;
		SystemBus * bus;
		
		// last exception cause
		unsigned int excCause;
	
		// for CPUEXCEPTION, coprocessor unusable number 
		Word copENum;

		// tracks branch delay slots
		Boolean isBranchD;

		// delayed load handling variables:
		// indicates if a delayed load is pending
		unsigned int loadPending;
		// register target
		unsigned int loadReg;
		// value to be loaded into register
		SWord loadVal;
		
		// general purpose registers, together with HI and LO registers
		SWord gpr[CPUREGNUM];
		
		// instruction to be executed
		Word currInstr;

		// previous virtual and physical addresses for PC, and previous
		// instruction executed; for book-keeping purposes and for handling
		// exceptions in BD slot
		Word prevPC;
		Word prevPhysPC;
		Word prevInstr;

		// current virtual and physical addresses for PC
		Word currPC;
		Word currPhysPC;
		
		// virtual values for PC after current one: they are needed to
		// emulate branch delay slot; no physical values are available since
		// conversion is needed (and sometimes possible) only for PC current
		// value
		Word nextPC;
		Word succPC;

		// CP0 components: special registers and TLB
		Word cpreg[CP0REGNUM];
		TLBEntry ** TLB;	
		Word TLBSize;
			
		// private methods
		
		void reset(void);
		void handleExc(void);
		void zapTLB(void);
		Boolean execInstr(Word instr);
		Boolean execRegInstr(Word * res, Word instr, Boolean * isBD);
		Boolean execImmInstr(Word * res, Word instr);
		Boolean execBranchInstr(Word instr, Boolean * isBD);
		Boolean execLoadInstr(Word instr);
		Boolean execStoreInstr(Word instr);
		Boolean userMode(void);
		Boolean kernelMode(void);
		Boolean mapVirtual(Word vaddr, Word * paddr, Word accType);		
		Boolean probeTLB(unsigned int * index, Word asid, Word vpn);
		void completeLoad(void);
		void randomRegTick(void);
		void pushKUIEVMStack(void);
		void popKUIEVMStack(void);
		void setTLBRegs(Word vaddr);
		Boolean checkForInt(void);
		Boolean cp0Usable(void);
		void setLoad(unsigned int loadCode, unsigned int regNum, SWord regVal);
		SWord signExtByte(Word val, unsigned int bytep);
		Word zExtByte(Word val, unsigned int bytep);
		SWord signExtHWord(Word val, unsigned int hwp);
		Word zExtHWord(Word val, unsigned int hwp);
		Word mergeByte(Word dest, Word src, unsigned int bytep);
		Word mergeHWord(Word dest, Word src, unsigned int hwp);
		Word merge(Word dest, Word src, unsigned int bytep, Boolean loadBig, Boolean startLeft);
};
